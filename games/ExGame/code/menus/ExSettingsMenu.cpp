// (c) 2016 Beem Media

#include "ExSettingsMenu.h"
#include "ExDialogMenu.h"
#include "EGUiGridWidget2.h"
#include "EGEngine.h"
#include "EGSettings2.h"
#include "EGMenuStack.h"
#include "ExChoiceWidgetMenu.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( ExSettingsMenu )
EG_CLASS_DECL( ExSettingsMenuItemWidget )
EG_CLASS_DECL( ExSettingsMenuGridWidget )

static const eg_vec2 ExSettingsMenu_WidgetHitRange( .5f , 1.f );

void ExSettingsMenu::OpenMenu( EGClient* Client , ex_settings_menu_page Type )
{
	EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
	if( MenuStack )
	{
		ExSettingsMenu* SettingsMenu = EGCast<ExSettingsMenu>(MenuStack->Push( eg_crc("ExplorSettings1") ));
		if( SettingsMenu )
		{
			SettingsMenu->InitAs( Type );
		}
	}
}

void ExSettingsMenu::InitAs( ex_settings_menu_page InType )
{
	m_SettingsItems.Clear();

	eg_string_crc TitleText = CT_Clear;
	switch( InType )
	{
	case ex_settings_menu_page::Game:
		TitleText = eg_loc("SettingsMenuTitleGameplaySettings","Gameplay Settings");
		break;
	case ex_settings_menu_page::Graphics:
		TitleText = eg_loc("SettingsMenuTitleGraphicsSettings","Graphics Settings");
		break;
	case ex_settings_menu_page::Audio:
		TitleText = eg_loc("SettingsMenuTitleAudioSettings","Audio Settings");
		break;
	case ex_settings_menu_page::Debug:
		TitleText = eg_loc("SettingsMenuTitleDebugSettings","Debug Settings");
		break;
	}

	if( EGUiTextWidget* TitleTextWidget = GetWidget<EGUiTextWidget>(eg_crc("TitleText")) )
	{
		TitleTextWidget->SetText( CT_Clear , eg_loc_text(TitleText) );
	}
	
	if( InType == ex_settings_menu_page::Debug )
	{
		EGArray<EGSettingsVar*> GlobalSettings;

		EGSettingsVar::GetAllSettingsMatchingFlags( GlobalSettings , EGS_F_DEBUG_EDITABLE );

		for( EGSettingsVar* Var : GlobalSettings )
		{
			if( Var )
			{
				exSettingsMenuItem NewItem;
				NewItem.Var = Var;
				NewItem.NameStr = Var->GetDisplayName();
				NewItem.ValueStr = nullptr;
				NewItem.Type = ex_settings_menu_item_t::Default;
				if( Var->IsBoolType() )
				{
					NewItem.Type = ex_settings_menu_item_t::Bool;
				}
				else if( Var->IsToggleType() )
				{
					NewItem.Type = ex_settings_menu_item_t::DropDown;
				}
				NewItem.DescText = CT_Clear;
				NewItem.bApplyImmediately = true;
				NewItem.bNeedsVidRestart = Var->GetFlags().IsSet( EGS_F_NEEDSVRESTART );
				m_SettingsItems.Append( NewItem );
			}
		}
	}
	else
	{
		struct exCreateSettingsInfo
		{
			ex_settings_menu_page Page;
			eg_cpstr VarId;
			eg_string_crc Description;
		};

		static const exCreateSettingsInfo WantedSettings[] =
		{
			{ ex_settings_menu_page::Game , "ExGameSettings.DialogTextSpeed" , eg_loc("SettingsMenuDescDialogSpeed","The speed at which text appears when interacting with NPCs.") },
			{ ex_settings_menu_page::Game , "ExGameSettings.CombatTextSpeed" , eg_loc("SettingsMenuDescCombatTextSpeed","The speed at which the combat log is revealed. Affects the wait time between combat actions.") },
			{ ex_settings_menu_page::Game , "ExPlayer.EnableHeadBob" , eg_loc("SettingsMenuEnableHeadBobDesc","Enables camera bob while navigating the world. Disabling may aid players that experience motion sickness.") },
			{ ex_settings_menu_page::Game , "ExGameSettings.WalkAnimation" , eg_loc("SettingsMenuWalkAnimationDesc","How the camera animates while moving the world. Half-Step or None is reminiscent of classic dungeon crawlers and may aid players that experience motion sickness. Also affects door animations.") },
			{ ex_settings_menu_page::Game , "ExGameSettings.TurnAnimation" , eg_loc("SettingsMenuTurnAnimationDesc","How the camera animates while rotating the world. Half-Step or None is reminiscent of classic dungeon crawlers and may aid players that experience motion sickness.") },
			{ ex_settings_menu_page::Audio , "Volume_Effect" , eg_loc("SettingsMenuDescVolumeEffect","Volume of sound effects.") },
			{ ex_settings_menu_page::Audio , "Volume_Music" , eg_loc("SettingsMenuDescVolumeMusic","Volume of music.") },
			{ ex_settings_menu_page::Graphics , "EGRenderer_ScreenRes" , eg_loc("SettingsMenuDescScreenRes","The screen resolution of the game. Lower resolutions may improve performance on some computer hardware.") },
			{ ex_settings_menu_page::Graphics , "ExEngineInst.ClassicAspectRatio" , eg_loc("SettingsMenuDescClassicAspectRatio","Forces the game to run in a 4:3 aspect ratio. A classic experience, but not recommended.") },
			{ ex_settings_menu_page::Graphics , "Ex.ScreenMode" , eg_loc("SettingsMenuDescScreenMode","How the game is rendered to the display device.") },
			{ ex_settings_menu_page::Graphics , "VSyncEnabled" , eg_loc("SettingsMenuDescVSync","Enabling reduces screen tearing. Disabling may improve performance on some computer hardware.") },
			{ ex_settings_menu_page::Graphics , "ExWorldRenderPipe.Gamma" , eg_loc("SettingsMenuDescGamma","Adjusts brightness and darkness levels.") },
			{ ex_settings_menu_page::Graphics , "PostFXAA" , eg_loc("SettingsMenuDescPostFXAA","Enables an FXAA effect that reduces the appearance of jagged edges. Disabling may improve performance on some hardware.") },
			{ ex_settings_menu_page::Graphics , "PostMotionBlur" , eg_loc("SettingsMenuDescMotionBlur","Enables a motion blur effect. Disabling may improve performance on some hardware.") },
			{ ex_settings_menu_page::Graphics , "ExWorldRenderPipe.EnableDepthOfField" , eg_loc("SettingsMenuDescDepthOfFieldPost","Enables a blur effect for distant objects. Disabling may improve performance on some hardware.") },
			{ ex_settings_menu_page::Graphics , "ExWorldRenderPipe.EnableCombatPostProcessing" , eg_loc("SettingsMenuDescCombatPost","Enables a background effect during combat encounters. Disabling is not recommended but may improve performance on some hardware during combat encounters.") },
			// "Volume_Speech",
			// "EGRenderer_ShaderQuality",
			// "TextureUseMipMaps",
			// "InputConfig.MouseAxisSensitivity",
			// "InputConfig.GamepadAxisSensitivity",
			// "InputConfig.MouseSmoothSamples",
			// "InputConfig.GamepadSmoothSamples",
		};

		for( const exCreateSettingsInfo& SettingInfo : WantedSettings )
		{
			if( SettingInfo.Page == InType )
			{
				if( EGString_Equals( SettingInfo.VarId , "Ex.ScreenMode" ) )
				{
					exSettingsMenuItem NewItem;
					NewItem.Var = nullptr;
					NewItem.NameStr = eg_loc("ExSettingsMenuScreenModeName","Screen Mode");
					NewItem.ValueStr = nullptr;
					NewItem.Type = ex_settings_menu_item_t::DropDown;
					NewItem.SpecialType = ex_settings_menu_special_t::ScreenMode;
					NewItem.DescText = SettingInfo.Description;
					NewItem.bApplyImmediately = true;
					NewItem.bNeedsVidRestart = true;
					m_SettingsItems.Append( NewItem );
				}
				else
				{
					EGSettingsVar* Var = EGSettingsVar::GetSettingByName( SettingInfo.VarId );
					if( Var )
					{
						exSettingsMenuItem NewItem;
						NewItem.Var = Var;
						NewItem.NameStr = Var->GetDisplayName();
						NewItem.ValueStr = nullptr;
						NewItem.Type = ex_settings_menu_item_t::Default;
						if( Var->IsBoolType() )
						{
							NewItem.Type = ex_settings_menu_item_t::Bool;
						}
						else if( Var->IsToggleType() )
						{
							NewItem.Type = ex_settings_menu_item_t::DropDown;
						}
						NewItem.SpecialType = ex_settings_menu_special_t::None;
						NewItem.DescText = SettingInfo.Description;
						NewItem.bApplyImmediately = true;
						NewItem.bNeedsVidRestart = Var->GetFlags().IsSet( EGS_F_NEEDSVRESTART );
						m_SettingsItems.Append( NewItem );
					}
				}
			}
		}
	}

	if( m_SettingsWidget )
	{
		m_SettingsWidget->RefreshGridWidget( m_SettingsItems.LenAs<eg_uint>() );
		SetFocusedWidget( m_SettingsWidget , 0 , false );
	}

	// Hide desc and bg so they aren't there during reveal
	if( m_DescBgWidget )
	{
		m_DescBgWidget->SetVisible( false );
	}

	if( m_DescWidget )
	{
		m_DescWidget->SetVisible( false );
	}
}

