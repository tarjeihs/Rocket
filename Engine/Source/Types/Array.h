#pragma once

template<typename TElement>
class TArray final
{
public:
    TArray()
        : Data(nullptr), Size(0), Capacity(0)
    {
    }

    explicit TArray(size_t InCapacity)
        : Data(nullptr), Size(0), Capacity(0)
    {
        Reserve(InCapacity);
    }

    TArray(std::initializer_list<TElement> List)
        : Data(nullptr), Size(0), Capacity(0)
    {
        Reserve(List.size());
        for (const TElement& Element : List)
        {
            Add(Element);
        }
    }

    ~TArray()
    {
        Clear();
    }

    TArray(const TArray<TElement>& Other)
    {
        Size = Other.Size;
        Capacity = Other.Capacity;

        Data = Allocate(Capacity);
        for (uint32_t Index = 0; Index < Size; ++Index)
        {
            new (&Data[Index]) TElement(Other.Data[Index]); // Copy CTOR
        }
    }

    TArray(TArray<TElement>&& Other) noexcept
    {
        Data = Other.Data;
        Size = Other.Size;
        Capacity = Other.Capacity;

        Other.Data = nullptr;
        Other.Size = 0;
        Other.Capacity = 0;
    }

    TArray<TElement>& operator=(const TArray<TElement>& Other)
    {
        if (this != &Other)
        {
            Clear();
            
            Size = Other.Size;
            Capacity = Other.Capacity;

            Data = Allocate(Capacity);
            for (uint32_t Index = 0; Index < Size; ++Index)
            {
                new (&Data[Index]) TElement(Other.Data[Index]);
            }
        }
        return *this;
    }

    TArray<TElement>& operator=(TArray<TElement>&& Other) noexcept
    {
        if (this != &Other)
        {
            Clear();

            Data = Other.Data;
            Size = Other.Size;
            Capacity = Other.Capacity;

            Other.Data = nullptr;
            Other.Size = 0;
            Other.Capacity = 0;
        }
        return *this;
    }

    TElement* Allocate(size_t NewCapacity)
    {
        return reinterpret_cast<TElement*>(new char[sizeof(TElement) * NewCapacity]);
    }

    void Add(const TElement& Element)
    {
        if (Size >= Capacity)
        {
            Reserve(Capacity == 0 ? 1 : Capacity * 2); // Geometric growth
        }
        new (&Data[Size]) TElement(Element);
        ++Size;
    }

    void Add(TElement&& Element)
    {
        if (Size >= Capacity)
        {
            Reserve(Capacity == 0 ? 1 : Capacity * 2); // Geometric growth
        }
        new (&Data[Size]) TElement(MoveTemp(Element));
        ++Size;
    }

    void RemoveAt(size_t Index)
    {
        RK_ASSERT(Index < Size, "Index is out of bounds");
        Data[Index].~TElement();
        for (size_t Index = 0; Index < Size; ++Index)
        {
            Data[Index] = std::move(Data[Index + 1]); // Move elements down
        }
        --Size;
    }

    void Resize(SizeType NewCapacity, TElement Value = TElement())
    {
        TElement* NewData = Allocate(NewCapacity);
        
        for (size_t Index = 0; Index < NewCapacity; ++Index)
        {
            new (&NewData[Index]) TElement(Value);
        }

        delete[] reinterpret_cast<char*>(Data);
        Data = NewData;
        Capacity = NewCapacity;
        Size = NewCapacity;
    }
    
    void Reserve(size_t NewCapacity)
    {
        if (NewCapacity > Capacity)
        {
            TElement* NewData = Allocate(NewCapacity);
            for (size_t Index = 0; Index < Size; ++Index)
            {
                new (&NewData[Index]) TElement(std::move(Data[Index]));
                Data[Index].~TElement();
            }

            delete[] reinterpret_cast<char*>(Data);
            Data = NewData;
            Capacity = NewCapacity;
        }
    }

    void Shrink()
    {
        if (Capacity < Size)
        {
            return;
        }

        TElement* NewData = Allocate(Size);
        for (SizeType Index = 0; Index < Size; ++Index)
        {
            new (&NewData[Index]) TElement(std::move(Data[Index]));
            Data[Index].~TElement();
        }

        delete[] reinterpret_cast<char*>(Data);
        Data = NewData;
        Capacity = Size;
    }

    void Clear()
    {
        if (Data)
        {
            for (size_t Index = 0; Index < Size; ++Index)
            {
                Data[Index].~TElement();
            }
            delete[] reinterpret_cast<char*>(Data);
        }
        Data = nullptr;
        Size = 0;
        Capacity = 0;
    }

    TElement* GetData()
    {
        return Data;
    }

    const TElement* GetData() const
    {
        return Data;
    }

    size_t GetSize() const
    {
        return Size;
    }

    size_t GetCapacity() const
    {
        return Capacity;
    }

    TElement& operator[](size_t Index)
    {
        RK_ASSERT(Index < Size, "Index is out of bounds");
        return Data[Index];
    }

    const TElement& operator[](size_t Index) const
    {
        RK_ASSERT(Index < Size, "Index is out of bounds");
        return Data[Index];
    }

    class FIterator
    {
    public:
        FIterator(TElement* InPtr) : Ptr(InPtr) {}

        TElement& operator*() { return *Ptr; }

        // Pre-increment operator
        FIterator& operator++()
        {
            ++Ptr;
            return *this;
        }

        // Post-increment operator
        FIterator operator++(int)
        {
            FIterator Temp = *this;
            ++Ptr;
            return Temp;
        }

        bool operator==(const FIterator& Other) const { return Ptr == Other.Ptr; }
        bool operator!=(const FIterator& Other) const { return Ptr != Other.Ptr; }

    private:
        TElement* Ptr;
    };

    class FConstIterator
    {
    public:
        FConstIterator(const TElement* InPtr) : Ptr(InPtr) {}

        const TElement& operator*() const { return *Ptr; }

        // Pre-increment operator
        FConstIterator& operator++()
        {
            ++Ptr;
            return *this;
        }

        // Post-increment operator
        FConstIterator operator++(int)
        {
            FConstIterator Temp = *this;
            ++Ptr;
            return Temp;
        }

        bool operator==(const FConstIterator& Other) const { return Ptr == Other.Ptr; }
        bool operator!=(const FConstIterator& Other) const { return Ptr != Other.Ptr; }

    private:
        const TElement* Ptr;
    };

    FIterator begin() { return FIterator(Data); }
    FIterator end() { return FIterator(Data + Size); }

    FConstIterator begin() const { return FConstIterator(Data); }
    FConstIterator end() const { return FConstIterator(Data + Size); }

private:
    TElement* Data;
    size_t Size;
    size_t Capacity;
};