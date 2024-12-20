#pragma once

#include <type_traits>

#include "EngineTypes.h"
#include "Types/Array.h"
#include "Types/Optional.h"
#include "Types/Pair.h"

template<typename T>
SizeType GetTypeHash(const T& Value) = delete;

template<typename T>
typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_enum<T>::value || std::is_pointer<T>::value, size_t>::type
GetTypeHash(const T& Value)
{
    return std::hash<T>()(Value);    
}

template<typename T, typename = void>
struct TGetTypeHash : std::false_type {};

template<typename T>
struct TGetTypeHash<T, std::void_t<decltype(GetTypeHash(std::declval<const T&>()))>> : std::true_type {};

template<typename T>
struct THash
{
    static SizeType Hash(const T& Key)
    {
        if constexpr (TGetTypeHash<T>::value)
        {
            return GetTypeHash(Key);
        }
        else 
        {
            return std::hash<T>()(Key);
        }
    }
};

template<typename TKey, typename TValue>
class TMap
{
public:
    using TPair = TPair<TKey, TValue>;
    using TOptionalPair = TOptional<TPair>;

    TMap(SizeType BucketSize = DefaultBucketSize)
    {
        Buckets.Resize(BucketSize);

        ElementCount = 0;
    }

    ~TMap()
    {
        Clear();
    }

    void Insert(const TKey& Key, const TValue& Value)
    {
        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
        TOptionalPair& Pair = Buckets[BucketIndex];
        
        if (!Pair.IsValid())
        {
            Pair = TPair(Key, Value);
            ++ElementCount;

            if (static_cast<float>(ElementCount) / Buckets.GetSize() > 0.75f)
            {
                Resize(Buckets.GetSize() * 2);
            }
        }
    }

    bool Remove(const TKey& Key)
    {
        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
        TOptionalPair& Pair = Buckets[BucketIndex];

        if (Pair.IsValid() && Pair->Key == Key)
        {
            Pair.Reset();
            --ElementCount;
            return true;
        }
        return false;
    }

    TValue* Find(const TKey& Key)
    {
        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
        TOptionalPair& Pair = Buckets[BucketIndex];

        if (Pair.IsValid() && Pair->Key == Key)
        {
            return &Pair->Value;
        }
        return nullptr;
    }

    void Clear()
    {
        Buckets.Clear();
        Buckets.Resize(DefaultBucketSize);
        ElementCount = 0;
    }

    class FIterator
    {
    public:
        FIterator(typename TArray<TOptionalPair>::FIterator InCurrent, typename TArray<TOptionalPair>::FIterator InEnd) 
            : Current(InCurrent), End(InEnd)
        {
        }

        FIterator& operator++()
        {
            do 
            {
                ++Current;
            } 
            while (Current != End && !(*Current).IsValid());

            return *this;
        }

        bool operator==(const FIterator& Other) const
        {
            return Current == Other.Current;
        }

        bool operator !=(const FIterator& Other) const
        {
            return Current != Other.Current;
        }

        TPair& operator*()
        {
            return **Current;
        }

        TPair* operator->()
        {
            return &(**Current);
        }

    private:
        typename TArray<TOptionalPair>::FIterator Current;
        typename TArray<TOptionalPair>::FIterator End;
    };

    class FConstIterator
    {
    public:
        FConstIterator(typename TArray<TOptionalPair>::FConstIterator InCurrent, typename TArray<TOptionalPair>::FConstIterator InEnd)
            : Current(InCurrent), End(InEnd)
        {
        }
    
        FConstIterator& operator++()
        {
            do
            {
                ++Current;
            } while (Current != End && !(*Current).IsValid());
    
            return *this;
        }
    
        bool operator==(const FConstIterator& Other) const
        {
            return Current == Other.Current;
        }
    
        bool operator!=(const FConstIterator& Other) const
        {
            return Current != Other.Current;
        }
    
        const TPair& operator*() const
        {
            return **Current;
        }
    
        const TPair* operator->() const
        {
            return &(**Current);
        }
    
    private:
        typename TArray<TOptionalPair>::FConstIterator Current;
        typename TArray<TOptionalPair>::FConstIterator End;
    };

    FIterator begin()
    {
        auto It = Buckets.begin();
        while (It != Buckets.end() && !(*It).IsValid())
        {
            ++It;
        }
        return FIterator(It, Buckets.end());
    }

    FIterator end()
    {
        return FIterator(Buckets.end(), Buckets.end());
    }

    FConstIterator cbegin() const
    {
        auto It = Buckets.cbegin();
        while (It != Buckets.cend() && !(*It).IsValid())
        {
            ++It;
        }
        return FConstIterator(It, Buckets.cend());
    }
    
    FConstIterator cend() const
    {
        return FConstIterator(Buckets.cend(), Buckets.cend());
    }

protected:
    void Resize(SizeType BucketSize)
    {
        TArray<TOptionalPair> NewBuckets;
        NewBuckets.Resize(BucketSize);

        for (TOptionalPair Pair : Buckets)
        {
            SizeType NewBucketIndex = GetBucketIndex(Pair->Key, NewBuckets.GetSize());
            NewBuckets[NewBucketIndex] = Pair;
        }

        Buckets = MoveTemp(NewBuckets);
    }

    SizeType GetBucketIndex(const TKey& Key, SizeType Size) const
    {
        return THash<TKey>::Hash(Key) % Size;
    }

private:
    TArray<TOptionalPair> Buckets;
    SizeType ElementCount;

    static constexpr int32 DefaultBucketSize = 16; 
};