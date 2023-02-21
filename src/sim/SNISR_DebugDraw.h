#pragma once
#include "sim/BaseObject.h"
#include <map>

SIM_DECLARE_CLASS_AND_PTR(cRenderResource);
typedef Eigen::Matrix<bool, Eigen::Dynamic, 1> tVectorXb;
class cSNISRDebugDrawBall : public cBaseObject
{
public:
    cSNISRDebugDrawBall(int obj_id);
    virtual void Init(const Json::Value &conf);
    virtual void Update(_FLOAT dt) override{};
    virtual void ApplyUserPerturbForceOnce(tPerturb *) override{};
    virtual std::vector<cRenderResourcePtr>
    GetRenderingResource() const override;
    virtual void UpdateImGui() override;
    virtual void UpdateRenderingResource(bool enable_edge) override;
protected:
    std::string mPosInfoPath;
    typedef tEigenArr<tMatrixXf> tPtData;
    std::vector<int> mOffset2LabelId;
    std::vector<std::string> mOffset2LabelName;
    std::map<int, int> mLabel2OffsetId;

    typedef std::vector<cRenderResourcePtr> tRenderResourcePerLabel;
    std::vector<tPtData> mOffsetToPoints;  // label to point pos
    std::vector<tRenderResourcePerLabel> mOffsetToRenderResource;    // label to resource
    tVectorXb mEnableDrawPts;

    virtual void InitRenderResource();
};