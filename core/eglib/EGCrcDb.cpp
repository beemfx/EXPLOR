// (c) 2016 Beem Media

#include "EGCrcDb.h"
#include "EGToolsHelper.h"
#include "EGFileData.h"

EGCrcDb* EGCrcDb::GlobalInstance = nullptr;

void EGCrcDb::Init()
{
	assert( nullptr == GlobalInstance );
	if( nullptr == GlobalInstance )
	{
		GlobalInstance = new EGCrcDb;
	}
}

void EGCrcDb::Deinit()
{
	assert( nullptr != GlobalInstance );
	if( GlobalInstance )
	{
		delete GlobalInstance;
		GlobalInstance = nullptr;
	}
}

EGCrcDb* EGCrcDb::Get()
{
	return GlobalInstance;
}

eg_string_crc EGCrcDb::StringToCrc( eg_cpstr Str )
{
	AddAndSaveIfInTool( Str );
	return eg_string_crc(Str);
}

eg_string_small EGCrcDb::CrcToString( eg_string_crc Crc )
{
	eg_string_small Out( "" );
	EGCrcDb* Db = Get();
	if( Db )
	{
		const egItem& Item = Db->FindString( Crc );
		if( Item.Crc == Crc && Item.Strings.IsValidIndex(0) )
		{
			Out = Item.Strings[0];
		}
	}

	return Out;
}

EGCrcDb::EGCrcDb()
: m_DefaultItem( CT_Clear )
{
	EGFileData DbFile( eg_file_data_init_t::HasOwnMemory );
	EGToolsHelper_OpenFile( EGString_ToWide(GetSaveFilename()) , DbFile );
	
	while( (DbFile.GetSize() - DbFile.Tell()) > 8 )
	{
		eg_char CrcAsString[9];
		DbFile.Read( &CrcAsString , 9 ); // Read an extra character for the ':'
		CrcAsString[8] = '\0';
		egItem NewItem( CT_Clear );
		NewItem.Crc = eg_string_crc( eg_string(CrcAsString).ToUIntFromHex() );

		eg_bool bEnd = false;
		eg_size_t ZeroCount = 0;
		eg_string_small String;
		while( !bEnd )
		{
			eg_char c = '\0';
			DbFile.Read( &c , sizeof(c) );
			if( c == '\0' )
			{
				ZeroCount++;
				if( 1 == ZeroCount )
				{
					NewItem.AddString( String );
					String.Clear();
				}
				else
				{
					eg_char End[2];
					DbFile.Read( &End , sizeof(End) );
					bEnd = true;
				}
			}
			else
			{
				ZeroCount = 0;
				String += c;
			}
		}
		m_List.Append( NewItem );
	}
}

EGCrcDb::~EGCrcDb()
{

}


eg_string_big EGCrcDb::GetSaveFilename() const
{
	return "_BUILD/strings.crcdb";
}

void EGCrcDb::AddString( eg_cpstr String, eg_bool bSaveIfNew )
{
	eg_string_crc Crc( String );

	if( Crc == eg_string_crc( CT_Clear ) )
	{
		return;
	}

	eg_bool bFound = false;
	eg_bool bAdded = false;

	for( egItem& Item : m_List )
	{
		if( Item.Crc == Crc )
		{
			bAdded = Item.AddString( String );
			bFound = true;
			break;
		}
	}

	if( !bFound )
	{
		egItem NewItem( CT_Clear );
		NewItem.Crc = Crc;
		NewItem.AddString( String );
		m_List.Append( NewItem );
		bAdded = true;
	}

	if( bAdded && bSaveIfNew )
	{
		Serialize();
	}
}

const EGCrcDb::egItem& EGCrcDb::FindString( eg_string_crc Crc )
{
	for( const egItem& Item : m_List )
	{
		if( Item.Crc == Crc )
		{
			return Item;
		}
	}

	return m_DefaultItem;
}

void EGCrcDb::Serialize()
{
	EGLogf( eg_log_t::Verbose , "Updating crc database." );

	EGFileData DbFile( eg_file_data_init_t::HasOwnMemory );

	const eg_char SEP = 0;
	const eg_char ENDL[] = { '\r' , '\n' };

	for( const egItem& Item : m_List )
	{
		DbFile.WriteStr8( EGString_Format( "%08X:" , Item.Crc.ToUint32() ) );
		for( const eg_string_small& Str : Item.Strings )
		{
			DbFile.WriteStr8( Str );
			DbFile.Write( &SEP , sizeof(SEP) );
		}
		DbFile.Write( &SEP , sizeof(SEP) );
		DbFile.Write( &ENDL , sizeof(ENDL) );
	}

	EGToolsHelper_SaveFile( EGString_ToWide(GetSaveFilename()) , DbFile );
}


