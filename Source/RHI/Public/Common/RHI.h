#pragma once

enum class ERHIType
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
    virtual void Resize() = 0;
    virtual void Render() = 0;

    virtual ERHIType GetRHIType() const { return ERHIType::None; }
	virtual IRHI* GetNonValidationRHI() { return this; }

};

extern IRHI* GRHI;

template<typename TRHI>
inline TRHI* CastRHI(IRHI* InRHI)
{
    return static_cast<TRHI*>(InRHI->GetNonValidationRHI());
}
    
template<typename TRHI>
inline TRHI* GetRHI()
{
    return CastRHI<TRHI>(GRHI);
}

class IRHIModule
{
public:
    virtual ~IRHIModule() = default;
    virtual IRHI* CreateRHI() = 0;
};

IRHIModule* CreateVulkanRHIModule();
