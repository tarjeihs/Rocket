#pragma once

enum class EShaderStage
{
	Vertex,
	Fragment,
	Compute
};

struct FShaderCreateInfo
{
	EShaderStage Stage;

	std::string Name;
	std::string Path;
	std::string Entrypoint;
};

class IShader
{
public:
	virtual ~IShader() = default;

	virtual void CreateShader(FShaderCreateInfo& CreateInfo) = 0;
	virtual void DestroyShader() = 0;
};