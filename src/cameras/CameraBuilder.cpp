#include "CameraBuilder.h"
#include "ArcBallCamera.h"
#include "FPSCamera.h"
#include "OrthoCamera.h"
#include "utils/DefUtil.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"
#include "utils/json/json.h"
#include <iostream>


CameraBase *cCameraBuilder::BuildCamera(const Json::Value &camera_json)
{
    CameraBase *cam = nullptr;
    std::string type_str = cJsonUtil::ParseAsString("camera_type", camera_json);
    Json::Value camera_pos_json =
        cJsonUtil::ParseAsValue("camera_pos", camera_json);
    Json::Value camera_focus_json =
        cJsonUtil::ParseAsValue("camera_focus", camera_json);
    Json::Value camera_up_json =
        cJsonUtil::ParseAsValue("camera_up", camera_json);
    SIM_ASSERT(camera_pos_json.size() == 3);
    SIM_ASSERT(camera_focus_json.size() == 3);
    tVector3 mCameraInitFocus =
        tVector3(camera_focus_json[0].asFloat(), camera_focus_json[1].asFloat(),
                 camera_focus_json[2].asFloat());
    tVector3 mCameraInitPos =
        tVector3(camera_pos_json[0].asFloat(), camera_pos_json[1].asFloat(),
                 camera_pos_json[2].asFloat());
    tVector3 mCameraInitUp =
        tVector3(camera_up_json[0].asFloat(), camera_up_json[1].asFloat(),
                 camera_up_json[2].asFloat());
    mCameraInitUp = mCameraInitUp.normalized();

    _FLOAT near_plane_dist = cJsonUtil::ParseAsFloat("near", camera_json);
    _FLOAT far_plane_dist = cJsonUtil::ParseAsFloat("far", camera_json);
    _FLOAT mCameraInitFov = 0;
    if (camera_json.isMember("fov") == true)
    {
        mCameraInitFov = cJsonUtil::ParseAsFloat("fov", camera_json);
    }

    // cam = new cArcBallCamera(mCameraInitPos, mCameraInitFocus,
    //                          tVector3(0, 1, 0), mCameraInitFov);
    eCameraType type = CameraBase::BuildCameraTypeFromStr(type_str);
    switch (type)
    {
    case eCameraType::FPS_CAMERA:
    {
        cam = new FPSCamera(mCameraInitPos, mCameraInitFocus, mCameraInitUp,
                            mCameraInitFov, near_plane_dist, far_plane_dist);

        break;
    }
    case eCameraType::ARCBALL_CAMERA:
    {
        cam =
            new cArcBallCamera(mCameraInitPos, mCameraInitFocus, mCameraInitUp,
                               mCameraInitFov, near_plane_dist, far_plane_dist);
        break;
    }
    case eCameraType::ORTHO_CAMERA:
    {

        _FLOAT init_box =
            cJsonUtil::ParseAsFloat("camera_init_box", camera_json);
        cam = new cOrthoCamera(mCameraInitPos, mCameraInitFocus, mCameraInitUp,
                               init_box, near_plane_dist, far_plane_dist);
        break;
    }
    default:
        SIM_ERROR("unsupported camera type {}", type_str);
        exit(1);
        break;
    }
    return cam;
}