/*******************************************************************************
EGQueue - Generic Queue

Queue Operations and Complexity (n = MAX):

EGQueue
-------
	Implemented as a linked list. Nodes are allocated in an EGStack and 
	obtained and released as needed.

	EnQueue O(1)
	DeQueue O(1)
	Front   O(1)

	Constructor O(n)
	Clear       O(n)

EGPriorityQue
-------------
	Implemented as a heap in a fixed array. There is no precedence when two
	items with the same priority are inserted into the queue.

	EnQueue O(log n)
	DeQueue O(log n)
	Front   O(1)

	Constructor O(1)
	Clear       O(1)
	IteralAll   O(n)

(c) 2012 Beem Software
*******************************************************************************/
#pragma once
#include "EGCircleArray.h"

template <class T, const eg_uint MAX>
class EGQueue
{
public:
	EGQueue()
	: m_Array()
	{

	}

	void EnQueue(const T& Item)
	{
		if(m_Array.IsFull())
		{
			//Too many items queued at once, or this queue should be bigger.
			assert(false);
			return;
		}

		m_Array.InsertLast( Item );
	}

	void DeQueue()
	{
		if(m_Array.Len() == 0 )
		{
			assert(false);
			return;
		}

		m_Array.RemoveFirst();
	}

	T& Front()
	{
		return m_Array[0];
	}

	const T& Front()const
	{
		return m_Array[0];
	}

	eg_uint Len()const
	{
		return m_Array.Len();
	}

	eg_bool HasItems()const
	{
		return Len() > 0;
	}

	eg_bool AtCapacity()const
	{
		return m_Array.IsFull();
	}

	void Clear()
	{
		m_Array.Clear();
	}

private:
	EGCircleArray<T,MAX> m_Array;
};

template <class T>
struct EGPriorityQueue_DefaultCmp
{
	eg_bool operator()(const T& left, const T& right)
	{
		return left > right;
	}
};

template <class T, const eg_uint MAX, class C = EGPriorityQueue_DefaultCmp<T>>
class EGPriorityQueue
{
public:
	EGPriorityQueue():m_Count(0){ }

	void EnQueue(const T& Item)
	{
		if(m_Count >= MAX)
		{
			//Too many items queued at once, or this queue should be bigger.
			assert(false);
			return;
		}

		C Compare;

		//We just stick this in at the end and heapify.
		m_Heap[m_Count] = Item;
		m_Count++;
		eg_uint nPos = m_Count;

		while((nPos > 1) && Compare(m_Heap[nPos-1], m_Heap[nPos/2-1]))
		{
			T Temp = m_Heap[nPos-1];
			m_Heap[nPos-1] = m_Heap[nPos/2-1];
			m_Heap[nPos/2-1] = Temp;
			nPos = nPos/2;
		}

		
	}

	void DeQueue()
	{
		if(0 == m_Count)
		{
			assert(false);
			return;
		}
		C Compare;
		eg_uint nPos = 1;
		m_Count--;
		m_Heap[0] = m_Heap[m_Count];

		while(true)
		{
			//We we don't have a left child we're done.
			if(nPos*2 > m_Count)
			{
				break;
			}

			//If we don't have a right child, we only have ne possible swap (with the left child) and we're done.
			if((nPos*2+1) > m_Count)
			{
				//If the left child is bigger swap
				if(Compare(m_Heap[nPos*2-1], m_Heap[nPos-1]))
				{
					T Temp = m_Heap[nPos-1];
					m_Heap[nPos-1] = m_Heap[nPos*2-1];
					m_Heap[nPos*2-1] = Temp;
				}
				break;
			}

			//If we are bigger than both children we are done.
			if(Compare(m_Heap[nPos-1], m_Heap[nPos*2-1]) && Compare(m_Heap[nPos-1], m_Heap[nPos*2+1-1]))
			{
				break;
			}

			//We aren't the biggest, so swap with biggest child (since we weren't
			//bigger than one of them we are for sure smaller than the biggest of
			//the children, so swap with it.
			eg_uint nBiggerChild = Compare(m_Heap[nPos*2-1], m_Heap[nPos*2+1-1]) ? nPos*2 : nPos*2+1;
			T Temp = m_Heap[nPos-1];
			m_Heap[nPos-1] = m_Heap[nBiggerChild-1];
			m_Heap[nBiggerChild-1] = Temp;
			nPos = nBiggerChild;
		}
	}

	T& Front()
	{
		assert(0 != m_Count); //Whatever is returned is garbage.
		return m_Heap[0];
	}

	const T& Front()const
	{
		assert(0 != m_Count); //Whatever is returned is garbage.
		return m_Heap[0];
	}

	eg_uint Len()const
	{
		return m_Count;
	}

	eg_bool HasItems()const
	{
		return m_Count > 0;
	}

	eg_bool AtCapacity()const
	{
		return m_Count == MAX;
	}

	void Clear()
	{
		//Because of the way memory is handled we have to do an O(n) operation.
		while(HasItems())
		{
			DeQueue();
		}
	}

	/*
	void Sort()
	{
		eg_uint Count = m_Count;
		while(HasItems())
		{
			T Item = Front();
			DeQueue();
			m_Heap[m_Count] = Item;
		}
		m_Count = Count;
	}
	*/

private:
	eg_uint m_Count;
	T       m_Heap[MAX];
};
