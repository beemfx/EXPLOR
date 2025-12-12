// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMainAndPauseMenuBase.h"
#include "EGEngine.h"
#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "ExSaveMgr.h"
#include "EGUiGridWidget.h"
#include "ExLoadingHints.h"
#include "ExMapInfos.h"
#include "EGSoundScape.h"
#include "ExSettingsMenu.h"
#include "ExEngineInst.h"
#include "ExAnimationMenu.h"
#include "ExProfileData.h"
#include "ExNewGameFlowMenu.h"
#include "ExAchievements.h"

EG_CLASS_DECL( ExMainAndPauseMenuBase )

static const eg_real MAINMENU_NEWS_DURATION = 10.f;

void ExMainAndPauseMenuBase::SetVisibilityForCredits( eg_bool bVisible )
{
	m_bInCredits = !bVisible;

	for( EGUiWidget* Obj : m_StuffToHideForCredits )
	{
		if( Obj )
		{
			Obj->SetVisible( bVisible );
		}
	}
}

void ExMainAndPauseMenuBase::Refresh()
{

}

void ExMainAndPauseMenuBase::OnInit()
{
	Super::OnInit();
	m_StuffToHideForCredits.Append( GetWidget( eg_crc( "MainOptions" ) ) );
	m_StuffToHideForCredits.Append( GetWidget( eg_crc( "NewsFeed" ) ) );
	m_StuffToHideForCredits.Append( GetWidget( eg_crc( "TitleShadow" ) ) );
	m_StuffToHideForCredits.Append( GetWidget( eg_crc( "TitleText" ) ) );
	m_StuffToHideForCredits.Append( GetWidget( eg_crc( "MainOptionsScrollbar" ) ) );

	m_VersionTextWidget = GetWidget<EGUiTextWidget>( eg_crc("VersionText") );
	if( m_VersionTextWidget )
	{
		m_VersionTextWidget->SetText( CT_Clear , EGFormat( eg_loc("VersionTextFormat","Version: {0}") , *eg_s_string_sml16( *ExEngineInst::GetBuildVersion() ) ) );
		m_StuffToHideForCredits.Append( m_VersionTextWidget );
	}

	m_OptionsGridWidget = GetWidget<EGUiGridWidget>( eg_crc("MainOptions") );
	assert( nullptr != m_OptionsGridWidget );
	m_OptionsScrollbarWidget = GetWidget<EGUiWidget>( eg_crc("MainOptionsScrollbar") );

	if( m_OptionsGridWidget )
	{
		m_OptionsGridWidget->SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform::BuildTranslation( m_SwapFinalOffset , 0.f , 0.f ) );
		m_OptionsGridWidget->OnItemChangedDelegate.Bind( this , &ThisClass::OnOptionUpdated );
		m_OptionsGridWidget->OnItemPressedDelegate.Bind( this , &ThisClass::OnOptionClicked );
	}

	if( m_OptionsScrollbarWidget )
	{
		m_OptionsScrollbarWidget->SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform::BuildTranslation( m_SwapFinalOffset , 0.f , 0.f ) );
	}

	Refresh();

	PopulateRootOptions();

	m_NewsFeed = GetWidget( eg_crc( "NewsFeed" ) );
	if( m_NewsFeed )
	{
		m_NewsFeed->SetVisible( false );
		m_NewsFeed->RunEvent( eg_crc( "SetHidden" ) );
	}

	m_Planet = GetWidget( eg_crc( "Planet" ) );
	m_PlanetLight = GetWidget( eg_crc( "PlanetLight" ) );
	m_Skybox = GetWidget( eg_crc( "Skybox" ) );

	if( !m_IsPause )
	{
		m_bShouldStartMusicOnActivate = true;
	}
	else
	{
		if( m_Planet )
		{
			m_Planet->SetVisible( false );
		}

		if( m_Skybox )
		{
			m_Skybox->SetVisible( false );
		}

		HideHUD();
	}
}

void ExMainAndPauseMenuBase::OnDeinit()
{
	Super::OnDeinit();
	if( m_IsPause )
	{
		ShowHUD( false );
	}
}

