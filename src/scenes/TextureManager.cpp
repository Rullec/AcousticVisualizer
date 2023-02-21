#include "TextureManager.h"
#include "utils/FileUtil.h"
#include "utils/LogUtil.h"
#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"
#include "vulkan/vulkan.h"

extern void CreateBuffer(VkDevice mDevice, VkPhysicalDevice mPhysicalDevice,
                         VkDeviceSize size, VkBufferUsageFlags usage,
                         VkMemoryPropertyFlags props, VkBuffer &buffer,
                         VkDeviceMemory &buffer_memory);
extern void CreateImage(VkDevice device, VkPhysicalDevice phy_device,
                        uint32_t width, uint32_t height,
                        VkSampleCountFlagBits num_samples, VkFormat format,
                        VkImageTiling tiling, VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties, VkImage &image,
                        VkDeviceMemory &imageMemory);
extern void transitionImageLayout(VkDevice device, VkQueue graphicsQueue,
                                  VkCommandPool commandPool, VkImage image,
                                  VkFormat format, VkImageLayout oldLayout,
                                  VkImageLayout newLayout);
extern void copyBufferToImage(VkDevice device, VkCommandPool commandPool,
                              VkQueue graphicsQueue, VkBuffer buffer,
                              VkImage image, uint32_t width, uint32_t height);
extern VkImageView CreateImageView(VkDevice device, VkImage image,
                                   VkFormat format,
                                   VkImageAspectFlags aspectFlags);
cTextureInfo::cTextureInfo(VkDevice device, VkPhysicalDevice phy_dev,
                           VkCommandPool commandPool, VkQueue graphicsQueue,
                           const std::string &path)
{
    mDevice = device;
    // 1. load the image from the file
    int tex_width, tex_height, tex_channels;
    // std::string mGroundPNGPath = "";
    SIM_ASSERT(cFileUtil::ExistsFile(path));
    stbi_uc *pixels = stbi_load(path.c_str(), &tex_width, &tex_height,
                                &tex_channels, STBI_rgb_alpha);
    tex_channels = 4;
    // if ( != 4)
    // {
    //     SIM_ERROR("texture file {} has {} channels, invalid", path,
    //               tex_channels);
    //     exit(1);
    // }

    VkFormat img_format = VK_FORMAT_R8G8B8A8_SRGB;

    int size = tex_width * tex_height * tex_channels;
    this->mSize[0] = tex_width;
    this->mSize[1] = tex_height;
    this->mSize[2] = tex_channels;

    VkDeviceSize image_size = tex_width * tex_height * tex_channels;
    VkDeviceMemory mTextureImageMemory;
    // create image and image view
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(device, phy_dev, image_size,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory);

        void *data = nullptr;
        vkMapMemory(device, stagingBufferMemory, 0, image_size, 0, &data);
        memcpy(data, pixels, size * sizeof(unsigned char));
        vkUnmapMemory(device, stagingBufferMemory);

        // 3. create & allocate the texture image
        // VK_FORMAT_R8G8B8_SRGB
        CreateImage(device, phy_dev, tex_width, tex_height,
                    VK_SAMPLE_COUNT_1_BIT, img_format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mTextureImage,
                    mTextureImageMemory);

        // 4. send the staging buffer to the image? how to do that?
        transitionImageLayout(device, graphicsQueue, commandPool, mTextureImage,
                              img_format, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(device, commandPool, graphicsQueue, stagingBuffer,
                          mTextureImage, static_cast<uint32_t>(tex_width),
                          static_cast<uint32_t>(tex_height));
        transitionImageLayout(device, graphicsQueue, commandPool, mTextureImage,
                              img_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
    {
        mTextureImageView = CreateImageView(device, mTextureImage, img_format,
                                            VK_IMAGE_ASPECT_COLOR_BIT);
    }

    stbi_image_free(pixels);
}

uint cTextureInfo::GetBufSize() const
{
    return uint(mSize[0] * mSize[1] * mSize[2]);
}
void cTextureInfo::Release()
{
    printf("release tex, size %d\n", GetBufSize());
    vkDestroyImageView(mDevice, mTextureImageView, nullptr);
    vkDestroyImage(mDevice, mTextureImage, nullptr);
    vkFreeMemory(mDevice, mTextureImageMemory, nullptr);
}
cTextureManager::cTextureManager()
{
    mTexName2Id.clear();
    mId2TexName.clear();
    mTextureInfoArray.clear();
}

// get or register just in time
int cTextureManager::FetchTextureId(VkDevice device, VkPhysicalDevice phy_dev,
                                    VkCommandPool commandPool,
                                    VkQueue graphicsQueue, std::string tex_name)
{
    if (tex_name.size() == 0)
    {
        return -1;
    }
    auto it = mTexName2Id.find(tex_name);
    if (it == mTexName2Id.end())
    {
        // creat new one
        mId2TexName.push_back(tex_name);

        // load texture
        mTextureInfoArray.push_back(std::make_shared<cTextureInfo>(
            device, phy_dev, commandPool, graphicsQueue, tex_name));
        mTexName2Id[tex_name] = mTextureInfoArray.size() - 1;
        return mTextureInfoArray.size() - 1;
    }
    else
    {
        return it->second;
    }
}

std::string cTextureManager ::GetTextureName(int tex_id)
{
    if (tex_id >= mId2TexName.size())
    {
        SIM_ERROR("tex id {} is invalid", tex_id);
        exit(1);
    }
    return mId2TexName[tex_id];
}

cTextureInfoPtr cTextureManager::GetTextureInfo(int tex_id)
{
    if (tex_id < 0 || tex_id >= this->mTextureInfoArray.size())
        return nullptr;
    else
        return this->mTextureInfoArray[tex_id];
}

static cTextureManagerPtr gTextureManager = nullptr;
cTextureManagerPtr GetTextureManager()
{
    if (gTextureManager == nullptr)
    {
        gTextureManager = std::make_shared<cTextureManager>();
    }
    return gTextureManager;
}

int cTextureManager::GetNumOfTextures() const
{
    return this->mTextureInfoArray.size();
}

void cTextureManager::Clear()
{
    mTexName2Id.clear();
    mId2TexName.clear();
    for (auto &x : mTextureInfoArray)
    {
        x->Release();
    }
    mTextureInfoArray.clear();
}