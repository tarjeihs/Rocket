#pragma once

template<typename TKey, typename TValue>
struct TPair
{
    TKey Key;
    TValue Value;

    TPair()
        : Key(), Value()
    {
    }

    TPair(const TKey& InKey, const TValue& InValue)
        : Key(InKey), Value(InValue)
    {
    }

    bool operator==(const TPair<TKey, TValue>& Other) const
    {
        return Key == Other.Key && Value == Other.Value;
    }
};