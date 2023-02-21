#include "sim/BaseObject.h"
#include "geometries/Primitives.h"
#include "utils/ColorUtil.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"
#include "utils/MathUtil.h"
#include "utils/RenderUtil.h"
#include <set>
#include <string>

static std::string gObjectTypeStr[eObjectType::NUM_OBJ_TYPES] = {
    "Yarn", "KinematicBody", "AcousticBody", "AcousticManager",
    "SNISR_Debug_Draw"};

cBaseObject::cBaseObject(eObjectType type, int id_) : mType(type), mObjId(id_)
{
    mObjName = "";
    mEnableDrawBuffer = true;
    mGravity.setZero();
    mEnableTextureUV = false;
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
void cBaseObject::ChangeVertexColor(int vid, const tVector3 &color)
{
    mVertexArray[vid]->mColor.segment(0, 3) = color;
}

void cBaseObject::ChangeVerticesColor(const tVector3 &color)
{
    for (int i = 0; i < GetNumOfVertices(); i++)
    {
        ChangeVertexColor(i, color);
    }
}
/**
 * \brief           change triangle color
 */
void cBaseObject::ChangeTriangleColor(int tri_id, const tVector3 &color)
{
    printf("begin to change tri %d color, num of tris %d\n", tri_id,
           mTriangleArray.size());
    mTriangleArray[tri_id]->mColor.segment(0, 3) = color;
    // tVector new_color = tVector4(color[0], color[1], color[2], mColorAlpha);
    // mVertexArray[mTriangleArray[tri_id]->mId0]->mColor =
    // new_color; mVertexArray[mTriangleArray[tri_id]->mId1]->mColor
    // = new_color;
    // mVertexArray[mTriangleArray[tri_id]->mId2]->mColor =
    // new_color;
}

void cBaseObject::ChangeTrianglesColor(const tVector3 &color)
{
    for (int i = 0; i < GetNumOfTriangles(); i++)
    {
        ChangeTriangleColor(i, color);
    }
}

/**
 * \brief           Calculate axis aligned bounding box
 */
#include <cfloat>
void cBaseObject::CalcAABB(tVector3 &min, tVector3 &max) const
{
    min = tVector3::Ones() * std::numeric_limits<float>::max();
    max = tVector3::Ones() * std::numeric_limits<float>::max() * -1;
    for (auto &x : mVertexArray)
    {
        for (int i = 0; i < 3; i++)
        {

            _FLOAT val = x->mPos[i];
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
        res[3] = 0;
        res.normalize();
        tri->mNormal = res;
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
        _FLOAT tri_area = mTriangleInitArea[t_id];
        mVertexArray[x->mId0]->mNormal += x->mNormal * tri_area;
        mVertexArray[x->mId1]->mNormal += x->mNormal * tri_area;
        mVertexArray[x->mId2]->mNormal += x->mNormal * tri_area;
    }

    // 3. averge each vertex
    for (int i = 0; i < mVertexArray.size(); i++)
    {
        auto &v = mVertexArray[i];
        v->mNormal[3] = 0;
        v->mNormal.normalize();
    }
    // cTimeUtil::End("update_v_normal");
}

/**
 * \brief           set the alpha channel for vertex color
 */
void cBaseObject::SetColorAlpha(_FLOAT val)
{
    mColorAlpha = val;
    for (auto &v : mTriangleArray)
    {
        v->mColor[3] = val;
    }
}

/**
 * \brief           get vertex color alpha
 */
_FLOAT cBaseObject::GetColorAlpha() const { return mColorAlpha; }

/**
 * \brief       calcualte the total area
 */
_FLOAT cBaseObject::CalcTotalArea() const
{
    _FLOAT total_area = 0;
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
template <class dtype>
void print_vec(std::string str, const std::vector<dtype> &vec)
{
    std::cout << str << std::endl;
    for (auto &x : vec)
    {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}
void cBaseObject::UpdateRenderResourceTriangles()
{
    int num_of_mater = mMatInfoArray.size();

    // 1. validate material info for each triangles
    tMeshMaterialInfo::ValidateMaterialInfo(mMatInfoArray,
                                            this->GetNumOfTriangles());

    // 2. calc for each material
    mTriangleDrawBufferArray.resize(num_of_mater);
    for (int mat_id = 0; mat_id < num_of_mater; mat_id++)
    {
        tVectorXf &buf = mTriangleDrawBufferArray[mat_id];

        const auto &tri_id_array = mMatInfoArray[mat_id]->mTriIdArray;
        // print_vec("material " + std::to_string(mat_id) + " tri id: ",
        //           tri_id_array);
        int num_of_tri_comp = tri_id_array.size();
        buf.resize(num_of_tri_comp * RENDERING_SIZE_PER_VERTICE * 3);
        buf.setConstant(std::nanf(""));

        Eigen::Map<tVectorXf> map(buf.data(), buf.size());
        printf("kin render resource id %d, draw vertices %d\n", mat_id,
               num_of_tri_comp * 3);
        int st = 0;
        for (auto &t_id : tri_id_array)
        {
            const auto &t = mTriangleArray[t_id];
            cRenderUtil::CalcTriangleDrawBufferSingle(
                mVertexArray, t, t->mColor, map, st, mEnableTextureUV);
        }
    }
}
void cBaseObject::UpdateRenderResourceLines()
{
    mEdgeBuffer.resize(mEdgeArray.size() * RENDERING_SIZE_PER_VERTICE * 2);
    mEdgeBuffer.noalias() = tVectorXf::Ones(mEdgeBuffer.size()) * std::nanf("");

    int st = 0;
    Eigen::Map<tVectorXf> map(mEdgeBuffer.data(), mEdgeBuffer.size());
    for (auto e : mEdgeArray)
    {
        cRenderUtil::CalcEdgeDrawBufferSingle(
            mVertexArray[e->mId0]->mPos, mVertexArray[e->mId1]->mPos,
            tVector4::Zero(), map, st, ColorBlack);
    }
}
void cBaseObject::UpdateRenderResourcePoints() { mPointDrawBuffer.resize(0); }
void cBaseObject::UpdateRenderingResource(bool enable_edge /*= true*/)
{
    mRenderResource.clear();

    UpdateRenderResourceTriangles();
    if (enable_edge)
        UpdateRenderResourceLines();
    else
    {
        mEdgeBuffer.resize(0);
    }
    UpdateRenderResourcePoints();

    // 2. point, edges, and the first triangle groups

    // other triangle groups
    for (int i = 0; i < mTriangleDrawBufferArray.size(); i++)
    {
        auto res = std::make_shared<cRenderResource>();
        res->mName = this->mObjName + "_resource" + std::to_string(i);

        res->mTriangleBuffer.mBuffer = mTriangleDrawBufferArray[i].data();
        res->mTriangleBuffer.mNumOfEle = mTriangleDrawBufferArray[i].size();
        res->mLineBuffer.mBuffer = this->mEdgeBuffer.data();
        res->mLineBuffer.mNumOfEle = mEdgeBuffer.size();
        res->mPointBuffer.mBuffer = mPointDrawBuffer.data();
        res->mPointBuffer.mNumOfEle = mPointDrawBuffer.size();

        res->mMaterialPtr = mMatInfoArray[i];
        mRenderResource.push_back(res);
    }
}

std::vector<cRenderResourcePtr> cBaseObject::GetRenderingResource() const
{
    return mRenderResource;
}

const std::vector<tVertexPtr> &cBaseObject::GetCollisionVertexArray() const
{
    return GetVertexArray();
}
const std::vector<tTrianglePtr> &cBaseObject::GetCollisionTriangleArray() const
{
    return GetTriangleArray();
}