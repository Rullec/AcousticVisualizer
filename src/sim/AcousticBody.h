#pragma once
#include "sim/BaseObject.h"

struct tAcousticMaterialProp
{
    tAcousticMaterialProp();
    tVector3 GetVector() const;
    void SetVector(const tVector3 &) const;

    float mRho, mA, mB;
};

struct tClickInfo
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    std::vector<int> mVid;
    std::vector<float> mAmp;
    tEigenArr<tVector3> mNormal;

    float mTS;
    float mAudioAmp;
    tClickInfo();
};

class cAcousticBody : public cBaseObject
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    cAcousticBody(int id_);

    virtual ~cAcousticBody();
    virtual void Init(const Json::Value &conf) override;
    virtual void ApplyUserPerturbForceOnce(tPerturb *) override{};
    virtual void Update(FLOAT dt) override;

protected:
    std::string mIniPath, mSurfaceObjPath, mMomentsPath, mEigenPath, mVmapPath;

    tVector3 mCamPos;
    tAcousticMaterialProp mAcousticProp;
    tClickInfo mClickInfo;
};