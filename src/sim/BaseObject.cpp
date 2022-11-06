#include "sim/BaseObject.h"
#include "geometries/Primitives.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"
#include "utils/MathUtil.h"
#include "utils/RenderUtil.h"
#include <string>
#include "utils/ColorUtil.h"
#include <set>

std::string gObjectTypeStr[eObjectType::NUM_OBJ_TYPES] = {
    "KinematicBody", "AcousticBody"};

cBaseObject::cBaseObject(eObjectType type, int id_) : mType(type), mObjId(id_)
{
    mObjName = "";
    mEnableDrawBuffer = true;
    mGravity.setZero();
}
int cBaseObject::GetObjId() const { return this->mObjId; }
/**
 * \brief           Set object name
 */
void cBaseObject::SetObjName(std::string name) { mObjName = name; }
/**
 * \brief           Get object name
 */
std::string cBaseObject::GetObjName() const { return mObjName; }

cBaseObject::~cBaseObject() {}

eObjectType cBaseObject::BuildObjectType(std::string str)
{
    eObjectType type = eObjectType::INVALID_OBJ_TYPE;
    for (int i = 0; i < eObjectType::NUM_OBJ_TYPES; i++)
    {
        if (gObjectTypeStr[i] == str)
        {
            type = static_cast<eObjectType>(i);
            break;
        }
    }

    SIM_ASSERT(type != eObjectType::INVALID_OBJ_TYPE);
    return type;
}

eObjectType cBaseObject::GetObjectType() const { return this->mType; }

int cBaseObject::GetNumOfTriangles() const { return mTriangleArray.size(); }
int cBaseObject::GetNumOfEdges() const { return mEdgeArray.size(); }
int cBaseObject::GetNumOfVertices() const { return mVertexArray.size(); }
const std::vector<tVertexPtr> &cBaseObject::GetVertexArray() const
{
    return this->mVertexArray;
}
const std::vector<tEdgePtr> &cBaseObject::GetEdgeArray() const
{
    return this->mEdgeArray;
}
const std::vector<tTrianglePtr> &cBaseObject::GetTriangleArray() const
{
    return this->mTriangleArray;
}

std::vector<tVertexPtr> &cBaseObject::GetVertexArrayRef()
{
    return mVertexArray;
}

std::vector<tEdgePtr> &cBaseObject::GetEdgeArrayRef() { return mEdgeArray; }
std::vector<tTrianglePtr> &cBaseObject::GetTriangleArrayRef()
{
    return mTriangleArray;
}

/**
 * \brief           change triangle color
 */
void cBaseObject::ChangeTriangleColor(int tri_id, const tVector3 &color)
{
    mTriangleArray[tri_id]->mColor.segment(0, 3) = color;
}

/**
 * \brief           Calculate axis aligned bounding box
 */
#include <cfloat>
void cBaseObject::CalcAABB(tVector4 &min, tVector4 &max) const
{
    min = tVector4::Ones() * std::numeric_limits<float>::max();
    max = tVector4::Ones() * std::numeric_limits<float>::max() * -1;
    for (auto &x : mVertexArray)
    {
        for (int i = 0; i < 3; i++)
        {

            FLOAT val = x->mPos[i];
            min[i] = (val < min[i]) ? val : min[i];
            max[i] = (val > max[i]) ? val : max[i];
        }
    }
}

void cBaseObject::Init(const Json::Value &conf)
{
    mObjName = cJsonUtil::ParseAsString(OBJECT_NAME_KEY, conf);
}

/**
 * \brief           Update the normal of triangles
 */
