#pragma once
#include "utils/DefUtil.h"
#include "utils/EigenUtil.h"

SIM_DECLARE_STRUCT_AND_PTR(tVertex);
SIM_DECLARE_STRUCT_AND_PTR(tTriangle);
SIM_DECLARE_STRUCT_AND_PTR(tMeshMaterialInfo);
struct tCPURenderBuffer
{
    tCPURenderBuffer();
    const float *mBuffer;
    uint mNumOfEle;
};

class cRenderResource : std::enable_shared_from_this<cRenderResource>
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    cRenderResource();
    std::string mName;
    tCPURenderBuffer mTriangleBuffer, mLineBuffer, mPointBuffer;
    tMeshMaterialInfoPtr mMaterialPtr;
};

SIM_DECLARE_PTR(cRenderResource);

class cRenderUtil
{
public:
    static void
    CalcTriangleDrawBufferSingle(const std::vector<tVertexPtr> &v_array,
                                 tTrianglePtr tri_ptr, const tVector4 &color,
                                 Eigen::Map<tVectorXf> &buffer, int &st_pos,
                                 bool enable_uv = false);

    static void CalcTriangleDrawBufferSingle(
        const tVector4 &v0, const tVector4 &v1, const tVector4 &v2,
        const tVector4 &n0, const tVector4 &n1, const tVector4 &n2,
        const tVector2 &uv0, const tVector2 &uv1, const tVector2 &uv2,
        const tVector4 &color0, const tVector4 &color1, const tVector4 &color2,
        Eigen::Map<tVectorXf> &buffer, int &st_pos, bool enable_uv = false);
    static void CalcEdgeDrawBufferSingle(tVertexPtr v0, tVertexPtr v1,
                                         const tVector4 &edge_normal,
                                         Eigen::Map<tVectorXf> &buffer,
                                         int &st_pos, const tVector4 &color,
                                         _FLOAT edge_amp = 1e-6);

    static void CalcEdgeDrawBufferSingle(const tVector4 &v0, const tVector4 &v1,
                                         const tVector4 &edge_normal,
                                         Eigen::Map<tVectorXf> &buffer,
                                         int &st_pos, const tVector4 &color,
                                         _FLOAT edge_amp = 1e-6);

    static void CalcPointDrawBufferSingle(const tVector4 &pos,
                                          const tVector4 &color,
                                          Eigen::Map<tVectorXf> &buffer,
                                          int &st_pos);
    static cRenderResourcePtr GetAxesRenderingResource();
    static cRenderResourcePtr
    GetGroundRenderingResource(_FLOAT ground_scale = 1e3, _FLOAT height = 0.0f,
                               std::string tex_path = "");
};

typedef std::vector<cRenderResourcePtr> cRenderResourcePtrArray;


struct tMeshMaterialInfo
{
    tMeshMaterialInfo();
    static bool MaterialComp(const tMeshMaterialInfoPtr &ptr0,
                             const tMeshMaterialInfoPtr &ptr1);
    static bool IsSame(const tMeshMaterialInfoPtr &ptr0,
                       const tMeshMaterialInfoPtr &ptr1);
    std::string mTexImgPath;
    std::string mName;
    std::vector<int> mTriIdArray;
    /*
    Ka: ambient color
    Kd: diffuse color
    Ks: specular color
    Ns: specular exponent
    */
    Eigen::Vector3f Ka, Kd, Ks;
    float Ns;
    bool mEnableTexutre;
    // confirm an array of mesh material can fully cover all triangles
    static bool
    ValidateMaterialInfo(const std::vector<tMeshMaterialInfoPtr> &mat_info,
                         int num_of_tris);
    static tMeshMaterialInfoPtr GetDefaultMaterialInfo();
};