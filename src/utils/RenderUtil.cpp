#include "RenderUtil.h"
#include "geometries/Primitives.h"
#include "utils/ColorUtil.h"
#include "utils/LogUtil.h"
#include <iostream>
cRenderResource::cRenderResource()
{
    mName = "";
    mMaterialPtr = nullptr;
}

tCPURenderBuffer::tCPURenderBuffer()
{
    mBuffer = nullptr;
    mNumOfEle = 0;
}
void cRenderUtil::CalcTriangleDrawBufferSingle(
    const std::vector<tVertexPtr> &v_array, tTrianglePtr tri_ptr,
    const tVector4 &color, Eigen::Map<tVectorXf> &buffer, int &st_pos,
    bool enable_uv)
{
    if (tri_ptr == nullptr)
        return;

    cRenderUtil::CalcTriangleDrawBufferSingle(
        v_array[tri_ptr->mId0]->mPos, v_array[tri_ptr->mId1]->mPos,
        v_array[tri_ptr->mId2]->mPos,

        v_array[tri_ptr->mId0]->mNormal, v_array[tri_ptr->mId1]->mNormal,
        v_array[tri_ptr->mId2]->mNormal,

        tri_ptr->muv[0], tri_ptr->muv[1], tri_ptr->muv[2],

        v_array[tri_ptr->mId0]->mColor, v_array[tri_ptr->mId1]->mColor,
        v_array[tri_ptr->mId2]->mColor, buffer, st_pos, enable_uv);
}

void cRenderUtil::CalcEdgeDrawBufferSingle(tVertexPtr v0, tVertexPtr v1,
                                           const tVector4 &edge_normal,
                                           Eigen::Map<tVectorXf> &buffer,
                                           int &st_pos, const tVector4 &color,
                                           _FLOAT edge_amp)
{
    cRenderUtil::CalcEdgeDrawBufferSingle(v0->mPos, v1->mPos, edge_normal,
                                          buffer, st_pos, color, edge_amp);
}

void cRenderUtil::CalcEdgeDrawBufferSingle(const tVector4 &v0,
                                           const tVector4 &v1,
                                           const tVector4 &edge_normal,
                                           Eigen::Map<tVectorXf> &buffer,
                                           int &st_pos, const tVector4 &color,
                                           _FLOAT float_edge)
{
    tVector3 bias_amp = float_edge * edge_normal.segment(0, 3); // 0.1 mm

    // pos, color, normal
    buffer.segment(st_pos, 3) = (v0.segment(0, 3) + bias_amp).cast<float>();
    buffer.segment(st_pos + 3, 4) = color.cast<float>();
    buffer.segment(st_pos + 7, 3) = tVector3(0, 0, 0).cast<float>();
    st_pos += RENDERING_SIZE_PER_VERTICE;

    buffer.segment(st_pos, 3) = (v1.segment(0, 3) + bias_amp).cast<float>();
    buffer.segment(st_pos + 3, 4) = color.cast<float>();
    buffer.segment(st_pos + 7, 3) = tVector3(0, 0, 0).cast<float>();
    st_pos += RENDERING_SIZE_PER_VERTICE;
}
void cRenderUtil::CalcPointDrawBufferSingle(const tVector4 &v_pos,
                                            const tVector4 &v_color,
                                            Eigen::Map<tVectorXf> &buffer,
                                            int &st_pos)
{
    buffer.segment(st_pos, 3) = v_pos.segment(0, 3).cast<float>();
    buffer.segment(st_pos + 3, 4) = v_color.cast<float>();
    st_pos += RENDERING_SIZE_PER_VERTICE;
}

static tVectorXf gAxesData, gGroundData;
cRenderResourcePtr cRenderUtil::GetAxesRenderingResource()
{
    auto res = std::make_shared<cRenderResource>();
    res->mName = "axes";
    gAxesData.noalias() = tVectorXf::Zero(6 * RENDERING_SIZE_PER_VERTICE);
    res->mMaterialPtr = tMeshMaterialInfo::GetDefaultMaterialInfo();

    int idx = 0;
    for (int i = 0; i < 3; i++)
    {
        /*
        1. pos R3
        2. color R4 (Red, Blue, Green)
        3. normal R3 (zero vec)
        4. uv R2 (nan)
        */

        tVector3 st_pos = tVector3::Zero();
        tVector3 end_pos = tVector3::Zero();
        end_pos[i] = 100.0;

        tVector4 color = tVector4::Zero();
        color[3] = 1.0;
        color[i] = 1.0;

        tVector2 uv = tVector2::Ones() * std::nanf("");

        // start point

        for (int j = 0; j < 2; j++)
        {
            // pos
            gAxesData.segment(idx, 3) =
                (j == 0 ? st_pos : end_pos).cast<float>();
            idx += 3;
            // color
            gAxesData.segment(idx, 4) = color.cast<float>();
            idx += 4;
            // normal
            gAxesData.segment(idx, 3).setZero();
            idx += 3;
            // uv
            gAxesData.segment(idx, 2) = uv.cast<float>();
            idx += 2;
        }
        // end point
    }
    res->mLineBuffer.mNumOfEle = gAxesData.size();
    res->mLineBuffer.mBuffer = gAxesData.data();
    return res;
}

