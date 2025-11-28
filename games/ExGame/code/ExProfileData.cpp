// (c) 2020 Beem Media. All Rights Reserved.

#include "ExProfileData.h"
#include "EGEngine.h"
#include "EGSettings2Types.h"

static const eg_int EX_PROFILE_ENDING_MOVIE_UNLOCKED_KEY = 0x12344321;
static const eg_int EX_PROFILE_CASTLE_GAME_UNLOCKED_KEY = 0x18744345;

static EGSettingsInt ExProfile_LastSaveSlot( "ExProfile.LastSaveSlot" , CT_Clear , 0 , EGS_F_USER_SAVED );
static EGSettingsInt ExProfile_LastContextMenuChoice( "ExProfile.LastContextMenuChoice" , CT_Clear , 0 , 0 /*EGS_F_USER_SAVED*/ );
static EGSettingsInt ExProfile_EndingMovieUnlocked( "ExProfile.DataOffset" , CT_Clear , 0 , EGS_F_USER_SAVED );
static EGSettingsInt ExProfile_CastleGameUnlocked( "ExProfile.MemoryOffset" , CT_Clear , 0 , EGS_F_USER_SAVED );


void ExProfileData_SetLastSaveSlot( eg_int NewValue )
{
	if( NewValue != ExProfile_LastSaveSlot.GetValueThreadSafe() )
	{
		ExProfile_LastSaveSlot.SetValue( NewValue );
		Engine_QueMsg( "engine.SaveConfig()" );
	}
}

eg_int ExProfileData_GetLastSaveSlot()
{
	return ExProfile_LastSaveSlot.GetValueThreadSafe();
}

void ExProfileData_SetLastContextMenuChoice( eg_int NewValue )
{
	if( ExProfile_LastContextMenuChoice.GetValueThreadSafe() != NewValue )
	{
		ExProfile_LastContextMenuChoice.SetValue( NewValue );
		// Engine_QueMsg( "engine.SaveConfig()" );
	}
}

eg_int ExProfileData_GetLastContextMenuChoice()
{
	return ExProfile_LastContextMenuChoice.GetValueThreadSafe();
}

void ExProfileData_SetEndingMovieUnlocked( eg_bool bNewValue )
{
	const eg_int NewValueEncoded = bNewValue ? EX_PROFILE_ENDING_MOVIE_UNLOCKED_KEY : 0;

	if( NewValueEncoded != ExProfile_EndingMovieUnlocked.GetValueThreadSafe() )
	{
		ExProfile_EndingMovieUnlocked.SetValue( NewValueEncoded );
		Engine_QueMsg( "engine.SaveConfig()" );
	}
}

eg_bool ExProfileData_GetEndingMovieUnlocked()
{
	return ExProfile_EndingMovieUnlocked.GetValueThreadSafe() == EX_PROFILE_ENDING_MOVIE_UNLOCKED_KEY;
}

void ExProfileData_SetCastleGameUnlocked( eg_bool bNewValue )
{
	const eg_int NewValueEncoded = bNewValue ? EX_PROFILE_CASTLE_GAME_UNLOCKED_KEY : 0;

	if( NewValueEncoded != ExProfile_CastleGameUnlocked.GetValueThreadSafe() )
	{
		ExProfile_CastleGameUnlocked.SetValue( NewValueEncoded );
		Engine_QueMsg( "engine.SaveConfig()" );
	}
}

eg_bool ExProfileData_GetCastleGameUnlocked()
{
	return ExProfile_CastleGameUnlocked.GetValueThreadSafe() == EX_PROFILE_CASTLE_GAME_UNLOCKED_KEY;
}
