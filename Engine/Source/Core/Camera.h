#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "Renderer/Common/Overlay.h"

enum class ECameraProjectionMode 
{ 
	Perspective, Orthographic
};

class PCamera
{
public:
	PCamera() 
	{
		Settings.ProjectionMode = ECameraProjectionMode::Perspective;
    	Settings.FoVY = 66.0f;
    	Settings.ZNear = 0.1f;
    	Settings.ZFar = 100.0f;

#if RK_DEBUG
		GOverlay->OnRender.Bind(this, &PCamera::OnImGuiRender);
#endif
	}

	void OnImGuiRender() 
	{
		const char* ProjectionModeEnumerateNames[2] = { "Perspective", "Orthographic" };

		ImGui::Begin("Camera Settings");
		ImGui::Combo("Projection Mode", reinterpret_cast<int*>(&Settings.ProjectionMode), ProjectionModeEnumerateNames, IM_ARRAYSIZE(ProjectionModeEnumerateNames));
		ImGui::DragFloat("Field of View", &Settings.FoVY, 1.0f, 30.0f, 145.0f);
		ImGui::DragFloat("Z Near", &Settings.ZNear, 1.0f, 0.1f, 1000.0f);
		ImGui::DragFloat("Z Far", &Settings.ZFar, 1.0f, 0.1f, 1000.0f);
		ImGui::End();

		ApplySettings();
	}

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

	void ApplySettings()
	{
		switch (Settings.ProjectionMode)
		{
			case ECameraProjectionMode::Orthographic: SetOrthographicProjection(glm::radians(Settings.FoVY), Settings.AspectRatio, Settings.ZNear, Settings.ZFar); break;
			case ECameraProjectionMode::Perspective: SetPerspectiveProjection(glm::radians(Settings.FoVY), Settings.AspectRatio, Settings.ZNear, Settings.ZFar); break;
		}
	}

	inline const glm::mat4& GetViewMatrix() const
	{
		return m_ViewMatrix;
	}

	inline const glm::mat4& GetProjectionMatrix() const
	{
		return m_Projection;
	}

	struct SCameraSettings 
	{
		ECameraProjectionMode ProjectionMode;
		float FoVY;
		float AspectRatio;
		float ZNear;
		float ZFar;
	} Settings;

private:
	glm::mat4 m_Projection = glm::identity<glm::mat4>();
	glm::mat4 m_ViewMatrix = glm::identity<glm::mat4>();
};