void cRenderUtil::CalcTriangleDrawBufferSingle(
    const tVector4 &v0, const tVector4 &v1, const tVector4 &v2,
    const tVector4 &n0, const tVector4 &n1, const tVector4 &n2,
    const tVector2 &uv0, const tVector2 &uv1, const tVector2 &uv2,
    const tVector4 &color0, const tVector4 &color1, const tVector4 &color2,
    Eigen::Map<tVectorXf> &buffer, int &st_pos, bool enable_uv)
{
    buffer.segment(st_pos, 3) = v0.segment(0, 3).cast<float>();
    buffer.segment(st_pos + 3, 4) = color0.cast<float>();
    // buffer[st_pos + 6] = 0.5;
    buffer.segment(st_pos + 7, 3) = n0.segment(0, 3).cast<float>();
    if (enable_uv)
        buffer.segment(st_pos + 10, 2) = uv0.cast<float>();
    else
        buffer.segment(st_pos + 10, 2).setZero();
    st_pos += RENDERING_SIZE_PER_VERTICE;

    buffer.segment(st_pos, 3) = v1.segment(0, 3).cast<float>();
    buffer.segment(st_pos + 3, 4) = color1.cast<float>();
    // buffer[st_pos + 6] = 0.5;
    buffer.segment(st_pos + 7, 3) = n1.segment(0, 3).cast<float>();
    if (enable_uv)
        buffer.segment(st_pos + 10, 2) = uv1.cast<float>();
    else
        buffer.segment(st_pos + 10, 2).setZero();
    st_pos += RENDERING_SIZE_PER_VERTICE;

    buffer.segment(st_pos, 3) = v2.segment(0, 3).cast<float>();
    buffer.segment(st_pos + 3, 4) = color2.cast<float>();
    buffer.segment(st_pos + 7, 3) = n2.segment(0, 3).cast<float>();
    if (enable_uv)
        buffer.segment(st_pos + 10, 2) = uv2.cast<float>();
    else
        buffer.segment(st_pos + 10, 2).setZero();
    // buffer[st_pos + 6] = 0.5;
    st_pos += RENDERING_SIZE_PER_VERTICE;
}

static const float ground_scale = 1000.0;

using tVector4f = Eigen::Matrix<float, 4, 1>;
using tVector3f = Eigen::Matrix<float, 3, 1>;
using tVector2f = Eigen::Matrix<float, 2, 1>;
cRenderResourcePtr cRenderUtil::GetGroundRenderingResource(_FLOAT ground_scale,
                                                           _FLOAT height,
                                                           std::string tex_path)
{
    // pos
    const tEigenArr<tVector3f> pos_array = {
        tVector3f(50.0f, height, -50.0f), tVector3f(-50.0f, height, -50.0f),
        tVector3f(-50.0f, height, 50.0f), tVector3f(50.0f, height, -50.0f),
        tVector3f(-50.0f, height, 50.0f), tVector3f(50.0f, height, 50.0f)};
    // color
    tVector4f color = tVector4f(0.7f, 0.7f, 0.7f, 1.0f);
    // normal
    tVector3f normal(0.0f, 1.0f, 0.0f);

    // uv
    const tEigenArr<tVector2f> uv_array = {
        tVector2f(ground_scale, 0.0f), tVector2f(0.0f, 0.0f),
        tVector2f(0.0f, ground_scale), tVector2f(ground_scale, 0.0f),
        tVector2f(0.0f, ground_scale), tVector2f(ground_scale, ground_scale)};
    gGroundData.resize(RENDERING_SIZE_PER_VERTICE * 6);
    for (int i = 0; i < 6; i++)
    {
        gGroundData.segment(RENDERING_SIZE_PER_VERTICE * i, 3) = pos_array[i];
        gGroundData.segment(RENDERING_SIZE_PER_VERTICE * i + 3, 4) = color;
        gGroundData.segment(RENDERING_SIZE_PER_VERTICE * i + 7, 3) = normal;
        gGroundData.segment(RENDERING_SIZE_PER_VERTICE * i + 10, 2) =
            uv_array[i];
    }
    auto res = std::make_shared<cRenderResource>();
    res->mTriangleBuffer.mBuffer = gGroundData.data();
    res->mTriangleBuffer.mNumOfEle = gGroundData.size();
    res->mMaterialPtr = std::make_shared<tMeshMaterialInfo>();
    res->mMaterialPtr->mTexImgPath = tex_path;
    res->mMaterialPtr->mEnableTexutre = true;
    return res;
}

