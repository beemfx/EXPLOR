// (c) 2016 Beem Media. All Rights Reserved.

#pragma once

#include "ExGameTypes.h"
#include "EGTimer.h"
#include "EGLocText.h"

class EGClient;
class ExGame;

class ExSaveMgr
{
private:

	enum class ex_save_status
	{
		DoesNotExist,
		Exists,
		WasDeleted,
		IsCorrupted,
	};

	struct exSlotData
	{
		ex_save_name SaveName = { '\0' };
		EGTimer::egSystemTime SaveTime;
		eg_bool bUsed = false;
		eg_bool bCorrupted = false;
		eg_bool bDeleted = false;
	};
	
private:
	
	static ex_save_name s_NextSaveName;

public:

	static eg_d_string GetNextSaveName(); // This is hackery since it passed data from the client to the server... but... We know it's single player so it works...

private:

	exSlotData m_SaveSlots[EX_SAVE_SLOTS];

public:
	static ExSaveMgr& Get(){ return s_Instance; }

	void Init( EGClient* Client ); // Must be called after profile data is initialized.
	void Deinit( EGClient* Client );

	eg_bool     DoesSaveExist( EGClient* Client , eg_uint Slot );
	eg_bool     IsSaveCorrupted( EGClient* Client , eg_uint Slot );
	void        GetSortedAvailableSaveSlots( EGClient* Client , EGArray<eg_uint>& Out );
	eg_bool     DoAnySavesExist( EGClient* Client );
	eg_uint     FindFreeSaveSlot( EGClient* Client );
	void        DeleteSave( EGClient* Client , eg_uint Slot );
	eg_cplocstr GetSaveName( EGClient* Client , eg_uint Slot );
	EGTimer::egSystemTime GetSaveTime( EGClient* Client , eg_uint Slot );
	eg_loc_text GetFormattedSaveName( EGClient* Client , eg_uint Slot );
	eg_loc_text GetFormattedSaveDateText( EGClient* Client , eg_uint Slot );
	void        CreateSave( EGClient* Client, const exPackedCreateGameInfo& CreateInfo , eg_cplocstr SaveName );
	void        LoadSave( EGClient* Client , eg_uint Slot );
	void        SaveSave( ExGame* Server , eg_uint Slot );

	static const eg_s_string_sml8 GetSaveSlotFilename( eg_uint Slot );

private:

	ex_save_status GetSaveStatus( eg_cpstr Filename , eg_d_string16& SaveNameOut , EGTimer::egSystemTime& SaveTimeOut );

private:

	static ExSaveMgr s_Instance;
};