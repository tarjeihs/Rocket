#pragma once

#include <vector>

struct FNode
{
	virtual ~FNode() = default;

	void AddChild(FNode Child)
    {
        Children.push_back(Child);
    }

	std::vector<FNode> Children;
};

template<typename TElement>
struct TNode : public FNode
{
    std::vector<TElement> Array;
};