void ExMainAndPauseMenuBase::OnActivate()
{
	Super::OnActivate();
	Refresh();
	SetVisibilityForCredits( true );
	if( m_bShouldStartMusicOnActivate )
	{
		m_bShouldStartMusicOnActivate = false;
		// We want the sun to start behind the planet and make it's way to
		// about the front. (IN the menu layout the sun starts on the left
		// size)
		m_PlanetLightRotation = eg_transform::BuildRotationY( EG_Rad(.3 * EG_PI / 2.f) );
		m_PlanetLightRotation.NormalizeThis();
		m_SunRiseTime = 0.f;

		EGSoundScape* SoundScape = GetPrimaryClient() ? GetPrimaryClient()->SDK_GetSoundScape() : nullptr;
		if( SoundScape )
		{
			SoundScape->FadeToBgMusic( "/Music/MainMenu" );
		}
	}
}

void ExMainAndPauseMenuBase::OnDeactivate()
{
	if( m_DailyMsgState == ex_news_s::SHOWING )
	{
		m_DailyMsgTimer = MAINMENU_NEWS_DURATION;
	}

	Super::OnDeactivate();
}

void ExMainAndPauseMenuBase::OnUpdate( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime , AspectRatio );

	if( IsActive() || (m_DailyMsgState != ex_news_s::SWAP_HIDE2 && m_DailyMsgState != ex_news_s::WAITING_FOR_NEWS) )
	{
		m_DailyMsgTimer += DeltaTime;
	}

	if( m_NewsFeed )
	{
		switch( m_DailyMsgState )
		{
		case ex_news_s::WAITING_FOR_NEWS:
		{
			if( m_DailyMsgTimer > 3.f && IsActive() )
			{
				if( ExLoadingHints::Get().IsReady() )
				{
					const exLoadingHintInfo HintData = ExLoadingHints::Get().GetRandomHint( m_Rng );
					m_LoadingHintText = HintData.HintText;
					m_NewsFeed->SetText( eg_crc( "BodyText" ) , eg_loc_text( m_LoadingHintText ) );
					m_NewsFeed->SetVisible( !m_bInCredits );
					m_NewsFeed->RunEvent( eg_crc( "Reveal" ) );
					m_DailyMsgTimer = 0.f;
					m_DailyMsgState = ex_news_s::SHOWING;
				}
				else
				{
					// If there were no messages attempt to refresh...
					m_DailyMsgTimer = 0.f;
				}
			}
		} break;
		case ex_news_s::SHOWING:
		{
			if( m_DailyMsgTimer > MAINMENU_NEWS_DURATION )
			{
				m_NewsFeed->RunEvent( eg_crc( "Hide" ) );
				m_DailyMsgTimer = 0.f;
				m_DailyMsgState = ex_news_s::SWAP_HIDE1;
			}
		} break;
		case ex_news_s::SWAP_HIDE1:
		{
			if( m_DailyMsgTimer > .5f )
			{
				m_NewsFeed->RunEvent( eg_crc( "SetHidden" ) );
				m_DailyMsgState = ex_news_s::SWAP_HIDE2;
			}
		} break;
		case ex_news_s::SWAP_HIDE2:
		{
			if( m_DailyMsgTimer > .75f && IsActive() )
			{
				const exLoadingHintInfo HintData = ExLoadingHints::Get().GetRandomHint( m_Rng );
				m_LoadingHintText = HintData.HintText;;
				m_NewsFeed->SetText( eg_crc( "BodyText" ) , eg_loc_text( m_LoadingHintText ) );
				m_NewsFeed->RunEvent( eg_crc( "Reveal" ) );
				m_DailyMsgState = ex_news_s::SHOWING;
			}
		} break;
		}
	}

	HandleSwapUpdate( DeltaTime );

	const eg_real PLANET_ROTATION_SECONDS = 180.f;
	const eg_real PLANET_LIGHT_ROT_SECONDS = -240.f;
	const eg_real SKYBOX_ROT_SECONDS = 440.f;

	m_PlanetRotation.RotateYThis( EG_Rad(DeltaTime * ((EG_PI * 2.f) / PLANET_ROTATION_SECONDS)) );
	m_PlanetRotation.NormalizeThis();
	m_SkyboxRotation.RotateZThis( EG_Rad(DeltaTime * ((EG_PI * 2.f) / SKYBOX_ROT_SECONDS)) );
	m_SkyboxRotation.NormalizeThis();


	if( m_Planet )
	{
		m_Planet->SetOffset( EGUiWidget::eg_offset_t::PRE , m_PlanetRotation );
	}

	if( m_Skybox )
	{
		m_Skybox->SetOffset( EGUiWidget::eg_offset_t::POST , m_SkyboxRotation );
	}

	m_SunRiseTime += DeltaTime;
	if( m_SunRiseTime < EG_Abs( .4f * (PLANET_LIGHT_ROT_SECONDS / 2.f) ) )
	{
		m_PlanetLightRotation.RotateYThis( EG_Rad(DeltaTime * ((EG_PI * 2.f) / PLANET_LIGHT_ROT_SECONDS)) );
		m_PlanetLightRotation.NormalizeThis();

		if( m_PlanetLight )
		{
			// Post offset will preserve distance from planet!
			m_PlanetLight->SetOffset( EGUiWidget::eg_offset_t::POST , m_PlanetLightRotation );
		}
	}
}

