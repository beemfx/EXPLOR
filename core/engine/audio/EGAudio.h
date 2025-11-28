// (c) 2011 Beem Media

#pragma once

#include "EGAudioTypes.h"

extern class EGAudioList* MainAudioList;

eg_bool EGAudio_Init( eg_string_crc DriverId );
void EGAudio_Deinit( void );
void EGAudio_BeginFrame( void );
void EGAudio_EndFrame( void );
void EGAudio_AddCB( ISoundCB* Cb );
void EGAudio_RemoveCB( ISoundCB* Cb );
void EGAudio_QueryAllSounds( EGArray<eg_string_small>& SoundsOut );

egs_sound EGAudio_CreateSound( eg_cpstr File );
void EGAudio_DestroySound( egs_sound Sound );
