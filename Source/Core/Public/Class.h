#pragma once

#include <cstdlib>
#include <type_traits>

template<typename T>
class TClass
{
public:
	TClass()
	{
		Class = (T*)malloc(sizeof(T));
	}

	~TClass()
	{
		free(Class);
	}

	TClass(const TClass<T>& Other)
	{
		Class = (T*)malloc(sizeof(T));
		memcpy(Class, Other.Class, sizeof(T));
	}

	TClass<T>& operator=(const TClass<T>& Other)
	{
		if (this != &Other)
		{
			free(Class);

			Class = (T*)malloc(sizeof(T));
			memcpy(Class, Other.Class, sizeof(T));
		}
		return *this;
	}

	template<typename U>
	TClass<T>& operator=(const TClass<U>& Other)
	{
		static_assert(std::is_base_of<T, U>::value, "U must be derived from T");

		free(Class);
		Class = (T*)malloc(sizeof(U));
		memcpy(Class, Other.Class, sizeof(U));

		return *this;
	}

	T* GetClass() const
	{
		return Class;
	}

	template<typename U>
	TClass<U> StaticCast() const
	{
		static_assert(std::is_base_of<T, U>::value || std::is_base_of<T, U>::value, "U must be a base or derived from T");
		TClass<U> CastedClass;
		CastedClass.Class = static_cast<U*>(Class);
		return CastedClass;
	}

private:
	T* Class;

	template<typename U> friend class TClass;
};

template<typename TObject>
TObject* New(TClass<TObject> Class)
{
	void* Memory = malloc(sizeof(TObject));
	TObject* Object = new (Memory) TObject(*reinterpret_cast<TObject*>(Class.GetClass()));
	return Object;
}