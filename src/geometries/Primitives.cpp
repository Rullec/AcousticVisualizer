#include "Primitives.h"
#include "utils/LogUtil.h"
#include "utils/MathUtil.h"
tVertex::tVertex()
{
    mMass = 0;
    mPos = tVector4(0, 0, 0, 1);
    muv.setZero();
    mColor.setZero();
}

tEdge::tEdge()
{
    mId0 = mId1 = -1;
    mRawLength = 0;
    mIsBoundary = false;
    mTriangleId0 = mTriangleId1 = -1;
    mK_spring = 0;
    mColor.setZero();
}

tTriangle::tTriangle()
{
    mId0 = mId1 = mId2 = -1;
    mEId0 = mEId1 = mEId2 = -1;
    mNormal.setZero();
    mColor.setZero();
}
tTriangle::tTriangle(int a, int b, int c) : mId0(a), mId1(b), mId2(c)
{
    mEId0 = mEId1 = mEId2 = -1;
    mNormal.setZero();
}

tRay::tRay(const tVector4 &ori, const tVector4 &end)
{
    SIM_ASSERT(cMathUtil::IsPoint(ori) == true);
    SIM_ASSERT(cMathUtil::IsPoint(end) == true);
    mOrigin = ori;
    mDir = (end - ori).normalized();
}

tRectangle::tRectangle()
{
    for (int i = 0; i < 4; i++)
    {
        mVertex[i].setZero();
    }
}