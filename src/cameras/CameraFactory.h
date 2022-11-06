#pragma once

#include "utils/DefUtil.h"
#include "utils/JsonUtil.h"
#include <memory>

SIM_DECLARE_CLASS_AND_PTR(CameraBase);
class cCameraFactory
{
public:
    static CameraBasePtr getInstance();
    static void ChangeCamera(const Json::Value &camera_json);
    static void DestroyInstance(CameraBase *);

protected:

    static CameraBasePtr instance;
};