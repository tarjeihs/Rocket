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

// A templated hash map implementation using open addressing with double hashing for collision resolution.
// TMap is a generic hash map that associates keys of type TKey with values of type TValue. It provides efficient insertion, retrieval, and removal of key-value pairs.
// The hash map uses open addressing with double hashing to handle collisions, ensuring good performance even at higher load factors.

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
        SizeType BucketSize = Buckets.GetSize();
        SizeType BucketIndex = PrimaryHash(Key, BucketSize);
        SizeType StepSize = SecondaryHash(Key);
        SizeType StartIndex = BucketIndex;

        while (Buckets[BucketIndex].IsValid())
        {
            if (Buckets[BucketIndex]->Key == Key) // Update existing key
            {
                Buckets[BucketIndex]->Value = Value;
                return;
            }

            // Probe using double hashing
            BucketIndex = (BucketIndex + StepSize) % BucketSize;
        }

        Buckets[BucketIndex] = TPair(Key, Value);
        ++ElementCount;

        if (static_cast<float>(ElementCount) / BucketSize > 0.75f)
        {
            Resize(BucketSize * 2);
        }
    }

    bool Remove(const TKey& Key)
    {
        SizeType BucketSize = Buckets.GetSize();
        SizeType BucketIndex = PrimaryHash(Key, BucketSize);
        SizeType StepSize = SecondaryHash(Key);
        SizeType StartIndex = BucketIndex;

        while (Buckets[BucketIndex].IsValid())
        {
            if (Buckets[BucketIndex]->Key == Key)
            {
                Buckets[BucketIndex].Reset();
                --ElementCount;

                // Rehash elements in the same cluster
                SizeType NextIndex = (BucketIndex + StepSize) % BucketSize;
                while (Buckets[NextIndex].IsValid())
                {
                    TOptionalPair MovedPair = Buckets[NextIndex];
                    Buckets[NextIndex].Reset();
                    --ElementCount;
                    Insert(MovedPair->Key, MovedPair->Value);
                    BucketIndex = NextIndex;
                    NextIndex = (BucketIndex + StepSize) % BucketSize;
                }

                return true;
            }

            BucketIndex = (BucketIndex + StepSize) % BucketSize;
            if (BucketIndex == StartIndex) // We've looped through all buckets
            {
                break;
            }
        }

        return false;
    }

    TValue* Find(const TKey& Key)
    {
        SizeType BucketSize = Buckets.GetSize();
        SizeType BucketIndex = PrimaryHash(Key, BucketSize);
        SizeType StepSize = SecondaryHash(Key);
        SizeType StartIndex = BucketIndex;

        while (Buckets[BucketIndex].IsValid())
        {
            if (Buckets[BucketIndex]->Key == Key)
            {
                return &Buckets[BucketIndex]->Value;
            }

            // Probe using double hashing
            BucketIndex = (BucketIndex + StepSize) % BucketSize;
            if (BucketIndex == StartIndex) // We've looped through all buckets
            {
                break;
            }
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
    void Resize(SizeType NewSize)
    {
        TArray<TOptionalPair> OldBuckets = MoveTemp(Buckets);
        Buckets.Resize(NewSize);
        ElementCount = 0;

        for (TOptionalPair& Pair : OldBuckets)
        {
            if (Pair.IsValid())
            {
                Insert(Pair->Key, Pair->Value); // Reinsert all valid pairs
            }
        }
    }

    SizeType PrimaryHash(const TKey& Key, SizeType Size) const
    {
        return THash<TKey>::Hash(Key) % Size;
    }

    SizeType SecondaryHash(const TKey& Key) const
    {
        return 1 + (THash<TKey>::Hash(Key) % (Buckets.GetSize() - 1));
    }

private:
    TArray<TOptionalPair> Buckets;
    SizeType ElementCount;

    static constexpr int32 DefaultBucketSize = 16; 
};