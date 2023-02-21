#include "DrawScene.h"
#include "SceneBuilder.h"
#include "cameras/ArcBallCamera.h"
#include "cameras/CameraFactory.h"
#include "geometries/Primitives.h"
#include "glm/glm.hpp"
#include "scenes/SimCallback.h"
#include "scenes/SimScene.h"
#include "scenes/TextureManager.h"
#include "utils/JsonUtil.h"
#include "utils/LogUtil.h"
#include "utils/RenderGrid.h"
#include "utils/TimeUtil.hpp"
#include "utils/json/json.h"
#include "vulkan/vulkan.h"
#include <iostream>
#include <optional>
#include <set>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#ifdef __linux__
#define VK_USE_PLATFORM_XCB_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#endif

#ifdef __APPLE__
#include <GLFW/glfw3.h>
#endif

std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef __APPLE__
std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                              "VK_KHR_portability_subset"};
#else
std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif

extern GLFWwindow *gGLFWWindow;

#ifdef NDEBUG
bool enableValidationLayers = false;
#else
bool enableValidationLayers = true;
#endif

const int MAX_FRAMES_IN_FLIGHT = 2;

int GetMaxFramesInFlight() { return MAX_FRAMES_IN_FLIGHT; }
// float fov = 45.0f;

#include "utils/MathUtil.h"
#include <array>
VkVertexInputBindingDescription tVkVertex::getBindingDescription()
{
    VkVertexInputBindingDescription desc{};
    desc.binding = 0;
    desc.stride = sizeof(tVkVertex); // return the bytes of this type occupies
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return desc;
}

/**
 * \brief       describe the description of the attributes.
 *
 *      we have two attributes, the position and color, we need to describe them
 * individualy, use two strucutre. The first strucutre describe the first
 * attribute (inPosition), we give the binding, location of this attribute, and
 * the offset
 *
 */
std::array<VkVertexInputAttributeDescription, 4>
tVkVertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 4> desc{};

    // position attribute
    desc[0].binding = 0; // binding point: 0
    desc[0].location = 0;
    desc[0].format = VK_FORMAT_R32G32B32_SFLOAT; // used for vec3f
    // desc[0].format = VK_FORMAT_R32G32_SFLOAT; // used for vec2f
    desc[0].offset = offsetof(tVkVertex, pos);
    SIM_ASSERT(SIM_VK_VERTEX_POS_OFFSET_BYTE == desc[0].offset);

    // color attribute
    desc[1].binding = 0;
    desc[1].location = 1;
    desc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // used for vec4f
    desc[1].offset = offsetof(tVkVertex, color);
    SIM_ASSERT(SIM_VK_VERTEX_COLOR_OFFSET_BYTE == desc[1].offset);

    // normal attribute
    desc[2].binding = 0;
    desc[2].location = 2;
    desc[2].format = VK_FORMAT_R32G32B32_SFLOAT; // used for vec3f
    desc[2].offset = offsetof(tVkVertex, normal);
    SIM_ASSERT(SIM_VK_VERTEX_NORMAL_OFFSET_BYTE == desc[2].offset);

    // texture atrribute
    desc[3].binding = 0;
    desc[3].location = 3;
    desc[3].format = VK_FORMAT_R32G32_SFLOAT; // used for vec2f
    desc[3].offset = offsetof(tVkVertex, texCoord);
    SIM_ASSERT(SIM_VK_VERTEX_TEX_OFFSET_BYTE == desc[3].offset);

    return desc;
}

/**
 * \brief       manually point out the vertices info, include:
    1. position: vec2f in NDC
    2. color: vec3f \in [0, 1]
*/

tVector4 cDrawScene::GetCameraPos() const
{
    tVector4 pos = tVector4::Ones();
    pos.segment(0, 3) = mCamera->GetCameraPos();
    return pos;
}
bool cDrawScene::IsMouseRightButton(int glfw_button)
{
    return glfw_button == GLFW_MOUSE_BUTTON_RIGHT;
}

bool cDrawScene::IsRelease(int glfw_action)
{
    return GLFW_RELEASE == glfw_action;
}

bool cDrawScene::IsPress(int glfw_action) { return GLFW_PRESS == glfw_action; }

tVector4 CalcCursorPointWorldPos_perspective_proj(float fov, float xpos,
                                                  float ypos, int height,
                                                  int width,
                                                  const tMatrix4 &view_mat_inv)
{
    SIM_ERROR("hasn't finished yet");
    return tVector4::Zero();
}

/**
 * \brief           get the cursor world position in orthogonal proj
 */
extern int gWindowHeight, gWindowWidth;
tVector4 cDrawScene::CalcCursorPointWorldPos() const
{
    double xpos, ypos;
    glfwGetCursorPos(gGLFWWindow, &xpos, &ypos);
#ifdef __APPLE__
    xpos /= 2;
    ypos /= 2;
#endif
    return mCamera->CalcCursorPointWorldPos(xpos, ypos, gWindowHeight,
                                            gWindowWidth);
}
tVkDrawBuffer::tVkDrawBuffer()
{
    mSize = 0;
    mIsCreated = false;
    mBuffer = nullptr;
    mMemory = nullptr;
}
void tVkDrawBuffer::DestroyAndFree(VkDevice device)
{
    vkDestroyBuffer(device, this->mBuffer, nullptr);
    vkFreeMemory(device, this->mMemory, nullptr);
    mSize = 0;
    mIsCreated = false;
}

cDrawScene::cDrawScene()
{
    mBufferReallocated = false;
    mSceneIsTheLastRenderPass = true;
    mInstance = nullptr;
    mCurFrame = 0;
    mFrameBufferResized = false;
    mLeftButtonPress = false;
    mMiddleButtonPress = false;
}

cDrawScene::~cDrawScene() {}

