#ifndef __MODEL_H__
#define __MODEL_H__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <string>

#include "Types.h"

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
        m_commandPool(commandPool),
        m_modelMatrix(glm::mat4(1.0f))
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
    glm::mat4 ModelMatrix()     const { return m_modelMatrix; }

    void Translate(glm::vec3 tranlation)
    {
        m_modelMatrix = glm::translate(m_modelMatrix, tranlation);
    }

    void Rotate(float degree, glm::vec3 axis)
    {
        m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(degree), axis);
    }

    void RescaleNormalizeMatrix(float scale)
    {
        m_normalizeMatrix = glm::scale(m_normalizeMatrix, glm::vec3(MaxScale()));
        m_normalizeMatrix = glm::scale(m_normalizeMatrix, glm::vec3(scale));
    }

    float MaxScale()
    {
        return std::max(std::max((m_boundaries[1] - m_boundaries[0]),
                                 (m_boundaries[3] - m_boundaries[2])),
                        (m_boundaries[5] - m_boundaries[4])) / 2;
    }

private:
    void createIndexBuffer();
    void createVertexBuffer();
    void Load(std::string fileName);

    glm::mat4 m_modelMatrix;
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