#pragma once
#include "sim/BaseObject.h"
#define NUM_THREADS 100
struct tAcousticMaterialProp
{
    tAcousticMaterialProp();
    tVector3 GetVector() const;
    void SetVector(const tVector3 &) const;

    FLOAT mRho, mA, mB;
};

struct tClickInfo
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    std::vector<int> mTriIds;
    std::vector<FLOAT> mAmp;
    tEigenArr<tVector3> mNormal;

    FLOAT mTS;
    FLOAT mAudioAmp;
    tClickInfo();
};
class ModalModel;
#include "io/TglMeshReader.hpp"
class FMMTransferEval;
class cAcousticBody : public cBaseObject
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    cAcousticBody(int id_);

    virtual ~cAcousticBody();
    virtual void Init(const Json::Value &conf) override;
    virtual void ApplyUserPerturbForceOnce(tPerturb *) override;
    virtual void Update(FLOAT dt) override;
    virtual void UpdateImGui() override;

protected:
    std::string mIniPath, mSurfaceObjPath, mMomentsPath, mEigenPath, mVmapPath;

    tVector3 mCamPos;
    tAcousticMaterialProp mAcousticProp;
    tClickInfo mClickInfo;

    virtual void InitAudioBuffer();
    int mNumOfFixed;
    ModalModel *mModalModel;
    std::vector<double> mForce_[NUM_THREADS];
    std::vector<int> mVertexMapSurfaceToTet;
    std::vector<FMMTransferEval *> transfer_;
    TriangleMesh<double> *mesh_;
    int running_threads = 0;
    std::vector<double> soundBuffer_[NUM_THREADS];
    virtual void AudioSynthesis(bool enable_scale = true);
    virtual void InitAudioGeo();
    void single_channel_synthesis(const Tuple3ui &tri, const Vector3d &dir,
                                  const Point3d &cam, float amplitude,
                                  int index_t);
    void run_thread(double collision_time, int selTriId, Vector3d nml,
                    Point3d CamPos, float amp, int index_t);
    std::vector<double> whole_soundBuffer;
};