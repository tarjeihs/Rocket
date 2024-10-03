#include "EnginePCH.h"
#include "GLTF.h"

void PGLTF::ImportGLTF(const std::string& Path, SMeshBinaryData& MeshBinaryObject)
{
    const SBlob& Blob = PFileSystem::ReadFileBinary(Path);

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

                MeshBinaryObject.Vertices.push_back(Vertex);
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

                MeshBinaryObject.Indices.push_back(IndexValue);
            }
        }
    }
}