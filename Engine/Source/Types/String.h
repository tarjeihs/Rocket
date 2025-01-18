#pragma once

#include <cstdlib>
#include <cstring>
#include <string>
#include "EngineTypes.h"

class FString
{
public:
    FString() 
        : Size(0)
    {
        Data = (char*)malloc(1);
        Data[0] = '\0';
    }

    FString(const char* InData, size_t InSize)
        : Size(InSize)
    {
        Data = (char*)malloc(Size + 1);
        memcpy(Data, InData, Size);
        Data[Size] = '\0';
    }

    FString(const char* InData)
        : Size(strlen(InData))
    {
        Data = (char*)malloc(Size + 1);
        memcpy(Data, InData, Size);
        Data[Size] = '\0';
    }

    FString(const std::string& InData)
        : Size(InData.size())
    {
        Data = (char*)malloc(Size + 1);
        memcpy(Data, InData.c_str(), Size);
        Data[Size] = '\0';
    }

    ~FString()
    {
        delete[] Data;
        Size = 0;
    }

    FString(const FString& Other)
        : Size(Other.Size)
    {
        Data = (char*)malloc(Size + 1);
        memcpy(Data, Other.Data, Size);
        Data[Size] = '\0';
    }

    FString(FString&& Other) noexcept
    {
        Data = Other.Data;
        Size = Other.Size;
        
        Other.Data = nullptr;
        Other.Size = 0;
    }

    FString& operator=(const FString& Other)
    {
        if (this != &Other)
        {
            delete[] Data;
            
            Size = Other.Size;
            Data = (char*)malloc(Size + 1);
            memcpy(Data, Other.Data, Size);
            Data[Size] = '\0';

        }
        return *this;
    }

    FString& operator=(FString&& Other) noexcept
    {
        if (this != &Other)
        {
            delete[] Data;

            Size = Other.Size;
            Data = Other.Data;

            Other.Data = nullptr;
            Other.Size = 0;
        }
        return *this;
    }

    const char* GetData() const
    {
        return Data;
    }

    std::size_t GetSize() const
    {
        return Size;
    }

    char& operator[](size_t Index)
    {
        return Data[Index];
    }

    const char& operator[](size_t Index) const
    {
        return Data[Index];
    }

    FString operator+(const FString& Concatenation) const
    {
        size_t NewSize = Size + Concatenation.Size;
        char* NewData = (char*)malloc(NewSize + 1);

        memcpy(NewData, Data, Size);
        memcpy(NewData + Size, Concatenation.Data, Concatenation.Size);
        
        NewData[NewSize] = '\0';
        return FString(NewData, NewSize);
    }

    FString operator+(const char* Concatenation) const
    {
        size_t NewSize = Size + strlen(Concatenation);
        char* NewData = (char*)malloc(NewSize + 1);

        memcpy(NewData, Data, Size);
        memcpy(NewData + Size, Concatenation, strlen(Concatenation));

        NewData[NewSize] = '\0';
        return FString(NewData, NewSize);
    }

    bool operator==(const FString& Other) const
    {
        if (Size == Other.Size)
        {
            return memcmp(Data, Other.Data, Size) == 0;
        }
        return false;
    }

    class FIterator
    {
    public:
        FIterator(char* InPointer) : Pointer(InPointer) {}
        
        char& operator*() { return *Pointer; }

        FIterator& operator++()
        {
            ++Pointer;
            return *this;
        }

        FIterator operator++(int32)
        {
            FIterator Temp = *this;
            ++Pointer;
            return Temp;
        }

        bool operator==(const FIterator& Other) const { return Pointer == Other.Pointer; }
        bool operator!=(const FIterator& Other) const { return Pointer != Other.Pointer; }

    private:
        char* Pointer;
    };

	class FConstIterator
	{
	public:
		FConstIterator(const char* InPointer) : Pointer(InPointer) {}

		const char& operator*() const { return *Pointer; }

		FConstIterator& operator++()
		{
			++Pointer;
			return *this;
		}

		FConstIterator operator++(int32)
		{
			FConstIterator Temp = *this;
			++Pointer;
			return Temp;
		}

		bool operator==(const FConstIterator& Other) const { return Pointer == Other.Pointer; }
		bool operator!=(const FConstIterator& Other) const { return Pointer != Other.Pointer; }

	private:
		const char* Pointer;
	};

    FIterator begin() { return FIterator(Data); }
    FIterator end() { return FIterator(Data + Size); }

    FConstIterator cbegin() const { return FConstIterator(Data); }
    FConstIterator cend() const { return FConstIterator(Data + Size); }

private:
    char* Data;
    size_t Size;
};

class FStringView
{
public:
    FStringView() 
        : Data(nullptr), Size(0) 
    {
    }

    explicit FStringView(const FString& String)
        : Data(String.GetData()), Size(String.GetSize())
    {
    }

    explicit FStringView(const char* InData)
        : Data(InData), Size(strlen(InData)) 
    {
    }

    const char* GetData() const
    {
        return Data;
    }

    size_t GetSize() const
    {
        return Size;
    }

    char operator[](size_t Index) 
    {
        return Data[Index];
    }

    const char operator[](size_t Index) const
    {
        return Data[Index];
    }

    bool operator==(const FStringView& Other) const
    {
        if (Size == Other.Size)
        {
            return memcmp(Data, Other.Data, Size) == 0;
        }
        return false;
    }

private:
    const char* Data;
    size_t Size;
};

uint32_t FNV1aHash(const FString& String);
uint32_t FNV1aHash(const FStringView& StringView);

inline SizeType GetTypeHash(const FString& String)
{
    return FNV1aHash(String);
};

inline SizeType GetTypeHash(const FStringView& StringView)
{
    return FNV1aHash(StringView);
}