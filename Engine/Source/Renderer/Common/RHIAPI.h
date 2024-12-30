#pragma once

#include "EngineTypes.h"

class IMesh;
class ITexture2D;

template<typename TObject>
struct TRHIAPI;

template<typename TObject>
TObject* NewObject();

uint32 LoadSceneObject(const FString& Path);
uint32 LoadSceneTexture(std::vector<uint8> Data);