void cDrawScene::CleanVulkan()
{
    CleanSwapChain();

    vkDestroySampler(mDevice, mTextureSampler, nullptr);

    vkDestroyImageView(mDevice, mDepthImageView, nullptr);
    vkDestroyImage(mDevice, mDepthImage, nullptr);
    vkFreeMemory(mDevice, mDepthImageMemory, nullptr);
    GetTextureManager()->Clear();

    vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
    mSimTriangleBuffer.DestroyAndFree(mDevice);
    mLineBuffer.DestroyAndFree(mDevice);
    mPointDrawBuffer.DestroyAndFree(mDevice);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(mDevice, mImageAvailableSemaphore[i], nullptr);
        vkDestroySemaphore(mDevice, mRenderFinishedSemaphore[i], nullptr);
        vkDestroyFence(mDevice, minFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    vkDestroyDevice(mDevice, nullptr);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyInstance(mInstance, nullptr);
}

/**
 * \brief               Create Graphcis Pipeline
 */
#include "utils/FileUtil.h"
std::vector<char> ReadFile(const std::string &filename)
{
    SIM_ASSERT(cFileUtil::ExistsFile(filename) == true);
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    // SIM_INFO("read file {} size {} succ", filename, fileSize);
    return buffer;
}

void cDrawScene::CreateGraphicsPipeline(const std::string mode,
                                        VkPipeline &pipeline)
{
    // load and create the module
    auto VertShaderCode = ReadFile("data/shaders/shader.vert.spv");
    auto FragShaderCode = ReadFile("data/shaders/shader.frag.spv");
    VkShaderModule VertShaderModule = CreateShaderModule(VertShaderCode);
    VkShaderModule FragShaderModule = CreateShaderModule(FragShaderCode);

    // put the programmable shaders into the pipeline
    // {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.module = VertShaderModule;
    vertShaderStageInfo.pName = "main"; // entry point
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.pSpecializationInfo =
        nullptr; // we can define some constants for optimizatin here

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.module = FragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertShaderStageInfo,
                                                       fragShaderStageInfo};
    // }

    // put the fixed stages into the pipeline
    // {
    auto bindingDesc = tVkVertex::getBindingDescription();
    auto attriDesc = tVkVertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attriDesc.size());
    vertexInputInfo.pVertexAttributeDescriptions = attriDesc.data();

    // don't know what's this
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    if (mode == "triangle")
    {
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
    else if (mode == "line")
    {
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    }
    else if (mode == "point")
    {
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }
    else
    {
        SIM_ERROR("unsupported mode {}", mode);
        exit(0);
    }

    // Viewport: which part of framebuffer will be rendered to
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)mSwapChainExtent.width;
    viewport.height = (float)mSwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // scissors: useless
    VkRect2D scissor{};
    scissor.extent = mSwapChainExtent;
    scissor.offset = {0, 0};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pScissors = &scissor;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.viewportCount = 1;

    // rasterizer: vertices into fragments
    VkPipelineRasterizationStateCreateInfo raster_info{};
    raster_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_info.depthClampEnable =
        VK_FALSE; // clamp the data outside of the near-far plane insteand of
                  // deleting them
    raster_info.rasterizerDiscardEnable =
        VK_FALSE; // disable the rasterization, it certainly should be disable
    raster_info.polygonMode = VK_POLYGON_MODE_FILL; // normal
    raster_info.lineWidth =
        1.0f; // if not 1.0, we need to enable the GPU "line_width" feature

    // raster_info.cullMode = VK_CULL_MODE_NONE; // don't cull
    raster_info.cullMode = VK_CULL_MODE_BACK_BIT; // back cull
    raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    // setting for possible shadow map
    raster_info.depthBiasEnable = VK_FALSE;
    raster_info.depthBiasConstantFactor = 0.0f;
    raster_info.depthBiasClamp = 0.0f;
    raster_info.depthBiasSlopeFactor = 0.0f;

    // setting for multisampling
    VkPipelineMultisampleStateCreateInfo multisampling_info{};
    multisampling_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_info.sampleShadingEnable = VK_FALSE;
    multisampling_info.rasterizationSamples = mSampleCount;
    multisampling_info.minSampleShading = 1.0f;
    multisampling_info.pSampleMask = nullptr;
    multisampling_info.alphaToCoverageEnable = VK_FALSE;
    multisampling_info.alphaToOneEnable = VK_FALSE;
    multisampling_info.sampleShadingEnable =
        VK_TRUE; // enable texture sampling shading (MSAA)
    multisampling_info.minSampleShading =
        .2f; // min fraction, closer to one is smooth

    // setting for color blending: the interaction between framebuffer and the
    // output of fragment shader configuration for each individual frame buffer
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    // which channels will be effected?
    colorBlendAttachmentState.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    // colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    // the global color blending setting
    VkPipelineColorBlendStateCreateInfo colorBlendState_info{};
    colorBlendState_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState_info.logicOpEnable = VK_FALSE;
    colorBlendState_info.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState_info.attachmentCount = 1;
    colorBlendState_info.pAttachments = &colorBlendAttachmentState;
    colorBlendState_info.blendConstants[0] = 0.0f;
    colorBlendState_info.blendConstants[1] = 0.0f;
    colorBlendState_info.blendConstants[2] = 0.0f;
    colorBlendState_info.blendConstants[3] = 0.0f;

    // some settings can be dynamically changed without recreating the pipelines
    // VkDynamicState dynamicStates[] = {
    // VK_DYNAMIC_STATE_VIEWPORT,
    // VK_DYNAMIC_STATE_LINE_WIDTH
    // };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    // dynamicState.dynamicStateCount = 2;
    dynamicState.dynamicStateCount = 0;
    // dynamicState.pDynamicStates = dynamicStates;
    dynamicState.pDynamicStates = nullptr;

    // pipeline layout (uniform values in our shaders)
    // now here is an empty pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    // add push constant for "enable_texture"
    {
        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = sizeof(tMaterialPushConstant);
        push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        pipelineLayoutInfo.pPushConstantRanges = &push_constant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
    }
    const VkPipelineLayoutCreateInfo *info_ptr = &pipelineLayoutInfo;
    SIM_ASSERT(vkCreatePipelineLayout(mDevice, info_ptr, nullptr,
                                      &mPipelineLayout) == VK_SUCCESS);
    // }

    // add depth test
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {};  // Optional

    // create the final pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shader_stages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &raster_info;
    pipelineInfo.pMultisampleState = &multisampling_info;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlendState_info;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = mPipelineLayout;
    pipelineInfo.renderPass = mRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    SIM_ASSERT(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1,
                                         &pipelineInfo, nullptr,
                                         &pipeline) == VK_SUCCESS)

    // destory the modules
    vkDestroyShaderModule(mDevice, VertShaderModule, nullptr);
    vkDestroyShaderModule(mDevice, FragShaderModule, nullptr);

    // SIM_INFO("Create graphics pipeline succ");
}

/**
 * \brief       Init vulkan and other stuff
 */