eg_bool ExMainAndPauseMenuBase::OnInput( eg_menuinput_t InputType )
{
	if( IsSwapping() )
	{
		return true;
	}

	if( InputType == eg_menuinput_t::BUTTON_BACK )
	{
		if( m_MainOptionsStack.Len() > 1 )
		{
			PopOptionSet();
			return true;
		}

		if( m_IsPause )
		{
			MenuStack_Pop();
			return true;
		}
	}
	
	return false;
}

void ExMainAndPauseMenuBase::OnInputCmds( const struct egLockstepCmds& Cmds )
{
	if( Cmds.WasMenuPressed( CMDA_GAMEMENU_CLOSE ) )
	{
		if( m_IsPause )
		{
			MenuStack_Pop();
			return;
		}
	}

	if( Cmds.WasMenuPressed( CMDA_MENU_DELETE ) )
	{
		if( m_OptionsGridWidget && m_MainOptionsStack.HasItems() && m_MainOptionsStack.Top().Choices.IsValidIndex( m_OptionsGridWidget->GetSelectedIndex() ) )
		{
			const exMenuOption& SelectedMenuOption = m_MainOptionsStack.Top().Choices[m_OptionsGridWidget->GetSelectedIndex()];
			if( SelectedMenuOption.DisplayText == m_SpecialCaseGameSlot )
			{
				m_SaveSlotToDelete = m_CurrentSaves.IsValidIndex( m_OptionsGridWidget->GetSelectedIndex() ) ? m_CurrentSaves[m_OptionsGridWidget->GetSelectedIndex()] : m_SpecialSaveIdForNewGame;
				if( ExSaveMgr::Get().DoesSaveExist( GetClientOwner() , m_SaveSlotToDelete ) )
				{
					ExDialogMenu_PushDialogMenu( GetOwnerClient() , exYesNoDialogParms( EGFormat( eg_loc( "PauseMenuConfirmDelete" , "Are you sure you want to delete {0}? This cannot be undone." ) , ExSaveMgr::Get().GetSaveName( GetOwnerClient() , m_SaveSlotToDelete ) ) , this , eg_crc("ConfirmDelete") , true ) );
				}
			}
		}
	}
}

void ExMainAndPauseMenuBase::OnOptionClicked( const egUIWidgetEventInfo& ItemInfo )
{
	assert( m_MainOptionsStack.HasItems() );
	if( m_MainOptionsStack.HasItems() && ItemInfo.GridWidgetOwner )
	{
		const EGArray<exMenuOption>& Options = m_MainOptionsStack.Top().Choices;

		if( eg_crc( "MainOptions" ) == ItemInfo.WidgetId && Options.IsValidIndex( ItemInfo.GridIndex ) )
		{
			const exMenuOption& Option = Options[ItemInfo.GridIndex];
			if( Option.Callback )
			{
				(this->*Option.Callback)();
			}
			else if( Option.DisplayText == m_SpecialCaseGameSlot )
			{
				const eg_uint SaveSlot = m_CurrentSaves.IsValidIndex( ItemInfo.GridIndex ) ? m_CurrentSaves[ItemInfo.GridIndex] : m_SpecialSaveIdForNewGame;
				const eg_bool bHasSave = ExSaveMgr::Get().DoesSaveExist( GetClientOwner() , SaveSlot );
				if( bHasSave )
				{
					ExSaveMgr::Get().LoadSave( GetClientOwner() , SaveSlot );
				}
				else
				{
					OnSelectNewGame();
				}
			}
		}
		else
		{
			assert( false ); // What is this?
		}
	}
	else
	{
		assert( false ); // How'd we get here?
	}
}

