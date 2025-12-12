// (c) 2017 Beem Media

#pragma once

#include "ExMenu.h"

class ExTextRevealWidget;

class ExConversationMenu: public ExMenu
{
	EG_CLASS_BODY( ExConversationMenu , ExMenu )

public:

		enum class ex_continue
		{
			RefreshDialog,
			ClearDialog,
			Close,
		};

private:

	enum ex_state
	{
		INTRO,
		MAKING_CHOICE,
		WAITING_FOR_SERVER,
		BLANK_OVERLAY,
		OUTTRO,
	};

	EGUiWidget*         m_OverlayObj;
	EGUiWidget*         m_ChoicesObj;
	EGUiWidget*         m_NameplateObj;
	EGUiWidget*         m_NameplateTxt;
	ExTextRevealWidget* m_ConvText2Choices;
	ExTextRevealWidget* m_ConvText3Choices;
	ExTextRevealWidget* m_ConvText4Choices;
	ExTextRevealWidget* m_CurConvTextWidget;
	eg_real             m_ChoiceOffset;
	EGSmBranchOptions   m_BranchOptions;
	eg_real             m_MenuTime;
	eg_real             m_InputCountdown;
	eg_bool             m_bTextFullyRevealed = false;
	const eg_real       m_InputCountdownDuration = .1f;
	ex_state            m_State:4;
	eg_bool             m_bEnd:1;
	eg_bool             m_bIsPlayingDialogSound:1;

public:

	ExConversationMenu(): ExMenu(), m_State(INTRO), m_bEnd(false){ }
	void Continue( ex_continue ContinueType );
	void RefreshNameplate();

private:

	virtual void OnInit() override final;
	virtual void OnDeinit() override final;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final;
	void OnObjPressed( const egUIWidgetEventInfo& Info );
	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo );
	void RefreshChoices( eg_bool bClear );
	void ContinueInternal();
	void EndConversation();

	eg_uint PopulateChoicesList();
	void SelectCurrentChoiceTextWidget();
};

