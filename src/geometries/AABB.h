#pragma once
#include "utils/DefUtil.h"
#include "utils/MathUtil.h"

SIM_DECLARE_STRUCT_AND_PTR(tVertex);

struct tAABB
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    explicit tAABB();
    tAABB(const tAABB &old_AABB);
    int GetMaxExtent() const;
    tVector4 GetExtent() const;
    tVector4 GetMiddle() const;
    void Reset();
    void Expand(const tVector3 &);
    void Expand(const tVector4 &);
    void Expand(const tVertexPtr &);
    void Increase(const tVector4 & dist);
    void Expand(const tAABB &);
    bool IsInvalid() const;
    bool Intersect(const tAABB &other_AABB);
    tVector4 mMin, mMax;
};