void ExMainAndPauseMenuBase::OnOptionUpdated( const egUIWidgetEventInfo& ItemInfo )
{
	assert( m_MainOptionsStack.HasItems() );
	if( m_MainOptionsStack.HasItems() && ItemInfo.GridWidgetOwner )
	{
		const EGArray<exMenuOption>& Options = m_MainOptionsStack.Top().Choices;

		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

		if( eg_crc( "MainOptions" ) == ItemInfo.WidgetId && ItemInfo.Widget && Options.IsValidIndex( ItemInfo.GridIndex ) )
		{
			const exMenuOption& Option = Options[ItemInfo.GridIndex];
			if( Option.DisplayText == m_SpecialCaseGameSlot )
			{
				const eg_uint SaveSlot = m_CurrentSaves.IsValidIndex( ItemInfo.GridIndex ) ? m_CurrentSaves[ItemInfo.GridIndex] : m_SpecialSaveIdForNewGame;
				const eg_bool bHasSave = ExSaveMgr::Get().DoesSaveExist( GetClientOwner() , SaveSlot );
				const eg_bool bIsSaveCorrupted = bHasSave && ExSaveMgr::Get().IsSaveCorrupted( GetClientOwner() , SaveSlot );
				const eg_loc_text DisplayText = bHasSave ? ExSaveMgr::Get().GetFormattedSaveName( GetClientOwner() , SaveSlot ) : eg_loc_text(eg_loc("PauseMenuNewGameText","New Game"));
				const eg_loc_text DateText = bHasSave && !bIsSaveCorrupted ? ExSaveMgr::Get().GetFormattedSaveDateText( GetClientOwner() , SaveSlot ) : CT_Clear;
				ItemInfo.Widget->SetText( eg_crc("ButtonText") , bHasSave && !bIsSaveCorrupted ? CT_Clear : DisplayText );
				ItemInfo.Widget->SetText( eg_crc("SaveNameText") , bHasSave && !bIsSaveCorrupted ? DisplayText : CT_Clear );
				ItemInfo.Widget->SetText( eg_crc("DateText") , DateText );
				if( ItemInfo.IsNewlySelected() )
				{
					RefreshHints();
				}
			}
			else
			{
				ItemInfo.Widget->SetText( eg_crc("ButtonText") , eg_loc_text( Option.DisplayText ) );
				ItemInfo.Widget->SetText( eg_crc("SaveNameText") , CT_Clear );
				ItemInfo.Widget->SetText( eg_crc("DateText") , CT_Clear );
			}
		}
		else
		{
			// assert( false ); // What is this? Could be bug in grid widget when grid is shrunk the no longer visible stuff is getting this called.
		}
	}
	else
	{
		assert( false ); // How'd we get here?
	}
}

eg_bool ExMainAndPauseMenuBase::IsSaveGameHighlighted()
{
	if( m_MainOptionsStack.HasItems() )
	{
		const EGArray<exMenuOption>& Options = m_MainOptionsStack.Top().Choices;

		if( m_OptionsGridWidget && Options.IsValidIndex( m_OptionsGridWidget->GetSelectedIndex() ) )
		{
			const exMenuOption& Option = Options[m_OptionsGridWidget->GetSelectedIndex()];
			if( Option.DisplayText == m_SpecialCaseGameSlot )
			{
				const eg_uint SaveSlot = m_CurrentSaves.IsValidIndex( m_OptionsGridWidget->GetSelectedIndex() ) ? m_CurrentSaves[m_OptionsGridWidget->GetSelectedIndex()] : m_SpecialSaveIdForNewGame;
				const eg_bool bHasSave = ExSaveMgr::Get().DoesSaveExist( GetClientOwner() , SaveSlot );
				return bHasSave;
			}
		}
	}
	
	return false;
}

void ExMainAndPauseMenuBase::RefreshHints()
{
	ClearHints();
	if( IsSaveGameHighlighted() )
	{
		AddHint( CMDA_MENU_DELETE , eg_loc_text(eg_loc("DeleteSaveHintText","Delete Save Game")) );
	}
	if( m_MainOptionsStack.Len() > 1 )
	{
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("MainMenuHintBack","Back")) );
	}
}

