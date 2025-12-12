// (c) 2020 Beem Media. All Rights Reserved.

#include "ExEndGameFlowMenu.h"
#include "ExAnimationMenu.h"
#include "ExFullScreenLoadingMenu.h"
#include "ExDialogMenu.h"
#include "EGSoundScape.h"

EG_CLASS_DECL( ExEndGameFlowMenu )

void ExEndGameFlowMenu::OnInit()
{
	Super::OnInit();

	EGSoundScape* SoundScape = GetPrimaryClient() ? GetPrimaryClient()->SDK_GetSoundScape() : nullptr;
	if( SoundScape )
	{
		SoundScape->FadeAllBgMusic( .5f );
	}
}

void ExEndGameFlowMenu::OnActivate()
{
	Super::OnActivate();

	AdvanceToNextState();
}

void ExEndGameFlowMenu::OnUpdate( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime , AspectRatio );

	m_TimeElapsed += DeltaTime;

	if( m_State == ex_flow_s::WaitForAudioToEnd && m_TimeElapsed >= .6f )
	{
		EGSoundScape* SoundScape = GetPrimaryClient() ? GetPrimaryClient()->SDK_GetSoundScape() : nullptr;
		if( SoundScape )
		{
			SoundScape->ClearBgMusicStack();
		}
		AdvanceToNextState();
	}
}

void ExEndGameFlowMenu::AdvanceToNextState()
{
	while( IsActive() )
	{
		switch( m_State )
		{
		case ex_flow_s::None:
		{
			m_TimeElapsed = 0.f;
			m_State = ex_flow_s::WaitForAudioToEnd;
		} break;
		case ex_flow_s::WaitForAudioToEnd:
		{
			m_State = ex_flow_s::EndingMovie;
		} break;
		case ex_flow_s::EndingMovie:
		{
			m_State = ex_flow_s::Credits;
		} break;
		case ex_flow_s::Credits:
		{
			m_State = ex_flow_s::Congratulations;
		} break;
		case ex_flow_s::Congratulations:
		{
			m_State = ex_flow_s::ReturnToTown;
		} break;
		case ex_flow_s::ReturnToTown:
		{
			m_State = ex_flow_s::Done;
		} break;
		case ex_flow_s::Done:
		{
		} break;
		}

		switch( m_State )
		{
		case ex_flow_s::None:
		case ex_flow_s::WaitForAudioToEnd:
			// Nothing to do... Just waiting.
			return;
		case ex_flow_s::EndingMovie:
			ExAnimationMenu::PlayMovie( GetClientOwner() , eg_crc("Ending") );
			break;
		case ex_flow_s::Credits:
		{
			MenuStack_PushTo( eg_crc("CreditsEndGameBgMenu") );
		} break;
		case ex_flow_s::Congratulations:
		{
			EGSoundScape* SoundScape = GetPrimaryClient() ? GetPrimaryClient()->SDK_GetSoundScape() : nullptr;
			if (SoundScape)
			{
				SoundScape->FadeToBgMusic( "/Music/MainMenu" );
			}
			exDialogParms CongratsMessage( CT_Clear );
			CongratsMessage.HeaderText = eg_loc_text(eg_loc("EndGameFlowCongratsHeader","Congratulations"));
			CongratsMessage.BodyText = eg_loc_text(eg_loc("EndGameFlowCongratsMessage","You have successfully completed E.X.P.L.O.R.|SUP|TM|SUP|: A New World. You will now be returned to town where you may continue adventuring."));
			ExDialogMenu_PushDialogMenu( GetClientOwner() , CongratsMessage );
		} break;
		case ex_flow_s::ReturnToTown:
		{
			if( EGClient* Client = GetClientOwner() )
			{
				Client->SDK_RunServerEvent( egRemoteEvent( eg_crc("ServerNotifyEvent") , eg_crc("EndGameReturnToTown") ) );
			}
		} break;
		case ex_flow_s::Done:
			MenuStack_Clear();
			MenuStack_PushTo( eg_crc("FullScreenLoadingMenu") );
			break;
		}
	}
}