#include "cameras/CameraBuilder.h"
cRenderResourcePtr gAxesRenderResource = nullptr;
cRenderResourcePtr gGroundRenderResource = nullptr;
void cDrawScene::Init(const std::string &conf_path)
{
    // init camera pos

    Json::Value root;
    cJsonUtil::LoadJson(conf_path, root);
    Json::Value camera_json = cJsonUtil::ParseAsValue("camera", root);
    mGroundPNGPath = cJsonUtil::ParseAsString(GROUND_PNG_PATH_KEY, root);
    mEnableGround = cJsonUtil::ParseAsBool("enable_ground", root);
    mGroundHeight = cJsonUtil::ParseAsfloat("ground_height", root);
    mEnableGrid = false;
    SetGroundHeight(mGroundHeight);
    cCameraFactory::ChangeCamera(camera_json);
    mCamera = cCameraFactory::getInstance();
    mEnableCamAutoRot = cJsonUtil::ParseAsBool("enable_camera_auto_rot", root);
    mEnableAxes = cJsonUtil::ParseAsBool("enable_axes", root);
    gAxesRenderResource = cRenderUtil::GetAxesRenderingResource();
    gGroundRenderResource = cRenderUtil::GetGroundRenderingResource(
        1e3, mGroundHeight, mGroundPNGPath);

    ClearRenderResource();
    if (mEnableAxes == true)
        AddRenderResource({gAxesRenderResource});
    AddRenderResource({gAxesRenderResource});
    mGrid = std::make_shared<cRenderGrid>();
    mGrid->GenGrid();
    AddRenderResource(mGrid->GetRenderingResource());

    InitVulkan();
}
void cDrawScene::SetGroundHeight(float val)
{
    gGroundRenderResource =
        cRenderUtil::GetGroundRenderingResource(1e3, val, mGroundPNGPath);
}

/**
 * \brief           Update the render
 */
void cDrawScene::Update(_FLOAT dt)
{
    // update the perturb
    {
        // update perturb
        tVector4 cursor_pos = CalcCursorPointWorldPos();
        tVector4 perturb_ori = GetPerturbOrigin(cursor_pos);
        tVector4 dir = cursor_pos - perturb_ori;
        dir[3] = 0;
        dir.normalize();
        cSimCallback::UpdateSimPerturbPos(perturb_ori, dir);
    }

    DrawFrame();
    if (mEnableCamAutoRot)
    {
        mCamera->RotateAlongYAxis(0.3);
    }
}

void cDrawScene::Resize(int w, int h)
{
    gWindowHeight = h;
    gWindowWidth = w;
    mFrameBufferResized = true;
}

void cDrawScene::CursorMove(int xpos, int ypos)
{
    if (mLeftButtonPress)
    {
        mCamera->MouseMove(xpos, ypos);
    }
    else if (mMiddleButtonPress)
    {
        mCamera->MiddleKeyMove(xpos, ypos);
    }
}

void cDrawScene::MouseButton(int button, int action, int mods)
{
    // 1. track the click
    if (button == GLFW_MOUSE_BUTTON_1)
    {
        if (action == GLFW_RELEASE)
        {
            mLeftButtonPress = false;
            mCamera->ResetFlag();
        }

        else if (action == GLFW_PRESS)
        {
            mLeftButtonPress = true;
            // if (mCamera->IsFirstMouse() == true)
            // {
            //     mCamera->MouseMove()
            // }
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_RELEASE)
        {
            mMiddleButtonPress = false;
            mCamera->ResetFlag();
        }

        else if (action == GLFW_PRESS)
        {
            mMiddleButtonPress = true;
        }
    }

    // 2. if simulation scene is enabled, check perturb
    if (cDrawScene::IsMouseRightButton(button) == true)
    {
        if (cDrawScene::IsPress(action) == true)
        {
            tVector4 tar_pos = CalcCursorPointWorldPos();

            tVector4 perturb_ori = GetPerturbOrigin(tar_pos);
            tRayPtr ray = std::make_shared<tRay>(perturb_ori, tar_pos);
            cSimCallback::CreatePerturb(ray);
        }
        else if (cDrawScene::IsRelease(action) == true)
        {
            cSimCallback::ReleasePerturb();
        }
    }

    cSimCallback::MouseButton(button, action, mods);
}

void cDrawScene::Key(int key, int scancode, int action, int mods)
{
    // std::cout << "[draw scene] key = " << key << std::endl;
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_R:
            Reset();
            break;
        case GLFW_KEY_T:
        {
            std::string path = cFileUtil::GenerateRandomFilename(
                "data/export_data/screenshort.ppm");
            ScreenShotDraw(path);
            printf("[debug] take screenshot to %s\n", path.c_str());
            break;
        }
        case GLFW_KEY_D:
        {
            mCamera->MoveRight();
            break;
        }
        case GLFW_KEY_A:
        {
            mCamera->MoveLeft();
            break;
        }
        case GLFW_KEY_LEFT_CONTROL:
        {
#ifdef __APPLE__
            mMiddleButtonPress = true;
            break;
#endif
        }
        default:
            break;
        }
    }
    if (action == GLFW_RELEASE)
    {
        switch (key)
        {
        case GLFW_KEY_LEFT_CONTROL:
#ifdef __APPLE__
            mMiddleButtonPress = false;
            mCamera->ResetFlag();
#endif
            break;
        default:
            break;
        }
    }

    cSimCallback::Key(key, scancode, action, mods);
}

void cDrawScene::Scroll(float xoff, float yoff)
{
    // printf("scroll %.1f %.1f\n", xoff, yoff);
    if (yoff > 0)
        mCamera->MoveForward();
    else if (yoff < 0)
        mCamera->MoveBackward();
}

/**
 * \brief           Reset the whole scene
 */
void cDrawScene::Reset() {}

/**
 * \brief           Do initialization for vulkan
 */
void cDrawScene::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();

    // 1. all changed
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline("triangle", mTriangleGraphicsPipeline);
    CreateGraphicsPipeline("line", mLinesGraphicsPipeline);
    CreateGraphicsPipeline("point", mPointGraphicsPipeline);
    CreateCommandPool();
    CreateColorResources(); // MSAA
    CreateDepthResources();
    CreateFrameBuffers();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateTriangleBufferSim();
    CreateLineBuffer();
    CreatePointBuffer();
    CreateMVPUniformBuffer();

    // CreateDescriptorPool(mDescriptorPool, 1);
    // CreateDescriptorSets(mDescriptorSets, 1); // recreate
    // CreateCommandBuffers();                   // create
    CreateSemaphores();
}

uint32_t findMemoryType(VkPhysicalDevice phy_device, uint32_t typeFilter,
                        VkMemoryPropertyFlags props);
