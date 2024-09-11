#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	void OnImGuiRender();

	void SetOrthographicProjection(float Zoom, float AspectRatio, float ZNear, float ZFar);

	void SetPerspectiveProjection(float FovY, float AspectRatio, float ZNear, float ZFar);

	void CalculateViewMatrix(const glm::vec3& Position, const glm::vec3& Rotation);

	void ApplySettings();

	inline const glm::mat4& GetViewMatrix() const
	{
		return ViewMatrix;
	}

	inline const glm::mat4& GetProjectionMatrix() const
	{
		return Projection;
	}

	struct SCameraSettings 
	{
		ECameraProjectionMode ProjectionMode;
		float FoVY;
		float ZNear;
		float ZFar;
	} Settings;

private:
	glm::mat4 Projection = glm::identity<glm::mat4>();
	glm::mat4 ViewMatrix = glm::identity<glm::mat4>();
};