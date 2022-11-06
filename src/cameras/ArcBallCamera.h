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
    // cArcBallCamera();
    cArcBallCamera(const tVector3 &pos, const tVector3 &center,
                   const tVector3 &up, FLOAT fov, FLOAT near_plane,
                   FLOAT far_plane);

    virtual ~cArcBallCamera();
    virtual tMatrix4 ViewMatrix() override;
    virtual void MoveForward() override;
    virtual void MoveBackward() override;
    virtual void MoveLeft() override;
    virtual void MoveRight() override;
    virtual void MoveUp() override;
    virtual void MoveDown() override;
    virtual void MiddleKeyMove(FLOAT mouse_x, FLOAT mouse_y) override;
    virtual void MouseMove(FLOAT mouse_x, FLOAT mouse_y) override;

protected:
};