void CreateBuffer(VkDevice mDevice, VkPhysicalDevice mPhysicalDevice,
                  VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags props, VkBuffer &buffer,
                  VkDeviceMemory &buffer_memory)
{
    // 1. create a vertex buffer
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    SIM_ASSERT(vkCreateBuffer(mDevice, &buffer_info, nullptr, &buffer) ==
               VK_SUCCESS);

    // 2. allocate: first meet the memory requirements
    VkMemoryRequirements mem_reqs{};
    vkGetBufferMemoryRequirements(mDevice, buffer, &mem_reqs);
    // mem_reqs
    //     .size; // the size of required amount of memorys in bytes, may differ
    //     from the "bufferInfo.size"
    // mem_reqs.alignment;      // the beginning address of this buffer
    // mem_reqs.memoryTypeBits; // unknown

    // 3. allocate: memory allocation
    VkMemoryAllocateInfo allo_info{};
    allo_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allo_info.allocationSize = mem_reqs.size;
    allo_info.memoryTypeIndex =
        findMemoryType(mPhysicalDevice, mem_reqs.memoryTypeBits, props);

    SIM_ASSERT(vkAllocateMemory(mDevice, &allo_info, nullptr, &buffer_memory) ==
               VK_SUCCESS);

    // 4. bind (connect) the buffer with the allocated memory
    vkBindBufferMemory(mDevice, buffer, buffer_memory, 0);
}

VkCommandBuffer beginSingleTimeCommands(VkDevice device,
                                        VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool,
                           VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void cDrawScene::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                            VkDeviceSize size)
{
    auto cmd_buffer = beginSingleTimeCommands(mDevice, mCommandPool);
    // // 1. create a command buffer
    // VkCommandBufferAllocateInfo allo_info{};
    // allo_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // allo_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    // allo_info.commandPool = mCommandPool;
    // allo_info.commandBufferCount = 1;
    // VkCommandBuffer cmd_buffer;
    // vkAllocateCommandBuffers(mDevice, &allo_info, &cmd_buffer);

    // // 2. begin to record the command buffer
    // VkCommandBufferBeginInfo begin_info{};
    // begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // begin_info.flags =
    //     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // we only use this
    //     command buffer for a single time
    // vkBeginCommandBuffer(cmd_buffer, &begin_info);

    // 3. copy from src to dst buffer
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(cmd_buffer, srcBuffer, dstBuffer, 1, &copy_region);

    endSingleTimeCommands(mDevice, mCommandPool, mGraphicsQueue, cmd_buffer);
    // // 4. end recording
    // vkEndCommandBuffer(cmd_buffer);

    // // 5. submit info
    // VkSubmitInfo submit_info{};
    // submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // submit_info.commandBufferCount = 1;
    // submit_info.pCommandBuffers = &cmd_buffer;

    // vkQueueSubmit(mGraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);

    // // wait, till the queue is empty (which means all commands have been
    // finished) vkQueueWaitIdle(mGraphicsQueue);

    // // 6. deconstruct
    // vkFreeCommandBuffers(mDevice, mCommandPool, 1, &cmd_buffer);
}

