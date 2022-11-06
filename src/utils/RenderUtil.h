#pragma once
#include "utils/DefUtil.h"
#include "utils/MathUtil.h"

SIM_DECLARE_STRUCT_AND_PTR(tVertex);

struct tCPURenderBuffer
{
    tCPURenderBuffer();
    const float *mBuffer;
    uint mNumOfEle;
};

class cRenderResource : std::enable_shared_from_this<cRenderResource>
{
public:
    std::string mName;
    tCPURenderBuffer mTriangleBuffer, mLineBuffer, mPointBuffer;
};

SIM_DECLARE_PTR(cRenderResource);

class cRenderUtil
{
public:
    static void CalcTriangleDrawBufferSingle(tVertexPtr v0, tVertexPtr v1,
                                             tVertexPtr v2,
                                             const tVector4 &color,
                                             Eigen::Map<tVectorXf> &buffer,
                                             int &st_pos);

    static void CalcEdgeDrawBufferSingle(tVertexPtr v0, tVertexPtr v1,
                                         const tVector4 &edge_normal,
                                         Eigen::Map<tVectorXf> &buffer,
                                         int &st_pos, const tVector4 &color);

    static void CalcEdgeDrawBufferSingle(const tVector4 &v0,
                                         const tVector4 &v1,
                                         const tVector4 &edge_normal,
                                         Eigen::Map<tVectorXf> &buffer,
                                         int &st_pos, const tVector4 &color);
    static void CalcPointDrawBufferSingle(const tVector4 &pos,
                                          const tVector4 &color,
                                          Eigen::Map<tVectorXf> &buffer,
                                          int &st_pos);
    static cRenderResourcePtr GetAxesRenderingResource();
};