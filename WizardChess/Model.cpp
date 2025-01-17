#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Types.h"
#include "Model.h"
#include "VulkanHelper.h"

#include <unordered_map>
#include <cmath>

void Model::Load(std::string fileNmae)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileNmae.c_str())) {
        throw std::runtime_error(warn + err);
    }

    //std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    m_boundaries[0] = attrib.vertices[0];
    m_boundaries[1] = attrib.vertices[0];
    m_boundaries[2] = attrib.vertices[1];
    m_boundaries[3] = attrib.vertices[1];
    m_boundaries[4] = attrib.vertices[2];
    m_boundaries[5] = attrib.vertices[2];

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            m_boundaries[0] = std::min(m_boundaries[0], vertex.pos[0]);
            m_boundaries[1] = std::max(m_boundaries[1], vertex.pos[0]);
            m_boundaries[2] = std::min(m_boundaries[2], vertex.pos[1]);
            m_boundaries[3] = std::max(m_boundaries[3], vertex.pos[1]);
            m_boundaries[4] = std::min(m_boundaries[4], vertex.pos[2]);
            m_boundaries[5] = std::max(m_boundaries[5], vertex.pos[2]);

            if (index.texcoord_index != -1)
            {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            vertex.color = { vertex.pos[0], vertex.pos[1], vertex.pos[2] };

#define USE_UNIQUE_VERTICES 0
#if 0
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                m_vertices.push_back(vertex);
            }

            m_indices.push_back(uniqueVertices[vertex]);
#else

            m_indices.push_back(m_vertices.size());
            m_vertices.push_back(vertex);
#endif
        }
    }

    float maxLength = std::max(std::max((m_boundaries[1] - m_boundaries[0]) / 2,
                                        (m_boundaries[3] - m_boundaries[2]) / 2),
                               (m_boundaries[5] - m_boundaries[4]) / 2);
    m_normalizeMatrix = glm::mat4(1.0f);
    m_normalizeMatrix = glm::scale(m_normalizeMatrix, glm::vec3(1.0f / maxLength));
    m_normalizeMatrix = glm::translate(m_normalizeMatrix,
                                       -glm::vec3((m_boundaries[0] + m_boundaries[1]) / 2,
                                                  (m_boundaries[2] + m_boundaries[3]) / 2,
                                                  (m_boundaries[4] + m_boundaries[5]) / 2));

    for (auto& vertex : m_vertices)
    {
        vertex.color[0] = (vertex.color[0] - m_boundaries[0]) / (m_boundaries[1] - m_boundaries[0]);
        vertex.color[1] = (vertex.color[1] - m_boundaries[2]) / (m_boundaries[3] - m_boundaries[2]);
        vertex.color[2] = (vertex.color[2] - m_boundaries[4]) / (m_boundaries[5] - m_boundaries[4]);
    }
}


void Model::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    copyBuffer(m_device, m_queue, m_commandPool, stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Model::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    copyBuffer(m_device, m_queue, m_commandPool, stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}