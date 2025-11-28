// eg_string_crc - A class for dealing with crc hashes. A string_crc can be
// created at compile-time using the eg_crc macro.
// (c) 2016 Beem Media

class eg_string_crc
{
private:
	eg_uint32 m_Crc;

public:
	//Constructors:
	eg_string_crc() = default;
	eg_string_crc( eg_ctor_t Ct ){ if( Ct == CT_Clear || Ct == CT_Default ){ m_Crc = 0; }  }
	constexpr eg_string_crc(const eg_string_crc& Crc): m_Crc(Crc.m_Crc){}
	explicit eg_string_crc(eg_cpstr16 s): m_Crc(Checksum(s)){}
	explicit eg_string_crc(eg_cpstr8 s): m_Crc(Checksum(s)){}
	constexpr explicit eg_string_crc(const eg_uint32 Crc): m_Crc(Crc){}

	//Assignment:
	const eg_string_crc& operator=(const eg_string_crc& Crc){ m_Crc = Crc.m_Crc; return *this; }

	friend eg_bool operator < ( const eg_string_crc& lhs , const eg_string_crc& rhs ) { return lhs.ToUint32() < rhs.ToUint32(); }

	//Append:
	void operator+=(eg_cpstr16 s)      { m_Crc = ChecksumAppend(m_Crc,s); }
	void operator+=(eg_cpstr8 s)       { m_Crc = ChecksumAppend(m_Crc,s); }
	void operator+=(const eg_string_base& s){ m_Crc = ChecksumAppend(m_Crc,s); }

	eg_string_crc operator + (eg_cpstr16 s ) const { eg_string_crc Out( ChecksumAppend(m_Crc,s)  ); return Out; }
	eg_string_crc operator + (eg_cpstr8 s ) const { eg_string_crc Out( ChecksumAppend(m_Crc,s)  ); return Out; }

	constexpr eg_bool operator==(const eg_string_crc& Crc)const{ return m_Crc == Crc.m_Crc; }
	constexpr eg_bool operator!=(const eg_string_crc& Crc)const{ return m_Crc != Crc.m_Crc; }
	eg_bool operator==( const eg_ctor_t Ct ) const { return *this == eg_string_crc( Ct ); }
	
	//Cast:
	constexpr eg_uint32 ToUint32()const{ return m_Crc; }
	//constexpr operator eg_uint32()const{ return m_Crc; }

public:
	static eg_uint32 Checksum( eg_cpstr16 s )
	{
		return ChecksumAppend( 0 , s );
	}

	static eg_uint32 Checksum( eg_cpstr8 s )
	{
		return ChecksumAppend( 0 , s );
	}

	static eg_int Compare( const eg_string_crc& lhs , const eg_string_crc& rhs )
	{
		if( lhs.m_Crc < rhs.m_Crc )return -1;
		else if( lhs.m_Crc > rhs.m_Crc )return 1;
		else return 0;
	}

	eg_bool IsNull()const{ return m_Crc == 0; }
	eg_bool IsNotNull()const{ return m_Crc != 0; }

	operator eg_bool() const { return !IsNull(); }
	
private:
	static eg_uint32 ChecksumAppend( eg_uint32 Org , eg_cpstr16 s )
	{
		//This is basically Bob Jenkins' One-At-A-Time-Hash.
		//http://www.burtleburtle.net/bob/hash/doobs.html
	
		eg_uint32 Out = Org;
		while(*s)
		{
			{
				Out += *s;
				Out += (Out<<10);
				Out ^= (Out>>6);
			}
			s++;
		}
		return Out;
	}

	static eg_uint32 ChecksumAppend( eg_uint32 Org , eg_cpstr8 s )
	{
		if( nullptr == s)return Org;

		eg_uint32 Out = Org;
		while(*s)
		{
			{
				Out += *s;
				Out += (Out<<10);
				Out ^= (Out>>6);
			}
			s++;
		}
		return Out;
	}

public:
	static eg_string_crc HashData( const void* Data , eg_size_t DataSize )
	{
		eg_string_crc Out( CT_Clear );

		if( nullptr == Data)
		{
			return Out;
		}

		eg_uint32 TempOut = 0;
		for( eg_size_t i=0; i<DataSize; i++ )
		{
			TempOut += static_cast<const eg_byte*>(Data)[i];
			TempOut += (TempOut<<10);
			TempOut ^= (TempOut>>6);
		}
		Out.m_Crc = TempOut;
		return Out;
	}
};

//
// Special helper code to get crc hashes at compile-time as well as switching on crc
//

//#pragma warning(push)
#pragma warning(disable:4307)
static constexpr eg_uint32 EG_Crc_Internal( const eg_str_const8& str , eg_uint32 Step , eg_uint32 Out )
{ 
	return Step == str.size() ? Out : EG_Crc_Internal( str , Step+1 , (Out + str[Step] + ((Out + str[Step])<<10))^((Out + str[Step] + ((Out + str[Step])<<10))>>6)); 
}

static constexpr eg_uint32 eg_crc_u32( const eg_str_const8& str )
{ 
	return EG_Crc_Internal( str , 0 , 0 );
}

static constexpr eg_uint32 EG_Crc_Internal( const eg_str_const16& str , eg_uint32 Step , eg_uint32 Out )
{ 
	return Step == str.size() ? Out : EG_Crc_Internal( str , Step+1 , (Out + str[Step] + ((Out + str[Step])<<10))^((Out + str[Step] + ((Out + str[Step])<<10))>>6)); 
}

static constexpr eg_uint32 eg_crc_u32( const eg_str_const16& str )
{ 
	return EG_Crc_Internal( str , 0 , 0 );
}
//#pragma warning(pop)

template<const eg_uint32 S> static constexpr inline const eg_string_crc __eg_make_crc()
{ 
	return eg_string_crc( S );
}

#define eg_crc( str ) __eg_make_crc<eg_crc_u32(str)>()
#define eg_loc( key , en ) __eg_make_crc<eg_crc_u32(key)>()

#define switch_crc( crc ) switch( crc.ToUint32() )
#define case_crc( str ) case eg_crc_u32(str)
