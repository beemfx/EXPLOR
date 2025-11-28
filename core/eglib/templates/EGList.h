/******************************************************************************
EGList - A class for handling lists of items. This was originally designed to
handled enitity lists. Note that items stored in the list must inherit from
IListable. The list itself doesn't not allocate memory as it is a doubly linked
list.

Insertion and removal is O(1).  It features the helper function GetOne() which
is used by entity management to get entities from the unused list. This is an
O(1) operation.

You may also iterate accross all items in the list using IterateAll. Which
calls a callback function.

(c) 2013 Beem Software
******************************************************************************/
#pragma once

class IListable
{
public: 
	IListable(): m_ListId(0), m_pPrev(nullptr), m_pNext(nullptr){}
	eg_uint GetListId()const{ return m_ListId; }

private:
	template <class U> friend class EGList;
	IListable* m_pPrev;
	IListable* m_pNext;
	eg_uint    m_ListId;
};

template <class T> class EGList
{
public:
	static const eg_uint DEFAULT_ID=1;
public:
	EGList(eg_uint Id)
	: m_Id(Id)
	, m_NumItems(0)
	, m_CachedGetByIndexItem( nullptr )
	, m_CachedGetByIndexIndex( 0 )
	{
		assert( 0 != Id ); //0 is meant to mean that an item is not in a list.
		m_Front.m_pPrev = nullptr;
		m_Front.m_pNext = &m_Back;
		m_Back.m_pPrev = &m_Front;
		m_Back.m_pNext = nullptr;
	}

	EGList()
	: m_Id(0)
	, m_NumItems(0)
	, m_CachedGetByIndexItem( nullptr )
	, m_CachedGetByIndexIndex( 0 )
	{

	}

	void Init(eg_uint Id)
	{
		assert(0 == m_NumItems); //Should not init unless empty.
		const_cast<eg_uint&>(m_Id) = Id;
		m_Front.m_pPrev = nullptr;
		m_Front.m_pNext = &m_Back;
		m_Back.m_pPrev = &m_Front;
		m_Back.m_pNext = nullptr;
		m_CachedGetByIndexItem = nullptr;
		m_CachedGetByIndexIndex = 0;
	}

	void Insert(T* p)
	{
		assert(0 != m_Id); //Must initialize this list before inserting items.
		assert(0 == p->m_ListId); //This was in some other list.
		//We just insert this between m_Back and m_Back->m_pPrev these always
		//exist because of the m_Front and m_Back dummy nodes.
		IListable* pPrev = m_Back.m_pPrev;
		pPrev->m_pNext = p;
		m_Back.m_pPrev = p;
		p->m_pPrev=pPrev;
		p->m_pNext=&m_Back;
		p->m_ListId = m_Id;
		m_NumItems++;

		m_CachedGetByIndexItem = nullptr;
		m_CachedGetByIndexIndex = 0;
	}

	void InsertFirst(T* p )
	{
		if( Len() > 0 )
		{
			InsertBefore( GetFirst() , p );
		}
		else
		{
			Insert( p );
		}
	}

	void InsertLast(T* p )
	{
		if( Len() > 0 )
		{
			InsertAfter( GetLast() , p );
		}
		else
		{
			Insert( p );
		}
	}

	void Remove(T* p)
	{
		if(p->m_ListId != m_Id)
		{
			assert(false); //Not in this list.
			return;
		}

		//We may use the usual removal method since we have m_Front and m_Back
		//so this element is always between two other elements.
		IListable* pPrev = p->m_pPrev;
		IListable* pNext = p->m_pNext;
		p->m_pPrev = nullptr;
		p->m_pNext = nullptr;
		p->m_ListId = 0;
		pPrev->m_pNext = pNext;
		pNext->m_pPrev = pPrev;
		m_NumItems--;

		m_CachedGetByIndexItem = nullptr;
		m_CachedGetByIndexIndex = 0;
	}

	T* GetFirst()
	{
		IListable* Out = m_Front.m_pNext;
		if( Out == &m_Back )Out = nullptr;
		return static_cast<T*>(Out);
	}

	const T* GetFirst()const 
	{
		IListable* Out = m_Front.m_pNext;
		if( Out == &m_Back )Out = nullptr;
		return static_cast<T*>(Out);
	}

	T* GetLast()
	{
		IListable* Out = m_Back.m_pPrev;
		if( Out == &m_Front )Out = nullptr;
		return static_cast<T*>(Out);
	}

	const T* GetLast()const 
	{
		IListable* Out = m_Back.m_pPrev;
		if( Out == &m_Front )Out = nullptr;
		return static_cast<T*>(Out);
	}

	T* GetNext( T* Item )
	{
		assert( Item->m_ListId == m_Id );
		IListable* Out = Item->m_pNext;
		if( Out == &m_Back )Out = nullptr;
		return static_cast<T*>(Out);
	}


	const T* GetNext( const T* Item )const
	{
		assert( Item->m_ListId == m_Id );
		IListable* Out = Item->m_pNext;
		if( Out == &m_Back )Out = nullptr;
		return static_cast<T*>(Out);
	}

	T* GetPrev( T* Item )
	{
		assert( Item->m_ListId == m_Id );
		IListable* Out = Item->m_pPrev;
		if( Out == &m_Front )Out = nullptr;
		return static_cast<T*>(Out);
	}


	const T* GetPrev( const T* Item )const
	{
		assert( Item->m_ListId == m_Id );
		IListable* Out = Item->m_pPrev;
		if( Out == &m_Front )Out = nullptr;
		return static_cast<T*>(Out);
	}

	T* GetByIndex( eg_size_t Index )
	{
		if( !IsValidIndex( Index ) )
		{
			assert( false ); //Invalid index.
			return nullptr;
		}
		// It's highly likely that quests will be gotten in order
		// e.g. for( eg_uint i=0; i<ExQuestMgr.GetNumQuests(); i++ ){ ExQuestMgr.GetQuestByIndex( i ); }
		// so we'll keep track of the last index we got, so if it's the next one
		// or previous one we can retrieve it quickly.
		if( nullptr == m_CachedGetByIndexItem )
		{
			m_CachedGetByIndexIndex = 0;
			m_CachedGetByIndexItem = GetFirst();
		}

		if( Index >= m_CachedGetByIndexIndex )
		{
			for( ; Index != m_CachedGetByIndexIndex && nullptr != m_CachedGetByIndexItem; m_CachedGetByIndexItem = GetNext( m_CachedGetByIndexItem ) )
			{
				m_CachedGetByIndexIndex++;
			}
		}
		else
		{
			for( ; Index != m_CachedGetByIndexIndex && nullptr != m_CachedGetByIndexItem; m_CachedGetByIndexItem = GetPrev( m_CachedGetByIndexItem ) )
			{
				m_CachedGetByIndexIndex--;
			}
		}

		return m_CachedGetByIndexItem;
	}

	const T* GetByIndex( eg_size_t Index )const
	{
		// Rather than keeping two code blocks exactly the same,
		// call the non-const version.
		return const_cast<EGList<T>*>(this)->GetByIndex( Index );
	}

	void InsertAfter( T* Item , T* NewItem )
	{
		assert( Item->GetListId() == m_Id );
		assert( 0 == NewItem->m_ListId );

		IListable* Next = Item->m_pNext;
		NewItem->m_pNext = Next;
		NewItem->m_pPrev = Item;
		Item->m_pNext = NewItem;
		Next->m_pPrev = NewItem;
		NewItem->m_ListId = m_Id;
		m_NumItems++;

		m_CachedGetByIndexItem = nullptr;
		m_CachedGetByIndexIndex = 0;
	}

	void InsertBefore( T* Item , T* NewItem )
	{
		assert( Item->GetListId() == m_Id );
		assert( 0 == NewItem->m_ListId );

		IListable* Prev = Item->m_pPrev;
		Prev->m_pNext = NewItem;
		NewItem->m_pPrev = Prev;
		NewItem->m_pNext = Item;
		Item->m_pPrev = NewItem;
		NewItem->m_ListId = m_Id;
		m_NumItems++;

		m_CachedGetByIndexItem = nullptr;
		m_CachedGetByIndexIndex = 0;
	}

	void InsertSorted( T* Item , eg_bool ( * IsSmaller)( const T& lhs , const T& rhs ) )
	{
		// If  the new item is smaller than the head, or the list is empty, this will be the first item.
		if( (0 == Len()) || IsSmaller(*Item,*GetFirst()) )
		{
			InsertFirst( Item );
		}
		else
		{
			T* InsertAfterItem = nullptr;
			for( T* Search : *this )
			{
				if( IsSmaller( *Item , *Search ) )
				{
					break;
				}

				InsertAfterItem = Search;
			}

			if( InsertAfterItem )
			{
				InsertAfter( InsertAfterItem , Item );
			}
			else
			{
				assert( false ); //Flawed logic?
			}

		}
	}

	T* GetOne()
	{
		if(0 == m_NumItems)
		{
			assert(false);
			return nullptr;
		}

		IListable* p = m_Front.m_pNext;
		return static_cast<T*>(p);
	}

	void IterateAll(void (*Cb)(T*,eg_uintptr_t), eg_uintptr_t d)
	{
		for(IListable* L = m_Front.m_pNext; L != &m_Back; L = L->m_pNext)
		{
			Cb(static_cast<T*>(L), d);
		}
	}

	template<typename F> void IterateAll( F& Cb )
	{
		for(IListable* L = m_Front.m_pNext; L != &m_Back; L = L->m_pNext)
		{
			Cb(static_cast<T*>(L));
		}
	}

	void Clear()
	{
		IterateAll( Clear_WipeId , reinterpret_cast<eg_uintptr_t>(this) );
		m_Front.m_pPrev = nullptr;
		m_Front.m_pNext = &m_Back;
		m_Front.m_ListId = 0;
		m_Back.m_pPrev = &m_Front;
		m_Back.m_pNext = nullptr;
		m_Back.m_ListId = 0;

		m_NumItems = 0;

		m_CachedGetByIndexItem = nullptr;
		m_CachedGetByIndexIndex = 0;
	}

	eg_size_t Len() const
	{
		return m_NumItems;
	}

	template<typename OutType>
	OutType LenAs() const { return static_cast<OutType>(Len()); }

	eg_bool IsEmpty()const{ return 0 == m_NumItems; }
	eg_bool HasItems()const{ return 0 != m_NumItems; }

	eg_uint GetId()const{ return m_Id; }

	eg_bool IsValidIndex( eg_size_t Index )const
	{
		return (0 <= Index && Index < m_NumItems);
	}

	//
	// Iterator stuff:
	//
	struct CItr
	{
		const EGList<T>* OwnerList;
		const T* ThisItem;
		CItr( const EGList<T>* InOwnerList , const T* InThisItem ): OwnerList(InOwnerList), ThisItem(InThisItem){ }
		eg_bool operator!=( const CItr& rhs ){ return OwnerList != rhs.OwnerList || ThisItem != rhs.ThisItem; }
		friend CItr& operator++( CItr& rhs ){ rhs.ThisItem = rhs.OwnerList->GetNext(rhs.ThisItem); return rhs; }
		const T* operator *(){ return ThisItem; }
	};
	CItr begin() const { return CItr(this,GetFirst()); }
	CItr end() const { return CItr(this,nullptr); }

	struct Itr
	{
		EGList<T>* OwnerList;
		T* ThisItem;
		Itr( EGList<T>* InOwnerList , T* InThisItem ): OwnerList(InOwnerList), ThisItem(InThisItem){ }
		eg_bool operator!=( const Itr& rhs ){ return OwnerList != rhs.OwnerList || ThisItem != rhs.ThisItem; }
		friend Itr& operator++( Itr& rhs ){ rhs.ThisItem = rhs.OwnerList->GetNext(rhs.ThisItem); return rhs; }
		T* operator *(){ return ThisItem; }
	};
	Itr begin() { return Itr(this,GetFirst()); }
	Itr end() { return Itr(this,nullptr); }

private:
	const eg_uint m_Id;
	eg_size_t m_NumItems;
	IListable m_Front;
	IListable m_Back;
	// For random access:
	T*        m_CachedGetByIndexItem;
	eg_size_t m_CachedGetByIndexIndex;
private:
	static void Clear_WipeId(T* L, eg_uintptr_t /*d*/)
	{
		L->m_ListId = 0;
	}
};
