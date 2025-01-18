#pragma once

#include <cstddef>

enum class EBufferUsageFlag : int8
{
    None            = 0,
    Storage         = 1 << 0,
    Uniform         = 1 << 1,
    Vertex          = 1 << 2,
    Index           = 1 << 3,
    Indirect        = 1 << 4,
};

enum class EBufferTransferFlag : int8
{
    None            = 0,
    Read            = 1 << 0,
    Write           = 1 << 1
};

enum class EBufferMemoryFlag : int8
{
    None            = 0,
    Host            = 1,    // CPU
    HostToDevice    = 2,    // CPU to GPU
    Device          = 3,    // GPU
    DeviceToHost    = 4,    // GPU to CPU
};

inline EBufferUsageFlag operator|(EBufferUsageFlag LHS, EBufferUsageFlag RHS) { return static_cast<EBufferUsageFlag>(static_cast<int8>(LHS) | static_cast<int8>(RHS)); }
inline EBufferUsageFlag operator&(EBufferUsageFlag LHS, EBufferUsageFlag RHS) { return static_cast<EBufferUsageFlag>(static_cast<int8>(LHS) & static_cast<int8>(RHS)); }

inline EBufferTransferFlag operator|(EBufferTransferFlag LHS, EBufferTransferFlag RHS) { return static_cast<EBufferTransferFlag>(static_cast<int8>(LHS) | static_cast<int8>(RHS)); }
inline EBufferTransferFlag operator&(EBufferTransferFlag LHS, EBufferTransferFlag RHS) { return static_cast<EBufferTransferFlag>(static_cast<int8>(LHS) & static_cast<int8>(RHS)); }

inline bool HasFlag(EBufferUsageFlag LHS, EBufferUsageFlag RHS) { return (static_cast<int8>(LHS) & static_cast<int8>(RHS)) != 0; }
inline bool HasFlag(EBufferTransferFlag LHS, EBufferTransferFlag RHS) { return (static_cast<int8>(LHS) & static_cast<int8>(RHS)) != 0; }

struct FBufferCreateInfo
{
    EBufferUsageFlag BufferUsageFlags;
    EBufferTransferFlag BufferTransferFlags;
    EBufferMemoryFlag MemoryUsageFlags;
    SizeType Size;
};

class IBuffer
{
public:
    virtual ~IBuffer() = default;

    virtual void Initialize(FBufferCreateInfo& CreateInfo) = 0;
    virtual void Shutdown() = 0;
    virtual void Submit(const void* Data, size_t Size, size_t Offset = 0) = 0;
};