// (c) 2017 Beem Media

#include "ExClient.h"
#include "ExGame.h"
#include "ExSaveMgr.h"
#include "ExGame.h"
#include "ExDialogMenu.h"
#include "ExConversationMenu.h"
#include "ExMenu.h"
#include "EGTextFormat.h"
#include "ExQuestItems.h"
#include "EGTextFormat.h"
#include "ExEnt.h"
#include "ExPlayerController.h"
#include "EGMenuStack.h"
#include "EGSoundScape.h"
#include "ExMapping.h"
#include "ExWorldRenderPipe.h"
#include "ExAchievements.h"

EG_CLASS_DECL( ExClient )

ExClient::ExClient()
: EGClient()
, m_HUD( this )
{
	#define REG_BH( _bh_ ) m_BhHandler.RegisterEvent( eg_crc(#_bh_) , &ExClient::Bh##_bh_ );
	REG_BH( RefreshHUD )
	REG_BH( OpenPauseMenu )
	REG_BH( OnReturnToMainMenu )
	REG_BH( OnInitialRepDataComplete )
	REG_BH( ShowFullScreenLoading )
	REG_BH( QuitFromInn )
	REG_BH( ProgressSaved )
	#undef REG_BH

	m_MouseInputMode = MOUSE_INPUT_CURSOR;
}

void ExClient::Init( const egClientId& Id , EGClass* GameClass )
{
	Super::Init( Id , GameClass );

	ExSaveMgr::Get().Init( this ); // Must be called after profile data is initialized.

	if( m_MenuStack )
	{
		m_MenuStack->SetMouseCursor( eg_crc("ENT_UI_ExMouseCursor") );
	}

	m_HUD.InitClientComponents();
}

void ExClient::Deinit()
{
	m_HUD.DeinitClientComponents();
	ExSaveMgr::Get().Deinit( this );
	Super::Deinit();
}

void ExClient::Update( eg_real DeltaTime )
{
	Super::Update( DeltaTime );

	m_HUD.Update( DeltaTime );

	ExUiSounds::Get().Update( DeltaTime );
	ExAchievements::Get().Update( DeltaTime );

	ExWorldRenderPipe* WorldRenderPipe = EGCast<ExWorldRenderPipe>(m_WorldRenderPipe);
	if( WorldRenderPipe )
	{
		WorldRenderPipe->UpdateFromGame( DeltaTime , EGCast<ExGame>(SDK_GetGame()) );
	}
}

void ExClient::BeginGame()
{
	Super::BeginGame();

	SDK_PushMenu( eg_crc( "MainMenu" ) );
}

void ExClient::OnMapLoadComplete()
{
	// All data has been wiped so we're gonna tell the server to
	// replicate data to us.
	assert( !m_bWaitingForInitialRepData );
	m_bWaitingForInitialRepData = true;
	SDK_RunServerEvent( egRemoteEvent( eg_crc( "OnSendInitialRepData" ) ) );
}

void ExClient::SetupCamera( class EGClientView& View ) const
{
	eg_transform CameraPose( CT_Default );
	if( const ExGame* Game = GetGameData() )
	{
		CameraPose = Game->GetCameraPose();
	}
	View.SetPose( CameraPose, 90.f, .1f, EX_CLIP_DISTANCE );
}

void ExClient::OnRemoteEvent( const egRemoteEvent& Event )
{
	if( m_BhHandler.ExecuteEvent( this , Event ) )
	{
		return;
	}

	ExGame* Game = GetGameData();
	if( Game )
	{
		Game->Client_HandleEvent( Event );
	}
}

void ExClient::Draw( eg_bool bForceSimple )
{
	if( GetGameData() )
	{
		GetGameData()->GetMappingModule()->Draw();
	}
	MainDisplayList->ClearRT( eg_color::Black );
	Super::Draw( bForceSimple );
}

void ExClient::BhRefreshHUD( const eg_event_parms& Parms )
{
	unused( Parms );

	m_HUD.Refresh();
}

void ExClient::BhOpenPauseMenu( const eg_event_parms& Parms )
{
	unused( Parms );

	SDK_PushMenu( eg_crc( "PauseMenu" ) );
}

void ExClient::BhOnReturnToMainMenu( const eg_event_parms& Parms )
{
	unused( Parms );

	SDK_PushMenu( eg_crc("MainMenu") );
}

void ExClient::BhOnInitialRepDataComplete( const eg_event_parms& Parms )
{
	unused( Parms );

	assert( m_bWaitingForInitialRepData );
	m_bWaitingForInitialRepData = false;

	GetGameData()->BhOnRosterChanged( eg_event_parms( CT_Clear ) );
	BhRefreshHUD( eg_event_parms( CT_Clear ) );
}

void ExClient::BhShowFullScreenLoading( const eg_event_parms& Parms )
{
	unused( Parms );

	if( !m_bIsFullScreenLoading )
	{
		SDK_PushMenu( eg_crc( "FullScreenLoadingMenu" ) );
	}
}

void ExClient::BhQuitFromInn( const eg_event_parms& Parms )
{
	unused( Parms );

	SDK_RunServerEvent( egRemoteEvent( eg_crc("ResetGame") ) );
	SDK_ClearMenuStack();
	SDK_PushMenu( eg_crc("MainMenu") );
}

void ExClient::BhProgressSaved( const eg_event_parms& Parms )
{
	unused( Parms );

	ProgressSavedDelegate.Broadcast();
}

const ExGame* ExClient::GetGameData() const
{
	return EGCast<const ExGame>( SDK_GetGame() );
}

ExGame* ExClient::GetGameData()
{
	return EGCast<ExGame>( SDK_GetGame() );
}

void ExClient::PushBgMusic( eg_cpstr Filename )
{
	if( m_BgScape )
	{
		m_BgScape->PushBgMusic( Filename );
	}
}

void ExClient::PopBgMusic()
{
	if( m_BgScape )
	{
		m_BgScape->PopBgMusic();
	}
}

void ExClient::SetLoadingOpacity( eg_real NewValue )
{
	if( !EG_IsEqualEps( NewValue , m_LoadingOpacity ) || (NewValue == 0.f && m_LoadingOpacity != 0.f) ) // Regardless of how close they are if it's set to 0 follow through.
	{
		m_LoadingOpacity = NewValue;
		OnLoadingOpacityChangedDelegate.Broadcast( m_LoadingOpacity );
	}
}
