// (c) 2015 Beem Media

#include "EGAudioFileMgr.h"
#include "EGEngine.h"
#include "EGLoader.h"
#include "EGAlwaysLoaded.h"
#include "EGCrcDb.h"

static const eg_char EGSOUNDMGR_SOUNDDEF_EXT[] = "egsnd";

egSoundDef EGAudioFileMgr::DEFAULT_SOUND;

void EGAudioFileMgr::Init()
{
	EGAlwaysLoadedFilenameList EnumList;

	AlwaysLoaded_GetFileList( &EnumList , EGSOUNDMGR_SOUNDDEF_EXT );
	//First we load all definitions into a scratch mem list
	EGTempSoundList SoundDefs( nullptr );
	m_TempSoundsList = &SoundDefs;
	for( egAlwaysLoadedFilename* FnItem : EnumList )
	{
		InsertEsoundFile( FnItem->Filename );
	}
	m_TempSoundsList = nullptr;

	// Then we copy them to a system mem list.
	m_SoundDefMap.Init( SoundDefs.Len() );
	m_SoundDefMap.SetNotFoundItem( &DEFAULT_SOUND );
	for( eg_uint i=0; i<SoundDefs.Len(); i++ )
	{
		m_SoundDefMap.Insert( SoundDefs.GetKeyByIndex( i ) , SoundDefs.GetByIndex( i ) );
	}

	AlwaysLoaded_GetFileList( &EnumList , "ogg" );
	m_AudioFileMap.Init( EnumList.Len() );
	for( egAlwaysLoadedFilename* FnItem : EnumList )
	{
		InsertOggFile( FnItem->Filename );
	}

	EGLogf( eg_log_t::Audio , "Sound Manager loaded %u sound definitions (%u bytes)" , m_SoundDefMap.Len() , m_SoundDefMap.Len()*sizeof(egSoundDef) );
	EGLogf( eg_log_t::Audio , "Sound Manager found %u ogg definitions (%u bytes)" , m_AudioFileMap.Len() , m_AudioFileMap.Len()*sizeof(EGAudioFileData) );
}

void EGAudioFileMgr::Deinit()
{
	for( eg_uint i=0; i<m_SoundDefMap.Len(); i++ )
	{
		delete m_SoundDefMap.GetByIndex( i );
	}
	m_SoundDefMap.Clear();

	for( eg_uint i=0; i<m_AudioFileMap.Len(); i++ )
	{
		EGAudioFileData* Buffer = m_AudioFileMap.GetByIndex( i );
		assert( Buffer->RefCount == 0 ); //Stray references?
		delete Buffer;
	}
	m_AudioFileMap.Clear();

	m_SoundDefMap.Deinit();
	m_AudioFileMap.Deinit();
}

void EGAudioFileMgr::Update( eg_real DeltaTime )
{
	unused( DeltaTime );

	MainLoader->ProcessCompletedLoads( EGLoader::LOAD_THREAD_SOUND );
}

void EGAudioFileMgr::QueryAllSounds( EGArray<eg_string_small>& SoundsOut ) const
{
	for( eg_size_t i=0; i<m_SoundDefMap.Len(); i++ )
	{
		const egSoundDef* Def = m_SoundDefMap.GetByIndex( i );
		if( Def )
		{
			SoundsOut.Append( EGCrcDb::CrcToString( Def->CrcId ) );
		}
	}
}

const egSoundDef* EGAudioFileMgr::GetSoundDef( eg_cpstr Filename )
{
	eg_string_crc CrcId = FilenameToCrcId( Filename , 0 , true );
	assert( m_SoundDefMap.Contains( CrcId ) ); //This resource doesn't exist.
	return m_SoundDefMap.Get( CrcId );
}

class EGAudioFileData* EGAudioFileMgr::GetAudioFile( eg_cpstr Filename )
{
	eg_string_crc CrcId = FilenameToCrcId( Filename , 0 , false );
	assert( CrcId.IsNull() || m_AudioFileMap.Contains( CrcId ) ); //This resource doesn't exist.
	return CrcId.IsNull() ? nullptr : m_AudioFileMap.Get( CrcId );
}

void EGAudioFileMgr::InsertEsoundFile( eg_cpstr Filename )
{
	EGSoundCollection( Filename , this );
}

void EGAudioFileMgr::InsertSound( const egSoundDef& Def )
{
	eg_bool Loaded = Def.bLoaded;
	if( !Loaded )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Sound %s, was not loaded." , Def.SoundFile );
		return;
	}
	egSoundDef* NewDef = new ( eg_mem_pool::System ) egSoundDef;
	if( nullptr == NewDef )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Couldn't load definition of sound %s, out of memory." , Def.SoundFile );
		assert( false ); //Out of memory?
		return;
	}

	*NewDef = Def;
	// assert( eg_string(NewDef->SoundFile).Len() > 0 ); //No sound in this definition?

	m_TempSoundsList->Insert( Def.CrcId , NewDef );
}

void EGAudioFileMgr::InsertOggFile( eg_cpstr Filename )
{
	EGAudioFileData* NewRsb = new ( eg_mem_pool::System ) EGAudioFileData( Filename );
	if( nullptr != NewRsb )
	{
		eg_string_crc CrcId = FilenameToCrcId( Filename , 0 , false );
		m_AudioFileMap.Insert( CrcId , NewRsb );
	}
}

eg_string_crc EGAudioFileMgr::FilenameToCrcId( eg_cpstr Filename , eg_uint ClampSize , eg_bool bAddToDatabase )
{
	eg_string String = Filename;
	String.ConvertToLower();
	if( ClampSize > 0 )String.ClampEnd( ClampSize );
	eg_string_crc Crc = bAddToDatabase ? EGCrcDb::StringToCrc(String) : eg_string_crc(String);
	return Crc;
}