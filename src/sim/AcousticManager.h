#pragma once
#include "AcousticManager.h"
#include "sim/BaseObject.h"

struct tAcousticIniData
{
    std::string type_str;
    std::string type_id;
    std::string obj_id;
    std::string ini_path;
    std::string surface_mesh_path;
};
typedef std::vector<tAcousticIniData> tDataVec;
SIM_DECLARE_CLASS_AND_PTR(cAcousticBody);
SIM_DECLARE_CLASS_AND_PTR(cKinematicBody);
class cAcousticManager : public cBaseObject
{
public:
    cAcousticManager(int id_);
    virtual void Init(const Json::Value &conf) override;
    virtual void ApplyUserPerturbForceOnce(tPerturb *) override;
    virtual void Update(_FLOAT dt) override;
    virtual void UpdateImGui() override;
    virtual void UpdateRenderingResource(bool enable_edge = true);
    virtual std::vector<cRenderResourcePtr> GetRenderingResource() const;

    virtual const std::vector<tVertexPtr> &GetVertexArray() const;
    virtual const std::vector<tEdgePtr> &GetEdgeArray() const;
    virtual const std::vector<tTrianglePtr> &GetTriangleArray() const;
    virtual void ChangeTriangleColor(int tri_id, const tVector3 &color);

    virtual const std::vector<tVertexPtr> &
    GetCollisionVertexArray() const override;
    virtual const std::vector<tTrianglePtr> &
    GetCollisionTriangleArray() const override;

protected:
    struct tSelInfo
    {
        int select_type_id;
        int select_data_id;
    };
    tSelInfo mCurSel;
    cAcousticBodyPtr mAcousticBody;
    cKinematicBodyPtr mSurf;
    std::vector<tDataVec> mDataArray;
    virtual void Reload();
};