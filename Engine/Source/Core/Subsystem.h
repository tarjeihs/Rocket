#pragma once

#include <vector>

class ISubsystem 
{
public:
    virtual ~ISubsystem() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(float DeltaTime) {}
};

struct SSubsystemStaticRegistry 
{
public:
    static SSubsystemStaticRegistry& GetStaticRegistry() 
    {
        static SSubsystemStaticRegistry StaticRegistry;
        return StaticRegistry;
    }

    std::vector<ISubsystem*>& GetSubsystems()
    {
        return Subsystems;
    }

    void AddSubsystem(ISubsystem* Subsystem)
    {
        Subsystems.push_back(Subsystem);
    }

private:
    std::vector<ISubsystem*> Subsystems;
};

#define REGISTER_SUBSYSTEM(ClassName) \
    static struct ClassName##AutoRegister \
    { \
        ClassName##AutoRegister() \
        { \
            SSubsystemStaticRegistry::GetStaticRegistry().AddSubsystem(new ClassName()); \
        } \
    } ClassName##AutoRegisterInstance;
