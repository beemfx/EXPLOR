///////////////////////////////////////////////////////////////////////////////
// EGCircleArray - A Circular Array
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////
#pragma once

template <class T , eg_size_t MAX> class EGCircleArray
{
public:
	EGCircleArray(): m_Front(0),m_Count(0){ }

	const T& operator[](const eg_uint Index)const
	{
		assert( Index < m_Count ); //The value returned is going to be garbage.
		return m_Array[(m_Front+Index)%MAX];
	}

	T& operator[](const eg_uint Index)
	{
		assert( Index < m_Count ); //The value returned is going to be garbage.
		return m_Array[(m_Front+Index)%MAX];
	}

	void InsertLast( const T& NewItem )
	{
		if( m_Count < MAX )
		{
			m_Array[(m_Front+m_Count)%MAX] = NewItem;
			m_Count++;
		}
		else
		{
			assert( false ); //No more room.
		}
	}

	void InsertLastAndOverwriteFirstIfFull( const T& NewItem )
	{
		if( m_Count < MAX )
		{
			InsertLast( NewItem );
		}
		else
		{
			m_Array[m_Front] = NewItem;
			m_Front = (m_Front+1)%MAX;
			//m_Count doesn't change.
		}
	}

	void RemoveFirst()
	{
		if( m_Count > 0 )
		{
			m_Front = (m_Front+1)%MAX;
			m_Count--;
		}
		else
		{
			assert( false ); //No items.
		}
	}

	void RemoveLast()
	{
		if( m_Count > 0 )
		{
			m_Count--;
		}
		else
		{
			assert( false ); //No items.
		}
	}

	void Clear()
	{
		m_Front = 0;
		m_Count = 0;
	}

	eg_bool IsFull()const{ return m_Count==MAX; }

	eg_uint Len()const{ return m_Count; }
private:
	T m_Array[MAX];
	eg_uint m_Front;
	eg_uint m_Count;
};