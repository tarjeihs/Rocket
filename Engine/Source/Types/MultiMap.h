//template<typename TKey, typename TValue>
//class TMap
//{
//public:
//    using TBucket = TDoubleLinkedList<TPair<TKey, TValue>>;
//
//    TMap(SizeType BucketSize = 16)
//    {
//        Buckets.Resize(BucketSize);
//
//        ElementCount = 0;
//    }
//
//    void Insert(const TKey& Key, const TValue& Value)
//    {
//        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
//        TBucket& Bucket = Buckets[BucketIndex];
//
//        Bucket.PushBack(TPair(Key, Value));
//        ++ElementCount;
//
//        if (static_cast<float>(ElementCount) / Buckets.GetSize() > 0.75f)
//        {
//            Resize(Buckets.GetSize() * 2);
//        }
//    }
//
//    bool Remove(const TKey& Key)
//    {
//        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
//        TBucket& Bucket = Buckets[BucketIndex];
//
//        for (auto It = Bucket.begin(); It != Bucket.end(); ++It)
//        {
//            if (It->Key == Key)
//            {
//                Bucket.Remove(*It);
//                --ElementCount;
//                return true;
//            }
//        }
//        return false;
//    }
//
//    TValue* Find(const TKey& Key)
//    {
//        SizeType BucketIndex = GetBucketIndex(Key, Buckets.GetSize());
//        TBucket& Bucket = Buckets[BucketIndex];
//
//        for (auto& Pair : Bucket)
//        {
//            if (Pair.Key == Key)
//            {
//                return &Pair.Value;
//            }
//        }
//        return nullptr;
//    }
//
//    void Clear()
//    {
//        for (TBucket& Bucket : Buckets)
//        {
//            Bucket.Clear();
//        }
//    }
//
//    SizeType IndexOfStart() const
//    {
//        for (SizeType Index = 0; Index < Buckets.GetSize(); ++Index)
//        {
//            if (!Buckets[Index].IsEmpty())
//            {
//                return Index;
//            }
//        }
//        return 0;
//    }
//
//    SizeType IndexOfEnd() const
//    {
//        for (SizeType Index = Buckets.GetSize() - 1; Index > 0; --Index)
//        {
//            if (!Buckets[Index].IsEmpty())
//            {
//                return Index;
//            }
//        }
//        return 0;
//    }
//
//    class FIterator
//    {
//    public:
//        using TBucketIterator = typename TBucket::FIterator;
//
//        FIterator(TArray<TBucket>* InBuckets, SizeType InBucketIndex, TBucketIterator InBucketIterator)
//            : Buckets(InBuckets), BucketIndex(InBucketIndex), BucketIterator(InBucketIterator) 
//        {
//            AdvanceToValidBucket();
//        }
//
//        FIterator& operator++()
//        {
//            if (!Buckets || BucketIndex >= Buckets->GetSize())
//            {
//                return *this;
//            }
//
//            ++BucketIterator;
//
//            if (BucketIterator == (*Buckets)[BucketIndex].end())
//            {
//                ++BucketIndex;
//                AdvanceToValidBucket();
//            }
//
//            return *this;
//        }
//
//        bool operator==(const FIterator& Other) const
//        {
//            // If both are "end" iterators
//            if (!Buckets && !Other.Buckets)
//            {
//                return true;
//            }
//
//            if (Buckets == Other.Buckets && BucketIndex == Other.BucketIndex && BucketIterator == Other.BucketIterator)
//            {
//                return true;
//            }
//
//            return false;
//        }
//
//        bool operator!=(const FIterator& Other) const
//        {
//            return !(*this == Other);
//        }
//
//        TPair<TKey, TValue>& operator*()
//        {
//            return *BucketIterator;
//        }
//
//        TPair<TKey, TValue>* operator->()
//        {
//            return &(*BucketIterator);
//        }
//
//            void AdvanceToValidBucket()
//    {
//        // Move forward until we find a non-empty bucket or reach the end
//        while (Buckets && BucketIndex < Buckets->GetSize() && (*Buckets)[BucketIndex].IsEmpty())
//        {
//            ++BucketIndex;
//        }
//
//        if (!Buckets || BucketIndex >= Buckets->GetSize())
//        {
//            // We've gone past the last bucket, this is the end.
//            // Set the iterator to a known end state.
//            BucketIterator = TBucketIterator(); // Default constructed = end
//        }
//        else
//        {
//            // Set to the start of the current (non-empty) bucket
//            BucketIterator = (*Buckets)[BucketIndex].begin();
//        }
//    }
//
//
//    private:
//        TArray<TBucket>* Buckets;
//        TBucketIterator BucketIterator;
//        SizeType BucketIndex;
//    };
//
//    class TConstIterator
//    {
//    public:
//        using BucketIteratorType = typename TBucket::FConstIterator;
//
//        TConstIterator(const TArray<TBucket>& InBuckets, SizeType BucketIndex, BucketIteratorType InBucketIterator)
//            : Buckets(&InBuckets), CurrentBucketIndex(BucketIndex), BucketIterator(InBucketIterator) 
//        {
//        }
//
//        TConstIterator& operator++()
//        {
//            ++BucketIterator;
//            return *this;
//        }
//
//        bool operator==(const TConstIterator& Other) const
//        {
//            return Buckets == Other.Buckets && CurrentBucketIndex == Other.CurrentBucketIndex && BucketIterator == Other.BucketIterator;
//        }
//
//        bool operator!=(const TConstIterator& Other) const
//        {
//            return !(*this == Other);
//        }
//
//        const TPair<TKey, TValue>& operator*() const
//        {
//            return *BucketIterator;
//        }
//
//        const TPair<TKey, TValue>* operator->() const
//        {
//            return &(*BucketIterator);
//        }
//
//    private:
//        const TArray<TBucket>* Buckets;
//        SizeType CurrentBucketIndex;
//        BucketIteratorType BucketIterator;
//    };
//
//    FIterator begin()
//    {
//        return FIterator(&Buckets, 0, Buckets[0].begin());
//    }
//
//    FIterator end()
//    {
//        return FIterator(&Buckets, Buckets.GetSize(), Buckets[Buckets.GetSize() - 1].begin());
//    }
//
//    TConstIterator begin() const
//    {
//        return FConstIterator(Buckets, 0, Buckets[0].begin());
//    }
//
//    TConstIterator end() const
//    {
//        return TConstIterator(Buckets, Buckets.GetSize(), typename TBucket::FConstIterator(nullptr));
//    }
//
//protected:
//    void Resize(SizeType BucketSize)
//    {
//        TArray<TBucket> NewBuckets;
//        NewBuckets.Resize(BucketSize);
//
//        for (TBucket& Bucket : Buckets)
//        {
//            for (TPair<TKey, TValue>& Pair : Bucket)
//            {
//                SizeType NewBucketIndex = GetBucketIndex(Pair.Key, NewBuckets.GetSize());
//                NewBuckets[NewBucketIndex].PushBack(Pair);
//            }
//        }
//
//        Buckets = MoveTemp(NewBuckets);
//    }
//
//    SizeType GetBucketIndex(const TKey& Key, SizeType Size) const
//    {
//        return THash<TKey>::Hash(Key) % Size;
//    }
//
//private:
//    TArray<TBucket> Buckets;
//    SizeType ElementCount;
//};