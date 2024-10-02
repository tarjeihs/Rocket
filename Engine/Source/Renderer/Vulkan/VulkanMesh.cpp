#include "EnginePCH.h"
#include "VulkanMesh.h"

#include "Renderer/Common/Material.h"
#include "Renderer/Vulkan/VulkanCommand.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanFrame.h"
#include "Renderer/Vulkan/VulkanMaterial.h"
#include "Renderer/Vulkan/VulkanSceneRenderer.h"
#include "Renderer/Vulkan/VulkanMemory.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanAllocator.h"
#include "Utils/Profiler.h"

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

    VertexBuffer = new PVulkanBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    IndexBuffer = new PVulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    StagingBuffer = new PVulkanBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    VertexBuffer->Allocate(VertexBufferSize);
    IndexBuffer->Allocate(IndexBufferSize);
    StagingBuffer->Allocate(VertexBufferSize + IndexBufferSize);

    void* Data = nullptr;
    vmaMapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingBuffer->Allocation, &Data);
    memcpy(Data, Vertices.data(), VertexBufferSize);
    memcpy((char*)Data + VertexBufferSize, Indices.data(), IndexBufferSize);
    vmaUnmapMemory(GetRHI()->GetSceneRenderer()->GetAllocator()->GetMemoryAllocator(), StagingBuffer->Allocation);

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

void PVulkanMesh::DrawIndirectInstanced(uint32_t ID)
{
    PROFILE_FUNC_SCOPE("PVulkanMesh::DrawIndirectInstanced")

    PVulkanFrame* Frame = GetRHI()->GetSceneRenderer()->GetParallelFramePool()->GetCurrentFrame();

    SUInt64PointerPushConstant PushConstant;
    PushConstant.DeviceAddress = DeviceAddress64;
    PushConstant.ObjectId = ID;

    Material->Bind();

    vkCmdPushConstants(Frame->GetCommandBuffer()->GetVkCommandBuffer(), Material->GraphicsPipeline->GetPipelineLayout()->GetVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SUInt64PointerPushConstant), &PushConstant);
    vkCmdBindIndexBuffer(Frame->GetCommandBuffer()->GetVkCommandBuffer(), IndexBuffer->Buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(Frame->GetCommandBuffer()->GetVkCommandBuffer(), static_cast<size_t>(IndexBuffer->AllocationInfo.size) / sizeof(uint32_t), 1, 0, 0, 0);

    Material->Unbind();
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

// TODO: MOVE THIS INTO A SEPERATE TOOLS CLASS, GLTF.H/CPP
void PVulkanMesh::Deserialize(SBlob& Blob)
{
    std::vector<SVertex> Vertices;
    std::vector<uint32_t> Indices;

    tinygltf::Model Model;
    tinygltf::TinyGLTF Loader;

    std::string Error;
    std::string Warning;

    Loader.LoadBinaryFromMemory(&Model, &Error, &Warning, Blob.Data.data(), Blob.Data.size());

    for (const auto& Mesh : Model.meshes)
    {
        for (const auto& Primitive : Mesh.primitives)
        {
            const float* Positions = nullptr;
            const float* TexCoords = nullptr;
            const float* Normals = nullptr;
            const float* Colors = nullptr;
            const uint8_t* indexData = nullptr;

            size_t TexCoordStride = 0;
            size_t PositionStride = 0;
            size_t NormalStride = 0;
            size_t ColorStride = 0;
            size_t IndexStride = 0;

            size_t VertexCount = 0;
            size_t IndexCount = 0;
            size_t TriangleCount = 0;

            if (Primitive.attributes.find("POSITION") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];
                
                Positions = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                PositionStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec3);
                VertexCount = Accessor.count;
            }

            if (Primitive.attributes.find("TEXCOORD_0") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];
                TexCoords = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                TexCoordStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec2);
            }

            if (Primitive.attributes.find("NORMAL") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];
                Normals = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                NormalStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec3);
            }

            if (Primitive.attributes.find("COLOR_0") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.find("COLOR_0")->second];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];
                Colors = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                ColorStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec4);
            }

            for (size_t Index = 0; Index < VertexCount; ++Index)
            {
                SVertex Vertex;

                if (Positions)
                {
                    const float* Position = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(Positions) + Index * PositionStride);
                    Vertex.Position = glm::vec3(Position[0], Position[1], Position[2]);
                }

                if (TexCoords) 
                {
                    const float* TexCoord = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(TexCoords) + Index * TexCoordStride);
                    Vertex.TexCoord = glm::vec2(TexCoord[0], TexCoord[1]);
                } 

                if (Normals) 
                {
                    const float* Normal = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(Normals) + Index * NormalStride);
                    Vertex.Normal = glm::vec3(Normal[0], Normal[1], Normal[2]);
                } 

                if (Colors) 
                {
                    const float* Color = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(Colors) + Index * ColorStride);
                    Vertex.Color = glm::vec4(Color[0], Color[1], Color[2], Color[3]);
                } 

                Vertices.push_back(Vertex);
            }

            if (Primitive.indices >= 0)
            {
                const tinygltf::Accessor &indexAccessor = Model.accessors[Primitive.indices];
                const tinygltf::BufferView &indexBufferView = Model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer &indexBuffer = Model.buffers[indexBufferView.buffer];
                indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
                IndexStride = indexAccessor.componentType;
                IndexCount = indexAccessor.count;
                TriangleCount = indexAccessor.count / 3;
            }

            for (size_t Index = 0; Index < IndexCount; ++Index)
            {
                uint32_t IndexValue;
                
                switch (IndexStride)
                {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* Value8 = reinterpret_cast<const uint8_t*>(indexData);
                        IndexValue = static_cast<uint32_t>(Value8[Index]);
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* Value16 = reinterpret_cast<const uint16_t*>(indexData);
                        IndexValue = static_cast<uint32_t>(Value16[Index]);
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* Value32 = reinterpret_cast<const uint32_t*>(indexData);
                        IndexValue = static_cast<uint32_t>(Value32[Index]);
                        break;
                    }
                }

                Indices.push_back(IndexValue);
            }
        
            //Stats.Vertices = VertexCount;
            //Stats.Triangles = TriangleCount;
        }

    }

    CreateMesh(Vertices, Indices);
}

IMaterial* PVulkanMesh::GetMaterial() const
{
    return Material;
}


void PVulkanMesh::ApplyMaterial(IMaterial* NewMaterial)
{
    Material = static_cast<PVulkanMaterial*>(NewMaterial);
//    Material->Init();
}