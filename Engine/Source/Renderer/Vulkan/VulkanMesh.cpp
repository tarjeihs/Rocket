#include "VulkanMesh.h"

#include <cstring>

#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanMesh::CreateMesh(PVulkanRHI* RHI, const std::vector<SVertex>& Vertices, const std::vector<uint32_t>& Indices)
{
	const size_t VertexBufferSize = Vertices.size() * sizeof(SVertex);
	const size_t IndexBufferSize = Indices.size() * sizeof(uint32_t);

	VertexBuffer = RHI->GetMemory()->CreateBuffer(VertexBufferSize, VMA_MEMORY_USAGE_GPU_ONLY, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	IndexBuffer = RHI->GetMemory()->CreateBuffer(IndexBufferSize, VMA_MEMORY_USAGE_GPU_ONLY, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	VkBufferDeviceAddressInfo BufferDeviceAddressInfo{};
	BufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	BufferDeviceAddressInfo.buffer = VertexBuffer->Buffer;
	DeviceAddress = vkGetBufferDeviceAddress(RHI->GetDevice()->GetVkDevice(), &BufferDeviceAddressInfo);

	// This buffer is used to transfer data from the CPU to the GPU
	SBuffer* StagingBuffer = new SBuffer();
	StagingBuffer = RHI->GetMemory()->CreateBuffer(VertexBufferSize + IndexBufferSize, VMA_MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	void* Data = StagingBuffer->AllocationInfo.pMappedData;
	memcpy(Data, Vertices.data(), VertexBufferSize);
	memcpy((char*)Data + VertexBufferSize, Indices.data(), IndexBufferSize);

	//Data = nullptr;
	//vmaMapMemory(RHI->GetMemory()->GetMemoryAllocator(), StagingBuffer->Allocation, &Data);
	//memcpy(Data, Vertices.data(), VertexBufferSize);
	//memcpy((char*)Data + VertexBufferSize, Indices.data(), IndexBufferSize);
	//vmaUnmapMemory(RHI->GetMemory()->GetMemoryAllocator(), StagingBuffer->Allocation);

	// Copy vertex and index data from the staging buffer (in CPU memory) immediately but synchronously to the GPU-only buffers, ensuring the data is guaranteed to be in GPU memory and ready for use.
	RHI->GetSceneRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
	{
		VkBufferCopy vertexCopy{ 0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = VertexBufferSize;

		vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), StagingBuffer->Buffer, VertexBuffer->Buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = VertexBufferSize;
		indexCopy.size = IndexBufferSize;

		vkCmdCopyBuffer(CommandBuffer->GetVkCommandBuffer(), StagingBuffer->Buffer, IndexBuffer->Buffer, 1, &indexCopy);
	});
	RHI->GetMemory()->FreeBuffer(StagingBuffer);
}

void PVulkanMesh::DestroyMesh(PVulkanRHI* RHI)
{
	RHI->GetMemory()->FreeBuffer(VertexBuffer);
	RHI->GetMemory()->FreeBuffer(IndexBuffer);
}

void PVulkanMesh::Bind(PVulkanRHI* RHI, PVulkanCommandBuffer* CommandBuffer, PVulkanPipelineLayout* PipelineLayout)
{
	SUInt64PointerPushConstant PushConstant;
	PushConstant.DeviceAddress = DeviceAddress;
	vkCmdPushConstants(CommandBuffer->GetVkCommandBuffer(), PipelineLayout->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SUInt64PointerPushConstant), &PushConstant);
	vkCmdBindIndexBuffer(CommandBuffer->GetVkCommandBuffer(), IndexBuffer->Buffer, 0, VK_INDEX_TYPE_UINT32);
}

void PVulkanMesh::Draw(PVulkanRHI* RHI, PVulkanCommandBuffer* CommandBuffer)
{
	vkCmdDrawIndexed(CommandBuffer->GetVkCommandBuffer(), static_cast<size_t>(IndexBuffer->AllocationInfo.size) / sizeof(uint32_t), 1, 0, 0, 0);
}
