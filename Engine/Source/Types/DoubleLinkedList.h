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
        explicit FIterator(TDoubleLinkedListNode<TElement>* InPointer)
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

    private:
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

private:
    TDoubleLinkedListNode<TElement>* Head;
    TDoubleLinkedListNode<TElement>* Tail;

    SizeType Size;
};