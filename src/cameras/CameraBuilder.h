#pragma once
class CameraBase;
#include <memory>
namespace Json
{
class Value;
};
class cCameraBuilder
{
public:
    static CameraBase *BuildCamera(const Json::Value &camera_json);
};