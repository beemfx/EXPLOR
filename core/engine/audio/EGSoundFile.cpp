// (c) 2015 Beem Media

#include "EGSoundFile.h"
#include "EGEngineConfig.h"
#include "EGCrcDb.h"
#include "EGPath2.h"

EG_CLASS_DECL( EGSndAsset )

egSoundDef::egSoundDef()
{
	Clear();
}

void egSoundDef::Clear()
{
	CrcId        = eg_crc("");
	Type         = eg_sound_class_t::Unknown;
	VolumeOverrideType   = eg_sound_class_t::Unknown;
	SoundFile[0] = '\0';
	Volume       = 1.0f;
	Range           = 5.0f;
	RangeFalloffPct = .5f;
	InstType     = eg_sound_inst_t::Unknown;
	Instances    = 1;
	bCentered    = false;
	bLoaded      = false;
	bLooping     = false;
	bStream      = false;
}

EGSoundCollection::EGSoundCollection( eg_cpstr Filename , IReceiver* Receiver )
: m_Receiver( Receiver )
{
	EGSndAsset* SoundAsset = EGDataAsset::LoadDataAsset<EGSndAsset>( EGString_ToWide( Filename ) );
	if( SoundAsset )
	{
		SoundAsset->ProcessSounds( Receiver );
		EGDeleteObject( SoundAsset );
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// EGAudioFile
//
///////////////////////////////////////////////////////////////////////////////
#include "EGEngine.h"
#include "EGLoader.h"

void EGAudioFileData::AddRef()
{
	RefCount++;
	if( 1 == RefCount && Filename.Len() > 0 )
	{
		EGLogf( eg_log_t::AudioActivity , __FUNCTION__" %s" , Filename.String() );
		assert( eg_loaded_s::NOT_LOADED == State );
		State = eg_loaded_s::LOADING;
		MainLoader->BeginLoad( this->Filename , this , EGLoader::LOAD_THREAD_SOUND );
	}
}

eg_int EGAudioFileData::Release()
{
	assert( RefCount > 0 ); if( RefCount > 0 ){ RefCount--; }

	if( 0 == RefCount )
	{
		EGLogf( eg_log_t::AudioActivity , __FUNCTION__" %s" , Filename.String() );

		assert( eg_loaded_s::LOADING == State || eg_loaded_s::LOADED == State );
		if( eg_loaded_s::LOADING == State )
		{
			MainLoader->CancelLoad( this );
		}
		if( Buffer ){ EGMem2_Free( Buffer ); Buffer = nullptr; }
		BufferSize = 0;
		State = eg_loaded_s::NOT_LOADED;
	}

	return RefCount;
}

void EGAudioFileData::DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size)
{
	unused( strFile );

	BufferSize = Size;
	Buffer = EGMem2_Alloc( BufferSize , eg_mem_pool::Audio );
	if( nullptr == Buffer )return;
	EGMem_Copy( Buffer , pMem , BufferSize );
}

void EGAudioFileData::OnLoadComplete(eg_cpstr strFile)
{
	unused( strFile );

	State = nullptr != Buffer ? eg_loaded_s::LOADED : eg_loaded_s::NOT_LOADED;
}

///////////////////////////////////////////////////////////////////////////////

void egSndFileData::operator=( const egSoundDef& rhs )
{
	SoundId = EGCrcDb::CrcToString( rhs.CrcId );
	Type = rhs.Type;
	VolumeOverrideType = rhs.VolumeOverrideType;
	Volume = rhs.Volume;
	AudibleDistance = rhs.Range;
	AudibleFallofPercent = rhs.RangeFalloffPct;
	InstanceType = rhs.InstType;
	MaxInstances = rhs.Instances;
	SoundFile = rhs.SoundFile;
	bLooping = rhs.bLooping;
	bStreaming = rhs.bStream;
	bCentered = rhs.bCentered;
}

egSndFileData::operator egSoundDef() const
{
	egSoundDef Out;

	eg_string SoundIdLower = *SoundId;
	SoundIdLower.ConvertToLower();
	Out.CrcId = EGCrcDb::StringToCrc( SoundIdLower );
	Out.Type = Type;
	Out.VolumeOverrideType = VolumeOverrideType;
	Out.Volume = Volume;
	Out.Range = AudibleDistance;
	Out.RangeFalloffPct = AudibleFallofPercent;
	Out.InstType = InstanceType;
	Out.Instances = MaxInstances;
	EGString_Copy( Out.SoundFile , *SoundFile.FullPath , countof(Out.SoundFile) );
	Out.bLooping = bLooping;
	Out.bStream = bStreaming;
	Out.bLoaded = true;
	Out.bCentered = bCentered;

	return Out;
}

///////////////////////////////////////////////////////////////////////////////

void EGSndAsset::PostLoad( eg_cpstr16 Filename, eg_bool bForEditor , egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor , RflEditor );

	if( !bForEditor )
	{
		for( egSndFileData& SoundData : m_Sounds )
		{
			//If there is no extension in the sound file, append the ogg one.
			egPathParts2 SoundFilePathParts = EGPath2_BreakPath( *SoundData.SoundFile.FullPath );
			if( SoundFilePathParts.Filename.Len() > 0 && SoundFilePathParts.Ext.Len() == 0 )
			{
				SoundFilePathParts.Ext = EXT_SND;
			}
			SoundData.SoundFile.FullPath = *SoundFilePathParts.ToString();
		}
	}
}
