// (c) 2017 Beem Media

#include "ExConversationMenu.h"
#include "ExGame.h"
#include "ExTextRevealWidget.h"
#include "EGUiGridWidget.h"
#include "ExGameSettings.h"

EG_CLASS_DECL( ExConversationMenu )

void ExConversationMenu::Continue( ex_continue ContinueType )
{
	m_State = MAKING_CHOICE;

	if( ContinueType == ex_continue::ClearDialog )
	{
		m_State = BLANK_OVERLAY;
	}

	ContinueInternal();

	if( ContinueType == ex_continue::Close )
	{
		m_bEnd = true;
	}
}

void ExConversationMenu::RefreshNameplate()
{
	if( m_NameplateTxt )
	{
		m_NameplateTxt->SetText( eg_loc_text(GetGame()->GetCurScriptSpeaker()) );
	}
}


void ExConversationMenu::OnInit()
{
	Super::OnInit();

	assert( !m_bIsPlayingDialogSound );

	m_OverlayObj = GetWidget( eg_crc( "Overlay" ) );
	m_ChoicesObj = GetWidget( eg_crc( "Choices" ) );
	m_NameplateObj = GetWidget( eg_crc( "NameplateBg" ) );
	m_NameplateTxt = GetWidget( eg_crc( "NameplateTxt" ) );
	m_ConvText2Choices = GetWidget<ExTextRevealWidget>( eg_crc( "ConvText2Choices" ) );
	m_ConvText3Choices = GetWidget<ExTextRevealWidget>( eg_crc( "ConvText3Choices" ) );
	m_ConvText4Choices = GetWidget<ExTextRevealWidget>( eg_crc( "ConvText4Choices" ) );

	if( m_NameplateObj ) { m_NameplateObj->SetVisible( false ); }
	if( m_NameplateTxt ) { m_NameplateTxt->SetVisible( false ); }

	auto SetupConvChoices = []( ExTextRevealWidget* Widget ) -> void
	{
		assert( nullptr != Widget );
		if( Widget )
		{
			Widget->SetVisible( false );
			Widget->RevealText( eg_loc_text(eg_crc("")) , ExGameSettings_DialogTextSpeed.GetValue() );
		}
	};

	SetupConvChoices( m_ConvText2Choices );
	SetupConvChoices( m_ConvText3Choices );
	SetupConvChoices( m_ConvText4Choices );

	if( m_OverlayObj )
	{
		m_OverlayObj->RunEvent( eg_crc( "Play" ) );
	}

	HideHUD();
}

void ExConversationMenu::OnDeinit()
{
	if( m_bIsPlayingDialogSound )
	{
		m_bIsPlayingDialogSound = false;
		ExUiSounds::Get().SetTextRevealPlaying( false );
	}

	Super::OnDeinit();
	ShowHUD( false );
}

void ExConversationMenu::OnUpdate( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime, AspectRatio );

	m_MenuTime += DeltaTime;
	if( INTRO == m_State && m_MenuTime > .25f )
	{
		m_State = MAKING_CHOICE;
		if( m_NameplateObj ) { m_NameplateObj->SetVisible( true ); }
		if( m_NameplateTxt ) { m_NameplateTxt->SetVisible( true ); }
		ContinueInternal();
	}

	m_InputCountdown += DeltaTime;

	if( m_ChoicesObj )
	{
		const eg_bool bWasTextFullyRevealed = m_bTextFullyRevealed;
		m_bTextFullyRevealed = MAKING_CHOICE == m_State && m_CurConvTextWidget && m_CurConvTextWidget->IsFullyRevealed();

		if( bWasTextFullyRevealed != m_bTextFullyRevealed )
		{
			m_InputCountdown = 0.f;
		}

		m_ChoicesObj->SetEnabled( m_bTextFullyRevealed && m_InputCountdown >= m_InputCountdownDuration );
		m_ChoicesObj->SetVisible( m_bTextFullyRevealed && m_InputCountdown >= m_InputCountdownDuration );
	}

	if( MAKING_CHOICE == m_State && m_bEnd )
	{
		EndConversation();
		if( m_NameplateObj ) { m_NameplateObj->SetVisible( false ); }
		if( m_NameplateTxt ) { m_NameplateTxt->SetVisible( false ); }
		if( m_CurConvTextWidget ) { m_CurConvTextWidget->SetVisible( false ); }
	}

	if( OUTTRO == m_State && m_MenuTime > .25f && IsActive() )
	{
		MenuStack_Pop();
	}

	const eg_bool bShouldBePlayingDialogSound = m_CurConvTextWidget && m_CurConvTextWidget->IsRevealing();

	if( bShouldBePlayingDialogSound )
	{
		if( !m_bIsPlayingDialogSound )
		{
			m_bIsPlayingDialogSound = true;
			ExUiSounds::Get().SetTextRevealPlaying( true );
		}
	}
	else
	{
		if( m_bIsPlayingDialogSound )
		{
			m_bIsPlayingDialogSound = false;
			ExUiSounds::Get().SetTextRevealPlaying( false );
		}
	}
}

