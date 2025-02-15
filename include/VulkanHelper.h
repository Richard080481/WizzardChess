#ifndef __VULKAN_HELPER_H__
#define __VULKAN_HELPER_H__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "VulkanDeviceManager.h"

#pragma warning (disable: 4505)

static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}


static void CreateBuffer(
    VkDeviceSize          size,
    VkBufferUsageFlags    usage,
    VkMemoryPropertyFlags properties,
    VkBuffer&             buffer,
    VkDeviceMemory&       bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = size;
    bufferInfo.usage       = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkDevice         device         = VK.Device();
    VkPhysicalDevice physicalDevice = VK.PhysicalDevice();

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

static void CopyBuffer(
    VkBuffer        srcBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    size)
{
    VkCommandBuffer commandBuffer = VK.BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    VK.EndSingleTimeCommands(commandBuffer);
}

static void CopyBufferToImage(
    VkBuffer    buffer,
    VkImage     image,
    uint32_t    width,
    uint32_t    height)
{
    VkCommandBuffer commandBuffer = VK.BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent =
    {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VK.EndSingleTimeCommands(commandBuffer);
}

static VkShaderModule CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(VK.Device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

static void CopyImage(
    VkCommandBuffer    commandBuffer,
    VkImage            src,
    VkImage            dst,
    VkImageAspectFlags aspectMask,
    uint32_t           width,
    uint32_t           height)
{
    const bool singleTimeCommand = (commandBuffer == VK_NULL_HANDLE);
    if (singleTimeCommand)
    {
        // Create a one-time command buffer
        commandBuffer = VK.BeginSingleTimeCommands();
    }

    // Define the region to copy
    VkImageCopy copyRegion = {};
    copyRegion.srcSubresource.aspectMask = aspectMask;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = { 0, 0, 0 };

    copyRegion.dstSubresource = copyRegion.srcSubresource;
    copyRegion.dstOffset = { 0, 0, 0 };
    copyRegion.extent = { width, height, 1 };

    // Record the copy command
    vkCmdCopyImage(commandBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    if (singleTimeCommand)
    {
        // Execute and clean up the command buffer
        VK.EndSingleTimeCommands(commandBuffer);
    }
}

static void CopyImageToBuffer(
    VkImage            image,
    VkBuffer           buffer,
    VkImageAspectFlags aspectMask,
    uint32_t           width,
    uint32_t           height)
{
    // Get device limits for row alignment
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(VK.PhysicalDevice(), &properties);
    VkDeviceSize alignment = properties.limits.optimalBufferCopyRowPitchAlignment;

    // Compute the required buffer row length
    uint32_t pixelSize = sizeof(float); // Assuming VK_FORMAT_D32_SFLOAT
    uint32_t rowStride = width * pixelSize; // Unaligned row size in bytes

    // Align to the optimal row pitch
    if (alignment > 0)
    {
        rowStride = static_cast<uint32_t>((rowStride + alignment - 1) & ~(alignment - 1));
    }

    // Create a one-time command buffer
    VkCommandBuffer commandBuffer = VK.BeginSingleTimeCommands();

    // Define the region to copy
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = rowStride / pixelSize; // Set aligned width
    region.bufferImageHeight = 0; // Keep tightly packed

    region.imageSubresource.aspectMask = aspectMask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    // Record the copy command
    vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);

    // Execute and clean up the command buffer
    VK.EndSingleTimeCommands(commandBuffer);
}

#endif // __VULKAN_HELPER_H__
