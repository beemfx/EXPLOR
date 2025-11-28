#pragma once

template <const eg_size_t MAX_SIZE> class EGBoolList
{
public:
	EGBoolList( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			UnsetAll();
		}
	}

	~EGBoolList()
	{

	}

	void Set(eg_uint nIndex)
	{
		if(nIndex >= MAX_SIZE)
		{
			assert(false);
			return;
		}

		eg_uint n = nIndex/32;
		eg_uint32 nFlag = 1<<(nIndex%32);

		m_aData[n] |= nFlag;
	}

	void Unset(eg_uint nIndex)
	{
		if(nIndex >= MAX_SIZE)
		{
			assert(false);
			return;
		}

		eg_uint n = nIndex/32;
		eg_uint32 nFlag = 1<<(nIndex%32);

		m_aData[n] &= ~nFlag;
	}

	eg_bool IsSet(eg_uint nIndex)const
	{
		if(nIndex >= MAX_SIZE)
		{
			assert(false);
			return false;
		}

		eg_uint n = nIndex/32;
		eg_uint32 nFlag = 1<<(nIndex%32);

		return 0 != (m_aData[n] & nFlag);
	}

	void SetTo( eg_uint nIndex , eg_bool bValue )
	{
		if( bValue )
		{
			Set( nIndex );
		}
		else
		{
			Unset( nIndex );
		}
	}

	void UnsetAll()
	{
		for(eg_uint i=0; i<countof(m_aData); i++)
		{
			m_aData[i] = 0;
		}
	}
	void SetAll()
	{
		for(eg_uint i=0; i<countof(m_aData); i++)
		{
			m_aData[i] = 0xFFFFFFFF;
		}
	}
private:
	eg_uint32 m_aData[MAX_SIZE/32 + 1];
};