void cDrawScene::CreateTriangleBufferSim(int size)
{
    mSimTriangleBuffer.mSize = size;

    CreateBuffer(mDevice, mPhysicalDevice, mSimTriangleBuffer.mSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 mSimTriangleBuffer.mBuffer, mSimTriangleBuffer.mMemory);
    mSimTriangleBuffer.mIsCreated = true;
    // CopyBuffer(stagingBuffer, mVertexBufferSim, buffer_size);
    // vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    // vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

/**
 * \brief           draw a single frame
        1. get an image from the swap chain
        2. executate the command buffer with that image as attachment in the
 framebuffer
        3. return the image to the swap chain for presentation
    These 3 actions, ideally shvould be executed asynchronously. but the
 function calls will return before the operations are actually finished. So the
 order of execution is undefined. we need "fences" ans "semaphores" to
 synchronizing swap chain

*/
void cDrawScene::DrawFrame()
{
    // 1. create descriptor set
    // 2. change swap chain
    // 3. record
    // 4. draw
    // CreateDescriptorSets(mDescriptorSets, 1);
    if (mEnableAxes)
        mRenderResourceArray.push_back(gAxesRenderResource);
    mRenderResourceArray.push_back(mGrid->GetRenderingResource()[0]);
    uint32_t imageIndex;
    bool need_to_recreate_swapchain =
        FenceAndAcquireImageFromSwapchain(imageIndex);

    // updating the uniform buffer values
    // cTimeUtil::Begin("update_buffers");
    UpdateMVPUniformValue(imageIndex);
    // UpdateVertexBufferGround(imageIndex);
    UpdateTriangleBufferSimObj(imageIndex);
    UpdateLineBuffer(imageIndex);
    UpdatePointBuffer(imageIndex);
    // cTimeUtil::End("update_buffers");
    // if (true == NeedRecreateCommandBuffers())
    // {
    DestoryCommandBuffers();
    // mBufferReallocated = false;
    // CreateCommandBuffers();
    // }

    // 2. submitting the command buffer
    std::vector<VkCommandBuffer> cmds = {mCommandBuffers[imageIndex]};
    SubmitCmdsAndPresent(cmds, imageIndex);
    mCurFrame = (mCurFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    if (need_to_recreate_swapchain == true)
    {
        // std::cout << "recreate the swapchain at the end of draw\n";
        mFrameBufferResized = false;
        RecreateSwapChain();
    }
    ClearRenderResource();
}
int GetNewSize(int cur_size, int tar_size)
{
    while (cur_size < tar_size)
    {
        cur_size *= 2;
    }
    return cur_size;
}
void cDrawScene::UpdatePointBuffer(int idx)
{
    SIM_ASSERT(mPointDrawBuffer.mIsCreated == true);

    int num_of_ele = 0;
    for (auto &x : mRenderResourceArray)
    {
        num_of_ele += x->mPointBuffer.mNumOfEle;
    }

    VkDeviceSize buffer_size = sizeof(float) * num_of_ele;

    if (buffer_size > mPointDrawBuffer.mSize)
    {
        int new_size = GetNewSize(mPointDrawBuffer.mSize, buffer_size);
        CreatePointBuffer(new_size);

        printf("[warn] point draw buffer size %d < resource size %d, "
               "reallocate! reallocate %d\n",
               mPointDrawBuffer.mSize, buffer_size, new_size);
        mBufferReallocated = true;
    }
    // 5. copy the vertex data to the buffer
    void *data = nullptr;
    // map the memory to "data" ptr;

    vkMapMemory(mDevice, mPointDrawBuffer.mMemory, 0, buffer_size, 0, &data);

    // write the data
    int offset = 0;
    char *char_data = static_cast<char *>(data);
    for (auto &x : mRenderResourceArray)
    {
        auto point_buffer = x->mPointBuffer;
        const float *sim_point_data = point_buffer.mBuffer;
        int cur_size = point_buffer.mNumOfEle * sizeof(float);
        memcpy(char_data + offset, sim_point_data, cur_size);
        offset += cur_size;
    }

    // unmap
    vkUnmapMemory(mDevice, mPointDrawBuffer.mMemory);
}

void cDrawScene::SubmitCmdsAndPresent(std::vector<VkCommandBuffer> cmds,
                                      uint32_t imageIndex)
{
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphore[mCurFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //
    submit_info.waitSemaphoreCount =
        1; // how much semaphres does this submission need to wait?
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = waitSemaphores;
    submit_info.pWaitDstStageMask = waitStages;

    submit_info.commandBufferCount = cmds.size();
    submit_info.pCommandBuffers = cmds.data();

    // when the commands are finished, which semaphore do we need to send?
    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphore[mCurFrame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signalSemaphores;

    vkResetFences(mDevice, 1, &minFlightFences[mCurFrame]);
    if (vkQueueSubmit(mGraphicsQueue, 1, &submit_info,
                      minFlightFences[mCurFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
        exit(0);
    }

    // 3. render finish and present the image to the screen
    PresentImageToScreen(signalSemaphores, imageIndex);
}
void cDrawScene::PresentImageToScreen(VkSemaphore *signalSemaphores,
                                      uint32_t imageIndex)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {mSwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);

    // we wait the GPU to finish its work after submitting
    vkQueueWaitIdle(mPresentQueue);
}

/**
 * \brief
 *      Begin the fence
 *      Get the current writing image index, then acquire this image from the
 * swap chain
 */
bool cDrawScene::FenceAndAcquireImageFromSwapchain(uint32_t &imageIndex)
{
    vkWaitForFences(mDevice, 1, &minFlightFences[mCurFrame], VK_TRUE,
                    UINT64_MAX);

    // 1. acquire an image from the swap chain
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX,
                                            mImageAvailableSemaphore[mCurFrame],
                                            VK_NULL_HANDLE, &imageIndex);

    bool ret = false;
    if (result == VK_ERROR_OUT_OF_DATE_KHR || mFrameBufferResized == true)
    {
        // the gGLFWWindow may be resized, we need to recreate it
        ret = true;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        SIM_ERROR("error");
    }

    if (mImagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(mDevice, 1, &mImagesInFlight[imageIndex], VK_TRUE,
                        UINT64_MAX);
    }

    mImagesInFlight[imageIndex] = mImagesInFlight[mCurFrame];
    return ret;
}
bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());
    for (const auto &x : availableExtensions)
    {
        requiredExtensions.erase(x.extensionName);
    }
    for (const auto &x : requiredExtensions)
    {
        std::cout << "physical device lack extension " << x << std::endl;
    }

    // if required extensions are empty, means that all requred extensions are
    // supported, return true;
    return requiredExtensions.empty();
}

void cDrawScene::CleanSwapChain()
{
    vkDestroyImageView(mDevice, mColorImageViewMSAA, nullptr);
    vkDestroyImage(mDevice, mColorImageMSAA, nullptr);
    vkFreeMemory(mDevice, mColorImageMemoryMSAA, nullptr);

    for (auto framebuffer : mSwapChainFramebuffers)
    {
        vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
    }
    vkFreeCommandBuffers(mDevice, mCommandPool,
                         static_cast<uint32_t>(mCommandBuffers.size()),
                         mCommandBuffers.data());
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    vkDestroyPipeline(mDevice, mTriangleGraphicsPipeline, nullptr);
    vkDestroyPipeline(mDevice, mLinesGraphicsPipeline, nullptr);
    vkDestroyPipeline(mDevice, mPointGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

    for (auto imageView : mSwapChainImageViews)
    {
        vkDestroyImageView(mDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);

    for (size_t i = 0; i < mSwapChainImages.size(); i++)
    {
        vkDestroyBuffer(mDevice, mMVPUniformBuffers[i], nullptr);
        vkFreeMemory(mDevice, mMVPUniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
}

struct MVPUniformBufferObject
{
    glm::vec4 camera_pos;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

/**
 * \brief           Create Descriptor set layout
 *      The descriptor is used to store the uniform object used in the shader,
 * we needs to spepcifiy how much uniform objects(descriptors), which is extacly
 * "layout"
 */
void cDrawScene::CreateDescriptorSetLayout()
{
    // given the binding: offer the same info in C++ side as the shaders
    VkDescriptorSetLayoutBinding mvpLayoutBinding{};
    mvpLayoutBinding.binding = 0; // the binding info should be the same
    mvpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mvpLayoutBinding.descriptorCount = 1;
    mvpLayoutBinding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT; // we use the descriptor in vertex shader
    mvpLayoutBinding.pImmutableSamplers = nullptr; // Optional

    // sampler
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding,
    // }; VkDescriptorSetLayoutCreateInfo layoutInfo{}; layoutInfo.sType =
    // VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    // layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    // layoutInfo.pBindings = bindings.data();

    // create the layout
    std::array<VkDescriptorSetLayoutBinding, 2> ubo_set = {
        mvpLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = ubo_set.size();
    layoutInfo.pBindings = ubo_set.data();

    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr,
                                    &mDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    /// layout creation done
}

/**
 * \brief       Create buffer for uniform objects
 */
void cDrawScene::CreateMVPUniformBuffer()
{
    // check the size of uniform buffer object
    VkDeviceSize bufferSize = sizeof(MVPUniformBufferObject);

    // set up the memory
    mMVPUniformBuffers.resize(mSwapChainImages.size());
    mMVPUniformBuffersMemory.resize(mSwapChainImages.size());

    // create each uniform object buffer
    for (size_t i = 0; i < mSwapChainImages.size(); i++)
    {
        CreateBuffer(mDevice, mPhysicalDevice, bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     mMVPUniformBuffers[i], mMVPUniformBuffersMemory[i]);
    }
}

/**
 * \brief           Update the uniform value
 *
 *  calculate the new uniform value and set them to the uniform buffer
 */
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 E2GLM(const tMatrix4 &em)
{
    glm::mat4 mat;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            mat[j][i] = em(i, j);
        }
    }
    return mat;
}
tMatrix4 GLM2E(const glm::mat4 &mat)
{
    tMatrix4 em;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            em(i, j) = mat[j][i];
        }
    }
    return em;
}
void cDrawScene::UpdateMVPUniformValue(int image_idx)
{
    MVPUniformBufferObject ubo{};
    tVector4 cam_pos = GetCameraPos();
    ubo.camera_pos = glm::vec4(cam_pos[0], cam_pos[1], cam_pos[2], 1.0f);
    // std::cout << "camera pos = " << mCamera->pos.transpose() << std::endl;
    ubo.model = glm::mat4(1.0f);
    tMatrix4 eigen_view = mCamera->ViewMatrix();
    ubo.view = E2GLM(eigen_view);
    ubo.proj = E2GLM(mCamera->ProjMatrix(mSwapChainExtent.width,
                                         mSwapChainExtent.height, true));
    // ubo.proj = glm::perspective(glm::radians(mCameraInitFov),
    //                             mSwapChainExtent.width /
    //                                 (float)mSwapChainExtent.height,
    //                             near_plane_dist, far_plane_dist);
    void *data;
    vkMapMemory(mDevice, mMVPUniformBuffersMemory[image_idx], 0, sizeof(ubo), 0,
                &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(mDevice, mMVPUniformBuffersMemory[image_idx]);
}

tVector4 cDrawScene::GetPerturbOrigin(const tVector4 &cursor_world_pos) const
{
    tVector4 perturb_origin = tVector4::Zero();
    switch (mCamera->GetType())
    {
    case eCameraType::ORTHO_CAMERA:
    {
        tVector3 cursor_world_pos_3f = cursor_world_pos.segment(0, 3);
        tVector3 cam_pos = mCamera->GetCameraPos();
        tVector3 cam_front = mCamera->GetCameraFront();

        perturb_origin.segment(0, 3) =
            (cursor_world_pos_3f -
             (cursor_world_pos_3f - cam_pos).dot(cam_front) * cam_front);
        break;
    }
    case eCameraType::ARCBALL_CAMERA:
    case eCameraType::FPS_CAMERA:
    {
        perturb_origin = GetCameraPos();
        break;
    }
    default:
        SIM_ERROR("invalid");
    }
    perturb_origin[3] = 1;
    return perturb_origin;
}

void cDrawScene::UpdateTriangleBufferSimObj(int image_idx)
{
    int num_of_ele = 0;
    for (auto &x : mRenderResourceArray)
    {
        num_of_ele += x->mTriangleBuffer.mNumOfEle;
    }
    VkDeviceSize buffer_size = sizeof(float) * num_of_ele;
    if (buffer_size == 0)
    {
        return;
    }
    if (buffer_size > mSimTriangleBuffer.mSize)
    {
        int new_size = GetNewSize(mSimTriangleBuffer.mSize, buffer_size);
        printf("[warn] tri draw buffer size %d < resource size %d, "
               "reallocate to %d\n",
               mSimTriangleBuffer.mSize, buffer_size, new_size);
        CreateTriangleBufferSim(new_size);
        mBufferReallocated = true;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(mDevice, mPhysicalDevice, buffer_size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    // 5. copy the vertex data to the buffer
    void *data = nullptr;
    // map the memory to "data" ptr;
    vkMapMemory(mDevice, stagingBufferMemory, 0, buffer_size, 0, &data);

    // write the data
    char *char_data = static_cast<char *>(data);
    int idx = 0;
    for (auto &x : mRenderResourceArray)
    {
        int size = x->mTriangleBuffer.mNumOfEle * sizeof(float);
        memcpy(char_data + idx, x->mTriangleBuffer.mBuffer, size);
        idx += size;
    }

    // unmap
    vkUnmapMemory(mDevice, stagingBufferMemory);

    // if mVertexBufferSim is invalid, create it

    CopyBuffer(stagingBuffer, mSimTriangleBuffer.mBuffer, buffer_size);
    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

/**
 * \brief           connect the vkBuffer with the descriptor pool
 */
void cDrawScene::CreateDescriptorPool(VkDescriptorPool &desc_pool,
                                      int num_resources) const
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount =
        num_resources * static_cast<uint32_t>(mSwapChainImages.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount =
        num_resources * static_cast<uint32_t>(mSwapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets =
        num_resources * static_cast<uint32_t>(mSwapChainImages.size());

    if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &desc_pool) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}
#include "scenes/TextureManager.h"
void cDrawScene::CreateDescriptorSets(VkDescriptorSetArray &desc_sets) const
{
    int n_resources = mRenderResourceArray.size();
    // std::cout << "begin to create descriptor set\n";
    // create the descriptor set
    int num_of_desc = n_resources * mSwapChainImages.size();
    std::vector<VkDescriptorSetLayout> layouts(num_of_desc,
                                               mDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = num_of_desc;
    allocInfo.pSetLayouts = layouts.data();

    desc_sets.resize(num_of_desc);
    // printf("[debug] create %d desc sets\n", num_of_desc);
    if (vkAllocateDescriptorSets(mDevice, &allocInfo, desc_sets.data()) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    // create each descriptor

    // mSwapChainImages; mUniformBuffers;
    auto tex_manager = GetTextureManager();
    for (size_t res_id = 0; res_id < n_resources; res_id++)
    {
        auto mat = mRenderResourceArray[res_id]->mMaterialPtr;
        std::string tex_path = (mat == nullptr) ? "" : mat->mTexImgPath;

        int tex_id = tex_manager->FetchTextureId(
            mDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, tex_path);
        cTextureInfoPtr tex_info = tex_manager->GetTextureInfo(tex_id);

        // for uniform code, we must find an alternative texture pointer, in
        // order to avoid the validation error

        if (tex_info == nullptr)
        {
            int num_of_tex = tex_manager->GetNumOfTextures();
            // printf("num of tex %d\n", num_of_tex);
            for (int i = 0; i < num_of_tex; i++)
            {
                std::string tex_name = tex_manager->GetTextureName(i);
                auto cur_tex = tex_manager->GetTextureInfo(i);
                uint size = cur_tex->GetBufSize();
                // printf("tex %d name %s size %d\n", i, tex_name.c_str(),
                // size);
                if (size > 0)
                {
                    tex_info = cur_tex;
                    // printf("select tex %d as an alternative\n", i);
                    break;
                }
            }
        }
        SIM_ASSERT(tex_info != nullptr);

        for (size_t img_id = 0; img_id < mSwapChainImages.size(); img_id++)
        {
            // printf("---------------%d----------------\n", i);
            VkDescriptorBufferInfo mvpbufferinfo{};
            mvpbufferinfo.buffer = mMVPUniformBuffers[img_id];
            mvpbufferinfo.offset = 0;
            mvpbufferinfo.range = sizeof(MVPUniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            // if (tex_info != nullptr)
            // set up texture image info
            {
                imageInfo.imageLayout =
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                imageInfo.imageView = tex_info->mTextureImageView;
                imageInfo.sampler = mTextureSampler;
            }

            // we have n_groups * n_swap descriptors, but we need to write two
            // times
            std::vector<VkWriteDescriptorSet> descriptorWrites(2);
            int desc_set_id = res_id * mSwapChainImages.size() + img_id;
            // printf("for group %d, swap chain %d, update desc set %d\n", g_id,
            //        img_id, desc_set_id);
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = desc_sets[desc_set_id];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &mvpbufferinfo;

            // set for texture
            // if (tex_info != nullptr)
            {
                descriptorWrites[1].sType =
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = desc_sets[desc_set_id];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo = &imageInfo;
            }
            vkUpdateDescriptorSets(mDevice, descriptorWrites.size(),
                                   descriptorWrites.data(), 0, nullptr);
        }
    }
    // std::cout << "end to create descriptor set\n";
    // exit(0);
}

/**
 * \brief           Create and fill the command buffer
 */
void cDrawScene::CreateCommandBuffers()
{
    // 1. create the command buffers
    mCommandBuffers.resize(mSwapChainFramebuffers.size());
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = mCommandPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = (uint32_t)mCommandBuffers.size();
    SIM_ASSERT(vkAllocateCommandBuffers(mDevice, &info,
                                        mCommandBuffers.data()) == VK_SUCCESS);

    // 2. record the command buffers
    for (int i = 0; i < mCommandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pInheritanceInfo = nullptr;
        beginInfo.flags = 0;
        SIM_ASSERT(VK_SUCCESS ==
                   vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo));

        // 3. start a render pass
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mRenderPass;
        renderPassInfo.framebuffer = mSwapChainFramebuffers[i];

        renderPassInfo.renderArea.extent = mSwapChainExtent;
        renderPassInfo.renderArea.offset = {0, 0};

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount =
            static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // full black clear color
        // VkClearValue clear_color = {1.0f, 1.0f, 1.0f, 1.0f};
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        // begin render pass
        vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        CreateTriangleCommandBuffers(i);
        CreateLineCommandBuffers(i);
        CreatePointCommandBuffers(i);

        // end render pass
        vkCmdEndRenderPass(mCommandBuffers[i]);

        SIM_ASSERT(VK_SUCCESS == vkEndCommandBuffer(mCommandBuffers[i]));
    }
    // SIM_INFO("Create Command buffers succ");
}
tMaterialPushConstant::tMaterialPushConstant()
{

    Ka = glm::vec4(1, 1, 1, 1);
    Kd = glm::vec4(1, 1, 1, 1);
    Ks = glm::vec4(1, 1, 1, 1);

    Ns = 64;
    enable_texture = false;
    enable_phongmodel = true;
    enable_basic_color = false;
}
void cDrawScene::CreateTriangleCommandBuffers(int img_id)
{
    vkCmdBindPipeline(mCommandBuffers[img_id], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      mTriangleGraphicsPipeline);

    /*
        different swapchain_img_id
            for different group (different texture)
                1. different descriptor set
                2. different offset
                3. different number of ele
    */
    uint cur_off = 0;
    int n_resources = mRenderResourceArray.size();
    mCmdBufferRecordInfo.mCmdBufferRecorded_tri_size.resize(n_resources);
    int render_id = 0;
    for (int res_id = 0; res_id < n_resources; res_id++)
    {
        VkBuffer vertexBuffers[] = {mSimTriangleBuffer.mBuffer};
        // VkBuffer vertexBuffers[] = {, mVertexBufferSim};
        VkDeviceSize offsets[] = {cur_off * sizeof(float)};
        vkCmdBindVertexBuffers(mCommandBuffers[img_id], 0, 1, vertexBuffers,
                               offsets);

        // update the uniform objects (descriptors)

        uint desc_set_id = res_id * mSwapChainImages.size() + img_id;
        // printf("for grp %d swap chain %d, use desc set %d to draw
        // triangle\n",
        //        grp_id, img_id, desc_set_id);
        vkCmdBindDescriptorSets(
            mCommandBuffers[img_id], VK_PIPELINE_BIND_POINT_GRAPHICS,
            mPipelineLayout, 0, 1, &(mDescriptorSets[desc_set_id]), 0, nullptr);

        int num_of_ele_in_buf =
            mRenderResourceArray[res_id]->mTriangleBuffer.mNumOfEle;
        int num_of_v = num_of_ele_in_buf / RENDERING_SIZE_PER_VERTICE;
        // std::cout << "num of ele in buf " << num_of_ele_in_buf << std::endl;
        // std::cout << "num of v in buf " << num_of_v << std::endl;
        {
            auto cur_render_res = mRenderResourceArray[res_id];
            tMaterialPushConstant constants;
            if (cur_render_res->mMaterialPtr == nullptr)
            {
                constants.Ka = glm::vec4(1, 1, 1, 1);
                constants.Kd = glm::vec4(1, 1, 1, 1);
                constants.Ks = glm::vec4(1, 1, 1, 1);
                constants.Ns = 64;
                constants.enable_texture = false;
                constants.enable_phongmodel = false;
                constants.enable_basic_color = false;
            }
            else
            {
                auto mat = cur_render_res->mMaterialPtr;

                constants.Ka = glm::vec4(mat->Ka[0], mat->Ka[1], mat->Ka[2], 1);
                constants.Kd = glm::vec4(mat->Kd[0], mat->Kd[1], mat->Kd[2], 1);
                constants.Ks = glm::vec4(mat->Ks[0], mat->Ks[1], mat->Ks[2], 1);
                constants.Ns = mat->Ns;
                constants.enable_phongmodel = true;
                constants.enable_basic_color = false;
                constants.enable_texture =
                    cur_render_res->mMaterialPtr->mEnableTexutre;
                // std::cout << "Kd = " << mat->Kd.transpose() << std::endl;
            }
            vkCmdPushConstants(mCommandBuffers[img_id], mPipelineLayout,
                               VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(tMaterialPushConstant), &constants);

            vkCmdDraw(mCommandBuffers[img_id], num_of_v, 1, 0, 0);
            cur_off += num_of_ele_in_buf;
        }
        // draaaaaaaaaaaaaaaaaaaaaaaaaaaaw!
        // uint32_t triangle_size =  / 3;
        // const float *draw_buffer = mRenderResource->mTriangleBuffer.mBuffer;

        // vk cmd draw, num of triangles' vertices = tri_buf_size /
        // RENDERING_PER_VERTEX;
        // printf("draw group %d, num tri buffer size %d, num of vertices %d, "
        //        "offset %d\n",
        //        grp_id, group_info_arr[grp_id].mNumTriBufferSize, cur_num_ele,
        //        cur_off);

        // old
        // {
        //     auto cur_grp_info = group_info_arr[grp_id];
        //     for (auto render_res_ind : cur_grp_info.mRenderResourceId)
        //     {
        //         auto cur_render_res = mRenderResourceArray[render_res_ind];

        //         std::cout << "buf size = "
        //                   << cur_render_res->mTriangleBuffer.mNumOfEle
        //                   << std::endl;
        //         int cur_num_vertex =
        //         cur_render_res->mTriangleBuffer.mNumOfEle /
        //                              RENDERING_SIZE_PER_VERTICE;
        //         int cur_num_tri = cur_num_vertex / 3;

        //         for (int i = 0; i < cur_num_tri; i++)
        //         {
        //             int bias = i * 3 * RENDERING_SIZE_PER_VERTICE;
        //             printf(
        //                 "v0 %.4f %.4f %.4f v1 %.4f %.4f %.4f v2 %.4f %.4f "
        //                 "%.4f\n",
        //                 cur_render_res->mTriangleBuffer.mBuffer[bias + 0],
        //                 cur_render_res->mTriangleBuffer.mBuffer[bias + 1],
        //                 cur_render_res->mTriangleBuffer.mBuffer[bias + 2],

        //                 cur_render_res->mTriangleBuffer
        //                     .mBuffer[bias + 0 + RENDERING_SIZE_PER_VERTICE],
        //                 cur_render_res->mTriangleBuffer
        //                     .mBuffer[bias + 1 + RENDERING_SIZE_PER_VERTICE],
        //                 cur_render_res->mTriangleBuffer
        //                     .mBuffer[bias + 2 + RENDERING_SIZE_PER_VERTICE],

        //                 cur_render_res->mTriangleBuffer
        //                     .mBuffer[bias + 0 + 2 *
        //                     RENDERING_SIZE_PER_VERTICE],
        //                 cur_render_res->mTriangleBuffer
        //                     .mBuffer[bias + 1 + 2 *
        //                     RENDERING_SIZE_PER_VERTICE],
        //                 cur_render_res->mTriangleBuffer
        //                     .mBuffer[bias + 2 +
        //                              2 * RENDERING_SIZE_PER_VERTICE]);
        //         }
        //         if (cur_num_vertex == 0)
        //             continue;
        //         // mRenderResourceArray
        //         tMaterialPushConstant constants;

        //         if (cur_render_res->mMaterialPtr == nullptr)
        //         {
        //             constants.Ka = glm::vec4(1, 1, 1, 1);
        //             constants.Kd = glm::vec4(1, 1, 1, 1);
        //             constants.Ks = glm::vec4(1, 1, 1, 1);
        //             constants.Ns = 64;
        //             constants.enable_texture = false;
        //         }
        //         else
        //         {
        //             auto mat = cur_render_res->mMaterialPtr;

        //             constants.Ka =
        //                 glm::vec4(mat->Ka[0], mat->Ka[1], mat->Ka[2], 1);
        //             constants.Kd =
        //                 glm::vec4(mat->Kd[0], mat->Kd[1], mat->Kd[2], 1);
        //             constants.Ks =
        //                 glm::vec4(mat->Ks[0], mat->Ks[1], mat->Ks[2], 1);
        //             constants.Ns = mat->Ns;
        //             constants.enable_phongmodel = true;
        //             constants.enable_texture =
        //                 cur_render_res->mMaterialPtr->mEnableTexutre;
        //         }
        //         vkCmdPushConstants(mCommandBuffers[img_id], mPipelineLayout,
        //                            VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        //                            sizeof(tMaterialPushConstant),
        //                            &constants);
        //         printf("resource %d draw vertices %d\n", render_id++,
        //                cur_num_vertex);
        //         printf("0------\n");
        //         vkCmdDraw(mCommandBuffers[img_id], cur_num_vertex, 1, 0, 0);
        //         cur_off += cur_render_res->mTriangleBuffer.mNumOfEle;
        //     }
        // }
    }
    // get num of triangle buffer
    // int num_of_ele_tri = GetNumOfTriangleVertices();
    // if (num_of_ele_tri > 0)
    // {

    // }
}

void cDrawScene::AddRenderResource(
    std::vector<cRenderResourcePtr> render_resource_array)
{
    mRenderResourceArray.insert(mRenderResourceArray.end(),
                                render_resource_array.begin(),
                                render_resource_array.end());
}

void cDrawScene::ClearRenderResource() { mRenderResourceArray.clear(); }

bool cDrawScene::NeedRecreateCommandBuffers()
{
    // printf("cmd buffer recreated check: prev %d %d %d now %d %d %d, buffer "
    //        "reallocated %d\n",
    //        mCmdBufferRecordInfo.mCmdBufferRecorded_line_size,
    //        mCmdBufferRecordInfo.mCmdBufferRecorded_tri_size,
    //        mCmdBufferRecordInfo.mCmdBufferRecorded_point_size,
    //        GetNumOfLineVertices(), GetNumOfTriangleVertices(),
    //        GetNumOfDrawPoints(), mBufferReallocated

    // );
    // check current buffer size, and previously recorded buffer size
    return true;
    // return mBufferReallocated == true ||
    //        mCmdBufferRecordInfo.mCmdBufferRecorded_line_size !=
    //            GetNumOfLineVertices() ||
    //        mCmdBufferRecordInfo.mCmdBufferRecorded_tri_size !=
    //            GetNumOfTriangleVertices() ||
    //        mCmdBufferRecordInfo.mCmdBufferRecorded_point_size !=
    //            GetNumOfDrawPoints();
}
void cDrawScene::DestoryCommandBuffers() {}