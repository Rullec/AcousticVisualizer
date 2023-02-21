#include "ArcBallCamera.h"
// #include "Utils/MathUtil.h"
#include "utils/MathUtil.h"
#include "utils/RotUtil.h"
cArcBallCamera::cArcBallCamera()
    : CameraBase(tVector3(2, 2, 2), tVector3(0, 0, 0), tVector3(0, 1, 0), 60,
                 1e-4, 1e5)
{
    mType = eCameraType::ARCBALL_CAMERA;

    mCamFront = mCamCenter - mCamPos;
    mCamFront.normalize();
    mouse_acc *= 5e-2;
    key_acc *= 2e-2;
    // pos = tVector3(1, 1, 0);
    // center = tVector3(0, 1, 0);
    // up = tVector3(0, 1, 0);
    // front = center - pos;
    // front.normalize();
}
#include "utils/TimeUtil.hpp"
static cTimePoint start_time_pt = cTimeUtil::GetCurrentTime_chrono();
cArcBallCamera::cArcBallCamera(const tVector3 &pos, const tVector3 &center,
                               const tVector3 &up, _FLOAT fov,
                               _FLOAT near_plane, _FLOAT far_plane)
    : CameraBase(pos, center, up, fov, near_plane, far_plane)
{
    mType = eCameraType::ARCBALL_CAMERA;
    mouse_acc *= 5e-2;
    key_acc *= 2e-1;
}

cArcBallCamera::~cArcBallCamera() {}

tMatrix4 cArcBallCamera::ViewMatrix()
{
    // auto cur_time = cTimeUtil::GetCurrentTime_chrono();
    // FLOAT cost_s =
    //     cTimeUtil::CalcTimeElaspedms(start_time_pt, cur_time) * 1e-3;
    // FLOAT rot_angle = 0.5 * cost_s * M_PI;
    // std::cout << "rot angle = " << rot_angle << std::endl;
    // tVector axis_angle = tVector::Zero();
    // axis_angle.segment(0, 3) = mCamUp.normalized() *
    // rot_angle; tMatrix3 rotmat = cRotUtil::AxisAngleToRotmat(axis_angle)
    //                        .topLeftCorner<3, 3>()
    //                        ;
    // mCamPos = rotmat * (mCamPos - mCamCenter) + mCamCenter;
    // start_time_pt = cur_time;
    return CameraBase::ViewMatrix();
}
void cArcBallCamera::MoveForward()
{
    // decrease the dist from center to pos
    mCamPos = (mCamPos - mCamCenter) * (1 - key_acc * 1e2) + mCamCenter;
}
void cArcBallCamera::MoveBackward()
{
    // increse the dist from center to pos
    mCamPos = (mCamPos - mCamCenter) * (1 + key_acc * 1e2) + mCamCenter;
}
void cArcBallCamera::MoveLeft()
{
    // no effect
}
void cArcBallCamera::MoveRight()
{
    // no effect
}
void cArcBallCamera::MoveUp()
{
    // no effect
}
void cArcBallCamera::MoveDown()
{
    // no effect
}

/**
 * \brief           Pinned the center and rotate this arcball camera when mouse
 * moved
 */
void cArcBallCamera::MouseMove(_FLOAT mouse_x, _FLOAT mouse_y)
{
    if (first_mouse)
    {
        last_x = mouse_x;
        last_y = mouse_y;
        first_mouse = false;
        return;
    }

    /*
        For screent normalized coordinate
        X,Y = (0, 0) is the left up corner
            = (1, 1) is the right down corner
        X+: from left to right
        Y+: from up to down
    */

    // 1. calculate the offset vector from last mouse pos to current mouse pos
    tVector3 offset_vec =
        tVector3(mouse_x - last_x, -1 * (mouse_y - last_y), 0) * mouse_acc;
    last_x = mouse_x;
    last_y = mouse_y;

    // 2. convert this vector to world frame, and opposite it (because we want
    // to rotate the object indeed)
    tVector3 offset_vec_world =
        -1 * ViewMatrix().block(0, 0, 3, 3).inverse() * offset_vec;

    // 3. calcualte center_to_pos vector, calculate the rotmat for our camera
    // (fixed center)
    tVector3 center_to_pos = mCamPos - mCamCenter;

    tMatrix3 rotmat =
        cRotUtil::AxisAngleToRotmat(
            cMathUtil::Expand(
                center_to_pos.normalized().cross(offset_vec_world), 0))
            .block(0, 0, 3, 3);

    center_to_pos = rotmat * center_to_pos;

    // 4. rotate the center to pos, update other variables, center is fixed
    mCamUp = rotmat * this->mCamUp;
    mCamPos = center_to_pos + mCamCenter;
    mCamFront = (mCamCenter - mCamPos).normalized();
}

/**
 * \brief           shift this arcball camera by mouse move
 */

void cArcBallCamera::MiddleKeyMove(_FLOAT mouse_x, _FLOAT mouse_y)
{
    // std::cout << "[debug] arcball middle key move = " << mouse_x << " "
    //           << mouse_y << std::endl;
    if (first_mouse)
    {
        last_x = mouse_x;
        last_y = mouse_y;
        first_mouse = false;
        return;
    }

    // 0. get move vector in NDC
    tVector3 offset_vec =
        tVector3(mouse_x - last_x, -1 * (mouse_y - last_y), 0) * mouse_acc;
    last_x = mouse_x;
    last_y = mouse_y;

    // 3. convert to world coordinate
    tVector3 offset_vec_world =
        -ViewMatrix().block(0, 0, 3, 3).inverse() * offset_vec;
    // move slower when the camera is near to the center
    _FLOAT dist_scale = (mCamPos - mCamCenter).norm();
    offset_vec_world *= dist_scale * 2e-1;

    // 4. move the camera
    // std::cout << "the camera shift = " << offset_vec_world.transpose()
    //           << std::endl;
    mCamPos += offset_vec_world;
    mCamCenter += offset_vec_world;
}