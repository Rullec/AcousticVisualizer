#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#ifdef __linux__
#define VK_USE_PLATFORM_XCB_KHR
#endif

#define GLFW_INCLUDE_VULKAN
#include "MainApp.h"
#include "scenes/DrawSceneImGUI.h"
#include "scenes/SimScene.h"
#include "utils/JsonUtil.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
GLFWwindow *gGLFWWindow = nullptr;
extern cDrawScenePtr gDrawScene;
extern cSimScenePtr gSimScene;
bool gEscPushed = false;
int gWindowWidth = 0, gWindowHeight = 0;

static void ResizeCallback(GLFWwindow *gGLFWWindow, int w, int h)
{
    gDrawScene->Resize(w, h);
}

static void CursorPositionCallback(GLFWwindow *gGLFWWindow, double xpos,
                                   double ypos)
{
    gDrawScene->CursorMove(xpos, ypos);
}

void MouseButtonCallback(GLFWwindow *gGLFWWindow, int button, int action,
                         int mods)
{
    gDrawScene->MouseButton(button, action, mods);
}

void KeyCallback(GLFWwindow *gGLFWWindow, int key, int scancode, int action,
                 int mods)
{
    gDrawScene->Key(key, scancode, action, mods);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        gEscPushed = true;
    }
}

void ScrollCallback(GLFWwindow *gGLFWWindow, double xoffset, double yoffset)
{
    gDrawScene->Scroll(xoffset, yoffset);
}

cMainApp::cMainApp() {}

void cMainApp::Init(const std::string &conf_path, cDrawSceneImGuiPtr draw_scene,
                    cSimScenePtr sim_scene)
{
    mDrawScene = draw_scene;
    mSimScene = sim_scene;
}

bool cMainApp::ShouldClose()
{
    return !(glfwWindowShouldClose(gGLFWWindow) == false &&
             gEscPushed == false);
}
void cMainApp::Mainloop()
{
    while (false == ShouldClose())
    {
        PollEvents();

        // 1. calculate gradient
        // 2. update vaue
        // 3.

        mSimScene->Update(1e-3);
        mSimScene->UpdateRenderingResource();
        mDrawScene->AddRenderResource(mSimScene->GetRenderResource());
        mDrawScene->PreUpdateImGui();
        mSimScene->UpdateImGui();
        mDrawScene->PostUpdateImGui();
        mDrawScene->Update(1e-3);
    }

    // call deconstrcutor of glfw
}
void cMainApp::PollEvents() { glfwPollEvents(); }
void cMainApp::InitGLFW()
{
    SIM_ASSERT(gWindowWidth > 0 && gWindowHeight > 0);
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    gGLFWWindow = glfwCreateWindow(gWindowWidth, gWindowHeight,
                                   "DiffFabric Simulator", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(gGLFWWindow, ResizeCallback);
    glfwSetCursorPosCallback(gGLFWWindow, CursorPositionCallback);
    glfwSetMouseButtonCallback(gGLFWWindow, MouseButtonCallback);
    glfwSetKeyCallback(gGLFWWindow, KeyCallback);
    glfwSetScrollCallback(gGLFWWindow, ScrollCallback);
}

void cMainApp::CloseGLFW()
{
    glfwDestroyWindow(gGLFWWindow);
    glfwTerminate();
}