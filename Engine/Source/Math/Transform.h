#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

struct STransform
{
	glm::vec3 m_Translation = glm::vec3(0.0f);
	glm::vec3 m_Rotation = glm::vec3(0.0f);
	glm::vec3 m_Scale = glm::vec3(1.0f);

	[[nodiscard]] glm::mat4 ToMatrix() const
	{
		glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), m_Translation);
		glm::mat4 RotationMatrix = glm::toMat4(glm::quat(m_Rotation));
		glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), m_Scale);
		return TranslationMatrix * RotationMatrix * ScaleMatrix;
	}
};