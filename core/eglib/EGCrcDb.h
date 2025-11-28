// (c) 2016 Beem Media
#pragma once

class EGCrcDb
{
public:
	struct egItem
	{
		eg_string_crc Crc;
		EGArray<eg_string_small> Strings;

		// CTOR
		egItem(): Crc( CT_Clear), Strings(){ }
		egItem( eg_ctor_t Ct ): Crc( Ct ), Strings(){ }

		// Helper
		eg_bool AddString( eg_cpstr Str )
		{
			eg_bool bExists = false;
			for( const eg_string_small Search : Strings )
			{
				if( Search == Str )
				{
					bExists = true;
					break;
				}
			}

			if( !bExists )
			{
				Strings.Append( Str );
			}

			return !bExists;
		}
	};

public:
	static void Init();
	static void Deinit();
	static EGCrcDb* Get();
	static eg_string_crc StringToCrc( eg_cpstr  Str );
	static eg_string_small CrcToString( eg_string_crc Crc );
	static void AddAndSaveIfInTool( eg_cpstr String ){ if( GlobalInstance ){ GlobalInstance->AddString( String, true ); } }

public:
	void AddString( eg_cpstr String, eg_bool bSaveIfNew );
	const egItem& FindString( eg_string_crc Crc );
	void Serialize();

private:
	EGCrcDb();
	~EGCrcDb();

	eg_string_big GetSaveFilename() const;

private:
	egItem          m_DefaultItem;
	EGArray<egItem> m_List;

private:
	static EGCrcDb* GlobalInstance;
};