void ExMainAndPauseMenuBase::OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice )
{
	if( ListenerParm == eg_crc("ConfirmQuit") && Choice == eg_crc("Yes") )
	{
		ExecuteQuitGame();
	}
	else if( ListenerParm == eg_crc("ConfirmWipeAchievements") && Choice == eg_crc("Yes") )
	{
		ExAchievements::Get().ResetAchievements();
	}
	else if( ListenerParm == eg_crc("ConfirmDelete") && Choice == eg_crc("Yes") )
	{
		ExSaveMgr::Get().DeleteSave( GetClientOwner() , m_SaveSlotToDelete );
		PopulateCurrentSaves();
		if( m_CurrentSaves.Len() > 1 )
		{
			EGArray<exMenuOption> MenuOptions;
			for( eg_int i=0; i < m_CurrentSaves.LenAs<eg_int>(); i++ )
			{
				MenuOptions.Append( exMenuOption( m_SpecialCaseGameSlot , nullptr ) );
			}
			m_MainOptionsStack.Top().Choices = MenuOptions;
			if( m_OptionsGridWidget )
			{
				m_OptionsGridWidget->RefreshGridWidget( m_MainOptionsStack.Top().Choices.LenAs<eg_uint>() );
			}
		}
		else
		{
			PopOptionSet();
		}
	}
}

void ExMainAndPauseMenuBase::PopulateRootOptions()
{
	m_MainOptionsStack.Clear();

	EGArray<exMenuOption> MenuOptions;

	PopulateRootOptions( MenuOptions );
	ClearHints();
	PushOptionSet( MenuOptions );
}

void ExMainAndPauseMenuBase::PopulateRootOptions( EGArray<exMenuOption>& MenuOptions )
{
	const eg_bool bCanContinue = !m_IsPause && ExSaveMgr::Get().DoesSaveExist( GetOwnerClient() , ExProfileData_GetLastSaveSlot() );
	const eg_bool bHasExtras = !m_IsPause;
	const eg_bool bCanLoadGame = !m_IsPause && ExSaveMgr::Get().DoAnySavesExist( GetOwnerClient() );
	
	if( m_IsPause )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuResumeText","Resume") , &ThisClass::OnSelectResume ) );
	}
	if( bCanContinue )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuContinueText","Continue") , &ThisClass::OnSelectContinue ) );
	}
	if( !m_IsPause )
	{
		if( bCanLoadGame )
		{
			MenuOptions.Append( exMenuOption( eg_loc("PauseMenuPlayText","Play") , &ThisClass::OnSelectPlay ) );
		}
		else
		{
			MenuOptions.Append( exMenuOption( eg_loc("PauseMenuNewGameText","New Game") , &ThisClass::OnSelectNewGame ) );
		}
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuExtrasText","Extras") , &ThisClass::OnSelectExtras ) );
	}
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuSettingsText","Settings") , &ThisClass::OnSelectOptions ) );
	if( m_IsPause )
	{
		const eg_bool bIsSaveAndQuit = GetGame() && GetGame()->IsSaveAndQuit();
		MenuOptions.Append( exMenuOption( bIsSaveAndQuit ? eg_loc("PauseMenuSaveAndQuitText","Save and Quit") : eg_loc("PauseMenuQuitText","Quit") , &ThisClass::OnSelectQuitGame ) );
	}
	if( !m_IsPause )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuExitGameText","Exit to Windows") , &ThisClass::OnSelectExitGame ) );
	}
	if( EX_CHEATS_ENABLED )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuDebugSubmenuText","Debug") , &ThisClass::OnSelectDebugSubmenu ) );
	}
}

void ExMainAndPauseMenuBase::PopulateSettingsOptions( EGArray<exMenuOption>& MenuOptions )
{
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuGameplaySettings","Gameplay") , &ThisClass::OnSelectGameplaySettings ) );
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuGraphicsSettings","Graphics") , &ThisClass::OnSelectGraphicsSettings ) );
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuAudioSettings","Audio") , &ThisClass::OnSelectAudioSettings ) );
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuControlsSettings","Controls") , &ThisClass::OnSelectControlsSettings ) );
	if( EX_CHEATS_ENABLED )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuDebugSettings","Debug") , &ThisClass::OnSelectDebugOptions ) );
	}
}

void ExMainAndPauseMenuBase::PushOptionSet( const EGArray<exMenuOption>& InOptions )
{
	const eg_bool bFirstSet = m_MainOptionsStack.IsEmpty();
	if( !bFirstSet )
	{
		m_MainOptionsStack.Top().LastSelected = m_OptionsGridWidget ? m_OptionsGridWidget->GetSelectedIndex() : 0;
	}
	m_MainOptionsStack.Push( InOptions );
	PlaySwap( bFirstSet );
}

