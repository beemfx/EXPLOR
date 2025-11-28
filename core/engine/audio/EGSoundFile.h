// (c) 2015 Beem Software

#pragma once

#include "EGLoader_Loadable.h"
#include "EGDataAsset.h"
#include "EGAssetPath.h"
#include "EGEngineConfig.h"
#include "EGSoundFile.reflection.h"

egreflect enum class eg_sound_class_t
{
	Unknown, 
	Effect,
	Ambience,
	Dialog, 
	MusicTrack,
};

egreflect enum class eg_sound_inst_t
{
	//This is ridiculous, in order seems to work fine. Probably should just
	//prevent sounds that are too far away from starting up. Especially
	//for effects.
	Unknown,
	Closest,
	Progressive,
};

struct egSoundDef
{
	static const eg_uint MAX_INSTANCES = 10; //maximum number of voices to be used for an effect.

	eg_string_crc    CrcId;
	eg_sound_class_t Type;
	eg_sound_class_t VolumeOverrideType;
	eg_real          Volume;
	eg_real          Range;
	eg_real          RangeFalloffPct;
	eg_sound_inst_t  InstType;
	eg_uint          Instances;
	eg_path          SoundFile;
	eg_bool          bLooping:1;
	eg_bool          bStream:1;
	eg_bool          bLoaded:1;
	eg_bool          bCentered:1;

	egSoundDef();
	void Clear();
};

class EGSoundCollection
{
public:

	class IReceiver
	{
	public:
		virtual void InsertSound( const egSoundDef& Def ) = 0;
	};

public:

	EGSoundCollection( eg_cpstr Filename , IReceiver* Receiver );

private:

	IReceiver*const     m_Receiver;
};

///////////////////////////////////////////////////////////////////////////////

egreflect struct egSndFileData
{
	egprop eg_d_string      SoundId          = "";
	egprop eg_sound_class_t Type             = eg_sound_class_t::Effect;
	egprop eg_sound_class_t VolumeOverrideType       = eg_sound_class_t::Unknown;
	egprop eg_real          Volume           = 1.f;
	egprop eg_real          AudibleDistance  = 5.f;
	egprop eg_real          AudibleFallofPercent = .5f;
	egprop eg_sound_inst_t  InstanceType     = eg_sound_inst_t::Closest;
	egprop eg_int           MaxInstances     = 1;
	egprop eg_asset_path    SoundFile        = EXT_SND;
	egprop eg_bool          bLooping         = false;
	egprop eg_bool          bStreaming       = false;
	egprop eg_bool          bCentered        = false;
 
	void operator = ( const egSoundDef& rhs );
	operator egSoundDef() const;
};

egreflect class EGSndAsset : public egprop EGDataAsset
{
	EG_CLASS_BODY( EGSndAsset , EGDataAsset )
	EG_FRIEND_RFL( EGSndAsset )

private:

	egprop EGArray<egSndFileData> m_Sounds;

public:

	void InsertSound( const egSndFileData& NewSoundData )
	{
		m_Sounds.Append( NewSoundData );
	}

	void ProcessSounds( EGSoundCollection::IReceiver* Receiver )
	{
		for( const egSndFileData& SoundData : m_Sounds )
		{
			Receiver->InsertSound( SoundData );
		}
	}

private:

	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;
};

/******************************************************************************
	EGAudioFile - Always loaded (sort of). Contains a filename for an audio
	file, and can be loaded on demand, and is reference counted.

	(c) 2015 Beem Software
******************************************************************************/

class EGAudioFileData : public ILoadable
{
public:

	enum class eg_loaded_s
	{
		NOT_LOADED,
		LOADING,
		LOADED,
	};

public:

	const eg_string Filename;
	eg_loaded_s     State;
	void*           Buffer;
	eg_size_t       BufferSize;
	eg_uint         RefCount;

	EGAudioFileData( eg_cpstr _Filename ): Filename(_Filename) , State(eg_loaded_s::NOT_LOADED) , Buffer(nullptr) , BufferSize(0) , RefCount(0){ }
	~EGAudioFileData(){ assert( 0 == RefCount ); }

	void AddRef();
	eg_int Release();

private:

	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size) override;
	virtual void OnLoadComplete(eg_cpstr strFile) override;
};