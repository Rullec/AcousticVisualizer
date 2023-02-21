#pragma once
#include "sim/BaseObject.h"
#include "utils/EigenUtil.h"

enum eKinematicBodyShape
{
    KINEMATIC_PLANE = 0,
    KINEMATIC_CUBE,
    KINEMATIC_SPHERE,
    KINEMATIC_CAPSULE,
    KINEMATIC_CUSTOM,
    NUM_OF_KINEMATIC_SHAPE,
    KINEMATIC_INVALID
};

SIM_DECLARE_STRUCT_AND_PTR(tMeshMaterialInfo);
class cKinematicBody : public cBaseObject
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    inline const static std::string
        TYPE_KEY = "type",
        MESH_PATH_KEY = "mesh_path", TARGET_AABB_KEY = "target_aabb",
        SCALE_KEY = "scale", TRANSLATION_KEY = "translation",
        ORIENTATION_KEY = "orientation", PLANE_EQUATION_KEY = "equation",
        PLANE_SCALE_KEY = "plane_scale",
        // the below setting only work for moving object
        IS_STATIC_KEY = "is_static",
        TARGET_TRANSLATION_KEY = "target_translation",
        TARGET_ORIENTATION_KEY = "target_orientation",
        ELASPED_TIME_SEC_KEY = "elasped_time_sec";
    cKinematicBody(int id_);
    virtual ~cKinematicBody();
    virtual void Init(const Json::Value &conf) override;
    static eKinematicBodyShape BuildKinematicBodyShape(std::string type_str);
    bool IsStatic() const;
    eKinematicBodyShape GetBodyShape() const;
    virtual void Update(_FLOAT dt) override;
    virtual void ApplyUserPerturbForceOnce(tPerturb *) override;
    // virtual void UpdatePos(FLOAT dt) override final;
    // virtual void UpdateRenderingResource() override final;
    virtual tMatrix4 GetCurWorldTransform() const; //
    void Reset() override;
    virtual tVector3 CalcCOM() const;
    virtual void MoveTranslation(const tVector3 &shift);
    virtual void ApplyScale(_FLOAT scale);
    virtual void SetCurrentPos(const tVector3 &pos);
    virtual void SetCurrentTransform(const tMatrix4 &transmat);
    virtual tVector3 GetCurrentPos() const;

protected:
    _FLOAT mCurTime;
    eKinematicBodyShape mBodyShape;
    std::string mCustomMeshPath;
    tVectorX mScaledMeshVertices; // the loaded obj mesh pos after AABB rescaled
    tVector3 mTargetAABBDontUseDirectly; // scale setting
    tVector3 mScaleDontUseDirectly;      // scale setting explictly
    tVector3 mInitPos;                   // init position for kinematic body
    tVector3 mInitOrientation;           // init orietnation for kinect body,
    tVector3 mTargetPos, mTargetOrientation; // target pos and orientation
    bool mIsStatic;
    tVector4 mPlaneEquation;
    _FLOAT mPlaneScale;
    _FLOAT mMovingElaspedTimeSec; // how many seconds does it cost for moving
                                  // object?
    tMatrix4 mCurWorldTransform;

    // methods
    void BuildCustomKinematicBody();
    void SetMeshPos();
    void BuildPlane();
    void UpdateCurWorldTransformByTime();
    tVector3 GetScaleVec() const;
};

SIM_DECLARE_PTR(cKinematicBody);