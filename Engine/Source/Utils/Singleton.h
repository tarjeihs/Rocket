#pragma once

template<typename TObject>
class TSingleton 
{
public:
    static TObject& Get()
    {
        static TObject Object;
        return Object;
    }

    TSingleton(const TSingleton&) = delete;
    TSingleton& operator=(const TSingleton&) = delete;

protected:
    TSingleton() = default;
    virtual ~TSingleton() = default;
};