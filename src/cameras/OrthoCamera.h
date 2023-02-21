#pragma once
#include "CameraBase.h"

class cOrthoCamera : public CameraBase
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    cOrthoCamera(const tVector3 &pos, const tVector3 &center,
                 const tVector3 &up, _FLOAT init_box, _FLOAT near_plane_dist,
                 _FLOAT far_plane_dist);
    virtual ~cOrthoCamera();

    virtual void MoveForward() override;
    virtual void MoveBackward() override;
    virtual void MoveLeft() override;
    virtual void MoveRight() override;
    virtual void MoveUp() override;
    virtual void MoveDown() override;

    virtual void MouseMove(_FLOAT mouse_x, _FLOAT mouse_y) override;
    virtual tMatrix4 ProjMatrix(_FLOAT screen_width, _FLOAT screen_height,
                                 bool is_vulkan = false) const override;
    virtual tVector4 CalcCursorPointWorldPos(_FLOAT xpos, _FLOAT ypos,
                                            int height, int width) override;

protected:
    _FLOAT mInitBoxSize; // init box width
};