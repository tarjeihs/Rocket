#pragma once

#include "EngineTypes.h"

class ISceneBuffer;
class IMaterial;
class IShader;

template<typename TObject>
struct TRHIAPI;

template<typename TObject>
TObject* NewObject();

uint32 AddInstance(std::vector<FVertex> Vertices, std::vector<uint32> Indices);
uint32 AddTexture(std::vector<uint8> Data);