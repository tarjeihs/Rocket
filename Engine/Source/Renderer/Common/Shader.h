#pragma once

#include <Windows.h>
#include <wrl.h>
#include <dxcapi.h>
#include <vector>
#include <string>
#include <functional>

using Microsoft::WRL::ComPtr;

class PShader
{
public:
	virtual ~PShader() = default;

protected:
	// Caution: Must manually free IDxcBlob pointer after use
	static void CompileShaderHLSL(const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile, std::function<void(const ComPtr<IDxcBlob>&)>&& Callback);
};