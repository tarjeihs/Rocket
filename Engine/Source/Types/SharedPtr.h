#pragma once

#include "Core/Assert.h"

template<typename TElement>
class TSharedPtr
{
public:
    TSharedPtr() 
        : Pointer(nullptr), RefCount(new uint32_t(1)) 
    {
    }

    explicit TSharedPtr(TElement* InPointer) 
        : Pointer(InPointer), RefCount(new uint32_t(1)) 
    {
    }

    ~TSharedPtr()
    {
        Release();
    }

    TSharedPtr(const TSharedPtr<TElement>& Other)
    {
        Pointer = Other.Pointer;
        RefCount = Other.RefCount;
        (*RefCount)++;
    }

    TSharedPtr(TSharedPtr<TElement>&& Other) noexcept
    {
        Pointer = Other.Pointer;
        RefCount = Other.RefCount;
        Other.Pointer = nullptr;
        Other.RefCount = nullptr;
    }

    TSharedPtr<TElement>& operator=(const TSharedPtr<TElement>& Other)
    {
        if (this != &Other)
        {
            Release(); // Release current object
            Pointer = Other.Pointer;
            RefCount = Other.RefCount;
            (*RefCount)++;
        }
        return *this;
    }

    TSharedPtr<TElement>& operator=(TSharedPtr<TElement>&& Other) noexcept
    {
        if (this != &Other)
        {
            Release(); // Release current object
            Pointer = Other.Pointer;
            RefCount = Other.RefCount;
            Other.Pointer = nullptr;
            Other.RefCount = nullptr;
        }
        return *this;
    }

    TElement& operator*() const
    {
        RK_ASSERT(Pointer != nullptr, "Dereferencing a null pointer.");
        return *Pointer;
    }

    TElement* operator->() const
    {
        RK_ASSERT(Pointer != nullptr, "Dereferencing a null pointer.");
        return Pointer;
    }

    TElement* Get() const
    {

        return Pointer;
    }

    uint32_t* GetRefCount() const
    {
        return RefCount;
    }

    void Reset(TElement* InPointer = nullptr)
    {
        static_assert(false, "Function implemention is missing");
    }

    void Release()
    {
        if (!RefCount)
        {
            return;
        }

        (*RefCount)--;

        if (RefCount == 0)
        {
            delete Pointer;
            delete RefCount;
            Pointer = nullptr;
            RefCount = nullptr;
        }
    }

private:
    TElement* Pointer;
    
    uint32_t* RefCount;
};

template<typename TPointer>
TSharedPtr<TPointer> MakeShared()
{
    return TSharedPtr<TPointer>(new TPointer());
}