void ExSettingsMenu::OnRevealComplete()
{
	Super::OnRevealComplete();

	if( m_DescBgWidget )
	{
		eg_int SelectedIndex = m_SettingsWidget ? m_SettingsWidget->GetSelectedIndex() : -1;
		m_DescBgWidget->SetVisible( m_SettingsItems.IsValidIndex( SelectedIndex ) && m_SettingsItems[SelectedIndex].DescText.IsNotNull() );
	}
}

void ExSettingsMenu::OnInit()
{
	Super::OnInit();

	m_SettingsWidget = GetWidget<ExSettingsMenuGridWidget>( eg_crc("SettingsItems") );
	m_DescBgWidget = GetWidget<EGUiWidget>( eg_crc("DescBg") );
	m_DescWidget = GetWidget<EGUiTextWidget>( eg_crc("Desc") );
	if( m_SettingsWidget )
	{
		m_SettingsWidget->CellChangedDelegate.Bind( this , &ThisClass::OnSettingsItemCellChanged );
		m_SettingsWidget->CellClickedDelegate.Bind( this , &ThisClass::OnSettingsItemCellClicked );
		m_SettingsWidget->CellSelectionChangedDelegate.Bind( this , &ThisClass::OnSettingsItemCellSelected );
	}

	RefreshHints();

	DoReveal();
}

