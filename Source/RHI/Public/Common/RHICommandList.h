#pragma once

#include <cstdint>
#include <vector>

class IRHICommandListContext
{
public:
    virtual ~IRHICommandListContext() = default;
    virtual void RHISetScissorRect(uint32_t X, uint32_t Y) = 0;
};

struct FRHICommandBase
{
    using TFunc = void(*)(FRHICommandBase*, IRHICommandListContext&);

    TFunc ExecuteAndDestruct = nullptr;
    uint32_t ByteSize = 0;

    void Dispatch(IRHICommandListContext& Context)
    {
        ExecuteAndDestruct(this, Context);
    }
};

template<typename TDerived>
struct TRHICommand : FRHICommandBase
{
    TRHICommand()
    {
        ByteSize = sizeof(TDerived);
        ExecuteAndDestruct = [](FRHICommandBase* Base, IRHICommandListContext& Context)
        {
            auto* Self = static_cast<TDerived*>(Base);
            Self->Execute_Internal(Context);
            Self->~TDerived();
        };
    }
};

class FRHICommandList
{
public:
    explicit FRHICommandList(IRHICommandListContext& InContext)
        : Context(&InContext) {}
    virtual ~FRHICommandList() = default;

    inline IRHICommandListContext& GetContext() const
    {
        return *Context;
    }

    template<typename TCommand, typename... TArgs>
    void Enqueue(TArgs&&... Args)
    {
        size_t Offset = Buffer.size();
        Buffer.resize(Offset + sizeof(TCommand));
        new (Buffer.data() + Offset) TCommand(std::forward<TArgs>(Args)...);
    }

    std::vector<uint8_t> Flush()
    {
        std::vector<uint8_t> Temp;
        Temp.swap(Buffer);
        return Temp;
    }

private:
    IRHICommandListContext* Context;
    std::vector<uint8_t> Buffer;
};

class FRHICommandListImmediate : public FRHICommandList
{
public:
    explicit FRHICommandListImmediate(IRHICommandListContext& Ctx) : FRHICommandList(Ctx), Context(Ctx)
    {
    }

    template<class TCmd, class... TArgs>
    void EnqueueImmediate(TArgs&&... args)
    {
        TCmd cmd(std::forward<TArgs>(args)...);
        cmd.Execute_Internal(Context);
    }
private:
    IRHICommandListContext& Context;
};

class FRHICommandListExecutor
{
public:
    static void Execute(IRHICommandListContext& Context, std::vector<uint8_t>&& Buffer)
    {
        uint8_t* Pointer = Buffer.data();
        uint8_t* EndPointer = Pointer + Buffer.size();

        while (Pointer < EndPointer)
        {
            auto* Command = reinterpret_cast<FRHICommandBase*>(Pointer);
            Command->Dispatch(Context);
            Pointer += Command->ByteSize;
        }
    }
};

struct FRHICommandSetScissorRect : TRHICommand<FRHICommandSetScissorRect>
{
    uint32_t X = 0, Y = 0;
    FRHICommandSetScissorRect(uint32_t InX, uint32_t InY) : X(InX), Y(InY) {}

    void Execute_Internal(IRHICommandListContext& Ctx)
    {
        Ctx.RHISetScissorRect(X, Y);
    }
};