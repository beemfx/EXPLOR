/******************************************************************************
	EGAudioFileMgr - Manages persistent audio data, namely all sound definitions
	and audio files. It always has the sound definitions. The audio file names
	are stored only (the data is not loaded, the EGAudioFile class is sued to
	manage actually loading the buffers.)

	(c) 2015 Beem Software
******************************************************************************/
#pragma once

#include "EGSoundFile.h"
#include "EGEngineTemplates.h"

class EGAudioFileMgr : private EGSoundCollection::IReceiver
{
public:

	EGAudioFileMgr():m_SoundDefMap(){ }
	void Init();
	void Deinit();
	void Update( eg_real DeltaTime );
	void QueryAllSounds( EGArray<eg_string_small>& SoundsOut ) const;

	const egSoundDef* GetSoundDef( eg_cpstr Filename );
	class EGAudioFileData* GetAudioFile( eg_cpstr Filename );

private:

	EGSysMemItemMap<struct egSoundDef*> m_SoundDefMap;
	EGSysMemItemMap<class EGAudioFileData*> m_AudioFileMap;

private:

	typedef EGItemMap<EGStringCrcMapKey,egSoundDef*> EGTempSoundList;

private:

	EGTempSoundList* m_TempSoundsList;
	void InsertEsoundFile( eg_cpstr Filename );
	void InsertOggFile( eg_cpstr Filename );
	virtual void InsertSound( const egSoundDef& Def ) override;
	static eg_string_crc FilenameToCrcId( eg_cpstr Filename , eg_uint ClampSize , eg_bool bAddToDatabase );
	static egSoundDef DEFAULT_SOUND;
};