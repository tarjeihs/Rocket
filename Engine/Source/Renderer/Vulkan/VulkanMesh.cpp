#include <vulkan/vulkan_core.h>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "EnginePCH.h"
#include "VulkanMesh.h"

#include <cstring>
#include <tiny_gltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Core/Assert.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanMemory.h"

void PVulkanMesh::Init() 
{
    
}

void PVulkanMesh::Cleanup() 
{
    Destroy();
}

void PVulkanMesh::CreateMesh(std::span<SVertex> Vertices, std::span<uint32_t> Indices)
{
    const size_t VertexBufferSize = Vertices.size() * sizeof(SVertex);
    const size_t IndexBufferSize = Indices.size() * sizeof(uint32_t);

    VertexBuffer = std::make_unique<SVulkanBuffer>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    IndexBuffer = std::make_unique<SVulkanBuffer>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    StagingBuffer = std::make_unique<SVulkanBuffer>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    VertexBuffer->Allocate(VertexBufferSize);
    IndexBuffer->Allocate(IndexBufferSize);
    StagingBuffer->Allocate(VertexBufferSize + IndexBufferSize);

    void* Data = nullptr;
    vmaMapMemory(GetRHI()->GetMemory()->GetMemoryAllocator(), StagingBuffer->Allocation, &Data);
    memcpy(Data, Vertices.data(), VertexBufferSize);
    memcpy((char*)Data + VertexBufferSize, Indices.data(), IndexBufferSize);
    vmaUnmapMemory(GetRHI()->GetMemory()->GetMemoryAllocator(), StagingBuffer->Allocation);


    VkBufferDeviceAddressInfo BufferDeviceAddressInfo{};
    BufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    BufferDeviceAddressInfo.buffer = VertexBuffer->Buffer;
    DeviceAddress64 = vkGetBufferDeviceAddress(GetRHI()->GetDevice()->GetVkDevice(), &BufferDeviceAddressInfo);

    // Copy vertex and index data from the staging buffer (in CPU memory) immediately but synchronously to the GPU-only buffers, ensuring the data is guaranteed to be in GPU memory and ready for use.
    // TODO: Move ImmediateSubmit into command buffer management class
    GetRHI()->GetSceneRenderer()->ImmediateSubmit([&](PVulkanCommandBuffer* CommandBuffer)
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
}

void PVulkanMesh::Draw(const STransform& Transform)
{
    glm::mat4 ModelMatrix = Transform.ToMatrix();
    glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ModelMatrix));

    SShaderStorageBufferObject Instance;
    Instance.ModelMatrix = ModelMatrix;
    Instance.NormalMatrix = NormalMatrix;

    BufferData.push_back(Instance);
    //GetRHI()->GetMemory()->UploadBuffer(Material->GraphicsPipeline->StorageBuffer,&Instance, sizeof(SShaderStorageBufferObject));
    Material->GraphicsPipeline->StorageBuffer->Update(&Instance, sizeof(Instance));

    //SubmitData.push_back([&]()
    //{
        SUInt64PointerPushConstant PushConstant;
        PushConstant.DeviceAddress = DeviceAddress64;
        PushConstant.ObjectId = SubmitData.size();

        Material->Bind(Transform);
        
        const PVulkanFrame* Frame = GetRHI()->GetSceneRenderer()->GetFramePool()->Pool[GetRHI()->GetSceneRenderer()->GetFramePool()->FrameIndex % 2];
        vkCmdPushConstants(Frame->CommandBuffer->GetVkCommandBuffer(), Material->GraphicsPipeline->GetPipelineLayout()->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SUInt64PointerPushConstant), &PushConstant);
        vkCmdBindIndexBuffer(Frame->CommandBuffer->GetVkCommandBuffer(), IndexBuffer->Buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(Frame->CommandBuffer->GetVkCommandBuffer(), static_cast<size_t>(IndexBuffer->AllocationInfo.size) / sizeof(uint32_t), 1, 0, 0, 0);

        Material->Unbind();
    //});
}

void PVulkanMesh::Destroy()
{
    VertexBuffer->Free();
    IndexBuffer->Free();
    StagingBuffer->Free();
    Material->Destroy();
}

void PVulkanMesh::Serialize(SBlob& Blob)
{
}

