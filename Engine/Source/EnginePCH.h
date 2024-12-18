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

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <tiny_gltf.h>
#include <spirv_cross/spirv.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_cross_containers.hpp>

#include "EngineSettings.h"
#include "Core/Assert.h"
#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/Delegate.h"
#include "Memory/Memory.h"
#include "Scene/Scene.h"
#include "Scene/Component.h"
#include "Scene/Entity.h"
#include "Scene/Registry.h"
#include "Renderer/RHI.h"
#include "Renderer/VulkanRHI.h"
#include "Renderer/Settings.h"
#include "Math/Math.h"
#include "Utils/FileSystem.h"
#include "Utils/Hash.h"
#include "Utils/Profiler.h"
#include "Types/Array.h"
#include "Types/SharedPtr.h"
#include "Types/DoubleLinkedList.h"
#include "Types/Map.h"
#include "Types/Pair.h"
#include "Types/String.h"
#include "Types/UniquePtr.h"
#include "EngineTypes.h"
#include "EngineMacros.h"