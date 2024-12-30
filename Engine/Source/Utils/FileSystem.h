#pragma once

#include <fstream>
#include <vector>

struct SBlob
{
    std::string Name;
    std::string Extension;

    std::vector<uint8_t> Data;
};

class PFileSystem 
{
public:
    static SBlob ReadFileBinary(const FString& Path)
    {
        SBlob Blob;

        std::string String(Path.GetData(), Path.GetSize());
        std::ifstream File(String, std::ios::binary | std::ios::ate);
        std::streamsize FileSize = File.tellg();
        Blob.Data.resize(static_cast<size_t>(FileSize));
        File.seekg(0, std::ios::beg);
        File.read(reinterpret_cast<char*>(Blob.Data.data()), FileSize);
        File.close();
        
        size_t DotPosition = String.find_last_of('.');
        size_t SeperatorPosition = String.find_last_of("/\\");
        
        Blob.Name = String.substr(SeperatorPosition + 1, DotPosition - SeperatorPosition - 1);
        Blob.Extension = String.substr(DotPosition + 1);

        return Blob;
    }
};