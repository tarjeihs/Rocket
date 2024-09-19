#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

struct STransform
{
	STransform()
	{
    	Translation = glm::vec3(0.0f);
		Rotation = glm::vec3(0.0f);
		Scale = glm::vec3(1.0f);
	}

	[[nodiscard]] glm::mat4 ToMatrix() const
	{
    	glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), Translation);
		glm::mat4 RotationMatrix = glm::toMat4(glm::quat(Rotation));
		glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);
		return TranslationMatrix * RotationMatrix * ScaleMatrix;
	}

	glm::vec3 Translation;
	glm::vec3 Rotation;
	glm::vec3 Scale;

	friend std::ostream& operator<<(std::ostream& OutputStream, const STransform& Transform) 
	{
	    OutputStream << "(Translation) X: " << Transform.Translation.x << ", Y: " << Transform.Translation.y << ", Z: " << Transform.Translation.z;
	    OutputStream << "(Rotation) X: " << Transform.Rotation.x << ", Y: " << Transform.Rotation.y << ", Z: " << Transform.Rotation.z;
	    OutputStream << "(Scale) X: " << Transform.Scale.x << ", Y: " << Transform.Scale.y << ", Z: " << Transform.Scale.z;
	    return OutputStream;
	}
};