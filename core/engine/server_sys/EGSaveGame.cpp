// (c) 2016 Beem Media

#include "EGSaveGame.h"
#include "EGFileData.h"

void EGSaveGame::Load( class EGFileData& In, eg_cpstr RefName )
{
	Clear();

	if( In.GetData() )
	{
		const eg_uint32* Id = In.GetDataAs<eg_uint32>();

		if( *Id == BINARY_FILE_ID )
		{
			Load_Binary( In, RefName );	
		}
	}
}

void EGSaveGame::Clear()
{
	m_ReadEntityIndex = 0;
	m_bNextDataIsBase64 = false;
	m_bNextPoseIsTransform = false;

	m_Headers.Clear();
	m_EntInfos.Clear();
	m_GlobalDataInfos.Clear();
}

void EGSaveGame::AddHeader( const egHeader& Header )
{
	m_Headers.Append( Header );
}

void EGSaveGame::AddEntity( const egEntInfo& EntInfo )
{
	assert( EntInfo.EntData.BinaryDataSize == EntInfo.BinaryData.Len() );
	m_EntInfos.Append( EntInfo );
}

void EGSaveGame::AddMapObjectInfo( const egMapObjectInfo& MapObjectInfo )
{
	m_MapObjects.Append( MapObjectInfo );
}

void EGSaveGame::AddGlobalData( const egGlobalDataInfo& GlobalDataInfo )
{
	m_GlobalDataInfos.Append( GlobalDataInfo );
}

void EGSaveGame::SaveBinary( class EGFileData& Out ) const
{
	// File ID
	Out.Write( &BINARY_FILE_ID , sizeof(BINARY_FILE_ID) );

	egChunkInfo ChunkHeader;
	ChunkHeader.Id = CHUNK_HEADER_ID;

	// Header:
	for( const egHeader& Header : m_Headers )
	{
		ChunkHeader.Type = eg_save_chunk_t::HEADER;
		ChunkHeader.DataSize = sizeof(Header);
		ChunkHeader.AdditionalSize = 0;
		Out.Write( &ChunkHeader , sizeof(ChunkHeader) );
		Out.Write( &Header , sizeof(Header) );
	}

	// Map Objects:
	for( const egMapObjectInfo& ObjInfo : m_MapObjects )
	{
		ChunkHeader.Type = eg_save_chunk_t::MAP_OBJECT;
		ChunkHeader.DataSize = sizeof(ObjInfo);
		ChunkHeader.AdditionalSize = 0;
		Out.Write( &ChunkHeader , sizeof(ChunkHeader) );
		Out.Write( &ObjInfo , sizeof(ObjInfo) );
	}

	// Entities:
	for( const egEntInfo& EntInfo : m_EntInfos )
	{
		ChunkHeader.Type = eg_save_chunk_t::ENTITY;
		ChunkHeader.DataSize = sizeof(EntInfo);
		ChunkHeader.AdditionalSize = EntInfo.AdditionalData.LenAs<eg_uint32>();
		Out.Write( &ChunkHeader , sizeof(ChunkHeader) );
		Out.Write( &EntInfo.EntData , sizeof(EntInfo.EntData) );
		assert( EntInfo.BinaryData.Len() == EntInfo.EntData.BinaryDataSize );
		Out.Write( EntInfo.BinaryData.GetArray() , EntInfo.BinaryData.Len() );
		Out.Write( EntInfo.AdditionalData.GetArray() , EntInfo.AdditionalData.Len() );
	}

	// Global Data:
	for( const egGlobalDataInfo& DataInfo : m_GlobalDataInfos )
	{
		ChunkHeader.Type = eg_save_chunk_t::GLOBAL_DATA;
		ChunkHeader.DataSize = DataInfo.BinaryData.LenAs<eg_uint32>();
		ChunkHeader.AdditionalSize = DataInfo.AdditionalData.LenAs<eg_uint32>();
		Out.Write( &ChunkHeader , sizeof(ChunkHeader) );
		Out.Write( DataInfo.BinaryData.GetArray() , ChunkHeader.DataSize );
		Out.Write( DataInfo.AdditionalData.GetArray() , ChunkHeader.AdditionalSize );
	}
}

