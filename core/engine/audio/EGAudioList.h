// (c) 2015 Beem Media

#pragma once

#include "EGAudioTypes.h"
#include "EGList.h"
#include "EGStaticAlloc.h"

enum class eg_audio_list_cmd
{
	SET_LISTENER,
	PLAY_SOUND,
	PLAY_SOUND_3D,
	UPDATE_AMBIENT_SOUND,
	STOP_SOUND,
	SET_VOLUME,
};

union egAudioListCmdData
{
	struct
	{
		eg_real4 vPos;
		eg_real4 vVel;
		eg_real4 vAt;
		eg_real4 vUp;	
	} SetListener;
	
	struct
	{
		egs_sound Sound;
	} PlaySound;

	struct
	{
		egs_sound Sound;
		eg_real4  Position;
	} PlaySound3D;

	struct
	{
		egs_sound Sound;
	} StopSound;

	struct
	{
		egs_sound Sound;
		eg_real   Volume;
	} SetVolume;

};

class EGAudioList
{
public:
	struct egCmd : public IListable
	{
		eg_audio_list_cmd     Cmd;
		egAudioListCmdData Data;
	};

	EGList<egCmd> List;
public:
	static const eg_uint LIST_ID=0xD9;
	EGAudioList(): List(LIST_ID), m_AssetState(0){ }
	void InitAudioList( void* Mem , eg_size_t MemSize , eg_uint AssetState );
	void DeinitAudioList( void );
	eg_uint GetAssetState()const{ return m_AssetState; }
	eg_bool IsOutOfMem()const{ return m_OutOfMem; }

	void PlaySound(egs_sound sound);
	void PlaySoundAt(egs_sound sound, const eg_vec4* pv3Pos);
	void StopSound( egs_sound Sound );
	void UpdateAmbientSound( egs_sound Sound , const eg_vec3& Pos , eg_bool bIsPlaying );
	void SetVolume( egs_sound Sound , eg_real Volume );
	void SetListener(const egAudioListenerInfo* pLisInfo);

	void Append( EGAudioList* Src );
private:
	void InsertSimpleCommand( const egCmd& Cmd );

	template<class A> A* Alloc( eg_size_t Size ){ A* Out = static_cast<A*>(m_Heap.Alloc( Size , __FUNCTION__ , __FILE__ , __LINE__)); m_OutOfMem = m_OutOfMem || nullptr == Out; return Out; }
	template<class A> void Free( A*& Obj ){ m_Heap.Free( Obj ); Obj = nullptr; }
private:
	eg_uint       m_AssetState;
	EGStaticAlloc m_Heap;
	eg_bool       m_OutOfMem:1; //If we run out of memory make not of it so that we can just skip the draw.
	//Some metrics.
	static eg_size_t MaxBytesUsed;
};