eg_bool ExSettingsMenu::OnInput( eg_menuinput_t InputType )
{
	switch( InputType )
	{
	case eg_menuinput_t::BUTTON_BACK:
	{
		SaveAndExit();
	} break;
	case eg_menuinput_t::BUTTON_RIGHT:
	{
		if( m_SettingsWidget && m_SettingsWidget == GetFocusedWidget() )
		{
			eg_uint Index = m_SettingsWidget->GetSelectedIndex();
			if( m_SettingsItems.IsValidIndex( Index ) && m_SettingsItems[Index].Type == ex_settings_menu_item_t::Default )
			{
				ChangeAtIndex( Index , true );
			}
		}
	} return true;

	case eg_menuinput_t::BUTTON_LEFT:
	{
		if( m_SettingsWidget && m_SettingsWidget == GetFocusedWidget() )
		{
			eg_uint Index = m_SettingsWidget->GetSelectedIndex();
			if( m_SettingsItems.IsValidIndex( Index ) && m_SettingsItems[Index].Type == ex_settings_menu_item_t::Default )
			{
				ChangeAtIndex( Index , false );
			}
		}
	} return true;

	default: break;
	}

	return false;
}

void ExSettingsMenu::OnInputCmds( const struct egLockstepCmds& Cmds )
{
	if( Cmds.WasMenuPressed( CMDA_MENU_BTN2 ) )
	{
		// RestoreDefaults(); //
		// MenuStack_Pop();
	}
}

void ExSettingsMenu::OnUpdate( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime , AspectRatio );

	if( !IsInputEnabled() )
	{
		m_CountdownTilInputForVidRestart -= DeltaTime;
		if( m_CountdownTilInputForVidRestart <= 0.f )
		{
			SetInputEnabled( true );
		}
	}

	if( m_bNeedsVidRestart )
	{
		m_bNeedsVidRestart = false;
		m_CountdownTilInputForVidRestart = .25f;
		SetInputEnabled( false );
		Engine_QueMsg( "engine.VidRestart()" );
	}
}

