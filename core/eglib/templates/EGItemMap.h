// (c) 2017 Beem Media

/*******************************************************************************
EGItemMapBase: A template class for mapping keys to items:

Operations:
	Construction EGItemMapBase<K, T> Variable(Default);
		T is the type of object that the strings map to. This must contain an
			= operator that will copy the data in a meaningful way. It is 
			perfectly fine for T to be a pointer type. But whatever it points
			to must be persistent throughout the life of the EGStringMap.
		Dummy is some default value that is return by the map when searching
			for a Key but doesn't find it. For pointer types you may set this
			to null (0).

	void Insert(K Key, T Item)
		Inserts Item at position Key. If there is already an item at position
		Key then that item will be replaced by Item. If inserting a new item
		would cause the size of the map to go beyond MAX an assert will be
		hit and nothing will be added, but the application will not crash.

	eg_bool Contains(K Key)
		Returns true of false depending on if Key is in the map.

	T& Get(K Key)
		Returns the Item at position Key. Note that if this item was not in
		the list the Dummy item is returned, so care should be taken that
		using such a value has meaning.

	void Remove(K Key)
		Removes the item at Key.

	void Clear()
		Removes all items for the map.

	eg_uint Len()const
		Returns the number of items currently in the map.

	T& GetAt(eg_uint Index)
		Returns an item in the map by Index where index is the item list in
		(semi) alphabetical order. And 0 <= Index < Len(). Note that
		this index is not fixed and if new items are added then an old index
		may or may not point to the same object. Generally this function
		should not be used.

	K& GetKeyAt(eg_uint Index)
		As above, but returns the Key at a given Index.

	Current Run-Time and Why:
		Let n be at worst MAX.
		Insertion is O(n)
		Deletion is O(n)
		Clearing all is O(1)
		Lookup (Contains or Get) is O(log n)
		Getting by index (key or item or Len()) is O(1)
		IterateAll is O(n)

		First note the main concern with the usage of a map in the game is
		with using a fixed amount of memory, and keeping that memory as small
		as possible while still maintaining a fast lookup time.

		Thus the map is maintained as a
		sorted array. New items are inserted in their sorted position and
		everything to the right of them is moved one spot to the right.
		Technically we could get faster insertion by storying the items as
		a tree, or even better a red black tree, but this would require
		significantly more memory, and within the game insertions are very
		rare, generally anything that uses a map inserts some stuff right
		away, then only the getters are called.

		Similarly delete is O(n) because even though it can find the element
		to be removed in O(log n) time, it must move all elements to the right
		of the element being removed left by one spot. As with insertion
		Remove is almost never called as generally when a map is reset Clear
		is called instead which is an O(1) operation.

		Since memory is a higher priority than insertion and deletion this
		schema has been chosen.

*******************************************************************************/

#pragma once

template <typename K,typename T,eg_size_t MAX>
class EGFixedItemMap
{
public:

	struct egPair
	{
		K Key;
		T Item;
	};

public:

	EGFixedItemMap( eg_ctor_t Ct , const T& NotFoundItem )
	: m_List( Ct )
	, m_NotFoundItem( NotFoundItem )
	{

	}

	void SetNotFoundItem( const T& NewNotFoundItem ){ m_NotFoundItem = NewNotFoundItem; }

	eg_bool Contains(const K& Key)const
	{
		return m_List.BinarySearch_Contains( Key );
	}

	eg_bool IsFull()const
	{
		return m_List.IsFull();
	}

	void Insert( const K& Key , const T& Item )
	{
		if( m_List.BinarySearch_Contains( Key ) )
		{
			m_List.BinarySearch_Get( Key ).Item = Item;
		}
		else
		{
			egPair NewItem = { Key , Item };
			m_List.BinarySearch_Insert( NewItem );
		}
	}

	const T& operator[](const K& Key)const{ return Get(Key); }
	
	T& operator[](const K& Key)
	{
		// This one attempts to add an item if it doesn't exist yet.

		if( !m_List.BinarySearch_Contains( Key ) && !m_List.IsFull() )
		{
			egPair NewItem = { Key , m_NotFoundItem };
			m_List.BinarySearch_Insert( NewItem );
		}

		eg_size_t Index = m_List.BinarySearch_GetIndexOf( Key );

		return m_List.IsValidIndex( Index ) ? m_List[Index].Item : m_NotFoundItem;
	}

