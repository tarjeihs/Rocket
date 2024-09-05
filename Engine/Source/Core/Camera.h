#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class PCamera
{
public:
	void SetOrthographicProjection(float Zoom, float AspectRatio, float ZNear, float ZFar)
	{
		m_Projection = glm::ortho(-AspectRatio * Zoom, AspectRatio * Zoom, -1.0f * Zoom, 1.0f * Zoom, ZNear, ZFar);
		m_Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
		m_Projection[3][1] *= -1.0f; // Invert Y-Axis Translation
	}

	void SetPerspectiveProjection(float FovY, float AspectRatio, float ZNear, float ZFar)
	{
		m_Projection = glm::perspective(FovY, AspectRatio, ZNear, ZFar);
		m_Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
	}

	void CalculateViewMatrix(const glm::vec3& Position, const glm::vec3& Rotation)
	{
		const glm::vec3 Forward = glm::vec3(glm::cos(Rotation.y) * glm::cos(Rotation.x), glm::sin(Rotation.x), glm::sin(Rotation.y) * glm::cos(Rotation.x));
		const glm::vec3 Direction = glm::normalize(Forward);
		const glm::vec3 Right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), Direction));
		const glm::vec3 Up = glm::cross(Direction, Right);

		m_ViewMatrix = glm::lookAt(Position, Position + Direction, Up);
	}

	inline const glm::mat4& GetViewMatrix() const
	{
		return m_ViewMatrix;
	}

	inline const glm::mat4& GetProjectionMatrix() const
	{
		return m_Projection;
	}

public:
	glm::mat4 m_Projection = glm::identity<glm::mat4>();
	glm::mat4 m_ViewMatrix = glm::identity<glm::mat4>();
};