#include "utils/LogUtil.h"
#include <iostream>
bool comp_resource(const cRenderResourcePtr &res1,
                   const cRenderResourcePtr &res2)
{
    SIM_ASSERT(res1 != nullptr && res2 != nullptr);
    return tMeshMaterialInfo::MaterialComp(res1->mMaterialPtr,
                                           res2->mMaterialPtr);
};
std::string GetTextureImgPathFromMaterial(tMeshMaterialInfoPtr mat)
{
    if (mat == nullptr)
        return "";
    else
        return mat->mTexImgPath;
}
bool IsEqual(const tVector3f &vec0, const tVector3f &vec1)
{
    return (vec0[0] == vec1[0]) && (vec0[1] == vec1[1]) && (vec0[2] == vec1[2]);
}
bool Vec3Comp(const tVector3f &vec0, const tVector3f &vec1)
{
    for (int i = 0; i < 3; i++)
    {
        if (vec0[i] != vec1[i])
        {
            return vec0[i] < vec1[i];
        }
    }
    return true;
}
bool tMeshMaterialInfo::IsSame(const tMeshMaterialInfoPtr &ptr0,
                               const tMeshMaterialInfoPtr &ptr1)
{
    return (ptr0 == ptr1) && (ptr0->mTexImgPath == ptr1->mTexImgPath) &&
           IsEqual(ptr0->Ka, ptr1->Ka) && IsEqual(ptr0->Kd, ptr1->Kd) &&
           IsEqual(ptr0->Ks, ptr1->Ks) && (ptr0->Ns == ptr1->Ns);
}
bool tMeshMaterialInfo::MaterialComp(const tMeshMaterialInfoPtr &ptr0,
                                     const tMeshMaterialInfoPtr &ptr1)
{
    if (ptr0 == nullptr || ptr1 == nullptr)
    {
        return ptr0 < ptr1;
    }
    if (ptr0->mTexImgPath != ptr1->mTexImgPath)
    {
        return ptr0->mTexImgPath < ptr1->mTexImgPath;
    }
    else
    {
        if (false == IsEqual(ptr0->Ka, ptr1->Ka))
        {
            return Vec3Comp(ptr0->Ka, ptr1->Ka);
        }
        else
        {
            if (false == IsEqual(ptr0->Kd, ptr1->Kd))
            {
                return Vec3Comp(ptr0->Kd, ptr1->Kd);
            }
            else
            {
                if (false == IsEqual(ptr0->Ks, ptr1->Ks))
                {
                    return Vec3Comp(ptr0->Ks, ptr1->Ks);
                }
                else
                {
                    if (ptr0->Ns != ptr1->Ns)
                    {
                        return ptr0->Ns < ptr1->Ns;
                    }
                    else
                    {
                        return ptr0 < ptr1;
                    }
                }
            }
        }
    }
}

bool tMeshMaterialInfo::ValidateMaterialInfo(
    const std::vector<tMeshMaterialInfoPtr> &mat_info, int num_of_tris)
{
    printf("------begin to validate material info------\n");
    tVectorXi info(num_of_tris);
    int INVLAID_ID = -1;
    info.setConstant(INVLAID_ID);
    int all_tris = 0;
    int mat_id = 0;
    for (auto &mat : mat_info)
    {

        all_tris += mat->mTriIdArray.size();
        for (auto &id : mat->mTriIdArray)
        {
            if (id < 0 || id >= num_of_tris)
            {
                SIM_WARN("check failed: tri {} is not inside [0, {}]", id,
                         num_of_tris - 1);
                return false;
            }
            if (info[id] != INVLAID_ID)
            {
                SIM_WARN("check failed: tri {} has two material {} {}", id,
                         info[id], mat_id);
                return false;
            }
            info[id] = true;
        }
        mat_id += 1;
    }
    if (all_tris != num_of_tris)
    {
        SIM_WARN("check failed: num of tris illegal {} != {}", all_tris,
                 num_of_tris);
        return false;
    }

    printf("------finished to validate material info------\n");
    return true;
}

tMeshMaterialInfo::tMeshMaterialInfo()
{
    mTexImgPath = "";
    mTriIdArray.clear();

    mTexImgPath = "";
    mTriIdArray.clear();
    Ka = Eigen::Vector3f(0, 0, 0);
    Kd = ColorBlue.segment(0, 3).cast<float>();
    Ks = Eigen::Vector3f(1, 1, 1);
    Ns = 64;
    mEnableTexutre = false;
}

tMeshMaterialInfoPtr tMeshMaterialInfo::GetDefaultMaterialInfo()
{
    tMeshMaterialInfoPtr mat = std::make_shared<tMeshMaterialInfo>();
    return mat;
}