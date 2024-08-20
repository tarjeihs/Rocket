#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Assert.h"
#include "Math/Math.h"

// TODO: With ECS, create a CameraComponent, where we on OnUpdate do the camera movement (mouse, keyboard)

class PCamera
{
public:
	void SetOrthographicProjection(float aspectRatio, float zNear, float zFar)
	{
		m_Projection = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, zNear, zFar);
	}

	void SetPerspectiveProjection(float fovy, float aspectRatio, float zNear, float zFar)
	{
		m_Projection = glm::perspective(fovy, aspectRatio, zNear, zFar);
	}

	inline const glm::mat4& GetViewMatrix() const
	{
		return m_ViewMatrix;
	}

	inline const glm::mat4& GetProjectionMatrix() const
	{
		return m_Projection;
	}

	void UpdateViewMatrix(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
	{
		const glm::vec3 w = { glm::normalize(direction) };
		const glm::vec3 u = { glm::normalize(glm::cross(w, glm::vec3{0.f, -1.f, 0.f})) };
		const glm::vec3 v = { glm::cross(w, u) };

		m_ViewMatrix = glm::mat4{ 1.f };
		m_ViewMatrix[0][0] = u.x;
		m_ViewMatrix[1][0] = u.y;
		m_ViewMatrix[2][0] = u.z;
		m_ViewMatrix[0][1] = v.x;
		m_ViewMatrix[1][1] = v.y;
		m_ViewMatrix[2][1] = v.z;
		m_ViewMatrix[0][2] = w.x;
		m_ViewMatrix[1][2] = w.y;
		m_ViewMatrix[2][2] = w.z;
		m_ViewMatrix[3][0] = -glm::dot(u, position);
		m_ViewMatrix[3][1] = -glm::dot(v, position);
		m_ViewMatrix[3][2] = -glm::dot(w, position);
	}

	void CalculateViewMatrix(glm::vec3 position, glm::vec3 rotation)
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		
		const glm::vec3 u = { (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v = { (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w = { (c2 * s1), (-s2), (c1 * c2) };

		m_ViewMatrix = glm::mat4{ 1.f };
		m_ViewMatrix[0][0] = u.x;
		m_ViewMatrix[1][0] = u.y;
		m_ViewMatrix[2][0] = u.z;
		m_ViewMatrix[0][1] = v.x;
		m_ViewMatrix[1][1] = v.y;
		m_ViewMatrix[2][1] = v.z;
		m_ViewMatrix[0][2] = w.x;
		m_ViewMatrix[1][2] = w.y;
		m_ViewMatrix[2][2] = w.z;
		m_ViewMatrix[3][0] = -glm::dot(u, position);
		m_ViewMatrix[3][1] = -glm::dot(v, position);
		m_ViewMatrix[3][2] = -glm::dot(w, position);
	}

public:
	glm::mat4 m_Projection = { 1.0f };
	glm::mat4 m_ViewMatrix = { 1.0f };
};
