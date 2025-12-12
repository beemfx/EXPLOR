// (c) 2020 Beem Media. All Rights Reserved.

#include "ExNewGameFlowMenu.h"
#include "ExFullScreenLoadingMenu.h"
#include "ExAnimationMenu.h"
#include "EGSoundScape.h"
#include "ExMapInfos.h"

EG_CLASS_DECL( ExNewGameFlowMenu )

void ExNewGameFlowMenu::RunFromState( ex_starting_s NewState )
{
	m_State = ex_new_game_s::None;

	// Decide "starting state" so that AdvanceToNextState knows where to go.
	switch( NewState )
	{
		case ex_starting_s::CreateGame:
			m_State = ex_new_game_s::None;
			if( m_FadeWidget )
			{
				m_FadeWidget->RunEvent( eg_crc("FadeClearToBlack") );
			}
			break;
		case ex_starting_s::PostGameCreation:
			m_State = ex_new_game_s::PostSaveCreated;
			if( m_FadeWidget )
			{
				m_FadeWidget->RunEvent( eg_crc("SetToBlack") );
			}
			break;
		default:
			assert( false ); // Cannot start from these states.
			return;
	}

	AdvanceToNextState();
}

void ExNewGameFlowMenu::HandleCancelFromNewGame()
{
	m_bCancelledNewGame = true;
}

void ExNewGameFlowMenu::OnInit()
{
	Super::OnInit();

	m_FadeWidget = GetWidget<EGUiWidget>( eg_crc("FadeWidget") );

	HideHUD();
}

void ExNewGameFlowMenu::OnDeinit()
{
	ShowHUD( false );

	Super::OnDeinit();
}

void ExNewGameFlowMenu::OnActivate()
{
	Super::OnActivate();

	AdvanceToNextState();
}

void ExNewGameFlowMenu::AdvanceToNextState()
{
	switch( m_State )
	{
	case ex_new_game_s::NotInited:
		break;
	case ex_new_game_s::None:
		m_State = ex_new_game_s::CreateSaveGame;
		break;
	case ex_new_game_s::CreateSaveGame:
		if( m_bCancelledNewGame )
		{
			MenuStack_Pop();
			return;
		}
		else
		{
			// m_State = ex_new_game_s::LoadIntoWorld;

			// We will do some trickery here, we're actually going clear the entire
			// menu stack (including this menu, and push a new instance of this menu
			// onto the stack so that it is the only menu on the stack (getting
			// rid of the main menu)
			MenuStack_Clear();
			ExNewGameFlowMenu* NewNewGameFlowMenu = EGCast<ExNewGameFlowMenu>( MenuStack_PushTo( eg_crc("NewGameFlowMenu") ) );
			if( NewNewGameFlowMenu )
			{
				NewNewGameFlowMenu->RunFromState( ex_starting_s::PostGameCreation );
			}
			return;
		}
		break;
	case ex_new_game_s::PostSaveCreated:
		m_State = ex_new_game_s::LoadIntoWorld;
		break;
	case ex_new_game_s::LoadIntoWorld:
		m_State = ex_new_game_s::IntroMovie;
		break;
	case ex_new_game_s::IntroMovie:
	{
		m_State = ex_new_game_s::CameraSplineMovie;
	} break;
	case ex_new_game_s::CameraSplineMovie:
	{
		m_State = ex_new_game_s::CreateParty;
	} break;
	case ex_new_game_s::LoadingGame:
		m_State = ex_new_game_s::CreateParty;
		break;
	case ex_new_game_s::CreateParty:
		m_State = ex_new_game_s::Done;
		break;
	case ex_new_game_s::Done:
		break;
	}

	switch( m_State )
	{
	case ex_new_game_s::NotInited:
	case ex_new_game_s::None:
	case ex_new_game_s::PostSaveCreated:
		break;
	case ex_new_game_s::CreateSaveGame:
		MenuStack_PushTo( eg_crc("GameCreateMenu") );
		break;
	case ex_new_game_s::IntroMovie:
		ExAnimationMenu::PlayMovie( GetClientOwner() , eg_crc("Intro") );
		if( m_FadeWidget )
		{
			m_FadeWidget->RunEvent( eg_crc("FadeBlackToClear") );
		}
		break;
	case ex_new_game_s::CameraSplineMovie:
		ExAnimationMenu::PlayMovie( GetClientOwner() , eg_crc("IntroCameraSpline") );
		break;
	case  ex_new_game_s::LoadIntoWorld:
	case ex_new_game_s::LoadingGame:
	{
		ExFullScreenLoadingMenu* LoadingMenu = EGCast<ExFullScreenLoadingMenu>(MenuStack_PushTo( eg_crc("FullScreenLoadingMenu") ));
		if( LoadingMenu )
		{
			LoadingMenu->SetDontSwitchMusic( true );
		}
	} break;
	case ex_new_game_s::CreateParty:
	{
		MenuStack_PushTo( eg_crc("PartyCreateMenu") );
		ExFullScreenLoadingMenu* LoadingMenu = EGCast<ExFullScreenLoadingMenu>(MenuStack_PushTo( eg_crc("FullScreenLoadingMenu") ));
		if( LoadingMenu )
		{
			LoadingMenu->SetOnlyFadeOut( true );
			LoadingMenu->SetDontSwitchMusic( true );
		}
		if( m_bBackgroundForCharacterCreate )
		{
			if( m_FadeWidget )
			{
				m_FadeWidget->RunEvent( eg_crc("FadeClearToBlack") );
			}
		}
	} break;
	case ex_new_game_s::Done:
	{
		MenuStack_Clear();
		if( m_bBackgroundForCharacterCreate )
		{
			ExFullScreenLoadingMenu* LoadingMenu = EGCast<ExFullScreenLoadingMenu>(MenuStack_PushTo( eg_crc("FullScreenLoadingMenu") ));
			if( LoadingMenu )
			{
				LoadingMenu->SetOnlyFadeOut( true );
			}
		}
		else
		{
			ExGame* Game = GetGame();
			EGClient* PC = GetPrimaryClient();
			EGSoundScape* SoundScape = PC ? PC->SDK_GetSoundScape() : nullptr;
			if( SoundScape && Game )
			{
				SoundScape->FadeToBgMusic( *Game->GetCurrentMapInfo().MusicTrack.Path );
			}
		}
	} break;
	}
}
