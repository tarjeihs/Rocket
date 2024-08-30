#pragma once

#include <glm/glm.hpp>

struct STransform
{
	glm::vec3 m_Translation = glm::vec3(0.0f);
	glm::vec3 m_Rotation = glm::vec3(0.0f);
	glm::vec3 m_Scale = glm::vec3(1.0f);

	glm::mat4 ToMatrix()
	{
	    const float c1 = glm::cos(m_Rotation.y);
	    const float s1 = glm::sin(m_Rotation.y);
	    const float c2 = glm::cos(m_Rotation.x);
	    const float s2 = glm::sin(m_Rotation.x);
	    const float c3 = glm::cos(m_Rotation.z);
	    const float s3 = glm::sin(m_Rotation.z);

	    const glm::vec4 col0 = glm::vec4(m_Scale.x * (c1 * c3 + s1 * s2 * s3), m_Scale.y * (c3 * s1 * s2 - c1 * s3), m_Scale.z * (c2 * s1), 0.0f);
	    const glm::vec4 col1 = glm::vec4(m_Scale.x * (c2 * s3), m_Scale.y * (c2 * c3), m_Scale.z * (-s2), 0.0f );
	    const glm::vec4 col2 = glm::vec4(m_Scale.x * (c1 * s2 * s3 - c3 * s1), m_Scale.y * (c1 * c3 * s2 + s1 * s3), m_Scale.z * (c1 * c2), 0.0f);
	    const glm::vec4 col3 = glm::vec4(m_Translation.x, m_Translation.y, m_Translation.z, 1.0f);

	    return glm::mat4(col0, col1, col2, col3);
	}
};