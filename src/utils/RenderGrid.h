#include "RenderBaseObject.h"

class cRenderGrid : public cRenderBaseObject
{
public:
    cRenderGrid(bool dimX = true, bool dimY = true, bool dimZ = false);
    void GenGrid(_FLOAT gap0 = 0.01, _FLOAT gap1 = 0.01,
                          _FLOAT range0 = 1, _FLOAT range1 = 1);
    virtual std::vector<cRenderResourcePtr>
    GetRenderingResource() const override;

protected:
    int dim0, dim1; // valid dim 0 and valid dim 1
    int dim2;       // invalid dim 2
    tVectorXf mBuffer;
};