void EGSaveGame::Load_Binary( class EGFileData& In, eg_cpstr RefName )
{
	In.Seek( eg_file_data_seek_t::Begin , 0 );
	eg_uint32 FileId = 0;
	In.Read( &FileId , sizeof(FileId) );

	if( FileId != BINARY_FILE_ID )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": %s is not a valid save file." , RefName );
		assert( false );
		return;
	}

	while( In.Tell() < In.GetSize() )
	{
		egChunkInfo ChunkInfo;
		eg_size_t ChunkSizeRead = In.Read( &ChunkInfo , sizeof(ChunkInfo) );
		if( ChunkSizeRead == sizeof(ChunkInfo) && ChunkInfo.Id == CHUNK_HEADER_ID )
		{
			switch( ChunkInfo.Type )
			{
				case eg_save_chunk_t::UNK:
				{
					EGLogf( eg_log_t::Error , __FUNCTION__ ": %s had an unknown chunk." , RefName );
					assert( false );
					return;
				} break;

				case eg_save_chunk_t::HEADER:
				{
					if( sizeof(egHeader) == ChunkInfo.DataSize && ChunkInfo.AdditionalSize == 0 )
					{
						egHeader ReadInfo( CT_Clear );
						In.Read( &ReadInfo , sizeof(ReadInfo) );
						m_Headers.Append( ReadInfo );
					}
					else
					{
						EGLogf( eg_log_t::Error , __FUNCTION__ ": %s had a header of the wrong size." , RefName );
						assert( false );
						return;
					}
				} break;

				case eg_save_chunk_t::MAP_OBJECT:
				{
					if( sizeof(egMapObjectInfo) == ChunkInfo.DataSize && ChunkInfo.AdditionalSize == 0 )
					{
						egMapObjectInfo ReadInfo( CT_Clear );
						In.Read( &ReadInfo , sizeof(ReadInfo) );
						m_MapObjects.Append( ReadInfo );
					}
					else
					{
						EGLogf( eg_log_t::Error , __FUNCTION__ ": %s had a map info of the wrong size." , RefName );
						assert( false );
						return;
					}
				} break;

				case eg_save_chunk_t::ENTITY:
				{
					if( sizeof(egEntInfo) == ChunkInfo.DataSize )
					{
						egEntInfo ReadInfo( CT_Clear );
						In.Read( &ReadInfo.EntData , sizeof(ReadInfo.EntData) );
						assert( ReadInfo.EntData.BinaryDataSize < 1024*1024*1 ); // If an ent info is > 1 meg, there is probably something wrong.
						if( ReadInfo.EntData.BinaryDataSize <= 1024*1024*1 )
						{
							ReadInfo.BinaryData.Resize( ReadInfo.EntData.BinaryDataSize );
							In.Read( ReadInfo.BinaryData.GetArray() , ReadInfo.BinaryData.Len() );
						}

						assert( ChunkInfo.AdditionalSize <= 1024*1024*1 ); // If an ent info is > 1 meg, there is probably something wrong.
						if( ChunkInfo.AdditionalSize <= 1024*1024*1 )
						{
							ReadInfo.AdditionalData.Resize( ChunkInfo.AdditionalSize );
							In.Read( ReadInfo.AdditionalData.GetArray() , ReadInfo.AdditionalData.Len() );
						}
						m_EntInfos.Append( ReadInfo );
					}
					else
					{
						EGLogf( eg_log_t::Error , __FUNCTION__ ": %s had an entity info of the wrong size." , RefName );
						assert( false );
						return;
					}
				} break;

				case eg_save_chunk_t::GLOBAL_DATA:
				{
					egGlobalDataInfo ReadInfo( CT_Clear );
					ReadInfo.BinaryData.Resize( ChunkInfo.DataSize );
					ReadInfo.AdditionalData.Resize( ChunkInfo.AdditionalSize );
					In.Read( ReadInfo.BinaryData.GetArray() , ReadInfo.BinaryData.Len() );
					In.Read( ReadInfo.AdditionalData.GetArray() , ReadInfo.AdditionalData.Len() );
					m_GlobalDataInfos.Append( ReadInfo );
				} break;
			}
		}
		else
		{
			EGLogf( eg_log_t::Error , __FUNCTION__ ": %s had a bad chunk." , RefName );
			assert( false );
			return; // Bail out on bad chunk, we probably won't find anything good from here.
		}
	}
}