void ExMainAndPauseMenuBase::PopOptionSet()
{
	ClearHints();
	m_MainOptionsStack.Pop();
	if( m_MainOptionsStack.Len() == 1 )
	{
		// If we are back to the root set it's possible the options changed due to deleting save games,
		// if that is the case we want to see the new options available.
		const eg_int PrevNumChoices = m_MainOptionsStack.Top().Choices.LenAs<eg_int>();
		m_MainOptionsStack.Top().Choices.Clear();
		PopulateRootOptions( m_MainOptionsStack.Top().Choices );
		if( PrevNumChoices != m_MainOptionsStack.Top().Choices.LenAs<eg_int>() )
		{
			m_MainOptionsStack.Top().LastSelected = 0;
		}
	}
	PlaySwap( false );
}

void ExMainAndPauseMenuBase::PlaySwap( eg_bool bFromEmpty )
{
	assert( m_SwapState == ex_menu_swap_s::None );
	if( m_SwapState == ex_menu_swap_s::None )
	{
		m_SwapTimeElapsed = 0.f;
		m_SwapState = bFromEmpty ? ex_menu_swap_s::MovingIn : ex_menu_swap_s::MovingOut;
		SetInputEnabled( false );
		if( bFromEmpty )
		{
			if( m_IsPause )
			{
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::MenuSlide );
			}
			OnSwap_PopulateOptions();
		}
		else
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::MenuSlide );
		}
	}
}

void ExMainAndPauseMenuBase::HandleSwapUpdate( eg_real DeltaTime )
{
	switch( m_SwapState )
	{
		case ex_menu_swap_s::None:
		{

		} break;

		case ex_menu_swap_s::MovingIn:
		case ex_menu_swap_s::MovingOut:
		{
			m_SwapTimeElapsed += DeltaTime;
			if( m_SwapTimeElapsed >= m_SwapDuration )
			{
				if( m_SwapState == ex_menu_swap_s::MovingOut )
				{
					m_SwapTimeElapsed = 0.f;
					m_SwapState = ex_menu_swap_s::MovingIn;
					OnSwap_PopulateOptions();
				}
				else
				{
					m_SwapTimeElapsed = 0.f;
					m_SwapState = ex_menu_swap_s::None;
					if( m_OptionsGridWidget )
					{
						m_OptionsGridWidget->SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform::BuildTranslation( 0.f , 0.f , 0.f ) );
					}
					if( m_OptionsScrollbarWidget )
					{
						m_OptionsScrollbarWidget->SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform::BuildTranslation( 0.f , 0.f , 0.f ) );
					}
					SetInputEnabled( true );
					OnSwap_Complete();
				}
			}
			else
			{
				eg_real SwapOffset = 0.f;
				if( m_SwapState == ex_menu_swap_s::MovingOut )
				{
					SwapOffset = EGMath_GetMappedCubicRangeValue( m_SwapTimeElapsed , eg_vec2(0.f,m_SwapDuration) , eg_vec2(0.f,m_SwapFinalOffset) );
				}
				else
				{
					SwapOffset = EGMath_GetMappedCubicRangeValue( m_SwapTimeElapsed , eg_vec2(0.f,m_SwapDuration) , eg_vec2(m_SwapFinalOffset,0.f) );
				}

				if( m_OptionsGridWidget )
				{
					m_OptionsGridWidget->SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform::BuildTranslation( SwapOffset , 0.f , 0.f ) );
				}
				if( m_OptionsScrollbarWidget )
				{
					m_OptionsScrollbarWidget->SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform::BuildTranslation( SwapOffset , 0.f , 0.f ) );
				}
			}

		} break;
	}
}

void ExMainAndPauseMenuBase::OnSwap_PopulateOptions()
{
	if( m_OptionsGridWidget )
	{
		m_OptionsGridWidget->ScrollToPos( 0.f , true );
		const eg_uint NewSize = m_MainOptionsStack.HasItems() ? m_MainOptionsStack.Top().Choices.LenAs<eg_uint>() : 0;
		m_OptionsGridWidget->RefreshGridWidget( NewSize );
		if( NewSize > 0 )
		{
			SetFocusedWidget( m_OptionsGridWidget , m_MainOptionsStack.Top().LastSelected , false );
		}
	}

	RefreshHints();
}

void ExMainAndPauseMenuBase::OnSwap_Complete()
{

}

