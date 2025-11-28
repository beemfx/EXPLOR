// (c) 2020 Beem Media. All Rights Reserved.

#pragma once


#include "ExMenu.h"

class ExNewGameFlowMenu : public ExMenu
{
	EG_CLASS_BODY( ExNewGameFlowMenu , ExMenu )

public:

	enum class ex_starting_s
	{
		CreateGame ,
		PostGameCreation ,
	};

private:

	enum class ex_new_game_s
	{
		NotInited,
		None,
		CreateSaveGame,
		PostSaveCreated,
		LoadIntoWorld,
		IntroMovie,
		CameraSplineMovie,
		LoadingGame,
		CreateParty,
		Done,
	};

private:

	EGUiWidget* m_FadeWidget = nullptr;
	ex_new_game_s m_State = ex_new_game_s::NotInited;
	eg_bool m_bCancelledNewGame = false;
	eg_bool m_bBackgroundForCharacterCreate = false;

public:

	void RunFromState( ex_starting_s NewState );
	void HandleCancelFromNewGame();

private:

	virtual void OnInit() override;
	virtual void OnDeinit() override;
	virtual void OnActivate() override;;
	void AdvanceToNextState();
};

