#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Asset/Asset.h"

class IMaterial;
class IMesh;

struct SAssetTable
{
    void AddAsset(SAsset&& Asset);
    void AddAsset(const SAsset& Asset);
    
    const SAsset* GetAssetByUUID(const uint32_t UUID) const;
    const SAsset* GetAssetByName(const std::string& Name) const;

    std::vector<SAsset>::iterator begin() { return Assets.begin(); }
    std::vector<SAsset>::iterator end() { return Assets.end(); }
    std::vector<SAsset>::const_iterator begin() const { return Assets.begin(); }
    std::vector<SAsset>::const_iterator end() const { return Assets.end(); }

private:
    std::vector<SAsset> Assets;
};

class PAssetManager
{
public:
    void LoadAsset(const std::string& FilePath, const std::string& Type, const std::string& Name);
    void SaveAsset(const std::string& FilePath, const std::string& Type, const std::string& Name);

    const SAsset* GetAsset(const std::string& Type, const std::string& Name) const;

    auto BeginTableIterator() const { return Tables.begin(); }
    auto EndTableIterator() const { return Tables.end(); }

    void Dispose();

private:
    std::unordered_map<std::string, SAssetTable> Tables;
};