#include "utils/TimeUtil.hpp"
// #include <iostream>
void cBaseObject::UpdateTriangleNormal()
{
    // we assume the rotation axis of v0, v1, v2 is the normal direction here
    // cTimeUtil::Begin("update_normal");
    for (auto &tri : mTriangleArray)
    {
        const tVector4 &v0 = mVertexArray[tri->mId0]->mPos;
        const tVector4 &v1 = mVertexArray[tri->mId1]->mPos;
        const tVector4 &v2 = mVertexArray[tri->mId2]->mPos;

        tVector4 tmp0 = (v1 - v0);
        tVector4 tmp1 = (v2 - v1);
        tVector4 res = tmp0.cross3(tmp1);
        res.normalize();
        tri->mNormal = res;
        // std::cout << tri->mNormal.transpose() << std::endl;
    }
    // cTimeUtil::End("update_normal");
}

/**
 * \brief       update the vertex from triangle normal
 */
#include <iostream>
void cBaseObject::UpdateVertexNormalFromTriangleNormal()
{
    // 1. clear all vertex normal
    // cTimeUtil::Begin("update_v_normal");
    for (auto &x : mVertexArray)
        x->mNormal.setZero();
    // 2. iter each edge
    for (int t_id = 0; t_id < mTriangleArray.size(); t_id++)
    {
        auto x = mTriangleArray[t_id];
        FLOAT tri_area = mTriangleInitArea[t_id];
        mVertexArray[x->mId0]->mNormal += x->mNormal * tri_area;
        mVertexArray[x->mId1]->mNormal += x->mNormal * tri_area;
        mVertexArray[x->mId2]->mNormal += x->mNormal * tri_area;
    }

    // 3. averge each vertex
    for (int i = 0; i < mVertexArray.size(); i++)
    {
        auto &v = mVertexArray[i];
        v->mNormal.normalize();
    }
    // cTimeUtil::End("update_v_normal");
}

/**
 * \brief           set the alpha channel for vertex color
 */
void cBaseObject::SetVertexColorAlpha(FLOAT val)
{
    mColorAlpha = val;
    for (auto &v : mVertexArray)
    {
        v->mColor[3] = val;
    }
}

/**
 * \brief           get vertex color alpha
 */
FLOAT cBaseObject::GetVertexColorAlpha() const { return mColorAlpha; }

/**
 * \brief       calcualte the total area
 */
FLOAT cBaseObject::CalcTotalArea() const
{
    FLOAT total_area = 0;
    for (auto &t : mTriangleArray)
    {
        total_area += cMathUtil::CalcTriangleArea(mVertexArray[t->mId0]->mPos,
                                                  mVertexArray[t->mId1]->mPos,
                                                  mVertexArray[t->mId2]->mPos);
    }
    return total_area;
}

void cBaseObject::UpdateImGui() {}

void cBaseObject::SetGravity(const tVector3 &g) { mGravity.noalias() = g; }

void cBaseObject::SetPointTriangleCollisionInfo(
    const std::vector<tPointTriangleCollisionInfoPtr> &info)
{
    mPointTriangleCollisionInfo = info;
}
void cBaseObject::SetEdgeEdgeCollisionInfo(
    const std::vector<tEdgeEdgeCollisionInfoPtr> &info)
{
    this->mEdgeEdgeCollisionInfo = info;
}

