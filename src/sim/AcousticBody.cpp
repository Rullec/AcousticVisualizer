#include "AcousticBody.h"
#include "INIReader.h"
#include "utils/FileUtil.h"
#include "utils/JsonUtil.h"
tAcousticMaterialProp::tAcousticMaterialProp()
{
    mRho = -1;
    mA = -1;
    mB = -1;
}

tClickInfo::tClickInfo()
{
    mVid.clear();
    mAmp.clear();
    mNormal.clear();
    mTS = -1;
    mAudioAmp = -1;
}

cAcousticBody::cAcousticBody(int id)
    : cBaseObject(eObjectType::ACOUSTICBODY_TYPE, id)
{
}
cAcousticBody::~cAcousticBody()
{
    
}
void cAcousticBody::Init(const Json::Value &conf)
{
    // 1. get ini path
    mIniPath = cJsonUtil::ParseAsString("ini_path", conf);
    if (cFileUtil::ExistsFile(mIniPath) == false)
    {
        SIM_ERROR("ini %s doesn't exist", mIniPath.c_str());
    }

    // 2. load ini path
    INIReader reader(mIniPath);

    if (reader.ParseError() != 0)
    {
        SIM_ERROR("cannot parse ini %s", mIniPath);
    }

    mSurfaceObjPath = reader.Get("mesh", "surface_mesh", "INVALID_SURFACE_OBJ");

    mClickInfo.mTS = reader.GetFloat("audio", "TS", -1);
    mClickInfo.mAudioAmp = reader.GetFloat("audio", "amplitude", -1);

    mMomentsPath = reader.Get("transfer", "moments", "INVALID_MOMENT");

    mEigenPath = reader.Get("modal", "shape", "INVALID_EV");

    mAcousticProp.mRho = reader.GetFloat("modal", "density", -1);
    mAcousticProp.mA = reader.GetFloat("modal", "alpha", -1);
    mAcousticProp.mB = reader.GetFloat("modal", "beta", -1);

    mVmapPath = reader.Get("modal", "vtx_map", "INVALID_VMAP");

    mClickInfo.mVid = {
        static_cast<int>(reader.GetInteger("collisions", "ID", -1))};

    mClickInfo.mAmp = {reader.GetFloat("collisions", "amplitude", -1)};

    ;
    mClickInfo.mNormal = {
        tVector3(reader.GetFloat("collisions", "norm1", -1),
                  reader.GetFloat("collisions", "norm2", -1),
                  reader.GetFloat("collisions", "norm3", -1))};

    mCamPos = tVector3(reader.GetFloat("camera", "x", -1),
                        reader.GetFloat("camera", "y", -1),
                        reader.GetFloat("camera", "z", -1));
}

void cAcousticBody::Update(FLOAT dt) {}