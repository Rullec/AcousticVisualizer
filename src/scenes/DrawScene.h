#pragma once
#include "Scene.h"
#include "utils/DefUtil.h"
#include "utils/RenderUtil.h"
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#define SIM_VK_VERTEX_POS_SIZE 3
#define SIM_VK_VERTEX_COLOR_SIZE 4
#define SIM_VK_VERTEX_NORMAL_SIZE 3
#define SIM_VK_VERTEX_TEX_SIZE 2
#define SIM_VK_VERTEX_POS_OFFSET_BYTE 0
#define SIM_VK_VERTEX_COLOR_OFFSET_BYTE                                        \
    (SIM_VK_VERTEX_POS_OFFSET_BYTE + sizeof(float) * SIM_VK_VERTEX_POS_SIZE)
#define SIM_VK_VERTEX_NORMAL_OFFSET_BYTE                                       \
    (SIM_VK_VERTEX_COLOR_OFFSET_BYTE + sizeof(float) * SIM_VK_VERTEX_COLOR_SIZE)
#define SIM_VK_VERTEX_TEX_OFFSET_BYTE                                          \
    (SIM_VK_VERTEX_NORMAL_OFFSET_BYTE +                                        \
     sizeof(float) * SIM_VK_VERTEX_NORMAL_SIZE)
#include "glm/glm.hpp"
struct tMaterialPushConstant
{
    tMaterialPushConstant();
    glm::vec4 Ka, Kd, Ks;
    float Ns;
    int32_t enable_texture;
    int32_t enable_phongmodel;
    int32_t enable_basic_color;

};

// #define SIM_VK_VERTEX_BUFFER_SIZE (10000 * 10000)
// #define SIM_VK_LINE_BUFFER_SIZE (10000 * 10000)
// #define SIM_VK_POINT_BUFFER_SIZE (10000 * 10000)
SIM_DECLARE_CLASS_AND_PTR(cRenderGrid);
struct tVkVertex
{
    float pos[SIM_VK_VERTEX_POS_SIZE];
    float color[SIM_VK_VERTEX_COLOR_SIZE];
    float normal[SIM_VK_VERTEX_NORMAL_SIZE];
    float texCoord[SIM_VK_VERTEX_TEX_SIZE];
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 4>
    getAttributeDescriptions();
};

// which queues do we want to support?
struct QueueFamilyIndices
{
    std::optional<uint32_t>
        graphicsFamily; // here we use optional, because any unit value would be
                        // valid and we need to distinguish from non-value case
    std::optional<uint32_t>
        presentFamily; // the ability to show the image to the screen
    /**
     * \brief           Judge: can we use this queue family?
     */
    bool IsComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct tVkDrawBuffer
{
    tVkDrawBuffer();
    VkBuffer mBuffer;
    bool mIsCreated;
    VkDeviceMemory mMemory;
    VkDeviceSize mSize;
    void DestroyAndFree(VkDevice device);
};

/**
 * \brief			Main Vulkan Draw Scene for acoustic simulator
 */
class cSimScene;
typedef std::vector<VkDescriptorSet> VkDescriptorSetArray;
SIM_DECLARE_STRUCT_AND_PTR(CameraBase);
class cDrawScene : public cScene
{
public:
    inline const static std::string GROUND_PNG_PATH_KEY = "ground_png_path";
    explicit cDrawScene();
    virtual ~cDrawScene();
    virtual void Init(const std::string &conf_path) override;
    virtual void
    AddRenderResource(std::vector<cRenderResourcePtr> render_resource);
    virtual void ClearRenderResource();
    virtual void Update(_FLOAT dt) override;
    void MainLoop();
    void Resize(int w, int h);
    virtual void CursorMove(int xpos, int ypos);
    virtual void MouseButton(int button, int action, int mods);
    void Key(int key, int scancode, int action, int mods);
    virtual void Scroll(float xoff, float yoff);
    virtual void Reset() override;
    tVector4 CalcCursorPointWorldPos() const;
    tVector4 GetCameraPos() const;
    static bool IsMouseRightButton(int glfw_button);
    static bool IsRelease(int glfw_action);
    static bool IsPress(int glfw_action);

protected:
    virtual void InitVulkan();
    virtual void DrawFrame();
    virtual bool FenceAndAcquireImageFromSwapchain(uint32_t &imageIndex);
    virtual void SubmitCmdsAndPresent(std::vector<VkCommandBuffer> cmds,
                                      uint32_t imageIndex);
    virtual void PresentImageToScreen(VkSemaphore *signalSemaphores,
                                      uint32_t imageIndex);

    void CleanVulkan();
    VkCommandBuffer CreateCommandBufferTool(VkCommandBufferLevel level,
                                            VkCommandPool pool, bool begin);
    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue,
                            VkCommandPool pool, bool free = true);
    void ScreenShotDraw(std::string path);
    virtual void CreateInstance();
    void CheckAvaliableExtensions() const;
    bool CheckValidationLayerSupport() const;
    void SetupDebugMessenger();
    void PickPhysicalDevice();
    virtual void CreateLogicalDevice();
    void CreateSurface();
    void CreateSwapChain();
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateImageViews(); // create "view" for swap chain images
    // void CreateSingleImageView(VkImage image, VkFormat format);
    void CreateGraphicsPipeline(const std::string mode, VkPipeline &pipeline);
    VkShaderModule CreateShaderModule(const std::vector<char> &code);
    void CreateRenderPass();
    void CreateFrameBuffers();
    void CreateDepthResources();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateTriangleCommandBuffers(int buffer_id);
    void CreateLineCommandBuffers(int buffer_id);
    void CreatePointCommandBuffers(int buffer_id);
    void CreateSemaphores();
    void RecreateSwapChain();
    virtual void CleanSwapChain();