	//Obtain an item from a key O(log n)
	const T& Get(const K& Key)const
	{
		eg_size_t Index = m_List.BinarySearch_GetIndexOf( Key );
		return m_List.IsValidIndex( Index ) ? m_List[Index].Item : m_NotFoundItem;
	};

	T& Get(const K& Key)
	{
		eg_size_t Index = m_List.BinarySearch_GetIndexOf( Key );
		return m_List.IsValidIndex( Index ) ? m_List[Index].Item : m_NotFoundItem;
	};

	void Delete( const K& Key )
	{
		m_List.BinarySearch_Delete( Key );
	}

	void Clear()
	{
		m_List.Clear();
	}

	eg_size_t Len() const
	{
		return m_List.Len();
	}

	T& GetByIndex( eg_size_t Index )
	{
		return m_List[Index].Item;
	}

	const T& GetByIndex(eg_size_t Index) const
	{
		return m_List[Index].Item;
	}

	const K& GetKeyByIndex(eg_size_t Index) const
	{
		return m_List[Index].Key;
	}

protected:
	
	EGFixedArray<egPair,MAX> m_List;
	T                        m_NotFoundItem;
};


template <typename K,typename T>
class EGItemMap
{
public:

	struct egPair
	{
		K Key;
		T Item;
	};

public:

	EGItemMap( const T& NotFoundItem )
	: m_List()
	, m_NotFoundItem( NotFoundItem )
	{

	}

	EGItemMap( eg_mem_pool MemPool , const T& NotFoundItem )
	: m_List( MemPool )
	, m_NotFoundItem( NotFoundItem )
	{

	}

	void SetNotFoundItem( const T& NewNotFoundItem ){ m_NotFoundItem = NewNotFoundItem; }

	eg_bool Contains(const K& Key)const
	{
		return m_List.BinarySearch_Contains( Key );
	}

	void Insert( const K& Key , const T& Item )
	{
		if( m_List.BinarySearch_Contains( Key ) )
		{
			m_List.BinarySearch_Get( Key ).Item = Item;
		}
		else
		{
			egPair NewItem = { Key , Item };
			m_List.BinarySearch_Insert( NewItem );
		}
	}

	const T& operator[](const K& Key)const{ return Get(Key); }

	T& operator[](const K& Key)
	{
		// This one attempts to add an item if it doesn't exist yet.

		if( !m_List.BinarySearch_Contains( Key ) )
		{
			egPair NewItem = { Key , m_NotFoundItem };
			m_List.BinarySearch_Insert( NewItem );
		}

		eg_size_t Index = m_List.BinarySearch_GetIndexOf( Key );

		return m_List.IsValidIndex( Index ) ? m_List[Index].Item : m_NotFoundItem;
	}

	//Obtain an item from a key O(log n)
	const T& Get(const K& Key)const
	{
		eg_size_t Index = m_List.BinarySearch_GetIndexOf( Key );
		return m_List.IsValidIndex( Index ) ? m_List[Index].Item : m_NotFoundItem;
	};

	T& Get(const K& Key)
	{
		eg_size_t Index = m_List.BinarySearch_GetIndexOf( Key );
		return m_List.IsValidIndex( Index ) ? m_List[Index].Item : m_NotFoundItem;
	};

	void Delete( const K& Key )
	{
		m_List.BinarySearch_Delete( Key );
	}

	void Clear()
	{
		m_List.Clear();
	}

	eg_size_t Len() const
	{
		return m_List.Len();
	}

	T& GetByIndex( eg_size_t Index )
	{
		return m_List[Index].Item;
	}

	const T& GetByIndex(eg_size_t Index) const
	{
		return m_List[Index].Item;
	}

	const K& GetKeyByIndex(eg_size_t Index) const
	{
		return m_List[Index].Key;
	}

	eg_bool IsEmpty() const { return m_List.IsEmpty(); }

protected:

	EGArray<egPair> m_List;
	T               m_NotFoundItem;
};

class EGStringCrcMapKey
{
private:

	eg_string_crc Crc;

public:

	EGStringCrcMapKey() = default;
	EGStringCrcMapKey( const eg_string_crc& rhs ): Crc( rhs ) {	}
	operator const eg_string_crc& () const { return Crc; }
	friend eg_bool operator < ( const EGStringCrcMapKey& lhs , const EGStringCrcMapKey& rhs ) { return lhs.Crc.ToUint32() < rhs.Crc.ToUint32(); }
};