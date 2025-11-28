/******************************************************************************
File: AudioFile.cpp
Class: IRawAudioFile
Purpose: See header.

(c) 2011 Beem Software
******************************************************************************/

#include "EGAudioFile.h"

EG_CLASS_DECL( EGAudioFile )

EGAudioFile* EGAudioFile::OpenAF( void* Data , eg_size_t DataSize )
{
	//Only ogg vorbis is supported, if other types of files were 
	//supported the file extension could be checked.
	EGClass* AudioFileClass = EGClass::FindClass( "EGOggVorbisFile" );
	EGAudioFile* pFile = nullptr;

	if( AudioFileClass )
	{
		pFile = EGNewObject<EGAudioFile>( AudioFileClass , eg_mem_pool::Audio );
		if( pFile )
		{
			pFile->Open( Data , DataSize );
		}
	}
	
	return pFile;
}

void EGAudioFile::CloseAF( EGAudioFile*& pFile )
{
	EG_SafeRelease( pFile );
}