    void CreateTriangleBufferSim(int size = 65536);
    void CreateLineBuffer(int size = 65536);
    void CreatePointBuffer(int size = 65536);
    void CreateMVPUniformBuffer();
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    // void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    //                   VkMemoryPropertyFlags props, VkBuffer &buffer,
    //                   VkDeviceMemory &buffer_memory);
    void CreateDescriptorSetLayout();
    void CreateColorResources(); // for MSAA
    void UpdateMVPUniformValue(int image_idx);
    void UpdateTriangleBufferSimObj(int idx);
    void UpdateLineBuffer(int idx);
    void UpdatePointBuffer(int idx);

    void CreateDescriptorPool(VkDescriptorPool &desc_pool,
                              int num_of_desc) const;
    void CreateDescriptorSets(VkDescriptorSetArray &desc_sets) const;

    int GetNumOfTriangleVertices() const;
    int GetNumOfLineVertices() const;
    int GetNumOfDrawPoints() const;
    tVector4 GetPerturbOrigin(const tVector4 &cursor_world_pos) const;

    VkInstance mInstance;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;       // logical device
    VkSurfaceKHR mSurface;  // window surface
    VkQueue mGraphicsQueue; // device queue (only one)
    VkQueue mPresentQueue;
    VkSwapchainKHR mSwapChain; // swapchain, literally frame buffer
    std::vector<VkImage> mSwapChainImages;
    std::vector<VkImageView> mSwapChainImageViews;
    VkFormat mSwapChainImageFormat;
    VkExtent2D mSwapChainExtent;
    VkDescriptorSetLayout mDescriptorSetLayout; // descriptors (uniform objects)
                                                // layout used in the shader
    VkPipelineLayout mPipelineLayout;           // uniform values in the shader
    VkRenderPass mRenderPass; // special settings for a render pass
    VkPipeline mTriangleGraphicsPipeline, mLinesGraphicsPipeline,
        mPointGraphicsPipeline;
    std::vector<VkFramebuffer> mSwapChainFramebuffers; //
    VkCommandPool mCommandPool;
    std::vector<VkCommandBuffer>
        mCommandBuffers; // each command buffer(queue) serves a frame buffer

    std::vector<VkSemaphore>
        mImageAvailableSemaphore; // the image acquired from the swap chain is
                                  // ready to be rendered to
    std::vector<VkSemaphore>
        mRenderFinishedSemaphore; // the rendering done, the image can be sent
                                  // back to the swap chain for presentation on
                                  // the screen.
    std::vector<VkFence>
        minFlightFences; // fences to do CPU-GPU synchronization
    std::vector<VkFence> mImagesInFlight;
    int mCurFrame;
    bool mFrameBufferResized;
    bool mAddNoiseWhenReset =
        false; // add noise when the simulation is reset or not

    tVkDrawBuffer mSimTriangleBuffer, mLineBuffer, mPointDrawBuffer;
    bool mBufferReallocated;
    // buffers used for uniform objects
    std::vector<VkBuffer> mMVPUniformBuffers;             // MVP uniform buffer
    std::vector<VkDeviceMemory> mMVPUniformBuffersMemory; // their memories
    VkDescriptorPool mDescriptorPool;

    VkDescriptorSetArray mDescriptorSets; // real descriptor, [g_id *
                                          // mSwapChainImages.size() + img_id]
    CameraBasePtr mCamera;

    struct
    {
        std::vector<VkDeviceSize> mCmdBufferRecorded_tri_size;
        VkDeviceSize mCmdBufferRecorded_line_size,
            mCmdBufferRecorded_point_size;
    } mCmdBufferRecordInfo;
    // add MSAA support (big buffer)
    VkImage mColorImageMSAA; // storage the MSAA image
    VkDeviceMemory mColorImageMemoryMSAA;
    VkImageView mColorImageViewMSAA;

    // add ground texture image
    // VkImage mTextureImage;
    // VkImageView mTextureImageView;

    // add depth attachment
    VkImage mDepthImage;
    VkDeviceMemory mDepthImageMemory;
    VkImageView mDepthImageView;

    VkSampler mTextureSampler;

    // MSAA (Multisampling anti-aliasing)
    VkSampleCountFlagBits mSampleCount = VK_SAMPLE_COUNT_1_BIT;

    bool mLeftButtonPress;
    bool mMiddleButtonPress;
    // tVector3 mCameraInitPos, mCameraInitFocus;
    // float mCameraInitFov;
    std::string mGroundPNGPath;
    bool mEnableAxes;
    bool mEnableGround;
    float mGroundHeight;
    bool mSceneIsTheLastRenderPass;
    bool mEnableCamAutoRot; // rotate the camera
    bool mEnableGrid;
    cRenderGridPtr mGrid;
    bool mSingleFrameSimulationMode = false;
    std::vector<cRenderResourcePtr> mRenderResourceArray;

    // cRenderResourceGrouperPtr
    //     mRenderResourceGrouper; // grouping the render resources
    virtual bool NeedRecreateCommandBuffers();
    virtual void DestoryCommandBuffers();
    virtual void SetGroundHeight(float val);
};
SIM_DECLARE_PTR(cDrawScene);