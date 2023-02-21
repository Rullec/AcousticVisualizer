#include "OrthoCamera.h"
#include "utils/LogUtil.h"

cOrthoCamera::cOrthoCamera(const tVector3 &pos_, const tVector3 &center_,
                           const tVector3 &up_, _FLOAT init_box,
                           _FLOAT near_plane_dist, _FLOAT far_plane_dist)
    : CameraBase(pos_, center_, up_, 0, near_plane_dist, far_plane_dist)

{
    mType = eCameraType::ORTHO_CAMERA;
    mInitBoxSize = init_box;
    SIM_ASSERT(mInitBoxSize > 0);
    mouse_acc *= 5e-2;
}

cOrthoCamera::~cOrthoCamera() {}
void cOrthoCamera::MoveForward() { mInitBoxSize *= 0.9; }
void cOrthoCamera::MoveBackward() { mInitBoxSize /= 0.9; }
void cOrthoCamera::MoveLeft() {}
void cOrthoCamera::MoveRight() {}
void cOrthoCamera::MoveUp() {}
void cOrthoCamera::MoveDown() {}
void cOrthoCamera::MouseMove(_FLOAT mouse_x, _FLOAT mouse_y)
{
    if (first_mouse)
    {
        last_x = mouse_x;
        last_y = mouse_y;
        first_mouse = false;
        return;
    }
    _FLOAT x_offset = (last_x - mouse_x) * mInitBoxSize;
    _FLOAT y_offset = (-mouse_y + last_y) * mInitBoxSize;
    last_x = mouse_x;
    last_y = mouse_y;
    x_offset *= mouse_acc;
    y_offset *= mouse_acc;

    // std::cout << "x offset = " << x_offset << std::endl;
    // std::cout << "y offset = " << y_offset << std::endl;
    tVector3 x_dir = mCamFront.cross(this->mCamUp);
    tVector3 y_dir = -mCamUp;
    x_dir.normalize();
    y_dir.normalize();
    tVector3 shift = x_dir * x_offset + y_dir * y_offset;
    // std::cout << "world shift = " << shift.transpose() << std::endl;
    mCamPos += shift;
    mCamCenter += shift;
}

tMatrix4 cOrthoCamera::ProjMatrix(_FLOAT screen_width, _FLOAT screen_height,
                                   bool is_vulkan /*= false*/) const
{
    _FLOAT gamma = screen_height / screen_width;

    tMatrix4 mat = tMatrix4::Zero();
    // ortho: refine
    if (is_vulkan == true)
    {
        mat(0, 0) = 1 / (mInitBoxSize);
        mat(0, 3) = 0;
        mat(1, 1) = -1 / (gamma * mInitBoxSize);
        mat(1, 3) = 0;
        mat(2, 2) = -1 / (mFarPlane - mNearPlane);
        mat(2, 3) = -mNearPlane / (mFarPlane - mNearPlane);
        mat(3, 3) = 1;
    }
    else
    {
        SIM_ERROR("invalid");
    }

    return mat;
}

tVector4 cOrthoCamera::CalcCursorPointWorldPos(_FLOAT xpos, _FLOAT ypos,
                                              int height, int width)
{
    tMatrix4 mat1 = tMatrix4::Identity();
    mat1(0, 0) = 1.0 / width;
    mat1(0, 3) = 0.5 / width;
    mat1(1, 1) = 1.0 / height;
    mat1(1, 3) = 0.5 / height;

    tMatrix4 mat2 = tMatrix4::Identity();
    mat2(0, 0) = 2;
    mat2(0, 3) = -1;
    mat2(1, 1) = -2;
    mat2(1, 3) = 1;

    tMatrix4 mat3 = ProjMatrix(width, height, true).inverse();
    mat3(1, 1) *= -1;
    tMatrix4 mat4 = ViewMatrix().inverse();
    tVector4 pixel_coord = mat2 * mat1 * tVector4(xpos, ypos, mNearPlane, 1);
    // std::cout << "screen pos = " << xpos << " " << ypos << std::endl;
    // std::cout << "pixel_coord = " << pixel_coord.transpose() << std::endl;
    // tVector pos_in_view_coords = mat3 * pixel_coord;
    // std::cout << "pos_in_view_coords = " << pos_in_view_coords.transpose()
    //           << std::endl;
    tVector4 world_pos = mat4 * mat3 * pixel_coord;
    // std::cout << "world_pos = " << world_pos.transpose() << std::endl;
    // std::cout << "mat4 = \n" << mat4 << std::endl;
    return world_pos;
}