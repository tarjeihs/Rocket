#include "Shader.h"

#include "Core/Assert.h"

void PShader::CompileShaderHLSL(const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile, std::function<void(const ComPtr<IDxcBlob>&)>&& Callback)
{
	HMODULE DxcDll = LoadLibrary("dxcompiler.dll");
	RK_ASSERT(DxcDll, "Failed to dynamically load DXC library (Ensure that the DLL is installed and environment is set).");

	// Define a function pointer type for DxcCreateInstance and get the address of the DxcCreateInstance function.
	typedef HRESULT(WINAPI* DxcCreateInstanceProc)(REFCLSID, REFIID, LPVOID*);
	DxcCreateInstanceProc DxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(DxcDll, "DxcCreateInstance");
	RK_ASSERT(DxcCreateInstance, "Failed to locate DxcCreateInstance function address.");

	// ComPtr must be released before we free the DXC DLL module, hence the scope.
	{
		RK_LOG_INFO("Current working directory: {}", std::filesystem::current_path().string());

		ComPtr<IDxcBlob> ShaderBlob;
		ComPtr<IDxcCompiler> Compiler;
		ComPtr<IDxcLibrary> Library;
		ComPtr<IDxcBlobEncoding> SourceBlob;
		ComPtr<IDxcOperationResult> OperationResult;

		// Create the DXC library instance
		HRESULT Result = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));
		RK_ASSERT(!FAILED(Result), "Failed to create DXC Library instance.");

		// Create the DXC compiler instance
		Result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
		RK_ASSERT(!FAILED(Result), "Failed to create DXC Compiler instance.");
		
		// Load and encode the shader source file
		Result = Library->CreateBlobFromFile(ShaderSourcePath.c_str(), nullptr, &SourceBlob);
		RK_ASSERT(!FAILED(Result), "Failed to load shader source file.");

		// Compilation arguments
		std::wstring TargetProfileW(TargetProfile.begin(), TargetProfile.end());
		std::vector<LPCWSTR> Arguments;
		Arguments.push_back(L"-E");
		Arguments.push_back(Entrypoint.c_str());
		Arguments.push_back(L"-T");
		Arguments.push_back(TargetProfileW.c_str());
		Arguments.push_back(L"-spirv");
		Arguments.push_back(L"-fspv-target-env=vulkan1.3");
		Arguments.push_back(L"-fspv-extension=SPV_KHR_physical_storage_buffer");

		// Compile HLSL into SPIR-V bytecode using DirectX Shader Compiler
		Result = Compiler->Compile(
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
		RK_ASSERT(!FAILED(Result), "Shader compilation failed.");

		HRESULT CompileStatus;
		Result = OperationResult->GetStatus(&CompileStatus);
		RK_ASSERT(!FAILED(Result), "Undefined compile status.");

		if (FAILED(CompileStatus))
		{
			ComPtr<IDxcBlobEncoding> ErrorBlob;
			Result = OperationResult->GetErrorBuffer(&ErrorBlob);
			if (SUCCEEDED(Result) && ErrorBlob)
			{
				std::string Exception((char*)ErrorBlob->GetBufferPointer(), ErrorBlob->GetBufferSize());
				RK_LOG_ERROR("Shader Compilation Error: {}", Exception);
			}
		}

		// Retrieve the compiled shader
		Result = OperationResult->GetResult(&ShaderBlob);
		RK_ASSERT(!FAILED(Result), "Unable to retrieve compiled shader.");

		Callback(ShaderBlob);
	}
	
	FreeLibrary(DxcDll);
}