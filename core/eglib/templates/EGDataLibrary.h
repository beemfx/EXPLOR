// EGDataLibrary
// (c) 2016 Beem Media
#pragma once
#include "EGXMLBase.h"
#include "EGCrcDb.h"
#include "EGFileData.h"

template< typename T >
class EGDataLibrary : public IXmlBase
{
public:

	typedef EGArray<T> EGInfoArray;

public:

	EGDataLibrary()
	: m_Infos()
	, m_DummyInfo( CT_Default )
	{

	}

	void Init( eg_cpstr Filename )
	{
		m_DummyInfo = T( CT_Default );
		AddDataFile( Filename );
	}

	void Deinit()
	{
		m_Infos.Clear();
	}

	void AddDataFile( eg_cpstr Filename )
	{
		eg_size_t PrevLen = m_Infos.Len();
		XMLLoad( Filename );
		EGLogf( eg_log_t::GameLib, "%s: %u data loaded.", XMLObjName(), static_cast<eg_uint>(m_Infos.Len()-PrevLen) );
	}

	void AddDataItem( eg_cpstr Id , const T& Item )
	{
		if( m_bWarnAboutDuplicateIds && Contains( eg_string_crc(Id) ) )
		{
			EGLogf( eg_log_t::Error , "%s already had an item with id %s (or crc collision), only the first item with this id will be found." , XMLObjName() , Id );
			assert( false );
		}
		T NewItem = Item;
		EGCrcDb::AddAndSaveIfInTool( Id );
		NewItem.Id = eg_string_crc(Id);
		m_Infos.Append( NewItem );
	}

	void AddOrReplaceDataItem( eg_cpstr Id , const T& Item )
	{
		if( !Contains( eg_string_crc(Id) ) )
		{
			AddDataItem( Id , Item );
			return;
		}

		eg_string_crc IdCrc = eg_string_crc(Id);

		for( T& Info : m_Infos )
		{
			if( Info.Id == IdCrc )
			{
				Info = Item;
				return;
			}
		}

		assert( false ); // Didn't really have this item.
	}

	const T& FindInfo( eg_string_crc Id ) const
	{
		for( const T& Info : m_Infos )
		{
			if( Info.Id == Id )
			{
				return Info;
			}
		}

		return m_DummyInfo;
	}

	eg_bool Contains( eg_string_crc Id ) const
	{
		for( const T& Info : m_Infos )
		{
			if( Info.Id == Id )
			{
				return true;
			}
		}

		return false;
	}

	eg_string GetIdAsString( eg_string_crc Id ) const
	{
		eg_string IdAsString = "UNK_NEED_DB";
		if( EGCrcDb::Get() )
		{
			EGCrcDb::egItem CrcIdItem = EGCrcDb::Get()->FindString( Id );
			if( CrcIdItem.Crc == Id && CrcIdItem.Strings.Len() > 0 )
			{
				IdAsString = CrcIdItem.Strings[0];
			}
		}
		return IdAsString;
	}

	void Serialize( EGFileData& Out )
	{
		Out.WriteStr8( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
		eg_string_big LibName = GetLibraryName();
		LibName.ConvertToLower();
		Out.WriteStr8( EGString_Format("<%s>\r\n", LibName.String() ) );
		for( const T& Info : m_Infos )
		{
			eg_string_big ItemInfoString = SerializeItem( Info );
			Out.WriteStr8( EGString_Format( "\t%s\r\n" , ItemInfoString.String() ) );
		}
		Out.WriteStr8( EGString_Format("</%s>\r\n", LibName.String() ) );
	}

	void SetWarnAboutDuplicateIds( eg_bool bNewValue ) { m_bWarnAboutDuplicateIds = bNewValue; }

protected:

	T           m_DummyInfo;
	EGInfoArray m_Infos;
	eg_bool m_bWarnAboutDuplicateIds = true;

protected:

	virtual void PoplateItemFromTag( T& Info , const EGXmlAttrGetter& AttGet ) const = 0;
	virtual eg_cpstr GetLibraryName() const = 0;
	virtual eg_cpstr GetItemTag() const = 0;
	virtual eg_string_big SerializeItem( const T& Info ) const
	{
		unused( Info ); 
		return EGString_Format( "<%s id=\"%s\" not_implemented=\"\"/>" , GetItemTag() , GetIdAsString( Info.Id ).String() );
	}

protected:

	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& Getter ) override
	{
		if( Tag.EqualsI( GetItemTag() ) )
		{
			T NewInfo( CT_Default );
			eg_string IdString = Getter.GetString("id");
			NewInfo.Id = eg_string_crc(IdString);
			PoplateItemFromTag( NewInfo , Getter );
			AddDataItem( IdString , NewInfo );
		}
	}

private:

	virtual eg_cpstr XMLObjName() const override final { return GetLibraryName(); }
};