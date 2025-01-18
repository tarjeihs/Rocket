#pragma once

#include "Core/Assert.h"
#include "EngineTypes.h"

template<typename TElement>
struct TDoubleLinkedListNode
{
    TElement Element;
    TDoubleLinkedListNode* Next;
    TDoubleLinkedListNode* Previous;

    TDoubleLinkedListNode(const TElement& InElement)
        : Element(InElement), Next(nullptr), Previous(nullptr)
    {
    }
};

template<typename TElement>
class TDoubleLinkedList
{
public:
    TDoubleLinkedList()
        : Head(nullptr), Tail(nullptr), Size(0)
    {
    }

    ~TDoubleLinkedList()
    {
        Clear();
    }

	TDoubleLinkedList(const TDoubleLinkedList& Other)
		: Head(nullptr), Tail(nullptr), Size(0)
	{
		TDoubleLinkedListNode<TElement>* Current = Other.Head;
		while (Current)
		{
			PushBack(Current->Element);
			Current = Current->Next;
		}
	}

	TDoubleLinkedList(TDoubleLinkedList&& Other) noexcept
		: Head(Other.Head), Tail(Other.Tail), Size(Other.Size)
	{
		Other.Head = nullptr;
		Other.Tail = nullptr;
		Other.Size = 0;
	}

	TDoubleLinkedList& operator=(const TDoubleLinkedList& Other)
	{
		if (this != &Other)
		{
			Clear();

			TDoubleLinkedListNode<TElement>* Current = Other.Head;
			while (Current)
			{
				PushBack(Current->Element);
				Current = Current->Next;
			}
		}
		return *this;
	}

	TDoubleLinkedList& operator=(TDoubleLinkedList&& Other) noexcept
	{
		if (this != &Other)
		{
			Clear();

			Head = Other.Head;
			Tail = Other.Tail;
			Size = Other.Size;
			Other.Head = nullptr;
			Other.Tail = nullptr;
			Other.Size = 0;
		}
		return *this;
	}

    void PushFront(const TElement& Element)
    {
        TDoubleLinkedListNode<TElement>* Node = new TDoubleLinkedListNode<TElement>(Element);
        if (!Head)
        {
            Tail = Node;
        }
        else 
        {
            Node->Next = Head;
            Head->Previous = Node;
        }
        Head = Node;
        ++Size;
    }

    void PushBack(const TElement& Element)
    {
        TDoubleLinkedListNode<TElement>* Node = new TDoubleLinkedListNode<TElement>(Element);
        if (!Tail)
        {
            Head = Node;
            Tail = Head;
        }
        else 
        {
            Tail->Next = Node;
            Node->Previous = Tail;
            Tail = Node;
        }
        ++Size;
    }

    void PopFront()
    {
        RK_ASSERT(!IsEmpty(), "Attempting to pop element when list is empty.");

        if (Head == Tail)
        {
            delete Head;
            Head = nullptr;
            Tail = nullptr;
        }
        else 
        {
            TDoubleLinkedListNode<TElement>* OldHead = Head;
            Head = Head->Next;
            Head->Previous = nullptr;
            delete OldHead;
        }
        --Size;
    }

    void PopBack()
    {
        RK_ASSERT(!IsEmpty(), "Attempting to pop element when list is empty.");

        if (Tail == Head)
        {
            delete Tail;
            Head = nullptr;
            Tail = nullptr;
        }
        else 
        {
            TDoubleLinkedListNode<TElement>* OldTail = Tail;
            Tail = Tail->Previous;
            Tail->Next = nullptr;
            delete OldTail;        
        }
        --Size;
    }

    void Remove(const TElement& Element)
    {
        TDoubleLinkedListNode<TElement>* Node = Head;

        while (Node)
        {
            if (Node->Element == Element)
            {
                if (Node->Previous)
                {
                    Node->Previous->Next = Node->Next;
                }
                else
                {
                    Head = Node->Next;
                }

                if (Node->Next)
                {
                    Node->Next->Previous = Node->Previous;
                }
                else
                {
                    Tail = Node->Previous;
                }

                delete Node;
                --Size;
                return;
            }
            
            Node = Node->Next;
        }
    }

    void Clear()
    {
        while (Head)
        {
            PopFront();
        }
    }

    SizeType GetSize() const
    {
        return Size;
    }

    bool IsEmpty() const
    {
        return Size == 0;
    }

    class FIterator
    {
    public:
        explicit FIterator(TDoubleLinkedListNode<TElement>* InPointer = nullptr)
            : Pointer(InPointer)
        {
        }

        TElement& operator*() const
        {
            return Pointer->Element;
        }

        TElement* operator->() const
        {
            return &Pointer->Element;
        }

        FIterator& operator++() // Prefix increment
        {
            Pointer = Pointer->Next;
            return *this;
        }

        FIterator& operator++(int32_t) // Postfix increment
        {
            FIterator Temp = *this;
            ++(*this);
            return Temp;
        }

        FIterator& operator--() // Prefix decrement
        {
            Pointer = Pointer->Previous;
            return *this;
        }

        FIterator& operator--(int32_t) // Postfix decrement
        {
            FIterator Temp = *this;
            --(*this);
            return Temp;
        }

        bool operator==(const FIterator& Other) const
        {
            return Pointer == Other.Pointer;
        }

        bool operator!=(const FIterator& Other) const
        {
            return Pointer != Other.Pointer;
        }

        TDoubleLinkedListNode<TElement>* Pointer;
    };

    FIterator begin()
    {
        return FIterator(Head);
    }

    FIterator end()
    {
        return FIterator(nullptr);
    }

    FIterator rbegin()
    {
        return FIterator(Tail);
    }

    FIterator rend()
    {
        return FIterator(nullptr);
    }

    class FConstIterator
    {
    public:
        explicit FConstIterator(const TDoubleLinkedListNode<TElement>* InPointer = nullptr)
            : Pointer(InPointer)
        {
        }

        const TElement& operator*() const
        {
            return Pointer->Element;
        }

        const TElement* operator->() const
        {
            return &Pointer->Element;
        }

        FConstIterator& operator++() // Prefix increment
        {
            Pointer = Pointer->Next;
            return *this;
        }

        FConstIterator operator++(int32_t) // Postfix increment
        {
            FConstIterator Temp = *this;
            ++(*this);
            return Temp;
        }

        FConstIterator& operator--() // Prefix decrement
        {
            Pointer = Pointer->Previous;
            return *this;
        }

        FConstIterator operator--(int32_t) // Postfix decrement
        {
            FConstIterator Temp = *this;
            --(*this);
            return Temp;
        }

        bool operator==(const FConstIterator& Other) const
        {
            return Pointer == Other.Pointer;
        }

        bool operator!=(const FConstIterator& Other) const
        {
            return Pointer != Other.Pointer;
        }

    private:
        const TDoubleLinkedListNode<TElement>* Pointer;
    };

    FConstIterator begin() const
    {
        return FConstIterator(Head);
    }

    FConstIterator end() const
    {
        return FConstIterator(nullptr);
    }

    FConstIterator rbegin() const
    {
        return FConstIterator(Tail);
    }

    FConstIterator rend() const
    {
        return FConstIterator(nullptr);
    }

private:
    TDoubleLinkedListNode<TElement>* Head;
    TDoubleLinkedListNode<TElement>* Tail;

    SizeType Size;
};