void PVulkanMesh::Deserialize(SBlob& Blob)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;

    std::string err;
    std::string warn;

    bool ret = loader.LoadBinaryFromMemory(&model, &err, &warn, Blob.Data.data(), Blob.Data.size());

    std::vector<SVertex> vertices;
    std::vector<uint32_t> indices;

    for (const auto& mesh : model.meshes)
    {
        for (const auto& primitive : mesh.primitives)
        {
            // Load vertex positions
            const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
            const tinygltf::BufferView &posBufferView = model.bufferViews[posAccessor.bufferView];
            const tinygltf::Buffer &posBuffer = model.buffers[posBufferView.buffer];
            const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

            // Calculate the stride (distance between each vertex in the buffer)
            size_t posStride = posAccessor.ByteStride(posBufferView) ? posAccessor.ByteStride(posBufferView) : sizeof(glm::vec3);

            // Similarly, get texCoords, normals, and colors using the same approach
            const float* texCoords = nullptr;
            size_t texStride = 0;
            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
            {
                const tinygltf::Accessor &texAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView &texBufferView = model.bufferViews[texAccessor.bufferView];
                const tinygltf::Buffer &texBuffer = model.buffers[texBufferView.buffer];
                texCoords = reinterpret_cast<const float*>(&texBuffer.data[texBufferView.byteOffset + texAccessor.byteOffset]);
                texStride = texAccessor.ByteStride(texBufferView) ? texAccessor.ByteStride(texBufferView) : sizeof(glm::vec2);
            }

            const float* normals = nullptr;
            size_t normStride = 0;
            if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
            {
                const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView &normBufferView = model.bufferViews[normAccessor.bufferView];
                const tinygltf::Buffer &normBuffer = model.buffers[normBufferView.buffer];
                normals = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);
                normStride = normAccessor.ByteStride(normBufferView) ? normAccessor.ByteStride(normBufferView) : sizeof(glm::vec3);
            }

            const float* colors = nullptr;
            size_t colorStride = 0;
            if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
            {
                const tinygltf::Accessor &colorAccessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
                const tinygltf::BufferView &colorBufferView = model.bufferViews[colorAccessor.bufferView];
                const tinygltf::Buffer &colorBuffer = model.buffers[colorBufferView.buffer];
                colors = reinterpret_cast<const float*>(&colorBuffer.data[colorBufferView.byteOffset + colorAccessor.byteOffset]);
                colorStride = colorAccessor.ByteStride(colorBufferView) ? colorAccessor.ByteStride(colorBufferView) : sizeof(glm::vec4);
            }

            // Combine all attributes into SVertex
            for (size_t i = 0; i < posAccessor.count; ++i)
            {
                SVertex vertex = {};

                const float* pos = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(positions) + i * posStride);
                vertex.Position = glm::vec3(pos[0], pos[1], pos[2]);

                if (texCoords) {
                    const float* tex = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(texCoords) + i * texStride);
                    vertex.TexCoord = glm::vec2(tex[0], tex[1]);
                } else {
                    vertex.TexCoord = glm::vec2(0.0f, 0.0f);
                }

                if (normals) {
                    const float* norm = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(normals) + i * normStride);
                    vertex.Normal = glm::vec3(norm[0], norm[1], norm[2]);
                } else {
                    vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
                }

                if (colors) {
                    const float* color = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(colors) + i * colorStride);
                    vertex.Color = glm::vec4(color[0], color[1], color[2], color[3]);
                } else {
                    vertex.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                }

                vertices.push_back(vertex);
            }

            // Load indices (code remains the same as before)
            const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView &indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer &indexBuffer = model.buffers[indexBufferView.buffer];

            if (indexAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
            {
                const uint16_t* indices16 = reinterpret_cast<const uint16_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
                for (size_t i = 0; i < indexAccessor.count; ++i)
                {
                    indices.push_back(static_cast<uint32_t>(indices16[i]));
                }
            }
            else if (indexAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)
            {
                const uint32_t* indices32 = reinterpret_cast<const uint32_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
                for (size_t i = 0; i < indexAccessor.count; ++i)
                {
                    indices.push_back(indices32[i]);
                }
            }
            else
            {
                std::cerr << "Unsupported index format" << std::endl;
            }
        }
    }

    CreateMesh(vertices, indices);
}

void PVulkanMesh::BeginFrame()
{
    SubmitData.clear();
    BufferData.clear();
}

void PVulkanMesh::EndFrame()
{
    //GetRHI()->GetMemory()->UploadBuffer(Material->GraphicsPipeline->StorageBuffer, BufferData.data(), sizeof(SShaderStorageBufferObject) * 1);

    for (const std::function<void()>& Func : SubmitData)
    {
        Func();
    }
}

void PVulkanMesh::SetMaterial(const std::shared_ptr<PVulkanMaterial>& NewMaterial)
{
    Material = NewMaterial;

    Material->Init();
}
