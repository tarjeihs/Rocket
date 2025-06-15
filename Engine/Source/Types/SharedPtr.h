#pragma once

#include "Core/Assert.h"

struct FPointerReferenceCounter
{
    FPointerReferenceCounter()
        : StrongCounter(0), WeakCounter(0)
    {
    }

    FPointerReferenceCounter(uint32 InStrongCounter, uint32 InWeakCounter)
        : StrongCounter(InStrongCounter), WeakCounter(InWeakCounter)
    {
    }

    uint32 StrongCounter;
    uint32 WeakCounter;
};

template<typename TPointer>
class TSharedPtr
{
public:
    TSharedPtr()
        : Pointer(nullptr), PointerReferenceCounter() 
    {
    }
    
    explicit TSharedPtr(TPointer* InPointer)
        : Pointer(InPointer), PointerReferenceCounter(new FPointerReferenceCounter(1, 0))
    {
    }

    explicit TSharedPtr(TPointer* InPointer, FPointerReferenceCounter* InPointerReferenceCounter) 
        : Pointer(InPointer), PointerReferenceCounter(InPointerReferenceCounter)
    {
    }

    ~TSharedPtr()
    {
        Release();
    }

    TSharedPtr(const TSharedPtr<TPointer>& Other)
    {
        Pointer = Other.Pointer;
        PointerReferenceCounter = Other.PointerReferenceCounter;
        PointerReferenceCounter->StrongCounter++;
    }

    TSharedPtr(TSharedPtr<TPointer>&& Other) noexcept
    {
        Pointer = Other.Pointer;
        PointerReferenceCounter = Other.PointerReferenceCounter;
        Other.Pointer = nullptr;
        Other.PointerReferenceCounter = nullptr;
    }

    TSharedPtr<TPointer>& operator=(const TSharedPtr<TPointer>& Other)
    {
        if (this != &Other)
        {
            Release(); // Release current object
            Pointer = Other.Pointer;
            PointerReferenceCounter = Other.PointerReferenceCounter;
            PointerReferenceCounter->StrongCounter++;
        }
        return *this;
    }

    TSharedPtr<TPointer>& operator=(TSharedPtr<TPointer>&& Other) noexcept
    {
        if (this != &Other)
        {
            Release(); // Release current object
            Pointer = Other.Pointer;
            PointerReferenceCounter = Other.PointerReferenceCounter;
            Other.Pointer = nullptr;
            Other.PointerReferenceCounter = nullptr;
        }
        return *this;
    }

    TPointer& operator*() const
    {
        RK_ASSERT(Pointer != nullptr, "Dereferencing a null pointer.");
        return *Pointer;
    }

    TPointer* operator->() const
    {
        RK_ASSERT(Pointer != nullptr, "Dereferencing a null pointer.");
        return Pointer;
    }

    TPointer* Get() const
    {
        RK_ASSERT(Pointer != nullptr, "Dereferencing a null pointer.");
        return Pointer;
    }

    bool IsValid() const
    {
        return Pointer != nullptr && PointerReferenceCounter->StrongCounter != 0;
    }

    FPointerReferenceCounter* GetPointerReferenceCounter() const
    {
        return PointerReferenceCounter;
    }

    void Reset(TPointer* InPointer = nullptr)
    {
        static_assert(false, "Function implemention is missing");
    }

    void Release()
    {
        PointerReferenceCounter->StrongCounter--;

        if (PointerReferenceCounter->StrongCounter == 0)
        {
            delete Pointer;
            Pointer = nullptr;
            
            if (PointerReferenceCounter->WeakCounter == 0)
            {
                delete PointerReferenceCounter;
                PointerReferenceCounter = nullptr;
            }
        }
    }

private:
    TPointer* Pointer;

    FPointerReferenceCounter* PointerReferenceCounter;

    template<typename U>
    friend class TWeakPtr;
};

template<typename TPointer>
TSharedPtr<TPointer> MakeShared()
{
    return TSharedPtr<TPointer>(new TPointer());
}

template<typename TPointer, typename... TArgs>
TSharedPtr<TPointer> MakeShared(TArgs&&... Args)
{
    return TSharedPtr<TPointer>(new TPointer(std::forward<TArgs>(Args)...));
}