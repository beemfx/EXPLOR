// (c) 2016 Beem Media

#pragma once

class EGSaveGame
{
public:

	enum class eg_map_object_t
	{
		Unknown,
		Map,
		Terrain,
	};
	
	struct egHeader
	{
		eg_uint32     MapTimeMs;
		eg_bool       bSpawnMapsEnts:1;

		egHeader(): egHeader( CT_Clear ){ }

		egHeader( eg_ctor_t Ct )
		{
			if( Ct == CT_Clear || Ct == CT_Default )
			{
				zero( this );
			}
		}
	};

	struct egMapObjectInfo
	{
		eg_map_object_t Type;
		eg_path8        Filename;
		eg_transform    Pose;

		egMapObjectInfo(): egMapObjectInfo( CT_Clear ){ }

		egMapObjectInfo( eg_ctor_t Ct )
		{
			if( Ct == CT_Default  || Ct == CT_Clear)
			{
				Type = eg_map_object_t::Unknown;
				Pose = Ct;
				EGString_Copy( Filename , "" , countof(Filename) );
			}
			else
			{
				assert( false );
			}
		}
	};

	struct egEntInfo
	{
		struct egEntData
		{
			eg_ent_id       EntId;
			eg_string_crc   DefCrcId;
			eg_d_string     InitString;
			eg_transform    Pose;
			eg_uint         BinaryDataSize;
			eg_bool         bHasPose:1;
		};

		egEntData        EntData;
		EGArray<eg_byte> BinaryData;
		EGArray<eg_byte> AdditionalData;

		egEntInfo(): egEntInfo( CT_Clear ){ }

		egEntInfo( eg_ctor_t Ct )
		{
			if( Ct == CT_Default )
			{
				Reset();
			}
			else if( Ct == CT_Clear )
			{
				zero( &EntData );
				BinaryData.Clear();
			}
			else
			{
				assert( false );
			}
		}

		void Reset()
		{
			zero( &EntData );
			EntData.Pose = CT_Default;
			BinaryData.Clear();
		}
	};

	struct egGlobalDataInfo
	{
		EGArray<eg_byte> BinaryData;
		EGArray<eg_byte> AdditionalData;

		egGlobalDataInfo(): egGlobalDataInfo( CT_Clear ){ }

		egGlobalDataInfo( eg_ctor_t Ct )
		{
			if( Ct == CT_Clear || Ct == CT_Default )
			{
				BinaryData.Clear();
			}
			else
			{
				assert( false );
			}
		}
	};

private:
	static const eg_uint32 BINARY_FILE_ID = 0x56415345; /*"ESAV"*/
	static const eg_uint32 CHUNK_HEADER_ID = 0x4B4E4345; /*"ECNK"*/

	enum class eg_save_chunk_t : eg_uint32
	{
		UNK,
		HEADER,
		ENTITY,
		MAP_OBJECT,
		GLOBAL_DATA,
	};

	struct egChunkInfo
	{
		eg_uint32       Id;
		eg_save_chunk_t Type;
		eg_uint32       DataSize;
		eg_uint32       AdditionalSize;
	};

public:

	EGArray<egHeader>         m_Headers;
	EGArray<egMapObjectInfo>  m_MapObjects;
	EGArray<egEntInfo>        m_EntInfos;
	EGArray<egGlobalDataInfo> m_GlobalDataInfos;

private:

	// For XML parsing
	eg_size_t m_ReadEntityIndex;
	eg_bool   m_bNextDataIsBase64:1;
	eg_bool   m_bNextPoseIsTransform:1;

public:

	void Load( class EGFileData& In, eg_cpstr RefName );

	void Clear();
	void AddHeader( const egHeader& Header );
	void AddEntity( const egEntInfo& EntInfo );
	void AddMapObjectInfo( const egMapObjectInfo& MapObjectInfo );
	void AddGlobalData( const egGlobalDataInfo& GlobalDataInfo );
	void SaveBinary( class EGFileData& Out ) const;

private:
	
	void Load_Binary( class EGFileData& In, eg_cpstr RefName );
};