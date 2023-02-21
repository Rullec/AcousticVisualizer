#include "utils/RenderGrid.h"
#include "utils/ColorUtil.h"
#include <iostream>

cRenderGrid::cRenderGrid(bool dimX, bool dimY, bool dimZ)
{
    if (dimX + dimY + dimZ != 2)
    {
        printf("[error] render grid need two valid dim\n");
        exit(1);
    }

    if (dimX == false)
    {
        dim0 = 1;
        dim1 = 2;
        dim2 = 0;
    }
    else
    {
        dim0 = 0;
        if (dimY == true)
        {
            dim1 = 1;
            dim2 = 2;
        }
        else
        {
            dim1 = 2;
            dim2 = 1;
        }
    }
}
void cRenderGrid::GenGrid(_FLOAT gap0 /*= 0.01*/, _FLOAT gap1 /*= 0.01*/,
                          _FLOAT range0 /*= 10*/, _FLOAT range1 /*= 10*/)
{
    tVectorXf coords0 =
                  tVectorXf::LinSpaced(range0 / gap0 + 1, -range0, range0),
              coords1 =
                  tVectorXf::LinSpaced(range0 / gap0 + 1, -range0, range0);
    mBuffer.resize(2 * (coords0.size() + coords1.size()) *
                   RENDERING_SIZE_PER_VERTICE);
    printf("grid buf size %d\n", mBuffer.size());
    Eigen::Map<tVectorXf> map(mBuffer.data(), mBuffer.size());
    int st_pos = 0;
    // iterate on dim0
    tVector4 st, ed;
    for (int i = 0; i < coords0.size(); i++)
    {
        st.setZero(), ed.setZero();
        st[dim0] = coords0[i];
        st[dim1] = -range1;

        ed[dim0] = coords0[i];
        ed[dim1] = range1;
        cRenderUtil::CalcEdgeDrawBufferSingle(st, ed, tVector4::Zero(), map,
                                              st_pos, ColorBlack);
    }
    for (int i = 0; i < coords1.size(); i++)
    {
        st.setZero(), ed.setZero();
        st[dim1] = coords1[i];
        st[dim0] = -range0;

        ed[dim1] = coords1[i];
        ed[dim0] = range0;

        cRenderUtil::CalcEdgeDrawBufferSingle(st, ed, tVector4::Zero(), map,
                                              st_pos, ColorBlack);
    }
}
std::vector<cRenderResourcePtr> cRenderGrid::GetRenderingResource() const
{
    auto res = std::make_shared<cRenderResource>();
    res->mName = "2Dgrid";
    res->mLineBuffer.mNumOfEle = mBuffer.size();
    res->mLineBuffer.mBuffer = mBuffer.data();
    return {res};
}