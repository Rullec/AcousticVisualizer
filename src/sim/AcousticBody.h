#pragma once
#include "sim/BaseObject.h"
#define NUM_THREADS 100
struct tAcousticMaterialProp
{
    tAcousticMaterialProp();
    tVector3 GetVector() const;
    void SetVector(const tVector3 &) const;

    _FLOAT mRho, mA, mB;
};

struct tClickInfo
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    std::vector<int> mTriIds;
    std::vector<_FLOAT> mAmp;
    tEigenArr<tVector3> mNormal;

    _FLOAT mTS;
    _FLOAT mAudioAmp;
    tClickInfo();
};
class ModalModel;
#include "io/TglMeshReader.hpp"
class FMMTransferEval;
typedef Eigen::Matrix<float, 3, 1> tVector3f;
SIM_DECLARE_STRUCT_AND_PTR(tDiscretedWave);
class cAcousticBody : public cBaseObject
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    cAcousticBody(int id_);

    virtual ~cAcousticBody();
    virtual void Init(const Json::Value &conf) override;
    virtual void InitFromIni(std::string ini_path, bool enable_start_sound);
    virtual void ApplyUserPerturbForceOnce(tPerturb *) override;
    virtual void Update(_FLOAT dt) override;
    virtual void UpdateImGui() override;
    virtual void Shift(const tVector3 &pos);
    virtual std::string GetIniPath() const;
    tVectorXf GetVertexPosVec();
    tVectorXi GetTriIdVec();
    virtual void UpdateCamPos(const tVector3f &pos);
    virtual tVector3f GetCamPos() const;
    virtual void ClickTriangle(int tid, _FLOAT outside_scale,
                               _FLOAT overdamp_scale);

protected:
    std::string mIniPath, mSurfaceObjPath, mMomentsPath, mEigenPath, mVmapPath;

    float mCustomDampVectorScale;
    tVector3 mCamPos;
    tAcousticMaterialProp mAcousticProp;
    tClickInfo mClickInfo;

    tEigenArr<tVector3> mTrianglesCOM;
    virtual void InitAudioBuffer();
    int mNumOfFixed;
    ModalModel *mModalModel;
    std::vector<double> mForce_[NUM_THREADS];
    std::vector<int> mVertexMapSurfaceToTet;
    std::vector<FMMTransferEval *> transfer_;
    TriangleMesh<double> *mesh_;
    int running_threads = 0;
    std::vector<double> soundBuffer_[NUM_THREADS];
    virtual void AudioSynthesis(bool enable_outsider_amp = false,
                                _FLOAT outside_amp = 1.0);
    virtual void InitAudioGeo();
    void single_channel_synthesis(const Tuple3ui &tri, const Vector3d &dir,
                                  const Point3d &cam, float amplitude,
                                  int index_t);
    void run_thread(double collision_time, int selTriId, Vector3d nml,
                    Point3d CamPos, float amp, int index_t);
    std::vector<double> whole_soundBuffer;
    tDiscretedWavePtr mSynthesisAudio;
    bool mEnableRangeClick;
    float mRangeClickRadius;
    bool mEnableAudioScale;
};