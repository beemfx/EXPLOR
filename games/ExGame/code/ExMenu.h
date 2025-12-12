/******************************************************************************
ExMenu - Shared functionality for EXPLOR UI

(c) 2015 Beem Media
******************************************************************************/
#pragma once

#include "ExGame.h"
#include "ExClient.h"
#include "ExEnt.h"
#include "EGParse.h"
#include "EGMenu.h"
#include "EGUiWidget.h"
#include "ExBgComponent.h"
#include "ExHintBarWidget.h"
#include "EGInputTypes.h"
#include "ExUiSounds.h"


class ExMenu: public EGMenu , public ExBgComponent::ICb
{
	EG_CLASS_BODY( ExMenu , EGMenu )

protected:
	
	enum class ex_toggle_menu_t
	{
		QuestLog ,
		Map ,
		Inventory ,
		Character ,
	};

private:

	ExBgComponent* m_Bg = nullptr;
	eg_bool        m_bIsHudVisible = true;

public:

	virtual void Refresh(){ }

	void HudSetVisible( eg_bool bVisible , eg_bool bPlayTransition )
	{
		m_bIsHudVisible = bVisible;
		OnHudVisibleChanged( bVisible , bPlayTransition );
	}

	eg_bool IsHudVisible() const { return m_bIsHudVisible; }

protected:

	virtual void OnActivate() override
	{
		Super::OnActivate();

		if( nullptr == m_Bg || !m_Bg->IsRevealing() )
		{
			ExHintBarWidget* HintBar = GetWidget<ExHintBarWidget>(eg_crc("HintBar"));
			if( HintBar )
			{
				HintBar->SetVisible( true );
			}
		}
	}

	virtual void OnDeactivate() override
	{
		ExHintBarWidget* HintBar = GetWidget<ExHintBarWidget>(eg_crc("HintBar"));
		if( HintBar )
		{
			HintBar->SetVisible( false );
		}
		
		Super::OnDeactivate();
	}

	virtual void OnHudVisibleChanged(  eg_bool bVisible , eg_bool bPlayTransition )
	{
		unused( bVisible , bPlayTransition );
	}

	virtual void OnBgComponentAnimComplete( ExBgComponent* Bg ) override final
	{
		if( Bg && Bg == m_Bg )
		{
			OnRevealComplete();
		}
	}

	void DoReveal( eg_uint NumChoices = 0 , eg_bool bMuteAudio = false )
	{
		m_Bg = GetWidget<ExBgComponent>( eg_crc("Background") );
		if( m_Bg )
		{
			m_Bg->InitWidget( this );
			m_Bg->SetAudioMuted( bMuteAudio );
			SetAllObjectsVisibility( false );
			m_Bg->SetVisible( true );

			m_Bg->PlayReveal( NumChoices );
		}
	}

	void ClearHints()
	{
		ExHintBarWidget* HintBar = GetWidget<ExHintBarWidget>(eg_crc("HintBar"));
		if( HintBar )
		{
			HintBar->ClearHints();
		}
	}

	void AddHint( eg_cmd_t Cmd , const eg_loc_text& Text )
	{
		ExHintBarWidget* HintBar = GetWidget<ExHintBarWidget>( eg_crc( "HintBar" ) );
		if( HintBar )
		{
			HintBar->AddHint( Cmd , Text );
		}
	}

	void PlaySoundEvent( ExUiSounds::ex_sound_e EventType )
	{
		ExUiSounds::Get().PlaySoundEvent( EventType );
	}

	virtual void OnRevealComplete()
	{
		SetAllObjectsVisibility( true );
	}

	void HideHUD()
	{
		EGCast<ExClient>(GetOwnerClient())->HideHUD();
	}

	void ShowHUD( eg_bool bImmediatePortraits )
	{
		EGCast<ExClient>( GetOwnerClient() )->ShowHUD( bImmediatePortraits );
	}

	const ExGame* GetGame()const
	{
		return EGCast<const ExGame>( GetOwnerClient()->SDK_GetGame() );
	}

	ExGame* GetGame()
	{
		return EGCast<ExGame>( GetOwnerClient()->SDK_GetGame() );
	}

	const ExEnt* FindEntData( const eg_ent_id& Id )
	{
		const ExEnt* Out = nullptr;
		EGClient* Client = GetOwnerClient();
		if( nullptr == Client )
		{
			Client = GetPrimaryClient();
		}

		if( Client )
		{
			Out = EGCast<ExEnt>(Client->SDK_GetEnt( Id ));
		}

		return Out;
	}

	void RunServerEvent( const egRemoteEvent& Event )
	{
		EGClient* Client = GetOwnerClient();
		if( nullptr == Client )
		{
			Client = GetPrimaryClient();
		}

		if( Client )
		{
			Client->SDK_RunServerEvent( Event );
		}
	}

	eg_bool HandlePossibleMenuToggle( ex_toggle_menu_t MenuType , const struct egLockstepCmds& Cmds );
};