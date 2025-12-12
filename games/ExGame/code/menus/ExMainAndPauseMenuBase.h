// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "EGRandom.h"

class EGUiTextWidget;

class ExMainAndPauseMenuBase : public ExMenu , public IExDialogListener
{
	EG_CLASS_BODY( ExMainAndPauseMenuBase , ExMenu )

protected:

	enum class ex_news_s
	{
		WAITING_FOR_NEWS,
		SHOWING,
		SWAP_HIDE1,
		SWAP_HIDE2,
	};

	enum class ex_menu_swap_s
	{
		None,
		MovingOut,
		MovingIn,
	};

	typedef void (ExMainAndPauseMenuBase::*exOptionFunction)();

	struct exMenuOption
	{
		eg_string_crc DisplayText = CT_Clear;
		exOptionFunction Callback = nullptr;

		exMenuOption() = default;
		exMenuOption( eg_string_crc InDisplayText , exOptionFunction InCallback ): DisplayText( InDisplayText ) , Callback( InCallback ) { }
	};

	struct exOptionSet
	{
		EGArray<exMenuOption> Choices;
		eg_int                LastSelected = 0;

		exOptionSet() = default;
		exOptionSet( const EGArray<exMenuOption>& InOptions ): Choices( InOptions ) , LastSelected( 0 ) { }
	};

protected:

	EGArray<EGUiWidget*> m_StuffToHideForCredits;
	EGUiWidget*   m_NewsFeed;
	EGUiWidget*   m_Planet;
	EGUiWidget*   m_PlanetLight;
	EGUiWidget*   m_Skybox;
	EGUiGridWidget* m_OptionsGridWidget;
	EGUiWidget* m_OptionsScrollbarWidget;
	EGUiTextWidget* m_VersionTextWidget;
	ex_news_s     m_DailyMsgState;
	eg_real       m_DailyMsgTimer;
	eg_string_crc m_LoadingHintText = CT_Clear;
	eg_transform  m_PlanetRotation;
	eg_transform  m_PlanetLightRotation;
	eg_transform  m_SkyboxRotation;
	EGRandom      m_Rng = CT_Default;
	eg_real       m_SunRiseTime;
	eg_bool       m_bShouldStartMusicOnActivate;
	eg_bool       m_bInCredits;
	eg_bool       m_IsPause;

	EGArray<exOptionSet> m_MainOptionsStack;
	EGArray<eg_uint> m_CurrentSaves;
	eg_uint m_SaveSlotToDelete = 0;

	ex_menu_swap_s m_SwapState = ex_menu_swap_s::None;
	eg_real m_SwapTimeElapsed = 0.f;
	const eg_real m_SwapDuration = .25f;
	const eg_real m_SwapFinalOffset = 160.f;
	const eg_string_crc m_SpecialCaseGameSlot = eg_crc("SaveSlotHandler");
	const eg_uint m_SpecialSaveIdForNewGame = 0;

public:

	ExMainAndPauseMenuBase()
	: m_SkyboxRotation( CT_Default )
	, m_PlanetRotation( CT_Default ) 
	, m_PlanetLightRotation( CT_Default )
	{ 
		
	}

	
	void SetVisibilityForCredits( eg_bool bVisible );

	virtual void Refresh() override final;
	virtual void OnInit() override final;
	virtual void OnDeinit() override final;
	virtual void OnActivate() override final;
	virtual void OnDeactivate() override;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final;
	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override final;

	void OnOptionClicked( const egUIWidgetEventInfo& Info );
	void OnOptionUpdated( const egUIWidgetEventInfo& ItemInfo );

protected:

	eg_bool IsSaveGameHighlighted();
	void RefreshHints();

	virtual void OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice ) override;

	void PopulateRootOptions();
	void PopulateRootOptions( EGArray<exMenuOption>& MenuOptions );
	void PopulateSettingsOptions( EGArray<exMenuOption>& MenuOptions );

	void PushOptionSet( const EGArray<exMenuOption>& InOptions );
	void PopOptionSet();

	void PlaySwap( eg_bool bFromEmpty );
	eg_bool IsSwapping() const { return m_SwapState != ex_menu_swap_s::None; }
	void HandleSwapUpdate( eg_real DeltaTime );
	void OnSwap_PopulateOptions();
	void OnSwap_Complete();

	void PopulateCurrentSaves();

	void ExecuteQuitGame();

	void OnSelectResume();
	void OnSelectContinue();
	void OnSelectPlay();
	void OnSelectNewGame();
	void OnSelectOptions();
	void OnSelectDebugOptions();
	void OnSelectExtras();
	void OnSelectViewIntro();
	void OnSelectViewEnding();
	void OnSelectCredits();
	void OnSelectViewClassicIntro();
	void OnPlayCastleGame();
	void OnSelectQuitGame();
	void OnSelectExitGame();
	void OnSelectDebugSubmenu();
	void OnSelectDebugWarpMenu();
	void OnSelectDebugMonsterViewerMenu();
	void OnSelectDebugCombatMenu();
	void OnSelectDebugDebugMenu();
	void OnSelectDebugPartyCreateMenu();
	void OnSelectDebugRosterMenu();
	void OnSelectDebugResetAchievements();
	void OnSelectGameplaySettings();
	void OnSelectGraphicsSettings();
	void OnSelectAudioSettings();
	void OnSelectControlsSettings();
};
