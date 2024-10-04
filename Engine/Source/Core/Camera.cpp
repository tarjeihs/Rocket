#include "EnginePCH.h"
#include "Camera.h"

void PCamera::OnImGuiRender()
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

void PCamera::SetOrthographicProjection(float Zoom, float AspectRatio, float ZNear, float ZFar)
{
    Projection = glm::ortho(-AspectRatio * Zoom, AspectRatio * Zoom, -1.0f * Zoom, 1.0f * Zoom, ZNear, ZFar);
    Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
    Projection[3][1] *= -1.0f; // Invert Y-Axis Translation
}

void PCamera::SetPerspectiveProjection(float FovY, float AspectRatio, float ZNear, float ZFar)
{
    Projection = glm::perspective(FovY, AspectRatio, ZNear, ZFar);
    Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
}

void PCamera::CalculateViewMatrix(const glm::vec3& NewPosition, const glm::vec3& NewRotation)
{
    const glm::vec3 Forward = glm::vec3(glm::cos(NewRotation.y) * glm::cos(NewRotation.x), glm::sin(NewRotation.x), glm::sin(NewRotation.y) * glm::cos(NewRotation.x));
    const glm::vec3 Direction = glm::normalize(Forward);
    const glm::vec3 Right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), Direction));
    const glm::vec3 Up = glm::cross(Direction, Right);

    ViewMatrix = glm::lookAt(NewPosition, NewPosition + Direction, Up);
    Position = NewPosition;
    Rotation = NewRotation;
}

void PCamera::ApplySettings()
{
    switch (Settings.ProjectionMode)
    {
        case ECameraProjectionMode::Orthographic: SetOrthographicProjection(glm::radians(Settings.FoVY), GetWindow()->GetAspectRatio(), Settings.ZNear, Settings.ZFar); break;
        case ECameraProjectionMode::Perspective: SetPerspectiveProjection(glm::radians(Settings.FoVY), GetWindow()->GetAspectRatio(), Settings.ZNear, Settings.ZFar); break;
    }
}