#pragma once
#include "geometries/Primitives.h"
#include "utils/DefUtil.h"
#include <vector>
#include <map>

struct tTriangle2D : std::enable_shared_from_this<tTriangle2D>
{
    tTriangle2D(int mTriangleId_, const tVector2 &p0, const tVector2 &p1,
                const tVector2 &p2);
    int mTriangleId = -1;
    tVector2 pos0;
    tVector2 pos1;
    tVector2 pos2;
};
SIM_DECLARE_STRUCT_AND_PTR(tTriangle2D);

struct tGrid
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    tGrid(const tVector2 &mAABBSt, const tVector2 &mAABBEd);
    bool IsInsideAABB(const tVector2 &cur_pos) const;
    int FindInsideTriangle(const tVector2 &cur_pos, tVector3 &bary);
    int FindNearestTriangleWhenOutside(const tVector2 &cur_pos,
                                       tVector3 &bary) const;
    void AddTriangle(int tri_id, tTriangle2DPtr cur_tri);
    _FLOAT CalcDistanceFromPosToSquareGrid(const tVector2 &pos) const;
    std::vector<tTriangle2DPtr> mTrianglesArray;
    tVector2 mAABBSt, mAABBEd;
    std::map<int, int> mLocalTriId2GlobalTriId;
};
SIM_DECLARE_PTR(tGrid);

class cBoundVolume2D
{
public:
    cBoundVolume2D();
    void AddTriangle(const tVector2 &pos0, const tVector2 &pos1,
                     const tVector2 &pos2);
    void InitVolume();
    int FindIncludedTriangle(const tVector2 &pos, tVector3 &bary);
    int FindNearestTriangle(const tVector2 &pos, tVector3 &bary);
    int FindNearestTriangleAll(const tVector2 &pos, tVector3 &bary);
    void Shift(const tVector2 &shift);
protected:
    std::vector<tTriangle2DPtr> mTriangleArray;
    std::vector<tGridPtr> mGridArray;

    tVector2 mAABBMin, mAABBMax;
    void UpdateAABB(const tVector2 &pos);
};