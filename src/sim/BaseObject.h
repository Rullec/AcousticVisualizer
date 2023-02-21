#pragma once
#include "utils/DefUtil.h"
#include "utils/EigenUtil.h"
#include <memory>
#include <string>

/**
 * \brief           The ULTIMATE object type collections
 */
enum eObjectType
{
    YARN_TYPE = 0,
    KINEMATICBODY_TYPE,
    ACOUSTICBODY_TYPE,
    ACOUSTICMANAGER_TYPE,
    SNISR_DEBUG_DRAW_TYPE,
    NUM_OBJ_TYPES,
    INVALID_OBJ_TYPE
};

/**
 * \brief           base object class
 *
 */
namespace Json
{
class Value;
};

struct tVertex;
struct tEdge;
struct tTriangle;
struct tPerturb;
SIM_DECLARE_PTR(tVertex);
SIM_DECLARE_PTR(tEdge);
SIM_DECLARE_PTR(tTriangle);
SIM_DECLARE_STRUCT_AND_PTR(tPointTriangleCollisionInfo);
SIM_DECLARE_STRUCT_AND_PTR(tEdgeEdgeCollisionInfo);
SIM_DECLARE_STRUCT_AND_PTR(tMeshMaterialInfo);
SIM_DECLARE_CLASS_AND_PTR(cRenderResource);
class cBaseObject
{
public:
    inline static const std::string OBJECT_NAME_KEY = "object_name";
    explicit cBaseObject(eObjectType type, int obj_id);
    virtual ~cBaseObject();
    virtual int GetObjId() const;
    virtual void SetObjName(std::string);
    virtual std::string GetObjName() const;
    virtual void Init(const Json::Value &conf);
    static eObjectType BuildObjectType(std::string type);
    eObjectType GetObjectType() const;
    virtual void UpdateRenderingResource(bool enable_edge = true);
    virtual std::vector<cRenderResourcePtr> GetRenderingResource() const;
    virtual void Update(_FLOAT dt) = 0;
    virtual void ApplyUserPerturbForceOnce(tPerturb *) = 0;
    virtual void SetGravity(const tVector3 &g);
    // triangularize methods to visit the mesh data
    virtual int GetNumOfTriangles() const;
    virtual int GetNumOfEdges() const;
    virtual int GetNumOfVertices() const;
    void SetColorAlpha(_FLOAT val);
    _FLOAT GetColorAlpha() const;

    virtual const std::vector<tVertexPtr> &GetVertexArray() const;
    virtual const std::vector<tEdgePtr> &GetEdgeArray() const;
    virtual const std::vector<tTrianglePtr> &GetTriangleArray() const;

    virtual const std::vector<tVertexPtr> &GetCollisionVertexArray() const;
    virtual const std::vector<tTrianglePtr> &GetCollisionTriangleArray() const;

    virtual std::vector<tVertexPtr> &GetVertexArrayRef();
    virtual std::vector<tEdgePtr> &GetEdgeArrayRef();
    virtual std::vector<tTrianglePtr> &GetTriangleArrayRef();
    void SetPointTriangleCollisionInfo(
        const std::vector<tPointTriangleCollisionInfoPtr> &info);
    void SetEdgeEdgeCollisionInfo(
        const std::vector<tEdgeEdgeCollisionInfoPtr> &info);
    void ChangeTriangleColor(int tri_id, const tVector3 &color);
    void ChangeTrianglesColor(const tVector3 &color);
    void ChangeVertexColor(int vid, const tVector3 &color);
    void ChangeVerticesColor(const tVector3 &color);
    virtual void CalcAABB(tVector3 &min, tVector3 &max) const;
    _FLOAT CalcTotalArea() const;
    virtual void UpdateImGui();
    virtual void Reset();

protected:
    _FLOAT mColorAlpha = 1.0;
    int mObjId;
    std::string mObjName;
    eObjectType mType;
    tVector3 mGravity;
    bool mEnableDrawBuffer; // enable to open draw buffer
    // std::vector<tVertexPtr > mVertexArray;
    // std::vector<tEdge *> mEdgeArray;
    // std::vector<tTriangle *> mTriangleArray;
    tVectorXf mEdgeBuffer, mPointDrawBuffer;

    tEigenArr<tVectorXf>
        mTriangleDrawBufferArray; // different triangle may belongs to different
                                  // texture, and each texture must be rendered
                                  // in seperated resource. so, we use Array to
                                  // store the draw buffer, each entry for a
                                  // single materials
    std::vector<tMeshMaterialInfoPtr> mMatInfoArray; //

    std::vector<cRenderResourcePtr> mRenderResource;

    std::vector<float> mTriangleInitArea;
    std::vector<std::vector<int>> mVertexConnectedTriangles;
    std::vector<tPointTriangleCollisionInfoPtr> mPointTriangleCollisionInfo;
    std::vector<tEdgeEdgeCollisionInfoPtr> mEdgeEdgeCollisionInfo;
    std::vector<tVertexPtr> mVertexArray;
    std::vector<tEdgePtr> mEdgeArray;
    std::vector<tTrianglePtr> mTriangleArray;
    bool mEnableTextureUV;
    virtual void UpdateTriangleNormal();
    virtual void UpdateVertexNormalFromTriangleNormal();
    virtual void CalcTriangleInitArea();
    virtual void UpdateRenderResourceTriangles();
    virtual void UpdateRenderResourceLines();
    virtual void UpdateRenderResourcePoints();
};

SIM_DECLARE_PTR(cBaseObject);