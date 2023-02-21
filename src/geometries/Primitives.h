#pragma once
#include "utils/DefUtil.h"
#include "utils/EigenUtil.h"
struct tVertex
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    tVertex();
    tVertex(const tVertex &old_v);
    _FLOAT mMass;
    tVector4 mPos;
    tVector4 mNormal;
    tVector2 muv_simple; // the texture uv should be defined over face,
                         // instead of vertex. This variable is just for
                         // simulation usage instead of rendering
    tVector4 mColor;
};
SIM_DECLARE_PTR(tVertex);
struct tEdge
{
    tEdge();
    int mId0, mId1;
    _FLOAT mRawLength; // raw length of this edge
    bool mIsBoundary;  // does this edge locate in the boundary?
    int mTriangleId0,
        mTriangleId1; // The indices of the two triangles to which this side
                      // belongs. If this edge is a boundary, the mTriangleId1
                      // is -1
    _FLOAT mK_spring; // stiffness for springs
    tVector4 mColor;
};
SIM_DECLARE_PTR(tEdge);
// struct tEdge : public tEdge
// {
//     EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
//     tEdge();
//     FLOAT mK;
// };

struct tTriangle
{
    explicit tTriangle();
    explicit tTriangle(int a, int b, int c);
    int mId0, mId1, mId2;    // vertex id
    int mEId0, mEId1, mEId2; // edge id
    tVector4 mNormal;
    tVector4 mColor;
    tVector2
        muv[3]; // "texture" coordinate 2d, it means the plane coordinate for
                // a vertex over a fabric, but now the texture in rendering
};
SIM_DECLARE_PTR(tTriangle);
/**
 * \brief       an origin + a directed ray
 */
struct tRay
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    explicit tRay(const tVector4 &ori, const tVector4 &end);
    tVector4 mOrigin;
    tVector4 mDir;
};

struct tRectangle
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    tRectangle();
    tVector4 mVertex[4];
};