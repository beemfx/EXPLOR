// (c) 2017 Beem Media

#include "EGLocCompiler.h"
#include "EGCrcDb.h"

eg_loc_lang EGLocCompiler::StringToLocLang( eg_cpstr Str )
{
	if( EGString_EqualsI( Str , "en-US" ) )return eg_loc_lang::ENUS;
	if( EGString_EqualsI( Str , "es-ES" ) )return eg_loc_lang::ESES;
	if( EGString_EqualsI( Str , "fr-FR" ) )return eg_loc_lang::FRFR;
	if( EGString_EqualsI( Str , "de-DE" ) )return eg_loc_lang::DEDE;
	return eg_loc_lang::UNK;
}

eg_cpstr EGLocCompiler::LocLangToString( eg_loc_lang Lang )
{
	switch( Lang )
	{
	case eg_loc_lang::UNK:
		return "Unk";
	case eg_loc_lang::ENUS:
		return "en-US";
	case eg_loc_lang::ESES:
		return "es-ES";
	case eg_loc_lang::FRFR:
		return "fr-FR";
	case eg_loc_lang::DEDE:
		return "de-DE";
	}
	return "Unk";
}

EGLocCompiler::LocData::LocData( eg_cpstr InKey , const eg_loc_char* InLocString )
{
	EGCrcDb::AddAndSaveIfInTool( InKey );
	Key = eg_string_crc(InKey);
	eg_size_t StrLen = EGString_StrLen( InLocString );
	for( eg_size_t i=0; i<StrLen; i++ )
	{
		Buffer.Append( InLocString[i] );
	}
}

void EGLocCompiler::InitAsEnUS()
{
	m_Lang = eg_loc_lang::ENUS;
	m_List.Clear();
	m_MemFile.Clear();
}

void EGLocCompiler::LoadLocFile( eg_cpstr strFile )
{
	XMLLoad( strFile );
}

void EGLocCompiler::LoadLocFile( const EGFileData& File , eg_cpstr NameRef )
{
	XMLLoad( File.GetData() , File.GetSize() , NameRef );
}

void EGLocCompiler::CompileLocTexts()
{
	m_MemFile.Clear();
	// Write the language
	m_MemFile.Write( &m_Lang, sizeof( m_Lang ) );
	//Write the header, which is just a u32 of number of items.
	const eg_size_t NumItems = m_List.Len();
	m_MemFile.Write( &NumItems, sizeof( NumItems ) );

	for( eg_uint i = 0; i < NumItems; i++ )
	{
		//We just write out the length, then the full buffer.
		const LocData& Data = m_List[i];
		const eg_string_crc Crc = Data.Key;
		m_MemFile.Write( &Crc, sizeof( Crc ) );
		eg_size_t Length = Data.Buffer.Len();
		m_MemFile.Write( &Length, sizeof( Length ) );
		m_MemFile.Write( Data.Buffer.GetArray(), Length * sizeof( eg_loc_char ) );
	}
}

void EGLocCompiler::AddOrReplaceEntry( eg_cpstr Key, const eg_loc_char* Text )
{
	LocData NewLocData( Key, Text );

	if( m_List.Contains( NewLocData.Key ) )
	{
		EGLogf( eg_log_t::Warning, __FUNCTION__ ": Replacing %s.", Key );
		for( LocData& LocData : m_List )
		{
			if( LocData.Key == NewLocData.Key )
			{
				LocData.Buffer = NewLocData.Buffer;
				break;
			}
		}
	}
	else
	{
		m_List.Append( NewLocData );
	}
}

void EGLocCompiler::SaveXml( EGFileData& Out ) const
{
	auto Write = [&Out]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof(Buffer) , Args... );
		Out.WriteStr8( Buffer );
	};
	
	Write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
	Write( "<eloc language=\"%s\">\r\n" , LocLangToString( m_Lang ) );

	Write( "\t<!-- BEGIN %s TEXT -->\r\n" , GetXmlFilename() );
	for( const LocData& LocData : m_List )
	{
		eg_string_small Key = EGCrcDb::CrcToString( LocData.Key );
		eg_string_big Text;
		for( const eg_loc_char& c : LocData.Buffer )
		{
			Text.Append( static_cast<eg_char>(c) );
		}
		eg_string_small XmlFriendlyKey;
		eg_string_base::ToXmlFriendly( Key , XmlFriendlyKey );
		eg_string_big XmlFriendlyText;
		eg_string_base::ToXmlFriendly( Text , XmlFriendlyText );
		if( Key.Len() > 0 )
		{
			Write( "\t<text key=\"%s\" string=\"%s\" />\r\n" , XmlFriendlyKey.String() , XmlFriendlyText.String() );
		}
		else
		{
			Write( "\t<!-- Couldn't find key for %s -->\r\n" , XmlFriendlyText.String() );
		}
	}
	Write( "\t<!-- END %s TEXT -->\r\n" , GetXmlFilename() );
	Write( "</eloc>\r\n" );
}

