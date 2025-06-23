#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#if defined(_WIN32)
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <cstring>
#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <optional>
#include <thread>
#include <functional>
#include <cstdlib>
#include <format>
#include <string>
#include <chrono>
#include <unordered_map>
#include <utility>

#include <entt/entt.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Core/Public/Assert.h"
#include "Core/Public/Engine.h"
#include "Core/Public/Delegate.h"
#include "Scene/Public/Scene.h"
#include "Scene/Public/Component.h"
#include "Scene/Public/Entity.h"
#include "Scene/Public/Registry.h"
#include "IO/Public/Window.h"