void ExMainAndPauseMenuBase::PopulateCurrentSaves()
{
	m_CurrentSaves.Clear();
	ExSaveMgr::Get().GetSortedAvailableSaveSlots( GetOwnerClient() , m_CurrentSaves );

	const eg_bool bCanDoNewGame = ExSaveMgr::Get().FindFreeSaveSlot( GetOwnerClient() ) != 0;
	if( bCanDoNewGame )
	{
		m_CurrentSaves.Append( m_SpecialSaveIdForNewGame );
	}
}

void ExMainAndPauseMenuBase::ExecuteQuitGame()
{
	GetOwnerClient()->SDK_RunServerEvent( eg_crc( "QuitGame" ) );
	MenuStack_Clear();
	MenuStack_PushTo( eg_crc("MainMenu") );
}

void ExMainAndPauseMenuBase::OnSelectResume()
{
	MenuStack_Pop();
}

void ExMainAndPauseMenuBase::OnSelectContinue()
{
	ExSaveMgr::Get().LoadSave( GetOwnerClient() , ExProfileData_GetLastSaveSlot() );
}

void ExMainAndPauseMenuBase::OnSelectPlay()
{
	PopulateCurrentSaves();
	EGArray<exMenuOption> MenuOptions;
	for( eg_int i=0; i < m_CurrentSaves.LenAs<eg_int>(); i++ )
	{
		MenuOptions.Append( exMenuOption( m_SpecialCaseGameSlot , nullptr ) );
	}
	PushOptionSet( MenuOptions );
}

void ExMainAndPauseMenuBase::OnSelectNewGame()
{
	const eg_uint CreateSlot = ExSaveMgr::Get().FindFreeSaveSlot( GetOwnerClient() );

	if( 1 <= CreateSlot && CreateSlot <= EX_SAVE_SLOTS )
	{
		ExNewGameFlowMenu* NewGameFlowMenu = EGCast<ExNewGameFlowMenu>(MenuStack_PushTo( eg_crc("NewGameFlowMenu") ));
		if( NewGameFlowMenu )
		{
			NewGameFlowMenu->RunFromState( ExNewGameFlowMenu::ex_starting_s::CreateGame );
		}
	}
	else
	{
		ExDialogMenu_PushDialogMenu( GetOwnerClient() , exMessageDialogParms( eg_loc("TooManySaves","You have used all your save slots. You must delete a save in order to create a new game.") ) );
	}
}

void ExMainAndPauseMenuBase::OnSelectOptions()
{
	EGArray<exMenuOption> MenuOptions;
	PopulateSettingsOptions( MenuOptions );
	PushOptionSet( MenuOptions );
}

void ExMainAndPauseMenuBase::OnSelectDebugOptions()
{
	ExSettingsMenu::OpenMenu( GetClientOwner() , ex_settings_menu_page::Debug );
}

void ExMainAndPauseMenuBase::OnSelectExtras()
{
	EGArray<exMenuOption> MenuOptions;
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuViewIntroText","View Intro") , &ThisClass::OnSelectViewIntro ) );
	if( ExProfileData_GetEndingMovieUnlocked() )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuViewEndingText","View Ending") , &ThisClass::OnSelectViewEnding ) );
	}
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuCreditsText","Credits") , &ThisClass::OnSelectCredits ) );
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuViewClassicIntroText","View Classic '99 Intro") , &ThisClass::OnSelectViewClassicIntro ) );
	if( ExProfileData_GetCastleGameUnlocked() )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuPlayCastleText","Castle Game") , &ThisClass::OnPlayCastleGame ) );
	}
	PushOptionSet( MenuOptions );
}

void ExMainAndPauseMenuBase::OnSelectViewIntro()
{
	ExAnimationMenu::PlayMovie( GetClientOwner() , eg_crc("Intro") );
}

void ExMainAndPauseMenuBase::OnSelectViewEnding()
{
	ExAnimationMenu::PlayMovie( GetClientOwner() , eg_crc("Ending") );
}

void ExMainAndPauseMenuBase::OnSelectCredits()
{
	SetVisibilityForCredits( false );
	MenuStack_PushTo( eg_crc("CreditsMenu") );
}

void ExMainAndPauseMenuBase::OnSelectViewClassicIntro()
{
	ExAchievements::Get().UnlockAchievement( eg_crc("ACH_CLASSICMOVIE") );
	ExAnimationMenu::PlayMovie( GetClientOwner() , eg_crc("ClassicIntro") );
}

