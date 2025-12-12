// (c) 2016 Beem Media

#pragma once

#include "ExMenu.h"
#include "ExDialogMenu.h"

class ExCharacterPortraitWidget2;
class ExChoiceWidgetMenu;

class ExRosterMenu : public ExMenu , public IExDialogListener
{
	EG_CLASS_BODY( ExRosterMenu , ExMenu )

public:

	static void OpenRosterMenu( EGClient* Client , eg_bool bExchangeOnly );

private:

	enum class ex_menu_s
	{
		BROWSING,
		ADD_TO_PARTY,
		EXCHANGE_ORDER,
	};

	enum class ex_choice_t
	{
		CANCEL,
		CREATE_CHARACTER,
		VIEW_CHARACTER,
		REMOVE_FROM_PARTY,
		ADD_TO_PARTY,
		EXCHANGE_ORDER,
		DELETE_CHARACTER,
	};

private:

	EGUiGridWidget*             m_CharacterPortraits;
	EGUiGridWidget*             m_CharacterRoster;
	ExCharacterPortraitWidget2* m_Portraits[ExRoster::PARTY_SIZE];
	ex_menu_s                   m_MenuState;
	EGArray<ex_choice_t>        m_CurrentChoices;
	eg_int                      m_ChoiceTarget;
	const ExFighter*            m_RosterMemberInQuestion;
	eg_bool                     m_bIsExchangeOnly = false;

private:

	void OnRosterChanged();
	void SetMenuState( ex_menu_s NewState , eg_bool bAllowAudio = true );
	virtual void Refresh() override final;
	virtual void OnInit() override final;
	virtual void OnDeinit() override final;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final;
	void LeaveMenu();
	eg_bool IsCharacterInParty( const ExGame* Game , const ExFighter* Character ) const;
	void PushContextMenu( const ExFighter* RosterMember , eg_real x , eg_real y , eg_int PortraitIndex );
	void OnRosterMemberCellChanged( const egUIWidgetEventInfo& ItemInfo );
	void OnPartyMemberCellChanged( const egUIWidgetEventInfo& ItemInfo );
	void OnRosterMemberClicked( const egUIWidgetEventInfo& Info );
	void OnPartyMemberClicked( const egUIWidgetEventInfo& Info );
	eg_uint GetPartyIndexOfCharacter( const ExFighter* Character );
	eg_uint GetRosterIndexOfCharacter( const ExFighter* Character );
	void ExchangePartyMemberWithSlot( const ExFighter* Character , eg_uint Slot );
	void AddPartyMemberToSlot( const ExFighter* Character , eg_uint Slot );
	void RemovePartyMember( const ExFighter* Character );
	void OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex );
	eg_loc_text GetChoiceText( eg_uint Index ) const;
	virtual void OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice ) override final;
};