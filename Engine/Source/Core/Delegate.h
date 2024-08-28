#pragma once

#include <functional>
#include <vector>
#include <unordered_set>

template<typename... TArgs>
class PDelegate
{
public:
    using TFunc = std::function<void(TArgs...)>;

    void Bind(TFunc&& Func)
    {
        size_t HashID = Hash(Func);
        DelegateFunctions.insert({ HashID, std::move(Func) });
    }

    template<typename TObject>
    void Bind(TObject* Object, void (TObject::*Method)(TArgs...))
    {
        size_t HashID = Hash(Object, Method);
        TFunc Func = [Object, Method](TArgs... Args) { (Object->*Method)(std::forward<TArgs>(Args)...); };
        DelegateFunctions.insert({ HashID, Func });
    }

    template<typename TObject>
    void Unbind(TObject* Object, void (TObject::*Method)(TArgs...))
    {
        size_t HashID = Hash(Object, Method);
        DelegateFunctions.erase({ HashID, nullptr });
    }

    void Broadcast(TArgs... Args)
    {
        for (const SDelegateFunction& DelegateFunction : DelegateFunctions)
        {
            DelegateFunction.Func(Args...);
        }
    }

    void Clear()
    {
        DelegateFunctions.clear();
    }

    [[nodiscard]] size_t Size() const
    {
        return DelegateFunctions.size();
    }

private:
    struct SDelegateFunction
    {
        size_t HashID;
        TFunc Func;

        bool operator==(const SDelegateFunction& other) const
        {
            return HashID == other.HashID;
        }
    };

    struct SDelegateFunctionHashType
    {
        size_t operator()(const SDelegateFunction& DelegateFunction) const
        {
            return DelegateFunction.HashID;
        }
    };

    static size_t Hash(const TFunc& Callback)
    {
        return std::hash<const void*>{}(reinterpret_cast<const void*>(std::addressof(Callback)));
    }

    template <typename TObject>
    static size_t Hash(TObject* Object, void (TObject::*Func)(TArgs...))
    {
        return std::hash<TObject*>()(Object) ^ (reinterpret_cast<size_t>(*(void**)(&Func)) << 1);
    }

    std::unordered_set<SDelegateFunction, SDelegateFunctionHashType> DelegateFunctions;
};