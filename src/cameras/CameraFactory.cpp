#include "CameraFactory.h"
#include "ArcBallCamera.h"
#include "FPSCamera.h"
#include "OrthoCamera.h"
#include <iostream>

CameraBasePtr cCameraFactory::instance(new cArcBallCamera(),
                                       cCameraFactory::DestroyInstance);

CameraBasePtr cCameraFactory::getInstance() { return instance; }
void cCameraFactory::ChangeCamera(const Json::Value &camera_json)
{

    std::string type_str = cJsonUtil::ParseAsString("camera_type", camera_json);
    Json::Value camera_pos_json =
        cJsonUtil::ParseAsValue("camera_pos", camera_json);
    Json::Value camera_focus_json =
        cJsonUtil::ParseAsValue("camera_focus", camera_json);
    SIM_ASSERT(camera_pos_json.size() == 3);
    SIM_ASSERT(camera_focus_json.size() == 3);
    tVector3 mCameraInitFocus =
        tVector3(camera_focus_json[0].asFloat(), camera_focus_json[1].asFloat(),
                 camera_focus_json[2].asFloat());
    tVector3 mCameraInitPos =
        tVector3(camera_pos_json[0].asFloat(), camera_pos_json[1].asFloat(),
                 camera_pos_json[2].asFloat());

    FLOAT near_plane_dist = cJsonUtil::ParseAsFloat("near", camera_json);
    FLOAT far_plane_dist = cJsonUtil::ParseAsFloat("far", camera_json);
    FLOAT mCameraInitFov = 0;
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
        instance = std::make_shared<FPSCamera>(
            mCameraInitPos, mCameraInitFocus, tVector3(0, 1, 0), mCameraInitFov,
            near_plane_dist, far_plane_dist);

        break;
    }
    case eCameraType::ARCBALL_CAMERA:
    {
        instance = std::make_shared<cArcBallCamera>(
            mCameraInitPos, mCameraInitFocus, tVector3(0, 1, 0), mCameraInitFov,
            near_plane_dist, far_plane_dist);
        break;
    }
    case eCameraType::ORTHO_CAMERA:
    {

        FLOAT init_box =
            cJsonUtil::ParseAsFloat("camera_init_box", camera_json);
        instance = std::make_shared<cOrthoCamera>(
            mCameraInitPos, mCameraInitFocus, tVector3(0, 1, 0), init_box,
            near_plane_dist, far_plane_dist);
        break;
    }
    default:
        SIM_ERROR("unsupported camera type {}", type_str);
        exit(1);
        break;
    }
}
void cCameraFactory::DestroyInstance(CameraBase *ptr) { delete ptr; }