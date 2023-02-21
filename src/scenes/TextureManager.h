#pragma once
#include "utils/DefUtil.h"
#include "utils/EigenUtil.h"
#include "vulkan/vulkan.h"
#include <map>
#include <vector>
typedef Eigen::Matrix<unsigned char, Eigen::Dynamic, 1> tVectorXuc;

/*
Load, Get and mange texture buffer in the same place
*/
struct cTextureInfo
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    cTextureInfo(VkDevice device, VkPhysicalDevice phy_dev,
                 VkCommandPool commandPool, VkQueue graphicsQueue,
                 const std::string &path);
    uint GetBufSize() const;
    void Release();
    // tVectorXuc mData;
    tVector3 mSize; // width, height, channels
    VkDevice mDevice;
    VkImage mTextureImage;
    VkImageView mTextureImageView;
    VkDeviceMemory mTextureImageMemory;
};
SIM_DECLARE_STRUCT_AND_PTR(cTextureInfo);

class cTextureManager
{
public:
    cTextureManager();
    int FetchTextureId(VkDevice device, VkPhysicalDevice phy_dev,
                       VkCommandPool commandPool, VkQueue graphicsQueue,
                       std::string tex_name); // get or register just in time

    std::string GetTextureName(int tex_id);
    cTextureInfoPtr GetTextureInfo(int tex_id);
    int GetNumOfTextures() const;
    virtual void Clear();
protected:
    std::map<std::string, int> mTexName2Id;
    std::vector<std::string> mId2TexName;
    tEigenArr<cTextureInfoPtr> mTextureInfoArray;
};

SIM_DECLARE_CLASS_AND_PTR(cTextureManager);
cTextureManagerPtr GetTextureManager();
