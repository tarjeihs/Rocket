#pragma once

template<typename TPointer>
class TUniquePtr
{
public:
    explicit TUniquePtr(TPointer* InPointer = nullptr)
        : Pointer(InPointer)
    {
    }

    ~TUniquePtr()
    {
        delete Pointer;
    }

    TUniquePtr(const TUniquePtr&) = delete;
    TUniquePtr& operator=(const TUniquePtr&) = delete;

    TUniquePtr(TUniquePtr&& Other) noexcept
    {
        Pointer = Other.Pointer;
        Other.Pointer = nullptr;
    }

    TUniquePtr& operator=(TUniquePtr&& Other) noexcept
    {
        if (this != Other)
        {
            delete Pointer;
            Pointer = Other.Pointer;
            Other.Pointer = nullptr;
        }
        return *this;
    }

    TPointer& operator*() const
    {
        return *Pointer;
    }

    TPointer* operator->() const
    {
        return Pointer;
    }

    TPointer* Get() const
    {
        return Pointer;
    }

    TPointer* Release()
    {
        TPointer* Temp = Pointer;
        Pointer = nullptr;
        return Temp;
    }

    void Reset(TPointer* InPointer = nullptr)
    {
        delete Pointer;
        Pointer = InPointer;
    }

private:
    TPointer* Pointer;
};

template<typename TPointer>
TUniquePtr<TPointer> MakeUnique()
{
    return TUniquePtr<TPointer>(new TPointer());
}