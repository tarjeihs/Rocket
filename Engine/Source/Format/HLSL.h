#pragma once

struct FHLSL
{
    void* Data;
    size_t Size;
};

namespace Format
{
    FHLSL ImportHLSL(const std::string& Path, const std::string& Entrypoint, const std::string& TargetProfile);
}
