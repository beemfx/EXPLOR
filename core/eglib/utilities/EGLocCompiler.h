// (c) 2017 Beem Media

#pragma once

#include "EGXMLBase.h"
#include "EGFileData.h"

class EGLocCompiler: public IXmlBase
{
private:

	class LocData
	{
	public:
		eg_string_crc        Key;
		EGArray<eg_loc_char> Buffer;
		LocData(): Key(CT_Clear) , Buffer() {}
		LocData( eg_cpstr InKey , const eg_loc_char* InLocString );
		~LocData(){ }
	};

	class EGLocTable: public EGArray<LocData>
	{
	public:
		eg_bool Contains( eg_string_crc Key )
		{
			for( eg_size_t i=0; i<Len(); i++ )
			{
				if( (*this)[i].Key == Key )
				{
					return true;
				}
			}
			return false;
		}
	};

public:

	EGLocCompiler()
	: m_MemFile( eg_file_data_init_t::HasOwnMemory )
	, m_List()
	, m_Lang( eg_loc_lang::UNK )
	{

	}

	EGLocCompiler( eg_cpstr strFile )
	: m_MemFile( eg_file_data_init_t::HasOwnMemory )
	, m_List()
	, m_Lang( eg_loc_lang::UNK )
	{
		LoadLocFile( strFile );
		CompileLocTexts();
	}

	void InitAsEnUS();
	void LoadLocFile( eg_cpstr strFile );
	void LoadLocFile( const EGFileData& File , eg_cpstr NameRef );
	void AddOrReplaceEntry( eg_cpstr Key , const eg_loc_char* Text );
	void SaveXml( EGFileData& Out ) const;

	void CompileLocTexts();
	
	const EGFileData& GetOutput(){ return m_MemFile; }

	static eg_loc_lang StringToLocLang( eg_cpstr Str );
	static eg_cpstr LocLangToString( eg_loc_lang Lang );

private:

	void Localize_ByLang_ENOffset( eg_cpstr InText , EGArray<eg_loc_char>* Out );
	eg_bool Localize_ByLang( const eg_string_base& In , LocData* Out );

private:

	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& Getter) override final;
	virtual eg_cpstr XMLObjName()const override final { return ("EGLocalizeCompiler"); }

private:

	eg_loc_lang      m_Lang;
	EGFileData       m_MemFile;
	EGLocTable       m_List;
};
