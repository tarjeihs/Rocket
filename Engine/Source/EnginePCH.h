#define IMGUI_DEFINE_MATH_OPERATORS

#pragma once

#include <cstring>
#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <optional>
#include <functional>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <cstdlib>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <tiny_gltf.h>

#include "Asset/Asset.h"
#include "Asset/AssetManager.h"
#include "Core/Assert.h"
#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/Delegate.h"
#include "Scene/Scene.h"
#include "Scene/Component.h"
#include "Scene/Entity.h"
#include "Scene/Registry.h"
#include "Renderer/RHI.h"
#include "Renderer/VulkanRHI.h"
#include "EngineMacros.h"
#include "Math/Math.h"
#include "Utils/FileSystem.h"
#include "Utils/Hash.h"