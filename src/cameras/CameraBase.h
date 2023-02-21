#pragma once
//
// Extract to base by Xudong on 2021-01-30
//

#include "utils/EigenUtil.h"
#include <iostream>
#include <memory>
// #include <cmath>

enum eCameraType
{
    FPS_CAMERA = 0, // default wasd FPS camear
    ARCBALL_CAMERA, // arcball camera
    ORTHO_CAMERA,   // orthogonal camera
    NUM_OF_CAMERA_TYPE
};

class CameraBase : std::enable_shared_from_this<CameraBase>
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    CameraBase(const tVector3 &pos, const tVector3 &center, const tVector3 &up,
               _FLOAT fov, _FLOAT near_plane, _FLOAT far_plane);
    virtual ~CameraBase() = 0;
    eCameraType GetType() const;
    static eCameraType BuildCameraTypeFromStr(std::string name);
    virtual tMatrix4 ViewMatrix();
    virtual tMatrix4 ProjMatrix(_FLOAT screen_width, _FLOAT screen_height,
                                bool is_vulkan = false) const;
    virtual tVector3 GetCameraPos() const;
    virtual tVector3 GetCameraCenter() const;
    virtual tVector3 GetCameraFront() const;
    virtual tVector3 GetCameraUp() const;
    virtual _FLOAT GetCameraFovDeg() const;
    virtual void MoveForward() = 0;
    virtual void MoveBackward() = 0;
    virtual void MoveLeft() = 0;
    virtual void MoveRight() = 0;
    virtual void MoveUp() = 0;
    virtual void MoveDown() = 0;

    virtual void MouseMove(_FLOAT mouse_x, _FLOAT mouse_y) = 0;
    virtual void MiddleKeyMove(_FLOAT mouse_x, _FLOAT mouse_y);
    virtual void SetXY(_FLOAT mouse_x, _FLOAT mouse_y);
    virtual void ResetFlag();

    virtual void SetKeyAcc(_FLOAT acc);
    virtual void SetMouseAcc(_FLOAT acc);
    virtual bool IsFirstMouse() const;

    virtual tVector4 CalcCursorPointWorldPos(_FLOAT xpos, _FLOAT ypos,
                                             int height, int width);
    virtual void RotateAlongYAxis(_FLOAT angle_deg);
    virtual std::string GetDescString();

protected:
    eCameraType mType;
    tVector3 mCamPos, mCamCenter, mCamUp, mCamFront;
    _FLOAT mNearPlane, mFarPlane;
    _FLOAT mFovDeg;
    _FLOAT mouse_acc;
    _FLOAT key_acc;

    _FLOAT last_x, last_y; // the previous x and y position of mouse
    bool first_mouse;      // whether it's the first mouse event
};