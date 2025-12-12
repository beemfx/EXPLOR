// (c) 2016 Beem Media

#include "ExChoiceWidgetMenu.h"
#include "ExUiSounds.h"
#include "ExBgComponent.h"
#include "EGUiGridWidget.h"

EG_CLASS_DECL( ExChoiceWidgetMenu )

ExChoiceWidgetMenu* ExChoiceWidgetMenu::PushMenu( EGMenu* OwnerMenu , const exConfig& Config )
{
	ExChoiceWidgetMenu* Out = nullptr;

	if( OwnerMenu && Config.Choices.Len() > 0 )
	{
		Out = EGCast<ExChoiceWidgetMenu>(OwnerMenu->MenuStack_PushTo( eg_crc("ChoiceWidgetMenu") ));
		if( Out )
		{
			Out->SetupConfig( Config );
		}
	}
	else
	{
		assert( false ); // Something was wrong with the call to open this choice widget.
	}

	return Out;
}

void ExChoiceWidgetMenu::SetupConfig( const exConfig& InConfig )
{
	m_Config = InConfig;

	const eg_vec4 CASCADE_START(0.f,0.f,0.f,1.f);
	const eg_vec3 CASCADE_OFFSET(10.f,-10.f,-10.f);

	if( m_Config.bPushCascaded )
	{
		ExChoiceWidgetMenu* OwnerMenu = EGCast<ExChoiceWidgetMenu>(GetParentMenu());
		if( OwnerMenu )
		{
			m_Config.Pose = OwnerMenu->m_Config.Pose;
			m_Config.Pose.TranslateThis( CASCADE_OFFSET );
		}
	}

	if( m_List )
	{
		// TODO: Currently this aligns the top of the list, may also want to be able to do bottom.
		m_List->SetOffset( EGUiWidget::eg_offset_t::PRE , m_Config.Pose );
	}

	if( m_Scrollbar )
	{
		m_Scrollbar->SetOffset( EGUiWidget::eg_offset_t::PRE , m_Config.Pose );
	}

	DoReveal( m_Config.Choices.LenAs<eg_uint>() );

	ExBgComponent* Bg = EGCast<ExBgComponent>(GetWidget(eg_crc("Background")));
	if( Bg )
	{
		Bg->SetOffset( EGUiWidget::eg_offset_t::PRE , m_Config.Pose );
	}
}

void ExChoiceWidgetMenu::OnInit()
{
	Super::OnInit();

	m_List = GetWidget<EGUiGridWidget>( eg_crc( "List" ) );
	if( m_List )
	{
		m_List->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
		m_List->OnItemPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
		m_List->RefreshGridWidget( 0 );
	}

	m_Scrollbar = GetWidget<EGUiWidget>( eg_crc("Scrollbar") );
}

void ExChoiceWidgetMenu::OnDeinit()
{
	Super::OnDeinit();
}

eg_bool ExChoiceWidgetMenu::OnInput( eg_menuinput_t InputType )
{
	if( InputType == eg_menuinput_t::BUTTON_BACK )
	{
		if( m_Config.bCanPop )
		{
			MenuStack_Pop();
			ChoiceAbortedDelegate.ExecuteIfBound( this );
		}
		return true;
	}

	return false;
}

void ExChoiceWidgetMenu::OnInputCmds( const struct egLockstepCmds& Cmds )
{
	if( m_Config.bForwardInputCmdsToParent )
	{
		EGMenu* ParentMenu = GetParentMenu();
		if( ParentMenu )
		{
			ParentMenu->OnInputCmds( Cmds );
		}
	}
}

void ExChoiceWidgetMenu::OnObjPressed( const egUIWidgetEventInfo& Info )
{
	if( !m_bIsRevealed ) return;

	if( Info.WidgetId == eg_crc( "List" ) )
	{
		ChoicePressedDelegate.ExecuteIfBound( this , Info.GridIndex );
	}
}

