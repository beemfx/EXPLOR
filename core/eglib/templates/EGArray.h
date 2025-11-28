// EGArray - A templated array that resizes as nesecary.
// (c) 2015 - Beem Software

#pragma once

#include "EGMemPool.h"
#include "EGCppLibAPI.h"

static const eg_size_t EGARRAY_INVALID_INDEX = 0x80000000;
static const eg_size_t EGARRAY_BASE_RESERVE_SIZE = 16;

class IEGArrayRflI
{
public:

	virtual eg_size_t RflGetArrayLen() const = 0;
	virtual void RflArrayAppendItem() = 0;
	virtual void RflArrayDeleteItemAt( eg_size_t Index ) = 0;
	virtual void RflArrayInsertItemAt( eg_size_t Index ) = 0;
	virtual void* RflGetArrayItemAt( eg_size_t Index ) = 0;
	virtual void RflClear() = 0;
	virtual void RflReserveArraySpace( eg_size_t ReserveSize ) = 0;
	virtual void RflTrimArrayReserved() = 0;
};

template<class T> class EGArray : public IEGArrayRflI
{
public:

	EGArray()
	: m_A(nullptr)
	, m_ASize(0)
	, m_Length(0)
	, m_MemPool( eg_mem_pool::DefaultArray )
	{ 

	}

	EGArray( eg_mem_pool MemPoolIn )
	: m_A(nullptr)
	, m_ASize(0)
	, m_Length(0)
	, m_MemPool( MemPoolIn )
	{ 

	}

	EGArray( const EGArray<T>& rhs )
	: EGArray()
	{
		*this = rhs;
	}

	EGArray( EGArray<T>&& rhs )
	: EGArray( rhs.m_MemPool )
	{
		*this = std::move( rhs );
	}

	const EGArray<T>& operator = ( EGArray<T>&& rhs )
	{
		if( this == &rhs )
		{
			return *this;
		}

		Clear();

		m_A = rhs.m_A;
		m_ASize = rhs.m_ASize;
		m_Length = rhs.m_Length;

		rhs.m_A = nullptr;
		rhs.m_ASize = 0;
		rhs.m_Length = 0;

		return *this;
	}

	~EGArray()
	{
		if( m_A )
		{
			delete [] m_A;
			m_A = nullptr;
		}
	}

	void Clear( eg_bool bFreeMem = true )
	{
		m_Length = 0;
		if( bFreeMem )
		{
			if( m_A )
			{
				delete [] m_A;
				m_A = nullptr;
			}
			m_ASize = 0;
		}
	}

	void Append( const EGArray<T>& RightList )
	{
		ExpandReserved( Len() + RightList.Len() );
		for( const T& Item : RightList )
		{
			Append( Item );
		}
	}

	const EGArray<T>& operator=( const EGArray<T>& rhs )
	{
		Resize(rhs.Len());
		for( eg_size_t i=0; i<rhs.Len(); i++ )
		{
			(*this)[i] = rhs[i];
		}
		return *this;
	}

	void Append( const T& NewItem )
	{
		const eg_size_t PlaceIndex = Len();

		if( PlaceIndex >= m_ASize )
		{
			ExpandReserved( PlaceIndex+1 );
		}

		if( PlaceIndex >= m_Length )
		{
			m_Length = PlaceIndex+1;
		}

		(*this)[PlaceIndex] = NewItem;
	}

	void AppendMove( T& NewItem )
	{
		const eg_size_t PlaceIndex = Len();

		if( PlaceIndex >= m_ASize )
		{
			ExpandReserved( PlaceIndex+1 );
		}

		if( PlaceIndex >= m_Length )
		{
			m_Length = PlaceIndex+1;
		}

		(*this)[PlaceIndex] = std::move(NewItem);
	}

#define EGARRAY_TYPE EGArray
#include "EGArray.shared.hpp"
#undef EGARRAY_TYPE

private:

	void ResizeReserved( eg_size_t Size )
	{
		if( Size == m_ASize )
		{
			return;
		}

		if( 0 == Size )
		{
			if( m_A )
			{
				delete [] m_A;
				m_A = nullptr;
			}
			m_ASize = 0;
			m_Length = 0;
			return;
		}

		if( nullptr == m_A )
		{
			assert( 0 == m_ASize );
			assert( 0 == m_Length );
			m_A = new ( m_MemPool ) T[Size];
			m_ASize = Size;
		}
		else
		{
			T* NewA = new ( m_MemPool ) T[Size];
			if( nullptr != NewA )
			{
				m_ASize = Size;
				m_Length = EG_Min( m_Length , m_ASize ); //Possibly shrink the array.
				for( eg_size_t i=0; i<m_Length; i++ )
				{
					NewA[i] = std::move( m_A[i] );
				}
				delete [] m_A;
				m_A = NewA;
			}
			else
			{
				assert( false ); //Couldn't resize array.
			}
		}
	}

	void ExpandReserved( eg_size_t MinSize )
	{
		if( m_ASize < MinSize )
		{
			eg_size_t WantedSize = EG_Max<eg_size_t>( MinSize , m_ASize*2 );
			if( 0 < WantedSize && WantedSize < EGARRAY_BASE_RESERVE_SIZE )
			{
				WantedSize = EGARRAY_BASE_RESERVE_SIZE;
			}
			ResizeReserved( WantedSize );
		}
	}

	//
	// Reflection Interface
	//

	virtual eg_size_t RflGetArrayLen() const override final 
	{ 
		return Len();
	}

	virtual void RflArrayAppendItem() override final
	{
		Resize( Len() + 1 );
	}

	virtual void RflArrayDeleteItemAt( eg_size_t Index ) override final
	{
		DeleteByIndex( Index );
	}

	virtual void* RflGetArrayItemAt( eg_size_t Index ) override final
	{
		return reinterpret_cast<void*>(&m_A[Index]);
	}

	virtual void RflArrayInsertItemAt( eg_size_t Index ) override final
	{
		eg_byte NewItemMem[sizeof(T)];
		zero( &NewItemMem );
		T* NewItem = new ( NewItemMem ) T();
		InsertAt( Index , std::move( *NewItem ) );
		NewItem->~T();
	}

	virtual void RflClear() override final
	{
		Clear();
	}

	virtual void RflReserveArraySpace( eg_size_t ReserveSize ) override final
	{
		Reserve( ReserveSize );
	}

	virtual void RflTrimArrayReserved() override final
	{
		TrimReserved();
	}

protected:

	T*                m_A;
	eg_size_t         m_ASize;  //Number of reserved elements
	eg_size_t         m_Length; //Actual length of the array.
	const eg_mem_pool m_MemPool;
};

