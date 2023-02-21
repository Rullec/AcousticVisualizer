//
// Created by Hanke on 2019-01-31.
//

#ifndef ROBOT_CAMERA_H
#define ROBOT_CAMERA_H
#include "CameraBase.h"

class FPSCamera : public CameraBase
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    FPSCamera(const tVector3 &pos, const tVector3 &center,
              const tVector3 &up, _FLOAT fov, _FLOAT near_plane, _FLOAT far_plane);
    virtual ~FPSCamera();

    virtual void MoveForward() override;
    virtual void MoveBackward() override;
    virtual void MoveLeft() override;
    virtual void MoveRight() override;
    virtual void MoveUp() override;
    virtual void MoveDown() override;
    virtual void MouseMove(_FLOAT mouse_x, _FLOAT mouse_y) override;

protected:
    void Init();
    _FLOAT pitch, yaw;
};

#endif // ROBOT_CAMERA_H
