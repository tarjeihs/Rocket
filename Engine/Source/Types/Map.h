#pragma once

#include <type_traits>

#include "EngineTypes.h"
#include "Types/Array.h"
#include "Types/DoubleLinkedList.h"
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
    using TBucket = TDoubleLinkedList<TPair<TKey, TValue>>;

    TMap(SizeType BucketSize = 16)
    {
        Buckets.Resize(BucketSize);

        ElementCount = 0;
    }

    void Insert(const TKey& Key, const TValue& Value)
    {
        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
        TBucket& Bucket = Buckets[BucketIndex];

        for (auto& Pair : Bucket)
        {
            if (Pair.Key == Key)
            {
                Pair.Value = Value;
                return;
            }
        }

        Bucket.PushBack(TPair(Key, Value));
        ++ElementCount;

        if (static_cast<float>(ElementCount) / Buckets.GetSize() > 0.75f)
        {
            Resize(Buckets.GetSize() * 2);
        }
    }

    bool Remove(const TKey& Key)
    {
        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
        TBucket& Bucket = Buckets[BucketIndex];

        for (auto It = Bucket.begin(); It != Bucket.end(); ++It)
        {
            if (It->Key == Key)
            {
                Bucket.Remove(*It);
                --ElementCount;
                return true;
            }
        }
        return false;
    }

    TValue* Find(const TKey& Key)
    {
        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
        TBucket& Bucket = Buckets[BucketIndex];

        for (auto& Pair : Bucket)
        {
            if (Pair.Key == Key)
            {
                return &Pair.Value;
            }
        }
        return nullptr;
    }

    void Clear()
    {
        for (auto& Bucket : Buckets)
        {
            Bucket.Clear();
        }
    }

protected:
    void Resize(SizeType BucketSize)
    {
        TArray<TBucket> NewBuckets;
        NewBuckets.Resize(BucketSize);

        for (auto& Bucket : Buckets)
        {
            for (auto& Pair : Bucket)
            {
                SizeType NewBucketIndex = GetBucketIndex(Pair.Key, NewBuckets.GetSize());
                NewBuckets[NewBucketIndex].PushBack(Pair);
            }
        }

        Buckets = MoveTemp(NewBuckets);
    }

    SizeType GetBucketIndex(const TKey& Key, SizeType Size) const
    {
        return THash<TKey>::Hash(Key) % Size;
    }

private:    
    TArray<TBucket> Buckets;

    SizeType ElementCount;
};