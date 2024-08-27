#pragma once

#include <vector>
#include <string>
#include <functional>
#include <cstdint>

#ifdef RK_PLATFORM_WINDOWS
	#include <Windows.h>
	#include <wrl.h>
	#include <dxc/dxcapi.h>
	
	using Microsoft::WRL::ComPtr;
#endif

#ifdef RK_PLATFORM_LINUX
	#include <dlfcn.h>
	#include <wchar.h>
	#include <dxc/dxcapi.h>
#endif

class PShader
{
public:
	virtual ~PShader() = default;

protected:

#ifdef RK_PLATFORM_WINDOWS
	static void CompileShaderHLSL(const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile, std::function<void(const ComPtr<IDxcBlob>&)>&& Callback);
#endif

#ifdef RK_PLATFORM_LINUX
	static void CompileShaderHLSL(const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile, std::function<void(const CComPtr<IDxcBlob>&)>&& Callback);
#endif
};