void ExChoiceWidgetMenu::OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
{
	ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

	if( ItemInfo.IsNewlySelected() )
	{
		ChoiceSelectedDelegate.ExecuteIfBound( this , ItemInfo.GridIndex );
	}

	if( ItemInfo.Widget && m_Config.Choices.IsValidIndex( ItemInfo.GridIndex ) )
	{
		ItemInfo.Widget->SetText( eg_crc("Text") , m_Config.Choices[ItemInfo.GridIndex] );
	}
}

void ExChoiceWidgetMenu::OnRevealComplete()
{
	Super::OnRevealComplete();
	m_bIsRevealed = true;
	if( m_List )
	{
		m_List->RefreshGridWidget( m_Config.Choices.LenAs<eg_uint>() );
		SetFocusedWidget( m_List , m_Config.Choices.IsValidIndex( m_Config.InitialSelection ) ? m_Config.InitialSelection : 0 , false );
		m_List->JumpToFinalScrollPos();
	}
}

void ExChoiceWidgetMenu::exConfig::SetPoseForPortrait( eg_int PortraitIndex , eg_bool bBigPortraits )
{
	if( bBigPortraits )
	{
		static const eg_real OFFSET_PER_CHOICE = 9.f*.8f;
		static const eg_real PORTRAIT_WIDTH = EX_PORTRAIT_WIDTH*EX_PORTRAIT_SCALE_BIG; // Size and scaling in this menu...
		static const eg_real BASE_VERTICAL_OFFSET = -7 * OFFSET_PER_CHOICE - 3.5f;
		const eg_real VerticalOffset = EG_Clamp<eg_int>( Choices.LenAs<eg_int>() - 1 , 0 , EX_CHOICE_WIDGET_VISIBLE_CHOICES ) * OFFSET_PER_CHOICE;

		Pose = eg_transform::BuildTranslation( -2.5f * PORTRAIT_WIDTH + PortraitIndex * PORTRAIT_WIDTH , BASE_VERTICAL_OFFSET + VerticalOffset , 0.f );
	}
	else
	{
		static const eg_real OFFSET_PER_CHOICE = 9.f*.8f;
		static const eg_real PORTRAIT_WIDTH = EX_PORTRAIT_WIDTH*EX_PORTRAIT_SCALE; // Size and scaling in this menu...
		static const eg_real BASE_VERTICAL_OFFSET = -9 * OFFSET_PER_CHOICE - 2.f;
		const eg_real VerticalOffset = EG_Clamp<eg_int>( Choices.LenAs<eg_int>() - 1 , 0 , EX_CHOICE_WIDGET_VISIBLE_CHOICES ) * OFFSET_PER_CHOICE;

		Pose = eg_transform::BuildTranslation( -2.5f * PORTRAIT_WIDTH + PortraitIndex * PORTRAIT_WIDTH , BASE_VERTICAL_OFFSET + VerticalOffset , 0.f );
	}
}

void ExChoiceWidgetMenu::exConfig::BoundToScreen( eg_real ScreenBottom )
{
	// Only dealing with going past the bottom of the screen for now since this is used in the options menu and that is the only risk.

	static const eg_real OFFSET_PER_CHOICE = 9.f*.8f;
	static const eg_real BASE_VERTICAL_OFFSET = -9 * OFFSET_PER_CHOICE - 2.f;
	
	const eg_int VisibleChoices = EG_Clamp<eg_int>( Choices.LenAs<eg_int>() - 1 , 0 , EX_CHOICE_WIDGET_VISIBLE_CHOICES );
	const eg_real VerticalOffset = VisibleChoices * OFFSET_PER_CHOICE;
	const eg_real BottomPos = Pose.GetTranslation().y - VerticalOffset;
	if( BottomPos < ScreenBottom )
	{
		Pose.TranslateThis( 0.f , ScreenBottom - BottomPos , 0.f );
	}
}
