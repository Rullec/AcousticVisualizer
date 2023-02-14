find_package(GLFW3 REQUIRED)


add_library(
    scenes_lib
    SceneBuilder.cpp
    Scene.cpp
    SimScene.cpp
    SimStateMachine.cpp
    DrawScene.cpp 
    DrawSceneVk.cpp 
    DrawSceneShot.cpp 
    DrawSceneImGUI.cpp
    SimCallback.cpp
    Raycaster.cpp
)

target_link_libraries(scenes_lib ${Vulkan_LIBRARIES} cam_lib imgui_lib implot_lib sim_lib ${GLFW3_LIBRARY} X11)