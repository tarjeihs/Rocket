#pragma once

template<typename TPointer>
class TUniquePtr
{
public:
    explicit TUniquePtr(TPointer* InPointer = nullptr)
        : Pointer(InPointer)
    {
    }

    TUniquePtr(std::nullptr_t)
        : Pointer(nullptr)
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

    TUniquePtr<TPointer>& operator=(TUniquePtr<TPointer>&& Other) noexcept
    {
        if (this != &Other)
        {
            delete Pointer;
            Pointer = Other.Pointer;
            Other.Pointer = nullptr;
        }
        return *this;
	}

	// Converting constructor for derived to base conversion
	template<typename U, typename = typename std::enable_if<std::is_base_of<TPointer, U>::value>::type>
	TUniquePtr(TUniquePtr<U>&& Other) noexcept
		: Pointer(Other.Pointer)
	{
		Other.Pointer = nullptr;
	}

	// Converting assignment operator for derived to base conversion
	template<typename U, typename = typename std::enable_if<std::is_base_of<TPointer, U>::value>::type>
	TUniquePtr& operator=(TUniquePtr<U>&& Other) noexcept
	{
		if (this != reinterpret_cast<TUniquePtr*>(&Other))
		{
			delete Pointer;
			Pointer = Other.Pointer;
			Other.Pointer = nullptr;
		}
		return *this;
	}

    TUniquePtr& operator=(std::nullptr_t)
    {
        Reset();
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

	// Granting access to private members for TUniquePtr<U> where U is derived from TPointer
	template<typename U> friend class TUniquePtr;
};

template<typename TPointer>
auto MakeUnique() -> TUniquePtr<typename std::remove_pointer<decltype(new TPointer())>::type>
{
	return TUniquePtr<TPointer>(new TPointer());
}

template<typename TPointer, typename... TArgs>
auto MakeUnique(TArgs&&... Args) -> TUniquePtr<typename std::remove_pointer<decltype(new TPointer())>::type>
{
	return TUniquePtr<TPointer>(new TPointer(std::forward<TArgs>(Args)...));
}