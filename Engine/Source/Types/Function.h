#pragma once

#include <type_traits>

template<typename T>
class TFunction
{
public:
	TFunction(T(*InFunc)() = nullptr)
		: Func(InFunc)
	{
	}

	TFunction(TFunction&& Other) noexcept
		: Func(std::exchange(Other.Func, nullptr))
	{
	}

	TFunction& operator=(TFunction&& Other) noexcept
	{
		if (this != &Other)
		{
			Func = std::exchange(Other.Func, nullptr);
		}
		return *this;
	}

	TFunction(const TFunction&) = delete;
	TFunction& operator=(const TFunction&) = delete;

	TFunction& operator=(T(*InFunc)())
	{
		Func = InFunc;
		return *this;
	}

	T operator()() const
	{
		return Func();
	}
private:
	T(*Func)();
	void* Object;
};