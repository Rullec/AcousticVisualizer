#include "KinematicBody.h"
#include "geometries/Primitives.h"
#include "utils/ColorUtil.h"
#include "utils/DefUtil.h"
#include "utils/JsonUtil.h"
#include "geometries/ObjUtil.h"
#include "utils/RenderUtil.h"
#include "utils/RotUtil.h"
#include <iostream>
std::string gBodyShapeStr[eKinematicBodyShape::NUM_OF_KINEMATIC_SHAPE] = {
    "plane", "cube", "sphere", "capsule", "custom"};
cKinematicBody::cKinematicBody(int id_)
    : cBaseObject(eObjectType::KINEMATICBODY_TYPE, id_)
{
    mIsStatic = true;
    mBodyShape = eKinematicBodyShape::KINEMATIC_INVALID;
    mCustomMeshPath = "";
    mTargetAABBDontUseDirectly = tVector4::Zero();
    mScaleDontUseDirectly.setZero();
    mPlaneEquation.setZero();
    mTargetPos.setZero();
    mCurTime = 0;
    mTargetOrientation.setZero();
}

cKinematicBody::~cKinematicBody() {}
void cKinematicBody::Init(const Json::Value &value)
{
    cBaseObject::Init(value);
    std::string type =
        cJsonUtil::ParseAsString(cKinematicBody::TYPE_KEY, value);
    mBodyShape = BuildKinematicBodyShape(type);
    switch (mBodyShape)
    {
    case eKinematicBodyShape::KINEMATIC_CUSTOM:
    {
        mCustomMeshPath =
            cJsonUtil::ParseAsString(cKinematicBody::MESH_PATH_KEY, value);
        cJsonUtil::ReadVectorJson(
            cJsonUtil::ParseAsValue(cKinematicBody::TARGET_AABB_KEY, value),
            mTargetAABBDontUseDirectly);

        cJsonUtil::ReadVectorJson(
            cJsonUtil::ParseAsValue(cKinematicBody::SCALE_KEY, value),
            mScaleDontUseDirectly);

        cJsonUtil::ReadVectorJson(
            cJsonUtil::ParseAsValue(cKinematicBody::TRANSLATION_KEY, value),
            mInitPos);
        cJsonUtil::ReadVectorJson(
            cJsonUtil::ParseAsValue(cKinematicBody::ORIENTATION_KEY, value),
            mInitOrientation);

        if (value.isMember(cKinematicBody::IS_STATIC_KEY) == true)
        {
            mIsStatic =
                cJsonUtil::ParseAsBool(cKinematicBody::IS_STATIC_KEY, value);
            if (mIsStatic == false)
            {
                // parse target translation, orientation
                cJsonUtil::ReadVectorJson(
                    cJsonUtil::ParseAsValue(
                        cKinematicBody::TARGET_ORIENTATION_KEY, value),
                    mTargetOrientation);
                cJsonUtil::ReadVectorJson(
                    cJsonUtil::ParseAsValue(
                        cKinematicBody::TARGET_TRANSLATION_KEY, value),
                    mTargetPos);
                // parse elasped time
                mMovingElaspedTimeSec = cJsonUtil::ParseAsfloat(
                    cKinematicBody::ELASPED_TIME_SEC_KEY, value);
            }
            // std::cout << "target orientation = "
            //           << mTargetOrientation.transpose() << std::endl;
            // std::cout << "target pos = " << mTargetPos.transpose() <<
            // std::endl; std::cout << "elasped time sec = " <<
            // mMovingElaspedTimeSec
            //           << std::endl;
            // exit(1);
        }
        else
        {
            mTargetPos = mInitPos;
            mTargetOrientation = mInitOrientation;
        }
        BuildCustomKinematicBody();
        UpdateCurWorldTransformByTime();
        SetMeshPos();
        break;
    }
    case eKinematicBodyShape::KINEMATIC_PLANE:
    {
        cJsonUtil::ReadVectorJson(
            cJsonUtil::ParseAsValue(cKinematicBody::PLANE_EQUATION_KEY, value),
            mPlaneEquation);
        // std::cout << "plane equation = " << mPlaneEquation.transpose() <<
        // std::endl;
        mPlaneScale =
            cJsonUtil::ParseAsfloat(cKinematicBody::PLANE_SCALE_KEY, value);
        BuildPlane();
        break;
    }
    default:
        SIM_ERROR("Unsupported kinematic shape {}", type);
    }

    // set edges color = black
    for (auto &e : this->mEdgeArray)
    {
        e->mColor = ColorBlack;
    }
    tVector4 min, max;
    CalcAABB(min, max);
    CalcTriangleInitArea();
    UpdateTriangleNormal();
    UpdateVertexNormalFromTriangleNormal();

    // std::cout << "[debug] obstacle aabb min = " << min.transpose() <<
    // std::endl; std::cout << "[debug] obstacle aabb max = " << max.transpose()
    // << std::endl;
}