eg_bool ExConversationMenu::OnInput( eg_menuinput_t InputType )
{
	if( InputType == eg_menuinput_t::BUTTON_BACK )
	{
		// MenuStack_Pop();
		return true;
	}

	if( m_CurConvTextWidget && !m_CurConvTextWidget->IsFullyRevealed() )
	{
		if( InputType == eg_menuinput_t::BUTTON_PRIMARY )
		{
		
			m_CurConvTextWidget->ForceFullReveal();
		}
		return true;
	}

	if( m_InputCountdown < m_InputCountdownDuration )
	{
		return false;
	}

	return false;
}

void ExConversationMenu::OnObjPressed( const egUIWidgetEventInfo& Info )
{
	if( eg_crc("Choices") == Info.WidgetId )
	{
		m_State = WAITING_FOR_SERVER;
		PlaySoundEvent( ExUiSounds::ex_sound_e::ConfirmDialogChoice );
		RunServerEvent( egRemoteEvent( eg_crc( "OnConversationChoice" ), Info.GridIndex ) );
	}
}

void ExConversationMenu::OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
{
	ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

	if( eg_crc("Choices") == ItemInfo.WidgetId && ItemInfo.Widget )
	{
		eg_uint Index = ItemInfo.GridIndex;
		if( m_BranchOptions.IsValidIndex( Index ) )
		{
			eg_loc_text LocText = eg_loc_text( m_BranchOptions[Index] );
			ItemInfo.Widget->SetText( eg_crc( "BaseText" ), LocText );
			ItemInfo.Widget->SetText( eg_crc( "HlText" ), LocText );
		}
	}
}

void ExConversationMenu::RefreshChoices( eg_bool bClear )
{
	EGUiGridWidget* ChoicesWidget = GetWidget<EGUiGridWidget>( eg_crc("Choices") );
	if( ChoicesWidget )
	{
		ChoicesWidget->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
		ChoicesWidget->OnItemPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
		eg_uint NumChoices = bClear ? 0 : PopulateChoicesList();
		ChoicesWidget->RefreshGridWidget( NumChoices );
		ChoicesWidget->SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform::BuildTranslation(0.f,m_ChoiceOffset,0.f) );
		SetFocusedWidget( NumChoices > 0 ? ChoicesWidget : nullptr , 0 , false );
	}
}

void ExConversationMenu::ContinueInternal()
{
	eg_loc_text ConvText = EGFormat( GetGame()->GetCurScriptDialog(), GetGame() );

	if( m_State == BLANK_OVERLAY )
	{
		ConvText = eg_loc_text(CT_Clear);
	}

	SelectCurrentChoiceTextWidget();
	if( m_CurConvTextWidget )
	{
		m_CurConvTextWidget->RevealText( ConvText , ExGameSettings_DialogTextSpeed.GetValue() );
	}

	RefreshNameplate();

	// Now that we have the choices stored in ConvText, lets refresh the grid.
	RefreshChoices( false );
}

void ExConversationMenu::EndConversation()
{
	m_State = OUTTRO;
	m_MenuTime = 0.f;
	if( m_OverlayObj )
	{
		m_OverlayObj->RunEvent( eg_crc("Clear") );
	}

	if( m_CurConvTextWidget )
	{
		m_CurConvTextWidget->SetText( eg_crc( "" ), eg_loc_text( eg_crc( "" ) ) );
	}

	RefreshChoices( true );
}

eg_uint ExConversationMenu::PopulateChoicesList()
{
	eg_uint ListSizeOut = 0;

	if( MAKING_CHOICE == m_State )
	{
		GetGame()->GetCurScriptChoices( m_BranchOptions );
		ListSizeOut = static_cast<eg_uint>(m_BranchOptions.Len());
	}
	return ListSizeOut;
}

void ExConversationMenu::SelectCurrentChoiceTextWidget()
{
	static const eg_real CHOICE_HEIGHT = 10.5f;
	
	m_CurConvTextWidget = nullptr;

	auto HideTextWidget = []( ExTextRevealWidget* Widget ) -> void
	{
		if( Widget )
		{
			Widget->SetVisible( false );
		}
	};

	HideTextWidget( m_ConvText2Choices );
	HideTextWidget( m_ConvText3Choices );
	HideTextWidget( m_ConvText4Choices );

	const eg_uint NumChoices = PopulateChoicesList();
	if( NumChoices <= 2 )
	{
		m_CurConvTextWidget = m_ConvText2Choices;
		m_ChoiceOffset = 0;
	}
	else if( NumChoices == 3 )
	{
		m_CurConvTextWidget = m_ConvText3Choices;
		m_ChoiceOffset = CHOICE_HEIGHT;
	}
	else
	{
		m_CurConvTextWidget = m_ConvText4Choices;
		m_ChoiceOffset = CHOICE_HEIGHT*2.f;
	}

	if( m_CurConvTextWidget ) { m_CurConvTextWidget->SetVisible( true ); }
}
