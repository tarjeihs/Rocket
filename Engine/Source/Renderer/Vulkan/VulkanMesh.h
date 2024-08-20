#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

struct SVertex
{
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
};

class PVulkanCommandBuffer;
class PVulkanPipelineLayout;
class PVulkanRHI;
class SBuffer;

class PVulkanMesh
{
public:
    void CreateMesh(PVulkanRHI* RHI, const std::vector<SVertex>& Vertices, const std::vector<uint32_t>& Indices);
    void DestroyMesh(PVulkanRHI* RHI);
    void Bind(PVulkanRHI* RHI, PVulkanCommandBuffer* CommandBuffer, PVulkanPipelineLayout* PipelineLayout);
    void Draw(PVulkanRHI* RHI, PVulkanCommandBuffer* CommandBuffer);

    SBuffer* VertexBuffer;
    SBuffer* IndexBuffer;
    VkDeviceAddress DeviceAddress;
};