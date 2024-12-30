#pragma once

#include "Types/Vertex.h"
#include "Types/Array.h"

struct FGeometry
{
    TArray<FVertex> Vertices;
    TArray<uint32> Indices;
};

struct FCubeGeometry : public FGeometry
{
    FCubeGeometry()
    {
        Vertices = {
            { glm::vec3 { -0.5f, -0.5f,  0.5f }, glm::vec3 { 0.0f, 0.0f,  1.0f }, glm::vec2 { 0.0f, 0.0f } },
            { glm::vec3 {  0.5f, -0.5f,  0.5f }, glm::vec3 { 0.0f, 0.0f,  1.0f }, glm::vec2 { 1.0f, 0.0f } },
            { glm::vec3 {  0.5f,  0.5f,  0.5f }, glm::vec3 { 0.0f, 0.0f,  1.0f }, glm::vec2 { 1.0f, 1.0f } },
            { glm::vec3 { -0.5f,  0.5f,  0.5f }, glm::vec3 { 0.0f, 0.0f,  1.0f }, glm::vec2 { 0.0f, 1.0f } },

            { glm::vec3 { -0.5f, -0.5f, -0.5f }, glm::vec3 { 0.0f, 0.0f, -1.0f }, glm::vec2 { 0.0f, 0.0f } },
            { glm::vec3 {  0.5f, -0.5f, -0.5f }, glm::vec3 { 0.0f, 0.0f, -1.0f }, glm::vec2 { 1.0f, 0.0f } },
            { glm::vec3 {  0.5f,  0.5f, -0.5f }, glm::vec3 { 0.0f, 0.0f, -1.0f }, glm::vec2 { 1.0f, 1.0f } },
            { glm::vec3 { -0.5f,  0.5f, -0.5f }, glm::vec3 { 0.0f, 0.0f, -1.0f }, glm::vec2 { 0.0f, 1.0f } },
        };

        Indices = {
            0, 1, 2,    2, 3, 0, // Front face
            5, 4, 7,    7, 6, 5, // Back face
            4, 0, 3,    3, 7, 4, // Left face
            1, 5, 6,    6, 2, 1, // Right face
            3, 2, 6,    6, 7, 3, // Top face
            4, 5, 1,    1, 0, 4, // Bottom face
        };
    }
};