void ExSettingsMenu::OnSettingsItemCellClicked( egUiGridWidgetCellClickedInfo2& CellInfo )
{
	//EGLogf( eg_log_t::General , "Clicked setting %u at <%f,%f>" , Info.Index , Info.HitPoint.x , Info.HitPoint.y );

	// We only care about clicking on the right half:
	if( m_SettingsItems.IsValidIndex( CellInfo.GridIndex ) )
	{
		const exSettingsMenuItem& SettingItem = m_SettingsItems[CellInfo.GridIndex];

		const eg_bool bClickInRange = EG_IsBetween( CellInfo.HitPoint.x , ExSettingsMenu_WidgetHitRange.x , ExSettingsMenu_WidgetHitRange.y );
		const eg_bool bValidBoolClick = SettingItem.Type == ex_settings_menu_item_t::Bool && (bClickInRange || !CellInfo.bFromMouse);
		const eg_bool bValidDropDownClick = SettingItem.Type == ex_settings_menu_item_t::DropDown && (bClickInRange || !CellInfo.bFromMouse);
		if( bClickInRange || bValidBoolClick || bValidDropDownClick )
		{
			if( SettingItem.Type == ex_settings_menu_item_t::DropDown )
			{
				OpenContextMenu( CellInfo.GridIndex );
			}
			else if( SettingItem.Type == ex_settings_menu_item_t::Default || SettingItem.Type == ex_settings_menu_item_t::Bool )
			{
				const eg_real AdjClickPos = EGMath_GetMappedRangeValue( CellInfo.HitPoint.x , eg_vec2( ExSettingsMenu_WidgetHitRange.x , ExSettingsMenu_WidgetHitRange.y ) , eg_vec2( -1.f , 1.f ) );
				const eg_bool bInc = AdjClickPos >= 0.f;
				ChangeAtIndex( CellInfo.GridIndex , bInc );
			}
		}
	}
}

void ExSettingsMenu::OnSettingsItemCellChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
{

	if( CellInfo.GridItem )
	{
		ex_settings_menu_item_t Type = ex_settings_menu_item_t::Default;
		eg_bool bIsChecked = false;

		if( m_SettingsItems.IsValidIndex( CellInfo.GridIndex ) )
		{
			const exSettingsMenuItem& SettingItem = m_SettingsItems[CellInfo.GridIndex];

			Type = SettingItem.Type;

			CellInfo.GridItem->SetText( eg_crc( "NameText" ) , eg_loc_text( SettingItem.NameStr ) );

			if( Type == ex_settings_menu_item_t::Bool )
			{
				CellInfo.GridItem->SetText( eg_crc( "ValueText" ) , CT_Clear );
				bIsChecked = SettingItem.Var && SettingItem.Var->GetBoolValue();
			}
			else
			{
				if( SettingItem.SpecialType == ex_settings_menu_special_t::ScreenMode )
				{
					const eg_int Mode = GetFullScreenMode();
					EGArray<eg_loc_text> Texts;
					GetFullScreenModeTexts( Texts );
					eg_loc_text ValueText = Texts.IsValidIndex( Mode ) ? Texts[Mode] : CT_Clear;
					CellInfo.GridItem->SetText( eg_crc( "ValueText" ) , ValueText );
				}
				else if( SettingItem.Var )
				{
					// Get gamma to look more traditional
					if( EGString_Equals( SettingItem.Var->GetVarName() , "ExWorldRenderPipe.Gamma" ) )
					{
						const EGSettingsClampedInt* SettingAsClampedInt = static_cast<EGSettingsClampedInt*>(SettingItem.Var);
						const eg_int IntValue = SettingAsClampedInt->GetValue();
						const eg_int Int10sPart = IntValue >= 10 ? IntValue/10 : 0;
						const eg_int OnesPart = IntValue - (Int10sPart*10);
						eg_d_string16 FormattedGamma = EGSFormat16( L"{0}.{1}" , Int10sPart , OnesPart );
						eg_loc_text ValueText( *FormattedGamma );
						CellInfo.GridItem->SetText( eg_crc( "ValueText" ) , ValueText );
					}
					else
					{
						eg_loc_text ValueText = SettingItem.Var->ToLocText();
						CellInfo.GridItem->SetText( eg_crc( "ValueText" ) , ValueText );
					}
				}
			}
		}
		else
		{
			CellInfo.GridItem->SetText( eg_crc("NameText") , eg_loc_text(eg_loc("SettingsMenuInvalidText","INVALID")) );
			CellInfo.GridItem->SetText( eg_crc("ValueText") , eg_loc_text(eg_loc("SettingsMenuInvalidText","INVALID")) );
		}

		if( Type == ex_settings_menu_item_t::Bool )
		{
			CellInfo.GridItem->RunEvent( CellInfo.bIsSelected ? eg_crc("SelectBox") : eg_crc("Deselect") );
			CellInfo.GridItem->RunEvent( bIsChecked ? eg_crc("SetChecked") : eg_crc("SetUnchecked") );
		}
		else if( Type == ex_settings_menu_item_t::Default )
		{
			CellInfo.GridItem->RunEvent( CellInfo.bIsSelected ? eg_crc("SelectArrow") : eg_crc("Deselect") );
			CellInfo.GridItem->RunEvent( eg_crc("SetNoCheck") );
		}
		else
		{
			CellInfo.GridItem->RunEvent( CellInfo.bIsSelected ? eg_crc("SelectBox") : eg_crc("Deselect") );
			CellInfo.GridItem->RunEvent( eg_crc("SetNoCheck") );
		}
	}
}

