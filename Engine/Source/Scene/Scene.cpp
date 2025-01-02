#include "EnginePCH.h"
#include "Scene.h"

void PScene::OnImGuiRender()
{
    ImGui::Begin("Scene");
    Registry->View<FUUIDComponent, FTransformComponent>([](FUUIDComponent& UUIDComponent, FTransformComponent& TransformComponent)
    {
        ImGui::Text("UUID: %llu", static_cast<uint64>(UUIDComponent.UUID));
        ImGui::DragFloat3(("Translation##" + std::to_string(static_cast<uint64_t>(UUIDComponent.UUID))).c_str(), &TransformComponent.Transform.Translation[0]);
        ImGui::DragFloat3(("Rotation##" + std::to_string(static_cast<uint64_t>(UUIDComponent.UUID))).c_str(), &TransformComponent.Transform.Rotation[0]);
        ImGui::DragFloat3(("Scale##" + std::to_string(static_cast<uint64_t>(UUIDComponent.UUID))).c_str(), &TransformComponent.Transform.Scale[0]);
    });
    ImGui::End();
}