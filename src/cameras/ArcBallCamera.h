//
// Created by Xudong on 2021/01/30
//
#pragma once
#include "CameraBase.h"

/**
 * \brief           Arcball camera
 */
class cArcBallCamera : public CameraBase
{
public:
    cArcBallCamera();
    cArcBallCamera(const tVector3 &pos, const tVector3 &center,
                   const tVector3 &up, _FLOAT fov, _FLOAT near_plane,
                   _FLOAT far_plane);

    virtual ~cArcBallCamera();
    virtual tMatrix4 ViewMatrix() override;
    virtual void MoveForward() override;
    virtual void MoveBackward() override;
    virtual void MoveLeft() override;
    virtual void MoveRight() override;
    virtual void MoveUp() override;
    virtual void MoveDown() override;
    virtual void MiddleKeyMove(_FLOAT mouse_x, _FLOAT mouse_y) override;
    virtual void MouseMove(_FLOAT mouse_x, _FLOAT mouse_y) override;

protected:
};