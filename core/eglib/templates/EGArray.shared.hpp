// (c) 2017 Beem Media

#ifndef EGARRAY_TYPE
#define EGARRAY_TYPE EGArrayBody
#define EGARRAY_TYPE_DEFINED
template<typename T> class EGArrayBody {
#endif

public:

	T* GetArray(){ return m_A; }
	const T* GetArray()const{ return m_A; }

	void Resize( eg_size_t NewSize )
	{
		if( NewSize == 0 )
		{
			Clear();
		}
		else
		{
			ExpandReserved( EG_Max( NewSize , m_Length ) );
			eg_size_t OldLen = m_Length;
			m_Length = EG_Min( NewSize , m_ASize );

			// If we added any new items make sure they are zeroed.
			eg_byte ZeroedMem[sizeof(T)];
			for( eg_size_t i=OldLen; i<m_Length; i++ )
			{
				zero( &ZeroedMem );
				T* NewZeroed = new ( ZeroedMem ) T();
				m_A[i] = std::move( *NewZeroed );
				NewZeroed->~T();
			}
		}
	}

	void ExtendToAtLeast( eg_size_t NewMinSize )
	{
		if( Len() < NewMinSize )
		{
			if( NewMinSize > m_ASize )
			{
				Reserve( EG_Max<eg_size_t>( NewMinSize , m_ASize * 2 ) );
			}
			Resize( NewMinSize );
		}
	}

	void Reserve( eg_size_t Size )
	{
		if( Size > m_ASize )
		{
			ResizeReserved( Size );
		}
	}

	void TrimReserved()
	{
		ResizeReserved( m_Length );
	}

	const T& operator[]( eg_size_t Index ) const
	{
		assert( Index < m_Length ); //Nothing to return! Crash likely.
		return m_A[Index];
	}

	T& operator[]( eg_size_t Index )
	{
		assert( Index < m_Length ); //Nothing to return! Crash likely, need to resize first.
		return m_A[Index];
	}

	const T& Last() const { assert( Len() > 0 ); return (*this)[Len()-1]; }
	T& Last() { assert( Len() > 0 ); return (*this)[Len()-1]; }

	eg_size_t Len()const{ return m_Length; }
	eg_int LenAsInt() const { return static_cast<eg_int>(Len()); }
	eg_uint LenAsUInt() const { return static_cast<eg_uint>(Len()); }
	template<class RetType> RetType LenAs() const { return static_cast<RetType>(Len()); }
	eg_size_t GetReservedLen() const { return m_ASize; }
	eg_bool IsEmpty() const { return Len() == 0; }
	eg_bool HasItems() const { return Len() > 0; }

	void AppendUnique( const T& NewItem )
	{
		if( !Contains( NewItem ) )
		{
			Append( NewItem );
		}
	}

	void InsertAt( eg_size_t Index , const T& NewItem )
	{
		if( Index >= Len() )
		{
			assert( Index == Len() );
			Append( NewItem );
		}
		else
		{
			Resize( Len()+1 );
			for( eg_size_t i = Len()-1; i>Index; i-- )
			{
				(*this)[i] = std::move((*this)[i-1]);
			}

			(*this)[Index] = NewItem;
		}
	}

	void Append( const T* Arr , eg_size_t ArrCount )
	{
		ExpandReserved( Len() + ArrCount );
		for( eg_size_t i=0; i<ArrCount; i++ )
		{
			Append( Arr[i] );
		}
	}

	void ExtendSize( eg_size_t AddSize )
	{
		Resize( Len() + AddSize );
	}

	void DeleteByIndex( eg_size_t Index )
	{
		if( Index < m_Length )
		{
			for( eg_size_t i=Index; i<(m_Length-1); i++ )
			{
				m_A[i] = std::move( m_A[i+1] );
			}

			m_Length--;
			if( m_Length == 0 )
			{
				Clear();
			}
		}
		else
		{
			assert( false ); // NO such index.
		}
	}

	void DeleteByItem( const T& rhs )
	{
		for( eg_size_t i=0; i<Len(); i++ )
		{
			if( m_A[i] == rhs )
			{
				DeleteByIndex( i );
				return;
			}
		}

		assert( false ); // No such thing to remove.
		return;
	}

	template<typename Predicate>
	void DeleteAllByPredicate( Predicate Test )
	{
		// Could be more optimal but works
		eg_bool bFoundSomething = true;
		while( bFoundSomething )
		{
			bFoundSomething = false;
			for( eg_size_t i=0; i<Len() && !bFoundSomething; i++ )
			{
				if( Test( m_A[i] ) )
				{
					DeleteByIndex( i );
					bFoundSomething = true;
				}
			}
		}
	}

	eg_bool Contains( const T& rhs ) const
	{
		return GetIndexOf( rhs ) != EGARRAY_INVALID_INDEX;
	}

	eg_size_t GetIndexOf( const T& rhs ) const
	{
		for( eg_size_t i=0; i<Len(); i++ )
		{
			if( m_A[i] == rhs )
			{
				return i;
			}
		}

		return EGARRAY_INVALID_INDEX;
	}

	eg_bool IsValidIndex( eg_size_t Index ) const
	{
		return 0 <= Index && Index < Len();
	}

	//
	// Stack operations
	//
	void Push( const T& NewItem )
	{
		Append( NewItem );
	}

	void Pop()
	{
		if( Len() > 0 )
		{
			Resize( Len() - 1 );
		}
	}

	T& Top()
	{
		assert( Len() > 0 ); // Crash shall occur
		return (*this)[Len()-1];
	}

	const T& Top() const
	{
		assert( Len() > 0 ); //Crash shall occur
		return (*this)[Len()-1];
	}

	void SetAll( const T& To )
	{
		for( eg_size_t i=0; i<Len(); i++ )
		{
			(*this)[i] = To;
		}
	}

//
// Sorting
//
private:

	template<typename L> static eg_int Sort_Partition( L& IsLessThan , T* A , eg_int Lo , eg_int Hi )
	{
		const T& Pivot = A[Hi];
		eg_int i = Lo;     // place for swapping
		for( eg_int j = Lo; j< Hi; j++ )
		{
			if( IsLessThan( A[j] , Pivot ) )
			{
				T Temp = std::move( A[i] );
				A[i] = std::move( A[j] );
				A[j] = std::move( Temp );           
				i++;
			}
		}

		T Temp = std::move( A[i] );
		A[i] = std::move( A[Hi] );
		A[Hi] = std::move( Temp );
		return i;
	}

	template<typename L> static void Sort_QuickSort( L& IsLessThan , T* A , eg_int Lo , eg_int Hi )
	{
		if( Lo < Hi )
		{
			eg_int p = Sort_Partition( IsLessThan , A , Lo , Hi );
			Sort_QuickSort( IsLessThan , A , Lo , p-1 );
			Sort_QuickSort( IsLessThan , A , p+1 , Hi );
		}
	};

public:

	template<typename L> void Sort( L& IsLessThan )
	{
		Sort_QuickSort( IsLessThan , m_A , 0 , static_cast<eg_int>(m_Length-1) );
	}

	void Sort()
	{
		Sort( []( const T& Left , const T& Right )->eg_bool{ return Left < Right; } );
	}

	//
	// Binary Search
	//
	void BinarySearch_PrepSort()
	{
		Sort( []( const T& left , const T& right )->eg_bool{ return left.Key < right.Key; } );
	}

	void BinarySearch_Insert( const T& NewItem )
	{
		// Find the spot to insert the new item.
		for( eg_size_t i=0; i<Len(); i++ )
		{
			if( !((*this)[i].Key < NewItem.Key) )
			{
				InsertAt( i , NewItem );
				return;
			}
		}

		Append( NewItem );
	}

	template<typename K> eg_size_t BinarySearch_GetIndexOf( const K& Key ) const
	{
		// If this list is not sorted by Key prior to calling this
		// the result may be incorrect, and it's also possible that the game can
		// lock up here, so use with caution.

		eg_signed_size_t imin = 0;
		eg_signed_size_t imax = static_cast<eg_signed_size_t>(m_Length) - 1;
		while( imax >= imin )
		{
			eg_signed_size_t imid = (imin + imax) / 2;
			eg_bool bKeyLessThan = Key < m_A[imid].Key;

			if( bKeyLessThan )
			{
				imax = imid-1;
			}
			else
			{
				eg_bool bItemLessThan = m_A[imid].Key < Key;
				if( bItemLessThan )
				{
					imin = imid+1;
				}
				else
				{
					return static_cast<eg_size_t>(imid);
				}
			}
		}

		return m_Length;
	}

	template<typename K> eg_bool BinarySearch_Contains( const K& Key ) const
	{		
		return IsValidIndex( BinarySearch_GetIndexOf( Key ) );
	}

	template<typename K> const T& BinarySearch_Get( const K& Key ) const
	{
		eg_size_t Index = BinarySearch_GetIndexOf( Key );
		assert( IsValidIndex( Index ) ); // Will crash...
		return (*this)[Index];
	}

	template<typename K> T& BinarySearch_Get( const K& Key )
	{
		eg_size_t Index = BinarySearch_GetIndexOf( Key );
		assert( IsValidIndex( Index ) ); // Will crash...
		return (*this)[Index];
	}

	template<typename K> void BinarySearch_Delete( const K& Key )
	{
		eg_size_t Index = BinarySearch_GetIndexOf( Key );
		DeleteByIndex( Index ); // Will assert if this was invalid.
	}

	//
	// Iterator stuff:
	//
	struct CItr
	{
		const T* List;
		eg_size_t Index;
		CItr( const T* OwnerList , eg_size_t InIndex ): List( OwnerList ) , Index(InIndex){ }
		eg_bool operator!=( const CItr& rhs ){ return Index != rhs.Index || List != rhs.List; }
		friend const CItr& operator++( CItr& rhs ){ rhs.Index++; return rhs; }
		const T& operator *(){ return List[Index]; }
	};
	const CItr begin() const { return CItr(m_A,0); }
	const CItr end() const { return CItr(m_A,Len()); }

	struct Itr
	{
		T* List;
		eg_size_t Index;
		Itr( T* OwnerList , eg_size_t InIndex ): List( OwnerList ) , Index(InIndex){ }
		eg_bool operator!=( const Itr& rhs ){ return Index != rhs.Index || List != rhs.List; }
		friend Itr& operator++( Itr& rhs ){ rhs.Index++; return rhs; }
		T& operator *(){ return List[Index]; }
	};
	Itr begin() { return Itr(m_A,0); }
	Itr end() { return Itr(m_A,Len()); }

#ifdef EGARRAY_TYPE_DEFINED
#undef EGARRAY_TYPE_DEFINED
#undef EGARRAY_TYPE
};
#endif