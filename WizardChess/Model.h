#ifndef __MODEL_H__
#define __MODEL_H__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

class Model
{
public:
    Model(
        VkPhysicalDevice phyicalDevice,
        VkDevice device,
        VkQueue queue,
        VkCommandPool commandPool,
        std::string fileName) :
        m_physicalDevice(phyicalDevice),
        m_device(device),
        m_queue(queue),
        m_commandPool(commandPool)
    {
        Load(fileName);
        createVertexBuffer();
        createIndexBuffer();
    }

    ~Model()
    {
        vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
        vkFreeMemory(m_device, m_indexBufferMemory, nullptr);

        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
    }

    VkBuffer  VertexBuffer()    const { return m_vertexBuffer; }
    VkBuffer  IndexBuffer()     const { return m_indexBuffer; }
    size_t    Indices()         const { return m_indices.size(); }
    glm::mat4 NormalizeMatrix() const { return m_normalizeMatrix; }

private:
    void createIndexBuffer();
    void createVertexBuffer();
    void Load(std::string fileName);

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    float m_boundaries[6] = {};
    glm::mat4 m_normalizeMatrix;

    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkQueue m_queue;
    VkCommandPool m_commandPool;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;
};

#endif // __MODEL_H__