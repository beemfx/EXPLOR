// (c) 2017 Beem Media

#include "ExMenu.h"
#include "ExHudTextWidget.h"
#include "EGMenuStack.h"
#include "ExTextRevealComponent.h"
#include "ExGameSettings.h"
#include "ExHUDTranslatorWidget.h"

class ExHUDBase : public ExMenu
{
	EG_CLASS_BODY( ExHUDBase , ExMenu )

protected:

	EGUiMeshWidget* m_WaitingBgWidget;
	ExHudTextWidget* m_WaitingTextWidget;
	ExHUDTranslatorWidget* m_TranslatorWidget;
	eg_bool          m_bIsWaiting = false;
	eg_bool          m_bMenusOpen = false;
	eg_bool          m_bShouldShowHints = false;

protected:

	virtual void OnInit() override
	{
		Super::OnInit();

		m_WaitingBgWidget = GetWidget<EGUiMeshWidget>( eg_crc("WaitingBg") );
		m_WaitingTextWidget = GetWidget<ExHudTextWidget>( eg_crc("WaitingText") );
		m_TranslatorWidget = GetWidget<ExHUDTranslatorWidget>( eg_crc("Translator") );

		if( m_WaitingTextWidget )
		{
			m_WaitingTextWidget->HudHideNow();
		}
		if( m_WaitingBgWidget )
		{
			m_WaitingBgWidget->RunEvent( eg_crc("HideNow") );
		}

		m_bIsWaiting = false;
		ExGame* Game = GetGame();
		if( Game )
		{
			Game->OnClientNotifyWaitDelegate.AddUnique( this , &ThisClass::OnWait );
			Game->OnClientNotifyTurnStartDelegate.AddUnique( this , &ThisClass::OnTurnStart );
			Game->OnClientNotifyTurnEndDelegate.AddUnique( this , &ThisClass::OnTurnEnd );
			Game->OnClientGameStateChangedDelegate.AddUnique( this , &ThisClass::OnGameStateChanged );
		}

		EGMenuStack* MenuStack = GetClientOwner() ? GetClientOwner()->SDK_GetMenuStack() : nullptr;
		if( MenuStack )
		{
			MenuStack->MenuStackChangedDelegate.AddUnique( this , &ThisClass::OnMenuStackChanged );
		}

		OnGameStateChanged();
	}

	virtual void OnDeinit() override
	{
		ExGame* Game = GetGame();
		if( Game )
		{
			Game->OnClientNotifyWaitDelegate.RemoveAll( this  );
			Game->OnClientNotifyTurnStartDelegate.RemoveAll( this );
			Game->OnClientNotifyTurnEndDelegate.RemoveAll( this );
			Game->OnClientGameStateChangedDelegate.RemoveAll( this );
		}

		EGMenuStack* MenuStack = GetClientOwner() ? GetClientOwner()->SDK_GetMenuStack() : nullptr;
		if( MenuStack )
		{
			MenuStack->MenuStackChangedDelegate.RemoveAll( this );
		}

		Super::OnDeinit();
	}

	void OnMenuStackChanged( EGMenuStack* MenuStack )
	{
		m_bMenusOpen = MenuStack && MenuStack->Len() > 0;
		RefreshHints();
	}

	void OnWait()
	{
		m_bIsWaiting = true;
		if( IsHudVisible() && m_WaitingTextWidget )
		{
			m_WaitingTextWidget->HudPlayShow();
			if( m_WaitingBgWidget )
			{
				m_WaitingBgWidget->RunEvent( eg_crc("Show") );
			}
		}
	}

	void OnTurnStart()
	{

	}

	void OnTurnEnd()
	{
		if( IsHudVisible() && m_WaitingTextWidget )
		{
			m_WaitingTextWidget->HudPlayHide();
			if( m_WaitingBgWidget )
			{
				m_WaitingBgWidget->RunEvent( eg_crc("Hide") );
			}
		}
		m_bIsWaiting = false;
	}

	void OnGameStateChanged()
	{
		const ExGame::ex_game_s StatesForHint[] =
		{
			ExGame::ex_game_s::NONE,
			ExGame::ex_game_s::MOVING,
		};

		m_bShouldShowHints = false;
		if( ExGame* Game = GetGame() )
		{
			for( const ExGame::ex_game_s State : StatesForHint )
			{
				if( Game->GetGameState() == State )
				{
					m_bShouldShowHints = true;
				}
			}
		}
		RefreshHints();
	}

	void RefreshHints()
	{
		ClearHints();
		if( IsHudVisible() && !m_bMenusOpen && m_bShouldShowHints )
		{
			AddHint( CMDA_CONTEXTMENU , eg_loc_text(eg_loc("ContextMenuHelp","Character Options")) );
			AddHint( CMDA_GAME_PREVPMEMBER , CT_Clear );
			AddHint( CMDA_GAME_NEXTPMEMBER , eg_loc_text(eg_loc("ChangePlayerHelp","Switch Character")) );
			AddHint( CMDA_WAITTURN , eg_loc_text(eg_loc("WaitHelp","Wait")) );
		}
	}

	virtual void OnHudVisibleChanged( eg_bool bVisible , eg_bool bPlayTransition )
	{
		auto HandleMeshWidget = [bVisible,bPlayTransition]( EGUiMeshWidget* MeshWidget ) -> void
		{
			if( MeshWidget )
			{
				if( bVisible )
				{
					MeshWidget->RunEvent( bPlayTransition ? eg_crc("Show") : eg_crc("ShowNow") );
				}
				else
				{
					MeshWidget->RunEvent( bPlayTransition ? eg_crc("Hide") : eg_crc("HideNow") );
				}
			}
		};

		auto HandleTextWidget = [bVisible,bPlayTransition]( ExHudTextWidget* TextWidget ) -> void
		{
			if( TextWidget )
			{
				if( bVisible )
				{
					if( bPlayTransition )
					{
						TextWidget->HudPlayShow();
					}
					else
					{
						TextWidget->HudShowNow();
					}
				}
				else
				{
					if( bPlayTransition )
					{
						TextWidget->HudPlayHide();
					}
					else
					{
						TextWidget->HudHideNow();
					}
				}
			}
		};

		HandleMeshWidget( m_TranslatorWidget );
		HandleTextWidget( GetWidget<ExHudTextWidget>( eg_crc("DayText") ) );
		HandleTextWidget( GetWidget<ExHudTextWidget>( eg_crc("TimeText") ) );
		HandleTextWidget( GetWidget<ExHudTextWidget>( eg_crc("GoldText") ) );
		HandleTextWidget( GetWidget<ExHudTextWidget>( eg_crc("FoodText") ) );
		HandleTextWidget( GetWidget<ExHudTextWidget>( eg_crc("MapName") ) );
		HandleTextWidget( GetWidget<ExHudTextWidget>( eg_crc("FullDayAndTimeText") ) );
		HandleMeshWidget( GetWidget<EGUiMeshWidget>( eg_crc("InfoBG") ) );

		if( m_bIsWaiting )
		{
			HandleTextWidget( m_WaitingTextWidget );
			HandleMeshWidget( m_WaitingBgWidget );
		}
	}
};

EG_CLASS_DECL( ExHUDBase )
