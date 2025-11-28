// (c) 2015 Beem Media

#include "EGAudioList.h"

eg_size_t EGAudioList::MaxBytesUsed = 0;

void EGAudioList::InitAudioList( void* Mem , eg_size_t MemSize , eg_uint AssetState )
{
	m_AssetState = AssetState;
	m_OutOfMem = false;
	m_Heap.Init( Mem , MemSize , EG_ALIGNMENT );
	List.Clear();
}

void EGAudioList::DeinitAudioList( void )
{
	//const eg_size_t BytesUsed = Heap.GetInfo( IMemAlloc::INFO_ALLOC_MEM );
	//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( ( "Frame buffer: %u bytes" ) , BytesUsed ) );
	
	//Technically the display list is just a chunk of memory with no important
	//data and no destructors need to be run or anything, but freeing up the
	//data makes the allocator happy. It'll be more efficient to be able to
	//deinit the allocator without it asserting. Or have some kind of FreeAll
	//functionality.

	MaxBytesUsed = EG_Max(m_Heap.GetInfo( IMemAlloc::INFO_ALLOC_MEM ) , MaxBytesUsed );

	#if 0//defined( __DEBUG__ )
	//In debug mode we free everything
	while( List.Len() > 0 )
	{
		egCmd* Item = List.GetFirst();
		List.Remove( Item );

		switch( Item->Cmd )
		{
			case SET_WORLD_TF:
			case SET_VIEW_TF:
			case SET_PROJ_TF:
			case DLFUNC_SetBoneMats:
				Free( Item->Parm1.MatrixList );
				break;
			case DLFUNC_DrawRawTris:
				Free( Item->Parm1.VertexList );
				break;
			case SET_VIEW_CLIP:
				Free( Item->Parm1.ViewClip );
				break;
			case SET_LIGHT:
				Free( Item->Parm2.Light );
				break;
			default:
				break;
		}

		Free( Item );
	}
	assert( 0 == List.Len() );
	List.Clear();
	#else
	List.Clear();
	m_Heap.FreeAll();
	#endif
	
	m_Heap.Deinit();
	m_OutOfMem = false;
}

void EGAudioList::PlaySound(egs_sound sound)
{
	egCmd Cmd;
	Cmd.Cmd = eg_audio_list_cmd::PLAY_SOUND;
	Cmd.Data.PlaySound.Sound = sound;
	InsertSimpleCommand( Cmd );
}

void EGAudioList::PlaySoundAt(egs_sound sound, const eg_vec4* pv3Pos)
{
	egCmd Cmd;
	Cmd.Cmd = eg_audio_list_cmd::PLAY_SOUND_3D;
	Cmd.Data.PlaySound3D.Sound = sound;
	Cmd.Data.PlaySound3D.Position = *pv3Pos;
	InsertSimpleCommand( Cmd );
}

void EGAudioList::StopSound( egs_sound Sound )
{
	egCmd Cmd;
	Cmd.Cmd = eg_audio_list_cmd::STOP_SOUND;
	Cmd.Data.PlaySound.Sound = Sound;
	InsertSimpleCommand( Cmd );
}

void EGAudioList::UpdateAmbientSound( egs_sound Sound , const eg_vec3& Pos , eg_bool bIsPlaying )
{
	egCmd Cmd;
	Cmd.Cmd = eg_audio_list_cmd::UPDATE_AMBIENT_SOUND;
	Cmd.Data.PlaySound3D.Sound = Sound;
	Cmd.Data.PlaySound3D.Position = eg_vec4(Pos,bIsPlaying?1.f:0.f);
	InsertSimpleCommand( Cmd );
}

void EGAudioList::SetVolume( egs_sound Sound, eg_real Volume )
{
	egCmd Cmd;
	Cmd.Cmd = eg_audio_list_cmd::SET_VOLUME;
	Cmd.Data.SetVolume.Sound = Sound;
	Cmd.Data.SetVolume.Volume = Volume;
	InsertSimpleCommand( Cmd );
}

void EGAudioList::SetListener(const egAudioListenerInfo* pLisInfo)
{
	egCmd Cmd;
	Cmd.Cmd = eg_audio_list_cmd::SET_LISTENER;
	Cmd.Data.SetListener.vPos = pLisInfo->vPos;
	Cmd.Data.SetListener.vAt = pLisInfo->vAt;
	Cmd.Data.SetListener.vUp = pLisInfo->vUp;
	Cmd.Data.SetListener.vVel = pLisInfo->vVel;
	InsertSimpleCommand( Cmd );
}


void EGAudioList::InsertSimpleCommand( const egCmd& Cmd )
{
	void* ItemMem = Alloc<void*>( sizeof(egCmd) );
	if( nullptr == ItemMem )return;

	egCmd* NewItem = new( ItemMem ) egCmd;
	NewItem->Cmd  = Cmd.Cmd;
	NewItem->Data = Cmd.Data;
	List.InsertLast( NewItem );
}

void EGAudioList::Append( EGAudioList* Src )
{
	for( EGAudioList::egCmd* Cmd : Src->List )
	{
		switch( Cmd->Cmd )
		{
			case eg_audio_list_cmd::SET_LISTENER:
			case eg_audio_list_cmd::STOP_SOUND:
			case eg_audio_list_cmd::PLAY_SOUND:
			case eg_audio_list_cmd::UPDATE_AMBIENT_SOUND:
			case eg_audio_list_cmd::PLAY_SOUND_3D:
			case eg_audio_list_cmd::SET_VOLUME:
			{
				//These are okay to copy directly since they don't need any memory allocated or anything.
				InsertSimpleCommand( *Cmd );
			} break;
		}
	}
}