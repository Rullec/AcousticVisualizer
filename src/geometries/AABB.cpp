#include "AABB.h"
#include "geometries/Primitives.h"
#include "utils/LogUtil.h"
tAABB::tAABB() { Reset(); }

void tAABB::Expand(const tVector4 &vec)
{
    if (IsInvalid() == true)
    {
        mMin = vec;
        mMax = vec;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            mMin[i] = SIM_MIN(vec[i], mMin[i]);
            mMax[i] = SIM_MAX(vec[i], mMax[i]);
        }
    }
}
void tAABB::Expand(const tVertexPtr &ptr) { Expand(ptr->mPos); }

bool tAABB::IsInvalid() const
{
    SIM_ASSERT((mMax - mMin).minCoeff() >= 0);
    return mMin.hasNaN() || mMax.hasNaN();
}

void tAABB::Expand(const tAABB &new_AABB)
{
    if (new_AABB.IsInvalid())
    {
        SIM_WARN("new AABB is invalid!");
        return;
    }
    if (IsInvalid() == true)
    {
        mMin = new_AABB.mMin;
        mMax = new_AABB.mMax;
    }
    else
    {

        for (int i = 0; i < 3; i++)
        {
            mMin[i] = SIM_MIN(new_AABB.mMin[i], mMin[i]);
            mMax[i] = SIM_MAX(new_AABB.mMax[i], mMax[i]);
        }
    }
}

int tAABB::GetMaxExtent() const
{
    tVector4 extent = mMax - mMin;
    FLOAT max_extent = extent[0];
    int max_extent_id = 0;
    for (int i = 1; i < 3; i++)
    {
        if (extent[i] > max_extent)
        {
            max_extent = extent[i];
            max_extent_id = i;
        }
    }
    return max_extent_id;
}

tAABB::tAABB(const tAABB &old_AABB)
{
    mMin = old_AABB.mMin;
    mMax = old_AABB.mMax;
}
tVector4 tAABB::GetExtent() const { return mMax - mMin; }
tVector4 tAABB::GetMiddle() const
{
    tVector4 middle = (mMax + mMin) / 2;
    middle[3] = 0;
    return middle;
}

bool tAABB::Intersect(const tAABB &other_AABB)
{
    return ((mMin[0] <= other_AABB.mMax[0]) &&
            (mMax[0] >= other_AABB.mMin[0])) &&
           ((mMin[1] <= other_AABB.mMax[1]) &&
            (mMax[1] >= other_AABB.mMin[1])) &&
           ((mMin[2] <= other_AABB.mMax[2]) && (mMax[2] >= other_AABB.mMin[2]));
}
void tAABB::Reset()
{
    mMin = std::nan("") * tVector4::Ones();
    mMax = std::nan("") * tVector4::Ones();
    mMax[3] = 0;
    mMin[3] = 0;
}
void tAABB::Increase(const tVector4 & dist)
{
    mMin -= dist;
    mMax += dist;
}