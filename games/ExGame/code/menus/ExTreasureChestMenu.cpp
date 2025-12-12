#include "ExMenu.h"

class ExTreasureChestMenu : public ExMenu
{
	EG_CLASS_BODY( ExTreasureChestMenu , ExMenu )

private:

	enum class ex_menu_s
	{
		LOOKING,
		OPENING,
		OPENED,
	};

private:

	ex_menu_s m_MenuState;
	eg_real   m_CountdownToOpen;
	eg_bool   m_bOpened:1;

public:

	void SetMenuState( ex_menu_s NewState )
	{
		m_MenuState = NewState;

		ClearHints();

		switch( m_MenuState )
		{
		case ex_menu_s::LOOKING:
			AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("TreasureChestMenuHintOpen","Open Chest")) );
			AddHint( CMDA_MENU_BACK , eg_loc_text(eg_crc("BackOutText")) );
			break;
		case ex_menu_s::OPENING:
			m_CountdownToOpen = 0.f;
			m_bOpened = true;
			break;
		case ex_menu_s::OPENED:
			GetGame()->SDK_RunServerEvent( eg_crc("SetHandleEncounterAfterScript") );
			MenuStack_Pop();
			break;			
		}
	}

	virtual void OnInit() override final
	{ 
		Super::OnInit();
		SetMenuState( ex_menu_s::LOOKING );
	}

	virtual void OnDeinit() override final
	{
		Super::OnDeinit();
		GetGame()->SDK_RunServerEvent( eg_crc("EndTreasureChestIntraction") , m_bOpened );
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );
		if( m_MenuState == ex_menu_s::OPENING )
		{
			m_CountdownToOpen += DeltaTime;
			if( m_CountdownToOpen >= .75f ) // Same time as in edef for treasure chest
			{
				SetMenuState( ex_menu_s::OPENED );
			}
		}
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( InputType == eg_menuinput_t::BUTTON_PRIMARY )
		{
			if( m_MenuState == ex_menu_s::LOOKING )
			{
				GetGame()->SDK_RunServerEvent( eg_crc("DoTreasureChestOpening") );
				SetMenuState( ex_menu_s::OPENING );
			}
			else if( m_MenuState == ex_menu_s::OPENING )
			{
				// Impatient players may skip the opening.
				SetMenuState( ex_menu_s::OPENED );
			}

			return true;
		}
		else if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}

		return Super::OnInput( InputType );
	}
};

EG_CLASS_DECL( ExTreasureChestMenu )