void cBaseObject::CalcTriangleDrawBuffer(Eigen::Map<tVectorXf> &res,
                                         int &st) const
{
    int old_st = st;
    for (auto &tri : mTriangleArray)
    {
        cRenderUtil::CalcTriangleDrawBufferSingle(
            this->mVertexArray[tri->mId0], this->mVertexArray[tri->mId1],
            this->mVertexArray[tri->mId2], tri->mColor, res, st);
    }
    // 1. calculate affected vid
    // for (auto info : mPointTriangleCollisionInfo)
    // {
    //     // handle triangle
    //     if (info->mObj1->GetObjId() == mObjId)
    //     {
    //         // std::cout << "change triangle color " << info->mTriangleId1
    //         //           << std::endl;
    //         // change triangle color
    //         res.segment(
    //             old_st + 3 * RENDERING_SIZE_PER_VERTICE * info->mTriangleId1
    //             +
    //                 0 * RENDERING_SIZE_PER_VERTICE + 3,
    //             4) = ColorShoJoHi;
    //         res.segment(
    //             old_st + 3 * RENDERING_SIZE_PER_VERTICE * info->mTriangleId1
    //             +
    //                 1 * RENDERING_SIZE_PER_VERTICE + 3,
    //             4) = ColorShoJoHi;
    //         res.segment(
    //             old_st + 3 * RENDERING_SIZE_PER_VERTICE * info->mTriangleId1
    //             +
    //                 2 * RENDERING_SIZE_PER_VERTICE + 3,
    //             4) = ColorShoJoHi;
    //     }
    // }
    // for (auto info : mPointTriangleCollisionInfo)
    // {
    //     // handle triangle
    //     if (info->mObj1->GetObjId() == mObjId)
    //     {
    //         // std::cout << "change triangle color " << info->mTriangleId1
    //         //           << std::endl;
    //         // change triangle color
    //         res.segment(
    //             old_st + 3 * RENDERING_SIZE_PER_VERTICE * info->mTriangleId1
    //             +
    //                 0 * RENDERING_SIZE_PER_VERTICE + 3,
    //             4) = ColorShoJoHi;
    //         res.segment(
    //             old_st + 3 * RENDERING_SIZE_PER_VERTICE * info->mTriangleId1
    //             +
    //                 1 * RENDERING_SIZE_PER_VERTICE + 3,
    //             4) = ColorShoJoHi;
    //         res.segment(
    //             old_st + 3 * RENDERING_SIZE_PER_VERTICE * info->mTriangleId1
    //             +
    //                 2 * RENDERING_SIZE_PER_VERTICE + 3,
    //             4) = ColorShoJoHi;
    //     }
    // }
}

void cBaseObject::CalcPointDrawBuffer(Eigen::Map<tVectorXf> &res, int &st) const
{
    // std::cout << "v0 color = " << mVertexArray[0]->mColor.transpose()
    // << std::endl;
    int old_st = st;
    for (auto &v : mVertexArray)
    {
        cRenderUtil::CalcPointDrawBufferSingle(v->mPos, v->mColor, res, st);
    }
}

void cBaseObject::CalcEdgeDrawBuffer(Eigen::Map<tVectorXf> &res, int &st) const
{
    // std::set<int> affected_edge = {};
    // for (auto &info : this->mEdgeEdgeCollisionInfo)
    // {
    //     if (info->mObj0->GetObjId() == mObjId)
    //     {
    //         affected_edge.insert(info->mEdgeId0);
    //     }
    //     else
    //     {
    //         affected_edge.insert(info->mEdgeId1);
    //     }
    // }

    tVector4 normal = tVector4::Zero();

    for (int e_id = 0; e_id < mEdgeArray.size(); e_id++)
    {
        auto e = mEdgeArray[e_id];
        tVector4 color = e->mColor;
        // if (affected_edge.find(e_id) != affected_edge.end())
        // {
        //     color = ColorPurple;
        // }
        // 1. get the normal of this edge
        normal =
            (mVertexArray[e->mId0]->mNormal + mVertexArray[e->mId1]->mNormal) /
            2;

        cRenderUtil::CalcEdgeDrawBufferSingle(mVertexArray[e->mId0],
                                              mVertexArray[e->mId1], normal,
                                              res, st, color);
    }
}

void cBaseObject::Reset()
{
    mPointTriangleCollisionInfo.clear();
    mEdgeEdgeCollisionInfo.clear();
}

void cBaseObject::CalcTriangleInitArea()
{
    // create triangle area
    mTriangleInitArea.resize(GetNumOfTriangles());
    for (int i = 0; i < GetNumOfTriangles(); i++)
    {
        auto tri = mTriangleArray[i];

        mTriangleInitArea[i] = cMathUtil::CalcTriangleArea(
            mVertexArray[tri->mId0]->mPos, mVertexArray[tri->mId1]->mPos,
            mVertexArray[tri->mId2]->mPos);
    }
}