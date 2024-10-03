#pragma once

#include <vector>

struct SShaderModuleBinary
{
	void* Data;
	size_t Size;
};

class IShader
{
public:
	virtual ~IShader() = default;

	virtual void CreateShader(std::vector<SShaderModuleBinary> ShaderBinaryObject) = 0;
	virtual void DestroyShader() = 0;
};
