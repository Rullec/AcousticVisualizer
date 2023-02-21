#pragma once
#include "utils/MathUtil.h"
/**
 * \brief       discretize a cubic beizer curve (determined by 4 points)
 */
class cBezierCurve
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    explicit cBezierCurve(int num_of_div, const tVector2 &A,
                          const tVector2 &B, const tVector2 &C,
                          const tVector2 &D);
    virtual _FLOAT GetTotalLength() const;
    virtual int GetNumOfDrawEdges() const;
    virtual const tVectorXf &GetDrawBuffer();
    tEigenArr<tVector2> GetPointList();

protected:
    tVectorXf mDrawBuffer;
    int mNumOfDiv;
    tVector2 A, B, C, D;
    tMatrixX mPointList;
    virtual void InitPointlist(tMatrixX &point_lst);
};