void EGLocCompiler::Localize_ByLang_ENOffset( eg_cpstr InText, EGArray<eg_loc_char>* Out )
{
	Out->Clear();

	eg_cpstr Text = InText;
	eg_size_t WritePos = 0;
	eg_bool bNextIsEscape = false;
	for( eg_size_t ArrayIndex = 0; '\0' != *Text; Text++, ArrayIndex++ )
	{
		eg_char c = *Text;

		if( bNextIsEscape )
		{
			switch( c )
			{
			case 'n':
				c = '\n';
				break;
			default:
				c = c;
				break;
			}
		}

		if( c == '\\' && !bNextIsEscape )
		{
			bNextIsEscape = true;
		}
		else
		{
			( *Out ).ExtendToAtLeast( WritePos+1 );
			( *Out )[WritePos] = c;
			WritePos++;
			bNextIsEscape = false;
		}
	}
}

eg_bool EGLocCompiler::Localize_ByLang( const eg_string_base& In, LocData* Out )
{
	if( In.Len() == 0 )
	{
		Out->Buffer.Clear();
		return true;
	}

	switch( m_Lang )
	{
		//Roman Alphabet:
	case eg_loc_lang::UNK:
	case eg_loc_lang::DEDE:
	case eg_loc_lang::FRFR:
	case eg_loc_lang::ESES:
	case eg_loc_lang::ENUS:
		Localize_ByLang_ENOffset( In, &Out->Buffer );
		break;
	}

	return Out->Buffer.Len() != 0;
}

void EGLocCompiler::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& Getter )
{
	if( "eloc" == Tag )
	{
		eg_loc_lang Lang = StringToLocLang( Getter.GetString( "language" ) );

		if( m_List.HasItems() && m_Lang != Lang )
		{
			EGLogf( eg_log_t::Warning, __FUNCTION__ ": A single loc compilers shoud only compile one language into it. Clearing preivous languages." );
			m_List.Clear();
		}

		m_Lang = Lang;
	}
	else if( "text" == Tag )
	{
		eg_string_big StrText( CT_Clear );

		if( Getter.DoesAttributeExist( "string" ) )
		{
			StrText = Getter.GetString( "string" );
		}
		else if( Getter.DoesAttributeExist( "key_string" ) )
		{
			StrText = Getter.GetString( "key_string" );
		}
		else
		{
			EGLogf( eg_log_t::Warning, ( "locdb Warning: a string had no text." ), StrText.String() );
		}

		eg_string_big KeyString( CT_Clear );
		eg_string_crc Crc( CT_Clear );

		if( Getter.DoesAttributeExist( "key" ) )
		{
			KeyString = Getter.GetString( "key" );
			Crc = eg_string_crc( KeyString );
		}
		else if( Getter.DoesAttributeExist( "key_string" ) )
		{
			KeyString = Getter.GetString( "key_string" );
			Crc = eg_string_crc( KeyString );
		}

		EGCrcDb::AddAndSaveIfInTool( KeyString );

		if( Crc == eg_crc( "" ) )
		{
			EGLogf( eg_log_t::Warning, ( "locdb Warning: \"%s\" had no crc." ), StrText.String() );
		}
		else
		{
			LocData Data;
			eg_bool Success = Localize_ByLang( StrText, &Data );
			Data.Key = Crc;
			if( m_List.Contains( Crc ) )
			{
				EGLogf( eg_log_t::Warning, ( "egresource.locdb Warning: Crc already in database %08X, \"%s\" was not localized." ), Crc, StrText.String() );
			}
			else if( Success )
			{
				m_List.Append( Data );
			}
			else
			{
				EGLogf( eg_log_t::Warning, ( "egresource.locdb Warning: Failed to localize %08X: \"%s\"." ), Crc, StrText.String() );
			}
		}
	}
	else
	{
		assert( false );
	}
}
