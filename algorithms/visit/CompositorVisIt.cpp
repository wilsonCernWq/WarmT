#include "CompositorVisIt.h"

#include "avtSLIVRImgMetaData.h"
#include "avtSLIVRImgCommunicator.h"

#include <vector>

static avtSLIVRImgCommunicator* imgComm;
static std::vector<float*> tileBufferList;

namespace QCT {
namespace algorithms {
namespace visit {            
    
    Compositor_VisIt::Compositor_VisIt(const Mode& m,
                                       const uint32_t& width,
                                       const uint32_t& height)    
        : mode(m), W(width), H(height) {
        if (mpiRank == 0) { rgba = new float[4 * W * H](); }
        imgComm = new avtSLIVRImgCommunicator();
    };

    Compositor_VisIt::~Compositor_VisIt() {
        if (mpiRank == 0) { if (rgba) delete[] rgba; }
        for (auto b : tileBufferList) { if (b) delete[] b; }
        delete imgComm;
    };

    //! function to get final results
    const void *Compositor_VisIt::MapDepthBuffer() { return depth; };
    const void *Compositor_VisIt::MapColorBuffer() { return rgba; };
    void Compositor_VisIt::Unmap(const void *mappedMem) {};

    //! clear (the specified channels of) this frame buffer
    void Compositor_VisIt::Clear(const uint32_t channelFlags) {};

    //! status
    bool Compositor_VisIt::IsValid() {
        switch (mode) {
        case(ONENODE):
            return imgComm->OneNodeValid();
        case(ICET):
            return imgComm->IceTValid();
        }
    };

    //! begin frame
    void Compositor_VisIt::BeginFrame() {
        switch (mode) {
        case(ONENODE):
            imgComm->OneNodeInit(W, H);
            break;
        case(ICET):
            imgComm->IceTInit(W, H);
            break;
        }
    };
  
    //! end frame
    void Compositor_VisIt::EndFrame() {
        switch (mode) {
        case(ONENODE):
            imgComm->OneNodeComposite(rgba);
            break;
        case(ICET):
            imgComm->IceTComposite(rgba);
            break;
        }
    };

    //! upload tile
    void Compositor_VisIt::SetTile(Tile &tile) {
        // convert to tile data format 
        // (because of the formatting issues)
        float* ptr = new float[4 * tile.tileDim[0] * tile.tileDim[1]];
        for (auto i = 0; i < tile.tileSize; ++i) {
            ptr[4 * i + 0] = tile.r[i];
            ptr[4 * i + 1] = tile.g[i];
            ptr[4 * i + 2] = tile.b[i];
            ptr[4 * i + 3] = tile.a[i];
        }
        tileBufferList.push_back(ptr);
        /* x0 x1 y0 y1 */
        int e[4] = {(int)tile.region[0], (int)tile.region[2],
                    (int)tile.region[1], (int)tile.region[3]}; 
        // set tile
        switch (mode) {
        case(ONENODE):
            imgComm->OneNodeSetTile(ptr, e, *(tile.z));
            break;
        case(ICET):
            imgComm->IceTSetTile(ptr, e, *(tile.z));
            break;
        }
    };

};
};
};
