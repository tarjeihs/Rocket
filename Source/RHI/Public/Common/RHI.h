#pragma once

enum class ERHIInterfaceType : uint8_t
{
    None = 0,
    Vulkan,
    D3D12,
    Metal,
    WebGL
};

class IRHI
{
public:
    virtual ~IRHI() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Tick(float DeltaTime) = 0;

    [[nodiscard]] virtual const char* GetName() const  = 0;
    [[nodiscard]] virtual const char* GetVersion() const  = 0;
    [[nodiscard]] virtual ERHIInterfaceType  GetInterfaceType() const noexcept = 0;
    [[nodiscard]] virtual IRHI* GetNonValidationRHI() const noexcept { return const_cast<IRHI*>(this); }
};

class IRHIModule
{
public:
    virtual ~IRHIModule() = default;

    virtual IRHI* CreateRHI() = 0;
};

extern IRHI* GRHI;
extern IRHIModule* GRHIModule;

void SetRHIModule(ERHIInterfaceType InInterfaceType);

IRHIModule* CreateVulkanRHIModule();

template<typename TRHI>
inline TRHI* CastRHI(IRHI* InRHI)
{
#ifdef _DEBUG
    assert(InRHI && InRHI->GetInterfaceType() == TRHI::StaticType);
#endif
    return static_cast<TRHI*>(InRHI->GetNonValidationRHI());
}

template<typename TRHI>
inline TRHI* GetRHI()
{
    return CastRHI<TRHI>(GRHI);
}