eKinematicBodyShape
cKinematicBody::BuildKinematicBodyShape(std::string type_str)
{
    eKinematicBodyShape shape = eKinematicBodyShape::KINEMATIC_INVALID;
    for (int i = 0; i < eKinematicBodyShape::NUM_OF_KINEMATIC_SHAPE; i++)
    {
        if (gBodyShapeStr[i] == type_str)
        {
            shape = static_cast<eKinematicBodyShape>(i);
            break;
        }
    }
    SIM_ASSERT(shape != eKinematicBodyShape::KINEMATIC_INVALID);
    return shape;
}

bool cKinematicBody::IsStatic() const { return mIsStatic; }
#include "geometries/Triangulator.h"

/**
 * \brief           Build plane data strucutre
 */
void cKinematicBody::BuildPlane()
{
    // 1. build legacy XOZ plane, then do a transformation
    // for (int i = 0; i < 4; i++)
    cObjUtil::BuildPlaneGeometryData(mPlaneScale, this->mPlaneEquation,
                                     mVertexArray, mEdgeArray, mTriangleArray);
    for (auto &x : mVertexArray)
        x->mColor = ColorAn;
    for (auto &x : mTriangleArray)
        x->mColor = ColorMetalGray;
}

/**
 * \brief               Build custom kinematic body
 *      1. fill the scaled mesh vertices
 *      2. load the obj
 *      3. fill the color setting
 */
void cKinematicBody::BuildCustomKinematicBody()
{
    // std::cout << "mesh path = " << mCustomMeshPath << std::endl;
    cObjUtil::LoadObj(mCustomMeshPath, mVertexArray, mEdgeArray, mTriangleArray);

    // tMatrix trans = GetWorldTransform();
    tVector4 scale_vec = GetScaleVec();
    mScaledMeshVertices.noalias() = tVectorX::Zero(mVertexArray.size() * 3);
    for (int i = 0; i < mVertexArray.size(); i++)
    {
        auto &x = mVertexArray[i];
        mScaledMeshVertices.segment(3 * i, 3) =
            (scale_vec.cwiseProduct(x->mPos)).segment(0, 3);
        x->mColor = ColorAn;
    }
    for (auto &tri : mTriangleArray)
    {
        tri->mColor = ColorMetalGray;
    }
    // exit(0);
    cTriangulator::ValidateGeometry(mVertexArray, mEdgeArray, mTriangleArray);
}

/**
 * \brief           init the mesh pos by given init_pos and init_orien
 */
void cKinematicBody::SetMeshPos()
{
    tMatrix4 trans = GetCurWorldTransform();
    for (int i = 0; i < mVertexArray.size(); i++)
    {
        auto &x = mVertexArray[i];
        x->mPos =
            trans * cMathUtil::Expand(mScaledMeshVertices.segment(3 * i, 3), 1);
    }
}

// int cKinematicBody::GetDrawNumOfTriangles() const
// {
//     return mTriangleArray.size();
// }

// int cKinematicBody::GetDrawNumOfEdges() const { return
// mEdgeArray.size(); }

/**
 * \brief           Get the world transform of this kinematic body
 *          this matrix can convert local pos to world pos in homogeneous coords
 *          world_pos = T * local_pos
 */
