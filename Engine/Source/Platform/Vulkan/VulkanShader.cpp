#include "EnginePCH.h"
#include "VulkanShader.h"

#include <Windows.h>
#include <wrl.h>
#include <dxcapi.h>
#include <vector>

#include "Core/Assert.h"

using Microsoft::WRL::ComPtr;

SShaderProgram PVulkanShader::Compile(VkDevice LogicalDevice, EShaderType ShaderType, const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile)
{
	SShaderProgram ShaderProgram;

	// Load the DXC DLL dynamically
	HMODULE DxcDll = LoadLibrary("dxcompiler.dll");
	RK_ENGINE_ASSERT(DxcDll, "Failed to load dxcompiler.dll!");

	// Define a function pointer type for DxcCreateInstance and get the address of the DxcCreateInstance function.
	typedef HRESULT(WINAPI* DxcCreateInstanceProc)(REFCLSID, REFIID, LPVOID*);
	DxcCreateInstanceProc DxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(DxcDll, "DxcCreateInstance");
	RK_ENGINE_ASSERT(DxcCreateInstance, "Failed to locate DxcCreateInstance function address.");

	// ComPtr must be released before we free the DXC DLL module, hence the scope.
	{
		ComPtr<IDxcCompiler> Compiler;
		ComPtr<IDxcLibrary> Library;
		ComPtr<IDxcBlobEncoding> SourceBlob;
		ComPtr<IDxcOperationResult> OperationResult;

		// Create the DXC library instance
		HRESULT DxcResult = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));
		RK_ENGINE_ASSERT(!FAILED(DxcResult), "Failed to create DXC Library instance.");

		// Create the DXC compiler instance
		DxcResult = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
		RK_ENGINE_ASSERT(!FAILED(DxcResult), "Failed to create DXC Compiler instance.");
		RK_ENGINE_INFO("Current working directory: {}", std::filesystem::current_path().string());

		// Load and encode the shader source file
		DxcResult = Library->CreateBlobFromFile(ShaderSourcePath.c_str(), nullptr, &SourceBlob);
		RK_ENGINE_ASSERT(!FAILED(DxcResult), "Failed to load shader source file.");

		// Compilation arguments
		std::wstring TargetProfileW(TargetProfile.begin(), TargetProfile.end());
		std::vector<LPCWSTR> Arguments;
		Arguments.push_back(L"-E");
		Arguments.push_back(Entrypoint.c_str());
		Arguments.push_back(L"-T");
		Arguments.push_back(TargetProfileW.c_str());
		Arguments.push_back(L"-spirv");
		Arguments.push_back(L"-fvk-use-dx-layout");

		// Compile HLSL into SPIR-V bytecode using DirectX Shader Compiler
		DxcResult = Compiler->Compile(
			SourceBlob.Get(),
			ShaderSourcePath.c_str(),
			Entrypoint.c_str(),
			TargetProfileW.c_str(),
			Arguments.data(),
			Arguments.size(),
			nullptr,
			0,
			nullptr,
			&OperationResult);
		RK_ENGINE_ASSERT(!FAILED(DxcResult), "Shader compilation failed.");

		HRESULT CompileStatus;
		DxcResult = OperationResult->GetStatus(&CompileStatus);
		RK_ENGINE_ASSERT(!FAILED(DxcResult), "Undefined compile status.");

		if (FAILED(CompileStatus))
		{
			ComPtr<IDxcBlobEncoding> ErrorBlob;
			DxcResult = OperationResult->GetErrorBuffer(&ErrorBlob);
			if (SUCCEEDED(DxcResult) && ErrorBlob)
			{
				std::string Exception((char*)ErrorBlob->GetBufferPointer(), ErrorBlob->GetBufferSize());
				RK_ENGINE_ERROR("Shader Compilation Error: {}", Exception);
			}
		}

		// Containing the compiled shader
		ComPtr<IDxcBlob> ShaderBlob;
		DxcResult = OperationResult->GetResult(&ShaderBlob);
		RK_ENGINE_ASSERT(!FAILED(DxcResult), "Unable to retrieve compiled shader.");

		// Shader module configuration
		VkShaderModuleCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		CreateInfo.codeSize = ShaderBlob->GetBufferSize();
		CreateInfo.pCode = reinterpret_cast<const uint32_t*>(ShaderBlob->GetBufferPointer());

		VkShaderModule ShaderModule;
		VkResult Result = vkCreateShaderModule(LogicalDevice, &CreateInfo, nullptr, &ShaderModule);
		RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Shader Module.");

		VkPipelineShaderStageCreateInfo ShaderCreateInfo{};
		ShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ShaderCreateInfo.pNext = nullptr;
		ShaderCreateInfo.pName = "main";
		ShaderCreateInfo.module = ShaderModule;

		switch (ShaderType)
		{
			case RK_SHADERTYPE_VERTEXSHADER: { ShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break; }
			case RK_SHADERTYPE_FRAGMENTSHADER: { ShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break; }
			case RK_SHADERTYPE_COMPUTE_SHADER: { ShaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break; }
		}

		ShaderProgram = { ShaderCreateInfo, ShaderModule };
		ShaderPrograms.push_back(ShaderProgram);
	}

	FreeLibrary(DxcDll);

	return ShaderProgram;
}

void PVulkanShader::Free(VkDevice LogicalDevice, SShaderProgram& ShaderProgram)
{
	for (const auto& ShaderStage : ShaderPrograms)
	{
		// Memory for this shader module will be freed. It has been internally copied by Vulkan.
		vkDestroyShaderModule(LogicalDevice, ShaderStage.ShaderModule, nullptr);
	}
}