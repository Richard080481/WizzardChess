#ifndef __WIZARD_CHESS_H__
#define __WIZARD_CHESS_H__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

#include "Model.h"

#include "VulkanSurfaceManager.h"
#include "MemoryTracker.h"

#define SHADOW_MAP_WIDTH  2048
#define SHADOW_MAP_HEIGHT 2048

enum EModelName : unsigned int
{
    Cube   = 0,
    Bishop = 1,
    King   = 2,
    Knight = 3,
    Rook   = 4,
    Pawn   = 5,
    Queen  = 6,
};

class WizardChess
{
public:
    WizardChess(int width, int height) : m_width(width), m_height(height) {}
    ~WizardChess();

    void run();
    void SetFramebufferResized()
    {
        m_framebufferResized = true;
    }

    int WindowWidth()  const { return m_width; }
    int WindowHeight() const { return m_height; }

private:
    void     InitVulkan();
    void     MainLoop();
    void     CleanupSwapChain();
    void     Cleanup();
    void     RecreateSwapChain();
    void     CreateRenderPass();
    void     CreateDescriptorSetLayout();
    void     CreateGraphicsPipeline();
    void     CreateShadowPassGraphicsPipeline();
    void     CreateSwapChainFramebuffers();
    void     CreateShadowPassFramebuffers();
    void     CreateDepthResources();
    void     CreateShadowMapResources();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();
    bool     HasStencilComponent(VkFormat format);
    void     CreateTextureImage();
    void     CreateTextureImageView();
    void     CreateTextureSampler();
    void     CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void     TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void     LoadModel();
    void     CreateUniformBuffers();
    void     CreateDescriptorPool();
    void     CreateDescriptorSets();
    void     RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void     CreateSyncObjects();
    void     UpdateUniformBuffer(uint32_t currentImage);
    void     DrawFrame();
    void     SaveImageAsBMP(VkDevice device, VkImage image, VkFormat format, VkImageLayout oldLayout, int width, int height, std::string fileName);

    int m_width;
    int m_height;

    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    VkFramebuffer           m_shadowPassFramebuffer         = VK_NULL_HANDLE;

    VkRenderPass            m_renderPass                    = VK_NULL_HANDLE;
    VkDescriptorSetLayout   m_descriptorSetLayout           = VK_NULL_HANDLE;
    VkPipelineLayout        m_pipelineLayout                = VK_NULL_HANDLE;
    VkPipeline              m_graphicsPipeline              = VK_NULL_HANDLE;

    VkImage                 m_depthImage                    = VK_NULL_HANDLE;
    VkDeviceMemory          m_depthImageMemory              = VK_NULL_HANDLE;
    VkImageView             m_depthImageView                = VK_NULL_HANDLE;

    VkImage                 m_textureImage                  = VK_NULL_HANDLE;
    VkDeviceMemory          m_textureImageMemory            = VK_NULL_HANDLE;
    VkImageView             m_textureImageView              = VK_NULL_HANDLE;
    VkSampler               m_textureSampler                = VK_NULL_HANDLE;

    const VkFormat          m_shadowMapFormat               = VK_FORMAT_D32_SFLOAT;
    const VkExtent2D        m_shadowMapExtent               = { SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT };
    VkRenderPass            m_shadowRenderPass              = VK_NULL_HANDLE;
    VkDescriptorSetLayout   m_shadowPassDescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout        m_shadowPassPipelineLayout      = VK_NULL_HANDLE;
    VkPipeline              m_shadowPassGraphicsPipeline    = VK_NULL_HANDLE;
    VkImage                 m_shadowImage                   = VK_NULL_HANDLE;
    VkDeviceMemory          m_shadowImageMemory             = VK_NULL_HANDLE;
    VkImageView             m_shadowImageView               = VK_NULL_HANDLE;
    VkSampler               m_shadowImageSampler            = VK_NULL_HANDLE;

    std::unordered_map<EModelName, Model*>  m_uniqueModels;

    std::vector<VkBuffer>       m_vsUniformBuffers;
    std::vector<VkDeviceMemory> m_vsUniformBuffersMemory;
    std::vector<void*>          m_vsUniformBuffersMapped;

    std::vector<VkBuffer>       m_fsUniformBuffers;
    std::vector<VkDeviceMemory> m_fsUniformBuffersMemory;
    std::vector<void*>          m_fsUniformBuffersMapped;

    std::vector<VkBuffer>       m_shadowVsUniformBuffers;
    std::vector<VkDeviceMemory> m_shadowVsUniformBuffersMemory;
    std::vector<void*>          m_shadowVsUniformBuffersMapped;

    VkDescriptorPool             m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkDescriptorSet> m_shadowPassDescriptorSets;

    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence>     m_inFlightFences;
    uint32_t                 m_currentFrame = 0;
    uint32_t                 m_currentImage = 0;    // Which swap chain image is being rendered

    bool m_framebufferResized = false;
};

#endif // __WIZARD_CHESS_H__