tMatrix4 GetWorldTransform(const tVector4 &init_pos,
                           const tVector4 &init_ori)
{
    tMatrix4 trans = tMatrix4::Identity();
    trans.block(0, 3, 3, 1) = init_pos.segment(0, 3);
    trans.block(0, 0, 3, 3) =
        cRotUtil::EulerAnglesToRotMat(init_ori, eRotationOrder::XYZ)
            .topLeftCorner<3, 3>();
    return trans;
}

void cKinematicBody::UpdateCurWorldTransformByTime()
{
    if (IsStatic())
    {
        mCurWorldTransform = GetWorldTransform(mInitPos, mInitOrientation);
    }
    else
    {
        FLOAT cur_time = mCurTime;
        if (cur_time > mMovingElaspedTimeSec)
            cur_time = mMovingElaspedTimeSec;
        tVector4 cur_pos = (mMovingElaspedTimeSec - cur_time) /
                               mMovingElaspedTimeSec * mInitPos +
                           cur_time / mMovingElaspedTimeSec * mTargetPos;
        tVector4 cur_ori =
            (mMovingElaspedTimeSec - cur_time) / mMovingElaspedTimeSec *
                mInitOrientation +
            cur_time / mMovingElaspedTimeSec * mTargetOrientation;
        mCurWorldTransform = GetWorldTransform(cur_pos, cur_ori);
        // std::cout << "[kin] update cur world transform, cur pos = "
        //           << cur_pos.transpose()
        //           << ", cur ori = " << cur_ori.transpose() << std::endl;
    }
}

tVector4 cKinematicBody::GetScaleVec() const
{
    FLOAT eps = 1e-6;
    if (mTargetAABBDontUseDirectly.norm() > eps &&
        mScaleDontUseDirectly.norm() > eps)
    {
        SIM_ERROR("{} and {} can only be set once for body {}",
                  cKinematicBody::SCALE_KEY, cKinematicBody::TARGET_AABB_KEY,
                  this->mObjName);
        exit(1);
    }

    // use AABB
    tVector4 scale_vec = tVector4::Ones();
    if (mTargetAABBDontUseDirectly.norm() > eps)
    {
        tVector4 aabb_min, aabb_max;
        CalcAABB(aabb_min, aabb_max);
        tVector4 aabb = aabb_max - aabb_min;
        // std::cout << "init aabb = " << (aabb_max - aabb_min).transpose() <<
        // std::endl; exit(0);
        for (int i = 0; i < 3; i++)
        {
            if (std::fabs(aabb[i]) < 1e-10)
            {
                continue;
            }
            scale_vec[i] = mTargetAABBDontUseDirectly[i] / aabb[i];
        }
    }

    else
    {
        // use scale_mat
        scale_vec.segment(0, 3) = this->mScaleDontUseDirectly.segment(0, 3);
    }
    return scale_vec;
}
/**
 * \brief           update kinectmatic body
 */
void cKinematicBody::Update(FLOAT dt)
{
    mCurTime += dt;
    if (IsStatic() == false)
    {
        UpdateCurWorldTransformByTime();
        SetMeshPos();
        // std::cout << "update kin body transform, cur = \n"
        //           << mCurWorldTransform << std::endl;
    }
}

tMatrix4 cKinematicBody::GetCurWorldTransform() const
{
    return this->mCurWorldTransform;
}

void cKinematicBody::Reset()
{
    cBaseObject::Reset();
    this->mCurTime = 0;
}

tVector4 cKinematicBody::CalcCOM() const
{
    tVector4 com = tVector4::Zero();
    for (auto &v : this->mVertexArray)
    {
        com += v->mPos;
    }
    com /= mVertexArray.size();
    return com;
}
void cKinematicBody::MoveTranslation(const tVector4 &shift)
{
    for (auto &v : this->mVertexArray)
    {
        v->mPos += shift;
    }
}

void cKinematicBody::ApplyScale(FLOAT scale)
{
    for (auto &v : mVertexArray)
    {
        v->mPos.segment(0, 3) = v->mPos.segment(0, 3) * scale;
    }
}

void cKinematicBody::ApplyUserPerturbForceOnce(tPerturb *) {}

void cKinematicBody::SetCurrentPos(const tVector3 &pos)
{
    mCurWorldTransform.block(0, 3, 3, 1) = pos;
    SetMeshPos();
}
