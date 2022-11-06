#pragma once
#include "sim/BaseObject.h"
#include "utils/MathUtil.h"

struct tTriangle;
struct tEdge;
struct tVertex;
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

    virtual void Update(FLOAT dt) override;
    virtual void ApplyUserPerturbForceOnce(tPerturb *) override;
    // virtual void UpdatePos(FLOAT dt) override final;
    // virtual void UpdateRenderingResource() override final;
    virtual tMatrix4 GetCurWorldTransform() const; //
    void Reset() override;
    virtual tVector4 CalcCOM() const;
    virtual void MoveTranslation(const tVector4 &shift);
    virtual void ApplyScale(FLOAT scale);
    virtual void SetCurrentPos(const tVector3 &pos);

protected:
    FLOAT mCurTime;
    eKinematicBodyShape mBodyShape;
    std::string mCustomMeshPath;
    tVectorX
        mScaledMeshVertices;                 // the loaded obj mesh pos after AABB rescaled
    tVector4 mTargetAABBDontUseDirectly;     // scale setting
    tVector4 mScaleDontUseDirectly;          // scale setting explictly
    tVector4 mInitPos;                       // init position for kinematic body
    tVector4 mInitOrientation;               // init orietnation for kinect body,
    tVector4 mTargetPos, mTargetOrientation; // target pos and orientation
    bool mIsStatic;
    tVector4 mPlaneEquation;
    FLOAT mPlaneScale;
    FLOAT mMovingElaspedTimeSec; // how many seconds does it cost for moving
                                 // object?
    tMatrix4 mCurWorldTransform; //
    // methods
    void BuildCustomKinematicBody();
    void SetMeshPos();
    void BuildPlane();
    void UpdateCurWorldTransformByTime();
    tVector4 GetScaleVec() const;
    // virtual void InitDrawBuffer() override final;
};