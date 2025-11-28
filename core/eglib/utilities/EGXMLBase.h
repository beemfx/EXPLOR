// (c) 2011 Beem Media

#pragma once

class EGXmlAttrGetter;

class IXmlBase
{
private:
	struct egXMLState* m_pXML;
	eg_path            m_strFilename;
	
protected:

	void XMLLoad( eg_cpstr strFile );
	void XMLLoad( const void* Data , eg_size_t DataSize , eg_cpstr RefName );
	
	virtual void OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet ) = 0;
	virtual void OnTagEnd( const eg_string_base& Tag ){ unused( Tag ); }
	virtual void OnData( eg_cpstr DataStr , eg_uint DataLen ){ unused( DataStr ,DataLen ); }
	virtual eg_cpstr XMLObjName() const = 0;

	const eg_string_base& GetXmlTagUp( eg_uint ParentLevels ) const;
	eg_uint GetXmlTagLevels()const;

	eg_cpstr GetXmlFilename()const{ return m_strFilename; }

private:

	//Data handling functions:
	static void EXPAT_Start(void *userData, const eg_char *name, const eg_char **atts);
	static void EXPAT_End(void *userData, const eg_char *name);
	static void EXPAT_CharData(void *userData, const eg_char *s, int len);

	static eg_char* AllocChars(eg_uint NumChars);
	static void FreeChars(eg_char*& p);
	
	//Data handling helper functions:
	static void EXPAT_DumpData(IXmlBase* p);

	static void* EXPAT_Malloc(eg_size_t Size);
	static void* EXPAT_Realloc(void* p, eg_size_t Size);
	static void  EXPAT_Free(void* p);
};

class EGXmlAttrGetter
{
private:

	const eg_cpstr*const m_atts;
	eg_size_t            m_Count;

public:

	EGXmlAttrGetter( const EGXmlAttrGetter& ) = delete;
	EGXmlAttrGetter( const eg_cpstr* atts )
	: m_atts(atts)
	, m_Count(0)
	{ 
		m_Count = 0;
		for( eg_uint i=0; nullptr != m_atts[i]; i+=2 )
		{
			m_Count++;
		}
	}

	eg_size_t Num() const {	return m_Count; }

	eg_string_big GetAttributeNameByIndex( eg_size_t Index ) const
	{
		eg_string_big Out( CT_Clear );

		if( 0 <= Index && Index < m_Count )
		{
			Out = m_atts[Index*2];
		}

		return Out;
	}

	eg_string_big GetAttributeValueByIndex( eg_size_t Index ) const
	{
		eg_string_big Out( CT_Clear );

		if( 0 <= Index && Index < m_Count )
		{
			Out = m_atts[Index*2+1];
		}

		return Out;
	}

	eg_bool DoesAttributeExist( eg_cpstr Name )const
	{
		for( eg_uint i=0; nullptr != m_atts[i]; i+=2 )
		{
			if( EGString_Equals(Name , m_atts[i]) )
			{
				return true;
			}
		}
		return false;
	}

	eg_string_big GetString( eg_cpstr Name , eg_cpstr Default = (""))const
	{
		for( eg_uint i=0; nullptr != m_atts[i]; i+=2 )
		{
			if( EGString_Equals(Name , m_atts[i]) )
			{
				return m_atts[i+1];
			}
		}
		return eg_string_big(Default);
	}

	eg_bool GetBool( eg_cpstr Name , eg_bool Default = false )const
	{
		eg_bool Out = Default;

		if( DoesAttributeExist( Name ) )
		{
			eg_string_big StrParm = GetString( Name , Default ? "true" : "false" );
			if( StrParm.EqualsI( "true" ) || StrParm.EqualsI( "1" ) )
			{
				Out = true;
			}
			else if( StrParm.EqualsI( "false" ) || StrParm.EqualsI( "0" ) )
			{
				Out = false;
			}
			else
			{
				//assert( false ); // Not a valid bool value
				EGLogf( eg_log_t::Error , __FUNCTION__ ": An invalid bool present in the xml file." );
			}
		}

		return Out;
	}

	eg_uint GetUInt( eg_cpstr Name , eg_uint Default = 0 )const
	{
		for( eg_uint i=0; nullptr != m_atts[i]; i+=2 )
		{
			if( EGString_Equals(Name , m_atts[i]) )
			{
				return eg_string_big(m_atts[i+1]).ToUInt();
			}
		}
		return Default;
	}

	eg_int GetInt( eg_cpstr Name , int Default = 0 )const
	{
		for( eg_uint i=0; nullptr != m_atts[i]; i+=2 )
		{
			if( EGString_Equals(Name , m_atts[i]) )
			{
				return eg_string_big(m_atts[i+1]).ToInt();
			}
		}
		return Default;
	}

	eg_real GetFloat( eg_cpstr Name , eg_real Default = 0 )const
	{
		for( eg_uint i=0; nullptr != m_atts[i]; i+=2 )
		{
			if( EGString_Equals(Name , m_atts[i]) )
			{
				return eg_string_big(m_atts[i+1]).ToFloat();
			}
		}
		return Default;
	}

	void GetVec( eg_cpstr Name , eg_real* Out , eg_uint Count , eg_bool IsBase64 )const
	{
		eg_string_big sVec = GetString( Name );
		EGString_GetFloatList( sVec , sVec.Len() , reinterpret_cast<void*>(Out) , Count , IsBase64 );
	}

	template<typename T> void GetVec( eg_cpstr Name , T* Out , eg_uint Count , eg_bool IsBase64 )const
	{
		eg_string_big sVec = GetString( Name );
		EGString_GetFloatList( sVec , sVec.Len() , reinterpret_cast<void*>(Out) , Count , IsBase64 );
	}

	template<typename T> void GetIntVec( eg_cpstr Name , T* Out , eg_uint Count , eg_bool IsBase64 )const
	{
		eg_string_big sVec = GetString( Name );
		EGString_GetIntList( sVec , sVec.Len() , reinterpret_cast<void*>(Out) , Count , IsBase64 );
	}

	void GetIntArray( eg_cpstr Name , eg_int* Out , eg_uint Count , eg_bool IsBase64 )const
	{
		eg_string_big sVec = GetString( Name );
		EGString_GetIntList( sVec , sVec.Len() , reinterpret_cast<void*>(Out) , Count , IsBase64 );
	}

	void GetFloatArray( eg_cpstr Name , eg_real* Out , eg_uint Count , eg_bool IsBase64 ) const
	{
		eg_string_big sVec = GetString( Name );
		EGString_GetFloatList( sVec, sVec.Len(), reinterpret_cast<void*>( Out ), Count, IsBase64 );
	}

	eg_uint GetNumAttributes( void )const
	{
		eg_uint Count = 0;
		for( eg_uint i=0; nullptr != m_atts[i]; i+=2 )
		{
			Count++;
		}
		return Count;
	}

	void GetAttributePair( eg_uint Index , eg_string_base& Att , eg_string_base& Val )const
	{
		eg_uint Count = 0;
		for( eg_uint i=0; nullptr != m_atts[i]; i+=2 )
		{
			if( Count == Index )
			{
				Att = m_atts[i];
				Val = m_atts[i+1];
				return;
			}
			Count++;
		}
		assert( false );
	}
};