void ExSettingsMenu::OnSettingsItemCellSelected( EGUiGridWidget2* GridOwner , eg_uint CellIndex )
{
	eg_string_crc Desc = CT_Clear;

	if( m_SettingsItems.IsValidIndex( CellIndex ) )
	{
		if( GridOwner && !GridOwner->IsAudioMuted() )
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
		}

		Desc = m_SettingsItems[CellIndex].DescText;
	}
	
	if( m_DescBgWidget )
	{
		m_DescBgWidget->SetVisible( Desc.IsNotNull() );
	}
	if( m_DescWidget )
	{
		m_DescWidget->SetText( CT_Clear , eg_loc_text(Desc) );
	}

	RefreshHints();
}

void ExSettingsMenu::ChangeAtIndex( eg_uint Index , eg_bool bInc )
{
	if( m_SettingsWidget && m_SettingsItems.IsValidIndex( Index ) )
	{
		const exSettingsMenuItem& SettingItem = m_SettingsItems[Index];

		if( SettingItem.Var )
		{
			if( bInc && SettingItem.Var->CanInc() )
			{
				SettingItem.Var->Inc();
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
				m_SettingsWidget->RefreshGridWidget( m_SettingsItems.LenAs<eg_int>() );

				if( SettingItem.bNeedsVidRestart )
				{
					m_bNeedsVidRestart = true;
				}
			}
			else if( !bInc && SettingItem.Var->CanDec() )
			{
				SettingItem.Var->Dec();
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
				m_SettingsWidget->RefreshGridWidget( m_SettingsItems.LenAs<eg_int>() );
				if( SettingItem.bNeedsVidRestart )
				{
					m_bNeedsVidRestart = true;
				}
			}
		}
	}
	else
	{
		assert( false ); // Index out of range!
	}
}

void ExSettingsMenu::SaveAndExit()
{
	Engine_QueMsg( "engine.SaveConfig()" );
	if( m_bNeedsVidRestart )
	{
		Engine_QueMsg( "engine.VidRestart()" );
	}
	MenuStack_Pop();
}

void ExSettingsMenu::OpenContextMenu( eg_uint Index )
{
	if( m_SettingsItems.IsValidIndex( Index ) )
	{
		static const eg_real YSPACING = 12.f;

		static const eg_real X_OFFSET = 45.f;
		static eg_real Y_OFFSET = 68.f - 22.f + YSPACING;

		const eg_real ScrollOffset = m_SettingsWidget ? m_SettingsWidget->GetScrollOffset() : 0.f;

		eg_vec2 Position( X_OFFSET , Y_OFFSET - Index*YSPACING + ScrollOffset ); //-Index * YSPACING );

		
		m_SettingForContext = m_SettingsItems[Index].Var;
		m_SettingForContextSpecialType = m_SettingsItems[Index].SpecialType;

		ExChoiceWidgetMenu::exConfig ChoiceConfig;
		ChoiceConfig.bCanPop = true;
		ChoiceConfig.Pose = eg_transform::BuildTranslation( eg_vec3( Position.x , Position.y , 0.f ) );

		if( m_SettingForContextSpecialType == ex_settings_menu_special_t::ScreenMode )
		{
			GetFullScreenModeTexts( ChoiceConfig.Choices );
			ChoiceConfig.InitialSelection = GetFullScreenMode();
		}
		else
		{
			if( m_SettingForContext )
			{
				ChoiceConfig.InitialSelection = m_SettingForContext->GetSelectedToggle();
				m_SettingForContext->GetToggleValues( ChoiceConfig.Choices );

				if( ChoiceConfig.Choices.IsEmpty() )
				{
					ChoiceConfig.Choices.Append( eg_loc_text(eg_loc("SettingsMenuInvalidText","INVALID")) );
				}
			}
		}

		ChoiceConfig.BoundToScreen( -100.f );

		ExChoiceWidgetMenu* ChoiceMenu = ExChoiceWidgetMenu::PushMenu( this , ChoiceConfig );
		if( ChoiceMenu )
		{
			ChoiceMenu->ChoicePressedDelegate.Bind( this , &ThisClass::OnContextMenuChoiceMade );
		}
	}
}