template<class T,eg_size_t MAX_SIZE> class EGFixedArray
{
public:

	static const eg_size_t ARRAY_SIZE = MAX_SIZE;

public:

	EGFixedArray()
	: m_Length(0)
	{ 
		ResizeReserved( MAX_SIZE );
	}

	EGFixedArray( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			m_Length = 0;
			ResizeReserved( MAX_SIZE );
		}
		else if( Ct == CT_Preserve )
		{
			
		}
	}

	EGFixedArray( const EGFixedArray<T,MAX_SIZE>& rhs )
	: EGFixedArray()
	{
		*this = rhs;
	}

	void Clear( eg_bool bFreeMem = true )
	{
		unused( bFreeMem );

		m_Length = 0;
	}

	const EGFixedArray<T,MAX_SIZE>& operator=( const EGFixedArray<T,MAX_SIZE>& rhs )
	{
		Resize(rhs.Len());
		for( eg_size_t i=0; i<rhs.Len(); i++ )
		{
			(*this)[i] = rhs[i];
		}
		return *this;
	}

#define EGARRAY_TYPE EGFixedArray
#include "EGArray.shared.hpp"
#undef EGARRAY_TYPE

	void Append( const EGFixedArray<T,MAX_SIZE>& RightList )
	{
		ExpandReserved( Len() + RightList.Len() );
		for( const T& Item : RightList )
		{
			Append( Item );
		}
	}

	void Append( const T& NewItem )
	{
		if( !IsFull() )
		{
			const eg_size_t PlaceIndex = Len();

			if( PlaceIndex >= m_ASize )
			{
				ExpandReserved( PlaceIndex+1 );
			}

			if( PlaceIndex >= m_Length )
			{
				m_Length = PlaceIndex+1;
			}

			(*this)[PlaceIndex] = NewItem;
		}
		else
		{
			assert( false ); // Fixed size array can't take any more items.
		}
	}

	void AppendMove( T& NewItem )
	{
		if( !IsFull() )
		{
			const eg_size_t PlaceIndex = Len();

			if( PlaceIndex >= m_ASize )
			{
				ExpandReserved( PlaceIndex+1 );
			}

			if( PlaceIndex >= m_Length )
			{
				m_Length = PlaceIndex+1;
			}

			(*this)[PlaceIndex] = std::move(NewItem);
		}
		else
		{
			assert( false ); // Fixed size array can't take any more items.
		}
	}

	eg_bool IsFull() const { return m_Length >= ARRAY_SIZE; }
	
private:

	void ResizeReserved( eg_size_t Size )
	{
		unused( Size );
		assert( m_Length <= Size );
		m_Length = EG_Min( m_Length , countof(m_A) ); //Possibly shrink the array.
	}

	void ExpandReserved( eg_size_t MinSize )
	{
		ResizeReserved( MinSize );
	}

private:

	T           m_A[MAX_SIZE];
	eg_size_t   m_Length; //Actual length of the array.
	static const eg_size_t m_ASize = MAX_SIZE;
};
