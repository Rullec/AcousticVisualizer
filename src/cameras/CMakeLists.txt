set(src

    ArcBallCamera.cpp
    CameraBase.cpp
    CameraBuilder.cpp
    CameraFactory.cpp
    FPSCamera.cpp
    OrthoCamera.cpp
)

add_library(cam_lib ${src})
target_link_libraries(cam_lib PUBLIC utils_lib)