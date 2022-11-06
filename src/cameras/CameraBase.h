#pragma once
//
// Extract to base by Xudong on 2021-01-30
//

// #include "../Utils/EigenUtils.h"
#include "utils/MathUtil.h"
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
    CameraBase(const tVector3 &pos, const tVector3 &center,
               const tVector3 &up, FLOAT fov, FLOAT near_plane,
               FLOAT far_plane);
    virtual ~CameraBase() = 0;
    eCameraType GetType() const;
    static eCameraType BuildCameraTypeFromStr(std::string name);
    virtual tMatrix4 ViewMatrix();
    virtual tMatrix4 ProjMatrix(FLOAT screen_width, FLOAT screen_height,
                                 bool is_vulkan = false) const;
    virtual tVector3 GetCameraPos() const;
    virtual tVector3 GetCameraCenter() const;
    virtual tVector3 GetCameraFront() const;
    virtual tVector3 GetCameraUp() const;
    virtual FLOAT GetCameraFovDeg() const;
    virtual void MoveForward() = 0;
    virtual void MoveBackward() = 0;
    virtual void MoveLeft() = 0;
    virtual void MoveRight() = 0;
    virtual void MoveUp() = 0;
    virtual void MoveDown() = 0;

    virtual void MouseMove(FLOAT mouse_x, FLOAT mouse_y) = 0;
    virtual void MiddleKeyMove(FLOAT mouse_x, FLOAT mouse_y);
    virtual void SetXY(FLOAT mouse_x, FLOAT mouse_y);
    virtual void ResetFlag();

    virtual void SetKeyAcc(FLOAT acc);
    virtual void SetMouseAcc(FLOAT acc);
    virtual bool IsFirstMouse() const;

    virtual tVector4 CalcCursorPointWorldPos(FLOAT xpos, FLOAT ypos,
                                            int height, int width);
    virtual void RotateAlongYAxis(FLOAT angle_deg);
protected:
    eCameraType mType;
    tVector3 mCamPos, mCamCenter, mCamUp, mCamFront;
    FLOAT mNearPlane, mFarPlane;
    FLOAT mFovDeg;
    FLOAT mouse_acc;
    FLOAT key_acc;

    FLOAT last_x, last_y; // the previous x and y position of mouse
    bool first_mouse;     // whether it's the first mouse event
};