void ExMainAndPauseMenuBase::OnPlayCastleGame()
{
	MenuStack_PushTo( eg_crc("CastleGameMenu") );
}

void ExMainAndPauseMenuBase::OnSelectQuitGame()
{
	if( m_IsPause )
	{
		const eg_bool bIsSaveAndQuit = GetGame() && GetGame()->IsSaveAndQuit();
		if( bIsSaveAndQuit )
		{
			ExecuteQuitGame();
		}
		else
		{
			// Show dialog
			ExDialogMenu_PushDialogMenu( GetClientOwner() , 
				exYesNoDialogParms( 
					eg_loc("QuitFromDungeonWarningText","If you quit from the dungeon your progress since the last visit to town will be lost. Are you sure you want to quit?") ,
					this ,
					eg_crc("ConfirmQuit") ,
					true ) );
		}
	}
}

void ExMainAndPauseMenuBase::OnSelectExitGame()
{
	Engine_QueMsg( "engine.quit()" );
}

void ExMainAndPauseMenuBase::OnSelectDebugSubmenu()
{	
	EGArray<exMenuOption> MenuOptions;
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuDebugSettingsText","Debug Settings") , &ThisClass::OnSelectDebugOptions ) );
	if( m_IsPause )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuMapListText","Map List") , &ThisClass::OnSelectDebugWarpMenu ) );
	}
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuMonsterViewTextText","Monster Viewer") , &ThisClass::OnSelectDebugMonsterViewerMenu ) );
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuTestCombatText","Test Combat") , &ThisClass::OnSelectDebugCombatMenu ) );
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuDebugMenuText","Debug Menu") , &ThisClass::OnSelectDebugDebugMenu ) );
	MenuOptions.Append( exMenuOption( eg_loc("PauseMenuDebugWipeAchievements","Reset Achievements") , &ThisClass::OnSelectDebugResetAchievements ) );
	if( m_IsPause )
	{
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuDebugPartyCreate","Party Create") , &ThisClass::OnSelectDebugPartyCreateMenu ) );
		MenuOptions.Append( exMenuOption( eg_loc("PauseMenuDebugRoster","Roster Menu") , &ThisClass::OnSelectDebugRosterMenu ) );
	}
	PushOptionSet( MenuOptions );
}

void ExMainAndPauseMenuBase::OnSelectDebugWarpMenu()
{
	MenuStack_PushTo( eg_crc("DebugWarpMenu") );
}

void ExMainAndPauseMenuBase::OnSelectDebugMonsterViewerMenu()
{
	MenuStack_PushTo( eg_crc("DebugMonsterViewerMenu") );
}

void ExMainAndPauseMenuBase::OnSelectDebugCombatMenu()
{
	MenuStack_PushTo( eg_crc("CombatMenu") );
}

void ExMainAndPauseMenuBase::OnSelectDebugDebugMenu()
{
	MenuStack_PushTo( eg_crc("DebugMenu") );
}

void ExMainAndPauseMenuBase::OnSelectDebugPartyCreateMenu()
{
	MenuStack_PushTo( eg_crc("PartyCreateMenu") );
}

void ExMainAndPauseMenuBase::OnSelectDebugRosterMenu()
{
	MenuStack_PushTo( eg_crc("RosterMenu") );
}

void ExMainAndPauseMenuBase::OnSelectDebugResetAchievements()
{
	ExDialogMenu_PushDialogMenu( GetOwnerClient() , exYesNoDialogParms( eg_loc( "ConfirmWipeAchievements" , "Are you sure you want to reset your achievements?" ) , this , eg_crc("ConfirmWipeAchievements") , true ) );
}

void ExMainAndPauseMenuBase::OnSelectGameplaySettings()
{
	ExSettingsMenu::OpenMenu( GetClientOwner() , ex_settings_menu_page::Game );
}

void ExMainAndPauseMenuBase::OnSelectGraphicsSettings()
{
	ExSettingsMenu::OpenMenu( GetClientOwner() , ex_settings_menu_page::Graphics );
}

void ExMainAndPauseMenuBase::OnSelectAudioSettings()
{
	ExSettingsMenu::OpenMenu( GetClientOwner() , ex_settings_menu_page::Audio );
}

void ExMainAndPauseMenuBase::OnSelectControlsSettings()
{
	MenuStack_PushTo( eg_crc("ControlsSettingsMenu") );
}
