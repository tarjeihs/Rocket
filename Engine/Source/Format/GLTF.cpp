#include "EnginePCH.h"
#include "GLTF.h"

#include "Renderer/Common/Geometry.h"
#include <Renderer/Common/Texture2D.h>
#include <Renderer/Common/RHIAPI.h>
#include <Renderer/Vulkan/VkRenderer.h>

void GLTF::Import(const FString& Path, FPrefab& Prefab)
{
    const SBlob& Blob = PFileSystem::ReadFileBinary(Path);

    tinygltf::Model Model;
    tinygltf::TinyGLTF Loader;

    std::string Error;
    std::string Warning;

    Loader.LoadBinaryFromMemory(&Model, &Error, &Warning, Blob.Data.data(), Blob.Data.size());

    for (const auto& Node : Model.nodes)
    {
        if (Node.mesh == INDEX_Invalid)
        {
            continue;
        }

        auto& Mesh = Model.meshes[Node.mesh];

        for (const auto& Primitive : Mesh.primitives)
        {
            FSubmesh Submesh;
            Submesh.Name = Mesh.name;
            
            //auto& Material = Model.materials[Primitive.material];
            //
            //if (Material.pbrMetallicRoughness.baseColorTexture.index >= 0)
            //{
            //    auto& Texture = Model.textures[Material.pbrMetallicRoughness.baseColorTexture.index];
            //    if (Texture.source >= 0 && Texture.source < Model.images.size())
            //    {
            //        auto& Image = Model.images[Texture.source];
            //        
            //        if (ITexture2D** Ptr = Memory.Texture2D.Find(Image.name))
            //        {
            //            ITexture2D* AlbedoTexture = *Ptr;
            //            Submesh.Material.AlbedoID = AlbedoTexture->GetTextureID();
            //            
            //            continue;
            //        }
            //        
            //        FTexture2DCreateInfo Texture2DCreateInfo
            //        {
            //            Image.image.data(),
            //            Image.width,
            //            Image.height,
            //            Image.component,
            //            Image.bits,
            //            EImageFormat::SRGB
            //        };
            //        
            //        ITexture2D* AlbedoTexture = NewObject<ITexture2D>();
            //        AlbedoTexture->Initialize(Texture2DCreateInfo);
            //        GetGPUMemory()->AddTexture2D(AlbedoTexture);
            //        
            //        Submesh.Material.AlbedoID = AlbedoTexture->GetTextureID();
            //    }
            //}

            if (Node.translation.size())
            {
                glm::vec3 Translation = glm::vec3(Node.translation[0], Node.translation[1], Node.translation[2]);

                Submesh.Transform.Translation = Translation;
            }

            if (Node.rotation.size())
            {
                glm::quat Rotation = glm::quat(Node.rotation[3], Node.rotation[0], Node.rotation[1], Node.rotation[2]);

                Submesh.Transform.Rotation = glm::degrees(glm::eulerAngles(Rotation));
            }

            if (Node.scale.size())
            {
                glm::vec3 Scale = glm::vec3(Node.scale[0], Node.scale[1], Node.scale[2]);

                Submesh.Transform.Scale = Scale;
            }
            
            SizeType VertexOffset = Submesh.Vertices.GetSize();

            const float* Positions = nullptr;
            const float* TexCoords = nullptr;
            const float* Normals = nullptr;
            const uint8_t* Index = nullptr;

            SizeType TexCoordStride = 0;
            SizeType PositionStride = 0;
            SizeType NormalStride = 0;
            SizeType IndexStride = 0;

            SizeType VertexCount = 0;
            SizeType IndexCount = 0;
            SizeType TriangleCount = 0;

            if (Primitive.attributes.find("POSITION") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];

                Positions = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                PositionStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec3);
                VertexCount = Accessor.count;
            }

            if (Primitive.attributes.find("TEXCOORD_0") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];

                TexCoords = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                TexCoordStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec2);
            }

            if (Primitive.attributes.find("NORMAL") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.at("NORMAL")];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];

                Normals = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                NormalStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec3);
            }

            for (SizeType Index = 0; Index < VertexCount; ++Index)
            {
                FVertex Vertex;

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

                Submesh.Vertices.Add(Vertex);
            }

            if (Primitive.indices >= 0)
            {
                const tinygltf::Accessor& indexAccessor = Model.accessors[Primitive.indices];
                const tinygltf::BufferView& indexBufferView = Model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& indexBuffer = Model.buffers[indexBufferView.buffer];

                Index = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
                IndexStride = indexAccessor.componentType;
                IndexCount = indexAccessor.count;
                TriangleCount = indexAccessor.count / 3;

                for (size_t i = 0; i < IndexCount; ++i)
                {
                    uint32_t IndexValue;

                    switch (IndexStride)
                    {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* Value8 = reinterpret_cast<const uint8_t*>(Index);
                        IndexValue = static_cast<uint32_t>(Value8[i]) + static_cast<uint32_t>(VertexOffset);
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* Value16 = reinterpret_cast<const uint16_t*>(Index);
                        IndexValue = static_cast<uint32_t>(Value16[i]) + static_cast<uint32_t>(VertexOffset);
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* Value32 = reinterpret_cast<const uint32_t*>(Index);
                        IndexValue = static_cast<uint32_t>(Value32[i]) + static_cast<uint32_t>(VertexOffset);
                        break;
                    }
                    }

                    Submesh.Indices.Add(IndexValue);
                }
            }
            
            for (SizeType Index = 0; Index < Submesh.Indices.GetSize(); Index += 3)
            {
                uint32 Idx0 = Submesh.Indices[Index];
                uint32 Idx1 = Submesh.Indices[Index + 1];
                uint32 Idx2 = Submesh.Indices[Index + 2];

                FVertex& Vertex0 = Submesh.Vertices[Idx0];
                FVertex& Vertex1 = Submesh.Vertices[Idx1];
                FVertex& Vertex2 = Submesh.Vertices[Idx2];

                const glm::vec3& Position0 = Vertex0.Position;
                const glm::vec3& Position1 = Vertex1.Position;
                const glm::vec3& Position2 = Vertex2.Position;

                const glm::vec2& TexCoord0 = Vertex0.TexCoord;
                const glm::vec2& TexCoord1 = Vertex1.TexCoord;
                const glm::vec2& TexCoord2 = Vertex2.TexCoord;

                glm::vec3 Edge1 = Position1 - Position0;
                glm::vec3 Edge2 = Position2 - Position0;

                glm::vec2 DeltaTexCoord1 = TexCoord1 - TexCoord0;
                glm::vec2 DeltaTexCoord2 = TexCoord2 - TexCoord0;

                float F = 1.0f / (DeltaTexCoord1.x * DeltaTexCoord2.y - DeltaTexCoord2.x * DeltaTexCoord1.y + 1e-7f);

                glm::vec3 Tangent = F * (Edge1 * DeltaTexCoord2.y - Edge2 * DeltaTexCoord1.y);
                glm::vec3 Bitangent = F * (-Edge1 * DeltaTexCoord2.x + Edge2 * DeltaTexCoord1.x);

                Vertex0.Tangent += Tangent;
                Vertex1.Tangent += Tangent;
                Vertex2.Tangent += Tangent;

                Vertex0.Bitangent += Bitangent;
                Vertex1.Bitangent += Bitangent;
                Vertex2.Bitangent += Bitangent;
            }

            for (SizeType Index = 0; Index < Submesh.Vertices.GetSize(); ++Index)
            {
                FVertex& Vertex = Submesh.Vertices[Index];

                glm::vec3& N = Vertex.Normal;
                glm::vec3& T = Vertex.Tangent;
                glm::vec3& B = Vertex.Bitangent;

                T = glm::normalize(T - N * glm::dot(N, T));
                B = glm::normalize(glm::cross(N, T));
            }

            Prefab.Submeshes.Add(Submesh);
        }
    }
}