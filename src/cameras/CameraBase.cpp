#include "CameraBase.h"
#include "utils/LogUtil.h"
#include "utils/RotUtil.h"
const std::string gSceneTypeStr[eCameraType::NUM_OF_CAMERA_TYPE] = {
    "fps_cam", "arcball_cam", "ortho_cam"};
eCameraType CameraBase::BuildCameraTypeFromStr(std::string str)
{
    int i = 0;
    for (i = 0; i < eCameraType::NUM_OF_CAMERA_TYPE; i++)
    {
        // std::cout << gSceneTypeStr[i] << std::endl;
        if (str == gSceneTypeStr[i])
        {
            break;
        }
    }

    SIM_ASSERT(i != eCameraType::NUM_OF_CAMERA_TYPE);
    return static_cast<eCameraType>(i);
}
tMatrix4 CameraBase::ViewMatrix()
{
    return Eigen::lookAt(mCamPos, mCamCenter, mCamUp);
}
CameraBase::CameraBase(const tVector3 &pos, const tVector3 &center,
                       const tVector3 &up, FLOAT fov, FLOAT near_plane,
                       FLOAT far_plane)
    : mouse_acc(0.1f), key_acc(0.02f), last_x(0), last_y(0), first_mouse(true)
{
    mCamPos = pos;
    mCamCenter = center;
    mCamUp = up.normalized();
    mCamFront = (mCamCenter - mCamPos).normalized();
    mFovDeg = fov;
    mNearPlane = near_plane;
    mFarPlane = far_plane;
}
CameraBase::~CameraBase() {}
eCameraType CameraBase::GetType() const { return mType; }

void CameraBase::SetKeyAcc(FLOAT acc) { key_acc = static_cast<float>(acc); }

void CameraBase::SetMouseAcc(FLOAT acc)
{
    mouse_acc = static_cast<float>(acc);
}

void CameraBase::SetXY(FLOAT mouse_x, FLOAT mouse_y)
{
    last_x = mouse_x;
    last_y = mouse_y;
}
void CameraBase::ResetFlag() { first_mouse = true; }
// void CameraBase::SetStatus()
// {

//     this->pos = CameraUtil::pos;
//     this->center = CameraUtil::center;
//     this->up = CameraUtil::up;
//     this->front = CameraUtil::front;
//     this->pitch = CameraUtil::pitch;
//     this->yaw = CameraUtil::yaw;
// }

/**
 * \brief           perspective matrix (default)
 *  OPENGL convention. in vulkan environment, the (1,1) *= -1 for Y axis
 * opposition
 */
tMatrix4 CameraBase::ProjMatrix(FLOAT screen_width, FLOAT screen_height,
                                 bool is_vulkan /*= false*/) const

{
    FLOAT fov_rad = (mFovDeg / 180) * M_PI;
    FLOAT gamma = screen_width / screen_height;
    FLOAT tan_theta_2 = std::tan(fov_rad / 2);
    // std::cout << "fov_rad = " << fov_rad << std::endl;
    // std::cout << "tan theta / 2 = " << tan_theta_2 << std::endl;
    tMatrix4 view_mat = tMatrix4::Zero();
    view_mat(0, 0) = 1.0 / (gamma * tan_theta_2);
    view_mat(1, 1) = 1.0 / tan_theta_2;
    view_mat(2, 2) = -1 * (mNearPlane + mFarPlane) / (mFarPlane - mNearPlane);
    view_mat(2, 3) = -2 * mFarPlane * mNearPlane / (mFarPlane - mNearPlane);
    view_mat(3, 2) = -1;

    if (is_vulkan == true)
    {
        view_mat(1, 1) *= -1;
    }
    return view_mat;
}
tVector3 CameraBase::GetCameraPos() const { return mCamPos; }

tVector3 CameraBase::GetCameraCenter() const { return this->mCamCenter; }
tVector3 CameraBase::GetCameraUp() const { return this->mCamUp; }
FLOAT CameraBase::GetCameraFovDeg() const
{
    // for orthogonal cam, the fov can be zero; add this assert in order to
    // debug
    SIM_ASSERT(mFovDeg > 1);
    return this->mFovDeg;
}
bool CameraBase::IsFirstMouse() const { return this->first_mouse; }

/**
 * \brief           inverse persepctive projection
 */
tVector4 CameraBase::CalcCursorPointWorldPos(FLOAT xpos, FLOAT ypos,
                                            int height, int width)
{
    tMatrix4 view_mat_inv = this->ViewMatrix().inverse();
    tMatrix4 mat;
#ifdef __APPLE__
    xpos *= 2, ypos *= 2;
#endif
    FLOAT fov_rad = this->mFovDeg / 180 * M_PI;
    FLOAT tan_theta_2 = std::tan(fov_rad / 2);
    tVector4 test = tVector4(xpos, ypos, 1, 1);
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
    tMatrix4 mat3 = tMatrix4::Identity();
    mat3(0, 0) = width * 1.0 / height * tan_theta_2 * this->mNearPlane;

    mat3(1, 1) = tan_theta_2 * this->mNearPlane;
    mat3(2, 2) = 0, mat3(2, 3) = -this->mNearPlane;
    tMatrix4 mat4 = view_mat_inv;
    mat = mat4 * mat3 * mat2 * mat1;

    tVector4 pos = mat * tVector4(xpos, ypos, 1, 1);
    return pos;
}

tVector3 CameraBase::GetCameraFront() const { return this->mCamFront; }
/**
 * \brief
 */
void CameraBase::RotateAlongYAxis(FLOAT angle_deg)
{
    // keep the center
    tVector3 pos_to_center = this->mCamCenter - this->mCamPos;
    tVector3 new_pos_to_center =
        cRotUtil::AxisAngleToRotmat(tVector4(0, 1, 0, 0) * angle_deg *
                                     gDegreesToRadians)
            .block(0, 0, 3, 3)
             *
        pos_to_center;

    // change the pos
    mCamPos = mCamCenter - new_pos_to_center;
    // change the front vector
    mCamFront = (mCamCenter - mCamPos).normalized();
}
void CameraBase::MiddleKeyMove(FLOAT mouse_x, FLOAT mouse_y) {

}