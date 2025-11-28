#pragma once
#include "ExMenu.h"
#include "ExBgComponent.h"

class ExChoiceWidgetMenu: public ExMenu
{
	EG_CLASS_BODY( ExChoiceWidgetMenu , ExMenu )

public:

	struct exConfig
	{
		eg_transform         Pose = CT_Default;
		EGArray<eg_loc_text> Choices;
		eg_uint              InitialSelection = 0;
		eg_bool              bPushCascaded = false; // If this is set, Pose is ignored.
		eg_bool              bCanPop = false;
		eg_bool              bForwardInputCmdsToParent = false;

		void SetPoseForPortrait( eg_int PortraitIndex , eg_bool bBigPortraits );
		void BoundToScreen( eg_real ScreenBottom );
	};

	static ExChoiceWidgetMenu* PushMenu( EGMenu* OwnerMenu , const exConfig& Config );

public:

	typedef EGDelegate<void,ExChoiceWidgetMenu* /* SourceMenu */> ExWidgetDelegate;
	typedef EGDelegate<void,ExChoiceWidgetMenu* /* SourceMenu */,eg_uint /* ChoiceIndex */> ExChoiceDelegate;

	ExChoiceDelegate ChoicePressedDelegate;
	ExChoiceDelegate ChoiceSelectedDelegate;
	ExWidgetDelegate ChoiceAbortedDelegate;

private:

	exConfig        m_Config;
	EGUiGridWidget* m_List;
	EGUiWidget*     m_Scrollbar;
	eg_real         m_RevealTime;
	eg_bool         m_bIsRevealed:1;
	eg_bool         m_bIsCascaded:1;

public:

	void SetupConfig( const exConfig& InConfig );

	virtual void OnInit() override final;
	virtual void OnDeinit() override final;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final;
	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override final;
	void OnObjPressed( const egUIWidgetEventInfo& Info );
	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo );

	virtual void OnRevealComplete() override final;
};
