#ifndef __VULKAN_DEVICE_MANAGER_H__
#define __VULKAN_DEVICE_MANAGER_H__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <cassert>
#include <iostream>
#include <optional>

#include "VulkanSurfaceManager.h"

#define VK (*g_pVk)

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VulkanSurfaceManager;

class VulkanDeviceManager
{
public:
    VulkanDeviceManager() = default;
    ~VulkanDeviceManager();

    void Destroy();

    VkInstance       Instance()       const { return m_instance; }
    VkDevice         Device()         const { return m_device; }
    VkPhysicalDevice PhysicalDevice() const { return m_physicalDevice; }
    VkQueue          GraphicsQueue()  const { return m_graphicsQueue; }
    VkQueue          PresentQueue()   const { return m_presentQueue; }
    VkCommandPool    CommandPool()    const { return m_commandPool; }

    VulkanSurfaceManager* SurfaceManager() const { return m_pSurfaceManager; }

    void DestroyValidationLayerNames();
    void EnableValidationLayers(bool enableValidationLayers, const std::vector<const char*>* pValidationLayers);

    void DestroyDeviceExtensionNames();
    void EnableDeviceExtensions(const std::vector<const char*>* pDeviceExtension);

    void CreateGlfwWindow(int width, int height);
    void CreateSurface();

    std::vector<const char*> GetRequiredExtensions();
    void CreateInstance();
    void DestroyInstance();
    void SetupDebugMessenger();

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void DestroyLogicalDevice();

    void CreateSwapChain();
    void DestroySwapChain();

    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    void CreateCommandPool();
    void DestroyCommandPool();
    void CreateCommandBuffers(
        VkCommandBuffer*     pCommandBuffers,
        uint32_t             count = 1,
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBuffer BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void EndSingleTimeCommands(
        VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commandBuffer;

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphicsQueue);

        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
    }

    static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

private:
    VkInstance       m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice         m_device = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

    VulkanSurfaceManager* m_pSurfaceManager = nullptr;

    bool                     m_enableValidationLayers = false;
    std::vector<const char*> m_validationLayers;
    std::vector<const char*> m_deviceExtensions;
};

extern VulkanDeviceManager* g_pVk;

#endif // __VULKAN_DEVICE_MANAGER_H__
