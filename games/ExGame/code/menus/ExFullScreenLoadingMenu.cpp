// (c) 2016 Beem Media. All Rights Reserved.

#include "ExFullScreenLoadingMenu.h"
#include "ExGame.h"
#include "ExLoadingHints.h"
#include "EGRandom.h"
#include "EGSoundScape.h"
#include "ExMapInfos.h"

EG_CLASS_DECL( ExFullScreenLoadingMenu )

static const eg_real EX_FULL_SCREEN_LOADING_MENU_MIN_TIME = .5f;
static const eg_real EX_FULL_SCREEN_LOADING_MENU_MAX_TIME = 30.f;

void ExFullScreenLoadingMenu::OnInit()
{
	Super::OnInit();
	ExClient* EC = EGCast<ExClient>(GetOwnerClient());
	assert( m_Countdown == 0.f ); // Data should be wiped before INIT
	assert( !EC->m_bIsFullScreenLoading );
	EC->m_bIsFullScreenLoading = true;

	if( EC )
	{
		EC->SetLoadingOpacity( 1.f );
	}

	m_Bg = GetWidget( eg_crc( "Background" ) );
	m_Needle = GetWidget( eg_crc( "Needle" ) );
	m_LoadingText = GetWidget<EGUiTextWidget>( eg_crc( "LoadingText" ) );
	m_LoadingHintText = GetWidget<EGUiTextWidget>( eg_crc( "LoadingHintText" ) );

	exLoadingHintInfo LoadingHint = ExLoadingHints::Get().GetRandomHint( m_Rng );
	if( m_LoadingHintText )
	{
		m_LoadingHintText->SetText( CT_Clear , eg_loc_text( LoadingHint.HintText ) );
	}

	m_bHasSwitchedMusic = false;
	m_bHasEverBeenReady = false;
	m_TotalTime = 0.f;
}

void ExFullScreenLoadingMenu::OnDeinit()
{
	ExClient* EC = EGCast<ExClient>(GetOwnerClient());
	if( EC )
	{
		EC->m_bIsFullScreenLoading = false;
		EC->SetLoadingOpacity( 0.f );
	}

	Super::OnDeinit();
}

void ExFullScreenLoadingMenu::OnUpdate( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime , AspectRatio );

	ExClient* EC = EGCast<ExClient>(GetOwnerClient());
	m_Countdown += DeltaTime;
	m_TotalTime += DeltaTime;

	eg_bool bIsReady = IsGameLoadedAndReady( GetOwnerClient() );

	if( m_TotalTime < EX_FULL_SCREEN_LOADING_MENU_MIN_TIME )
	{
		bIsReady = false;
	}
	else if( m_TotalTime > EX_FULL_SCREEN_LOADING_MENU_MAX_TIME )
	{
		// Should never happen but we don't want to be stuck in a loading screen because music kept getting updated or something.
		bIsReady = true;
	}

	if( bIsReady || m_bOnlyFadeOut )
	{
		m_bHasEverBeenReady = true;
	}

	if( m_bHasEverBeenReady )
	{
		bIsReady = true;
	}

	if( !bIsReady )
	{
		m_Countdown = 0.f;
		m_bHasTriggeredLoadComplete = false;
	}
	else
	{
		if( !m_bHasTriggeredLoadComplete )
		{
			m_bHasTriggeredLoadComplete = true;
			GetGame()->HandleLoadingComplete();
		}
	}

	static const eg_real MIN_TIME = 2.f;
	static const eg_real FADE_OUT = 3.f;

	static_assert(FADE_OUT >= MIN_TIME , "Fade out must happen after blackness");

	if( m_bOnlyFadeOut )
	{
		bIsReady = true;
		m_bHasEverBeenReady = true;
	}

	if( m_Bg )
	{
		eg_real Alpha = EG_Clamp( EGMath_GetMappedRangeValue( m_Countdown , eg_vec2( MIN_TIME , FADE_OUT ) , eg_vec2( 1.f , 0.f ) ) , 0.f , 1.f );
		m_Bg->SetPalette( 0 , eg_color( 0.f , 0.f , 0.f , Alpha ).ToVec4() );
		if( EC )
		{
			EC->SetLoadingOpacity( Alpha );
		}
	}

	if( m_Needle )
	{
		m_Needle->SetVisible( m_Countdown < MIN_TIME && !m_bOnlyFadeOut );
	}

	if( m_LoadingText )
	{
		m_LoadingText->SetVisible( m_Countdown < MIN_TIME && !m_bOnlyFadeOut );
	}

	if( m_LoadingHintText )
	{
		m_LoadingHintText->SetVisible( m_Countdown < MIN_TIME && !m_bOnlyFadeOut );
	}

	if( !m_bHasSwitchedMusic && m_Countdown >= MIN_TIME )
	{
		m_bHasSwitchedMusic = true;
		if( !m_bDontSwitchMusic )
		{
			SwitchToNewMapMusic();
		}
	}

	if( m_Countdown > FADE_OUT )
	{
		MenuStack_Pop();
	}
}

void ExFullScreenLoadingMenu::SwitchToNewMapMusic()
{
	ExGame* Game = GetGame();
	EGClient* PC = GetPrimaryClient();
	EGSoundScape* SoundScape = PC ? PC->SDK_GetSoundScape() : nullptr;
	if( SoundScape && Game )
	{
		SoundScape->FadeToBgMusic( *Game->GetCurrentMapInfo().MusicTrack.Path );
	}
}

void ExFullScreenLoadingMenu::SetOnlyFadeOut(eg_bool bNewValue)
{
	m_bOnlyFadeOut = bNewValue;
	m_Countdown = 1.f; // Shorten wait till fade out.

	if( m_Needle )
	{
		m_Needle->SetVisible( false );
	}

	if( m_LoadingText )
	{
		m_LoadingText->SetVisible( false );
	}

	if( m_LoadingHintText )
	{
		m_LoadingHintText->SetVisible( false );
	}
}

eg_bool ExFullScreenLoadingMenu::IsGameLoadedAndReady( EGClient* Client )
{
	ExClient* EC = static_cast<ExClient*>(Client);
	const ExGame* GameData = EC ? EC->GetGameData() : nullptr;
	if( EC && GameData )
	{
		const eg_bool bCanNavigateWorld = GameData->GetPlayerEnt() != nullptr && GameData->CanPlayerMove() && EC->SDK_AskStateQuestion( eg_client_state_q::IS_CONNECTED ) && EC->SDK_AskStateQuestion( eg_client_state_q::HAS_MAP );
		const eg_bool bIsLoading = EC->SDK_AskStateQuestion( eg_client_state_q::IS_LOADING_THREAD_ACTIVE );
		const eg_bool bIsSaving = EC->SDK_AskStateQuestion( eg_client_state_q::IS_SAVING );
		const eg_bool bWaitingForInitData = EC->m_bWaitingForInitialRepData;

		return bCanNavigateWorld && !bIsLoading && !bIsSaving && !bWaitingForInitData;
	}

	assert( false ); // No game and client who is asking for this?

	return true;
}
