#pragma once

#include "Types/SharedPtr.h"

template<typename TPointer>
class TWeakPtr
{
public:
    TWeakPtr()
        : Pointer(nullptr), PointerReferenceCounter(nullptr)
    {
    }

    explicit TWeakPtr(const TSharedPtr<TPointer>& InSharedPtr)
        : Pointer(InSharedPtr.Pointer), PointerReferenceCounter(InSharedPtr.PointerReferenceCounter)
    {
        ++PointerReferenceCounter->WeakCounter;
    }

    ~TWeakPtr()
    {
        --PointerReferenceCounter->WeakCounter;
        if (PointerReferenceCounter->StrongCounter == 0 && PointerReferenceCounter->WeakCounter == 0)
        {
            delete PointerReferenceCounter;
            PointerReferenceCounter = nullptr;
        }
    }

    TWeakPtr& operator=(const TSharedPtr<TPointer>& InSharedPtr)
    {
        Pointer = InSharedPtr.Pointer;
        PointerReferenceCounter = InSharedPtr.PointerReferenceCounter;
        ++PointerReferenceCounter->WeakCounter;
        return *this;
    }

    TPointer& operator*() const
    {
        RK_ASSERT(IsValid(), "Dereferencing a dangling pointer.");
        return *Pointer;
    }

    TPointer* operator->() const
    {
        RK_ASSERT(IsValid(), "Dereferencing a dangling pointer.");
        return Pointer;
    }

    TPointer* Get() const
    {
        RK_ASSERT(IsValid(), "Dereferencing a dangling pointer.");
        return Pointer;
    }

    bool IsValid() const
    {
        return PointerReferenceCounter && PointerReferenceCounter->StrongCounter > 0;
    }

    TSharedPtr<TPointer> Lock() const
    {
        if (IsValid())
        {
            return *TSharedPtr<TPointer>(Pointer, PointerReferenceCounter);
        }
        return TSharedPtr<TPointer>();
    }

private:
    TPointer* Pointer;

    FPointerReferenceCounter* PointerReferenceCounter;
};