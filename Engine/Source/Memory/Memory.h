#pragma once

template<typename TTarget, typename TSource>
TTarget* Cast(TSource* Object)
{
    return static_cast<TTarget*>(Object);
}