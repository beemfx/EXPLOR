// (c) 2021 Beem Media. All Rights Reserved.

#include "ExCastleGameMenu.h"
#include "ExTextRevealWidget.h"
#include "EGUiGridWidget2.h"
#include "ExGameSettings.h"

EG_CLASS_DECL( ExCastleGameMenu )

void ExCastleGameMenu::OnInit()
{
	Super::OnInit();

	m_TitleTextWidget = GetWidget<EGUiTextWidget>( eg_crc("TitleText") );
	m_StoryTextWidget = GetWidget<ExTextRevealWidget>( eg_crc("StoryText") );
	if( m_ChoicesWidget = GetWidget<EGUiGridWidget2>( eg_crc("Choices") ) )
	{
		m_ChoicesWidget->CellChangedDelegate.Bind( this , &ThisClass::OnChoicesCellChanged );
		m_ChoicesWidget->CellClickedDelegate.Bind( this , &ThisClass::OnChoicesCellClicked );
	}

	DoReveal();
	if( EGUiWidget* FadeWidget = GetWidget<EGUiWidget>(eg_crc("FadeWidget") ) )
	{
		FadeWidget->SetVisible( true );
	}
}

void ExCastleGameMenu::OnDeinit()
{
	Super::OnDeinit();
}

void ExCastleGameMenu::OnUpdate( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime , AspectRatio );

	if( m_SmState.ProcState == egsm_proc_state::TERMINAL )
	{
		MenuStack_Pop();
	}

	m_InputCountdown += DeltaTime;

	if( m_ChoicesWidget )
	{
		const eg_bool bWasTextFullyRevealed = m_bTextFullyRevealed;
		m_bTextFullyRevealed = ex_castle_s::WaitingForChoice == m_CastleState && m_StoryTextWidget && m_StoryTextWidget->IsFullyRevealed();

		if( bWasTextFullyRevealed != m_bTextFullyRevealed )
		{
			m_InputCountdown = 0.f;
		}

		m_ChoicesWidget->SetEnabled( m_bTextFullyRevealed && m_InputCountdown >= m_InputCountdownDuration );
		m_ChoicesWidget->SetVisible( m_bTextFullyRevealed && m_InputCountdown >= m_InputCountdownDuration );
	}

	const eg_bool bShouldBePlayingDialogSound = m_StoryTextWidget && m_StoryTextWidget->IsRevealing();

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

eg_bool ExCastleGameMenu::OnInput( eg_menuinput_t InputType )
{
	if( InputType == eg_menuinput_t::BUTTON_BACK )
	{
		// MenuStack_Pop();
		return true;
	}

	if( m_StoryTextWidget && !m_StoryTextWidget->IsFullyRevealed() )
	{
		if( InputType == eg_menuinput_t::BUTTON_PRIMARY )
		{
			m_StoryTextWidget->ForceFullReveal();
		}
		return true;
	}

	if( m_InputCountdown < m_InputCountdownDuration )
	{
		return false;
	}

	return false;
}

void ExCastleGameMenu::OnRevealComplete()
{
	Super::OnRevealComplete();

	m_CastleState = ex_castle_s::Running;
	EGSmRuntime_Run( m_SmState , eg_crc("CastleGame") , CT_Clear , this );
}

ISmRuntimeHandler::egsmNativeRes ExCastleGameMenu::SmOnNativeEvent( eg_string_crc EventName , const EGArray<egsm_var>& Parms , const EGArray<eg_string_crc>& Branches )
{
	unused( EventName , Parms );

	switch_crc( EventName )
	{
		case_crc("SetTitleText"):
		{
			if( m_TitleTextWidget )
			{
				m_TitleTextWidget->SetText( CT_Clear , eg_loc_text(Parms[0].as_crc()) );
			}
			return egsmDefault();
		} break;
		case_crc("DisplayTextAndChoices"):
		{
			if( m_StoryTextWidget )
			{
				m_StoryTextWidget->RevealText( eg_loc_text(Parms[0].as_crc()) , ExGameSettings_DialogTextSpeed.GetValue() );
			}
			m_CurrentChoices.Clear();
			m_CurrentChoices.Append( Branches.GetArray() , Branches.Len() );
			if( m_ChoicesWidget )
			{
				m_ChoicesWidget->RefreshGridWidget( m_CurrentChoices.LenAs<eg_uint>() );
				EGMenu::SetFocusedWidget( m_ChoicesWidget , 0 , false );
			}
			m_CastleState = ex_castle_s::WaitingForChoice;
			return egsmYield();
		} break;
	}
	
	return ISmRuntimeHandler::SmOnNativeEvent( EventName , Parms , Branches );
}

void ExCastleGameMenu::OnChoicesCellChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
{
	if( CellInfo.GridItem && m_CurrentChoices.IsValidIndex( CellInfo.GridIndex ) )
	{
		eg_loc_text LocText = eg_loc_text( m_CurrentChoices[CellInfo.GridIndex] );
		CellInfo.GridItem->SetText( eg_crc("LabelText"), LocText );
		CellInfo.GridItem->RunEvent( CellInfo.bIsSelected ? eg_crc("Select") : eg_crc("Deselect") );
	}
}

void ExCastleGameMenu::OnChoicesCellClicked( egUiGridWidgetCellClickedInfo2& CellInfo )
{
	if( m_CurrentChoices.IsValidIndex( CellInfo.GridIndex ) )
	{
		PlaySoundEvent( ExUiSounds::ex_sound_e::ConfirmDialogChoice );

		const eg_string_crc ChoiceMade = m_CurrentChoices[CellInfo.GridIndex];
		m_CurrentChoices.Clear( false );
		if( m_ChoicesWidget )
		{
			m_ChoicesWidget->SetVisible( false );
			m_ChoicesWidget->SetEnabled( false );
			m_ChoicesWidget->RefreshGridWidget( m_CurrentChoices.LenAs<eg_uint>() );
		}
		m_CastleState = ex_castle_s::Running;
		EGSmRuntime_Resume( m_SmState , ChoiceMade , this );
	}
}
