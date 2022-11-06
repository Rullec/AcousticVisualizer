#pragma once
#include "utils/DefUtil.h"
#include "utils/MathUtil.h"

/**
 * \brief           user perturb force
 */
struct tTriangle;
struct tVertex;
struct tRay;
SIM_DECLARE_CLASS_AND_PTR(cBaseObject);
struct tPerturb
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    tPerturb();
    void InitTangentRect(const tVector4 &plane_normal);
    void UpdatePerturbPos(const tVector4 &cur_camera_pos, const tVector4 &dir);

    tVector4 GetPerturbForce() const;
    tVector4 CalcPerturbPos() const;
    tVector4 GetGoalPos() const;

    cBaseObjectPtr mObject;
    int mAffectedTriId; // triangle id
    // int mAffectedVerticesId[3];
    // tVertexPtr mAffectedVertices[3];
    tVector3 mBarycentricCoords; // barycentric coordinates of raw raycast
                                  // point on the affected triangle
protected:
    tVector4 mPerturbForce;
    tVector4 mShiftPlaneEquation;
    tVector4 mGoalPos;
};