void ExSettingsMenu::OnContextMenuChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
{
	unused( SourceMenu );

	MenuStack_PopTo( this );

	if( m_SettingForContextSpecialType == ex_settings_menu_special_t::ScreenMode )
	{
		SetFullScreenMode( ChoiceIndex );
		m_bNeedsVidRestart = true;
	}
	else
	{
		if( m_SettingForContext && m_SettingForContext->GetSelectedToggle() != ChoiceIndex )
		{
			m_SettingForContext->SetSelectedToggle( ChoiceIndex );
			if( m_SettingForContext->GetFlags().IsSet( EGS_F_NEEDSVRESTART ) )
			{
				m_bNeedsVidRestart = true;
			}
		}
	}

	m_SettingForContext = nullptr;
	m_SettingForContextSpecialType = ex_settings_menu_special_t::None;
	if( m_SettingsWidget )
	{
		m_SettingsWidget->RefreshGridWidget( m_SettingsItems.LenAs<eg_uint>() );
	}
}

void ExSettingsMenu::RefreshHints()
{
	ClearHints();

	if( m_SettingsWidget && m_SettingsItems.IsValidIndex( m_SettingsWidget->GetSelectedIndex() ) )
	{
		const exSettingsMenuItem& CurrentSetting = m_SettingsItems[m_SettingsWidget->GetSelectedIndex()];

		if( CurrentSetting.Type == ex_settings_menu_item_t::DropDown )
		{
			AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("SettingsMenuDropDownHint","Change Value")) );
		}
		else if( CurrentSetting.Type == ex_settings_menu_item_t::Bool )
		{
			AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("SettingsMenuBoolHint","Change Value")) );
		}
		else if( CurrentSetting.Type == ex_settings_menu_item_t::Default )
		{
			AddHint( CMDA_EX_MENU_LEFTRIGHT , eg_loc_text(eg_loc("SettingsMenuDefaultHint","Change Value")) );
		}
	}

	AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("SettingsMenuSaveAndExitHint","Back")) );

	// AddHint( CMDA_MENU_BTN2 , eg_loc_text( eg_loc( "SettingsRestoreDefaultsHint" , "Restore Defaults" ) ) );
}

void ExSettingsMenu::GetFullScreenModeTexts( EGArray<eg_loc_text>& Out )
{
	Out.Clear( false );
	Out.Append( eg_loc_text(eg_loc("ExSettingsMenuScreenModeWindow","Window")) );
	Out.Append( eg_loc_text(eg_loc("ExSettingsMenuScreenModeBorderlessWindow","Borderless Window")) );
	Out.Append( eg_loc_text(eg_loc("ExSettingsMenuScreenModeFullScreen","Exclusive Full Screen")) );
}

eg_int ExSettingsMenu::GetFullScreenMode()
{
	const eg_bool bWindowed = VideoConfig_IsWindowed.GetValueThreadSafe();
	const eg_bool bExclusiveFullScreen = VideoConfig_IsExclusiveFullScreen.GetValueThreadSafe();

	if( bWindowed )
	{
		return 0;
	}
	else if( bExclusiveFullScreen )
	{
		return 2;
	}

	return 1;
}

void ExSettingsMenu::SetFullScreenMode( eg_int ModeIndex )
{
	switch( ModeIndex )
	{
		case 0:
			VideoConfig_IsWindowed.SetValue( true );
			VideoConfig_IsExclusiveFullScreen.SetValue( false );
			break;
		case 1:
			VideoConfig_IsWindowed.SetValue( false );
			VideoConfig_IsExclusiveFullScreen.SetValue( false );
			break;
		case 2:
			VideoConfig_IsWindowed.SetValue( false );
			VideoConfig_IsExclusiveFullScreen.SetValue( true );
			break;
	}
}

eg_bool ExSettingsMenuGridWidget::OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured )
{
	const eg_bool bClickInRange = EG_IsBetween( WidgetHitPoint.x , ExSettingsMenu_WidgetHitRange.x , ExSettingsMenu_WidgetHitRange.y );
	if( !bClickInRange )
	{
		return false;
	}

	return Super::OnMouseMovedOver( WidgetHitPoint , bIsMouseCaptured );
}
