// (c) 2020 Beem Media. All Rights Reserved.

#include "ExPartyCreateMenu.h"
#include "ExRoster.h"
#include "ExTextEditWidget.h"
#include "EGUiGridWidget2.h"
#include "ExPortraits.h"
#include "EGUiImageWidget.h"
#include "ExGlobalData.h"
#include "ExDialogMenu.h"

EG_CLASS_DECL( ExPartyCreateMenu )
EG_CLASS_DECL( ExPartyCreateMenuHighlighter )

void ExPartyCreateMenu::OnInit()
{
	Super::OnInit();

	SetIgnoreAltNav( true );

	m_HelpText = GetWidget<EGUiTextWidget>( eg_crc("HelpText") );
	m_BeginGameButton = GetWidget<EGUiButtonWidget>( eg_crc("CreateButton") );

	m_BaseAList.Append( ex_attr_t::STR );
	m_BaseAList.Append( ex_attr_t::MAG );
	m_BaseAList.Append( ex_attr_t::END );
	m_BaseAList.Append( ex_attr_t::SPD );

	m_CompAList.Append( ex_attr_t::HP );
	m_CompAList.Append( ex_attr_t::MP );
	m_CompAList.Append( ex_attr_t::DMG_ );
	m_CompAList.Append( ex_attr_t::DEF_ );
	m_CompAList.Append( ex_attr_t::AGR );

	m_ClassList.Append( ex_class_t::Warrior );
	m_ClassList.Append( ex_class_t::Thief );
	m_ClassList.Append( ex_class_t::Cleric );
	m_ClassList.Append( ex_class_t::Mage );

	ExPortraits::Get().GetPlayerPortraitIds( m_PortraitList );

	ExGame* Game = GetGame();

	m_PartyCreateInfos.Resize( ExRoster::PARTY_SIZE );

	exPartyCreateMenuMemberInfo& TemplateInfo = m_PartyCreateInfos[0];

	if( Game )
	{
		
		for( eg_int i=0; i<m_PartyCreateInfos.LenAs<eg_int>(); i++ )
		{
			exPartyCreateMenuMemberInfo& Mi = m_PartyCreateInfos[i];
			Mi.OwnerMenu = this;
			Mi.PartyIndex = i;
			ExFighter* PartyMember = Game->GetPartyMemberByIndex( i );
			if( PartyMember )
			{
				Mi.PartyMemberData = *PartyMember;
				Mi.RefreshAttrPointPool();
			}
			else
			{
				Mi.PartyMemberData = CT_Clear;
				Mi.AttrPointPool = ExGlobalData::Get().GetCharacterCreateAllocationPoints();
			}

			Mi.PartyMemberData.SetAttackType( ex_attack_t::MELEE );
			Mi.PartyMemberData.ResolveReplicatedData();
			if( i == 0 )
			{
				Mi.HighlightWidget = GetWidget<ExPartyCreateMenuHighlighter>( eg_string_crc(*EGSFormat8("Highlight{0}",i)) );
				Mi.PortraitWidget = GetWidget<EGUiImageWidget>( eg_string_crc(*EGSFormat8("Portrait{0}",i)) );
				Mi.PortraitTextWidget = GetWidget<EGUiButtonWidget>( eg_string_crc(*EGSFormat8("PortraitText{0}",i)) );
				Mi.NameWidget = GetWidget<ExTextEditWidget>( eg_string_crc(*EGSFormat8("Name{0}",i)) );
				Mi.NameBgWidget = GetWidget<EGUiWidget>( eg_string_crc(*EGSFormat8("NameBg{0}",i)) );
				Mi.ClassWidget = GetWidget<EGUiButtonWidget>( eg_string_crc(*EGSFormat8("Class{0}",i)) );
				Mi.EditableAttributesWidget = GetWidget<EGUiGridWidget2>( eg_string_crc(*EGSFormat8("EditableAttributes{0}",i)) );
				Mi.PointsToAllocateWidget = GetWidget<EGUiTextWidget>( eg_string_crc(*EGSFormat8("PointsToAllocateText{0}",i)) );
				Mi.StartingInfoWidget = GetWidget<EGUiTextWidget>( eg_string_crc(*EGSFormat8("StartingInfo{0}",i)) );
			}
			else
			{
				const eg_transform Offset = eg_transform::BuildTranslation( i*40.f , 0.f , 0.f );

				Mi.HighlightWidget = EGCast<ExPartyCreateMenuHighlighter>(DuplicateWidget( TemplateInfo.HighlightWidget ));
				if( Mi.HighlightWidget )
				{
					Mi.HighlightWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}

				Mi.PortraitWidget = EGCast<EGUiImageWidget>(DuplicateWidget( TemplateInfo.PortraitWidget ));
				if( Mi.PortraitWidget )
				{
					Mi.PortraitWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}

				Mi.PortraitTextWidget = EGCast<EGUiButtonWidget>( DuplicateWidget( TemplateInfo.PortraitTextWidget ) );
				if( Mi.PortraitTextWidget )
				{
					Mi.PortraitTextWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}

				Mi.NameWidget = EGCast<ExTextEditWidget>( DuplicateWidget( TemplateInfo.NameWidget ) );
				if( Mi.NameWidget )
				{
					Mi.NameWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}

				Mi.NameBgWidget = EGCast<EGUiWidget>( DuplicateWidget( TemplateInfo.NameBgWidget ) );
				if( Mi.NameBgWidget )
				{
					Mi.NameBgWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}

				Mi.ClassWidget = EGCast<EGUiButtonWidget>( DuplicateWidget( TemplateInfo.ClassWidget ) );
				if( Mi.ClassWidget )
				{
					Mi.ClassWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}

				Mi.EditableAttributesWidget = EGCast<EGUiGridWidget2>( DuplicateWidget( TemplateInfo.EditableAttributesWidget ) );
				if( Mi.EditableAttributesWidget )
				{
					Mi.EditableAttributesWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}

				Mi.PointsToAllocateWidget = EGCast<EGUiTextWidget>( DuplicateWidget( TemplateInfo.PointsToAllocateWidget ) );
				if( Mi.PointsToAllocateWidget )
				{
					Mi.PointsToAllocateWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}

				Mi.StartingInfoWidget = EGCast<EGUiTextWidget>( DuplicateWidget( TemplateInfo.StartingInfoWidget ) );
				if( Mi.StartingInfoWidget )
				{
					Mi.StartingInfoWidget->SetOffset( EGUiWidget::eg_offset_t::POST , Offset );
				}
			}

			if( Mi.NameWidget )
			{
				Mi.NameWidget->InitWidget( EX_CHARACTER_NAME_ALLOWED_CHARS , countof(Mi.PartyMemberData.LocName)-1 );
				Mi.NameWidget->SetText( Mi.PartyMemberData.LocName );
				Mi.PartyMemberData.LocName[0] = '\0'; // Clear name as the text widget handles this...
			}

			if( Mi.PortraitTextWidget )
			{
				Mi.PortraitTextWidget->SetFocusEvents( eg_crc("SelectArrow") , eg_crc("Deselect") );
			}

			if( Mi.ClassWidget )
			{
				Mi.ClassWidget->SetFocusEvents( eg_crc("SelectArrow") , eg_crc("Deselect") );
			}

			if( Mi.PortraitTextWidget )
			{
				Mi.PortraitTextWidget->OnPressedDelegate.Bind( this , &ThisClass::OnPropertyButtonClicked );
			}

			if( Mi.ClassWidget )
			{
				Mi.ClassWidget->OnPressedDelegate.Bind( this , &ThisClass::OnPropertyButtonClicked );
			}

			if( Mi.EditableAttributesWidget )
			{
				Mi.EditableAttributesWidget->CellChangedDelegate.Bind( this , &ThisClass::OnEditableAttributeChanged );
				Mi.EditableAttributesWidget->CellClickedDelegate.Bind( this , &ThisClass::OnEditableAttributeClicked );
			}

			if( Mi.StartingInfoWidget )
			{
				Mi.StartingInfoWidget->SetWordWrapEnabled( false );
			}

			Mi.RefreshView();
		}
	}

	if( m_BeginGameButton )
	{
		m_BeginGameButton->OnPressedDelegate.Bind( this , &ThisClass::OnBeginGameClicked );
	}

	m_SelectedPartyMember = -1;
	RefreshHighlightedPartyMember();

	exMessageDialogParms MessageParms( eg_loc("PartyCreateTutorial","Welcome to party creation. Here you will create the characters that will begin exploring the world. The pre-made party is suitable for completing the game with reasonable challenge. Party members may be created and swapped out at any time by visiting the Inn.") );
	MessageParms.bMuteIntro = true;
	ExDialogMenu_PushDialogMenu( GetOwnerClient() , MessageParms  );
}

void ExPartyCreateMenu::OnDeinit()
{
	Super::OnDeinit();
}

void ExPartyCreateMenu::OnActivate()
{
	Super::OnActivate();

	if( GetFocusedWidget() != m_BeginGameButton )
	{
		if( m_PartyCreateInfos.IsValidIndex(0) )
		{
			SetFocusedWidget( m_PartyCreateInfos[0].NameWidget , 0 , true );
		}
	}
}

eg_bool ExPartyCreateMenu::OnInput( eg_menuinput_t InputType )
{
	EGUiWidget* SelectedWidget = GetFocusedWidget();
	exPartyCreateMenuMemberInfo* Mi = GetMemberInfoForWidget( SelectedWidget );

	if( InputType == eg_menuinput_t::BUTTON_RIGHT || InputType == eg_menuinput_t::BUTTON_LEFT )
	{
		const eg_bool bInc = InputType == eg_menuinput_t::BUTTON_RIGHT;

		if( SelectedWidget )
		{
			if( Mi )
			{
				if( Mi->PortraitTextWidget == SelectedWidget )
				{
					Mi->TogglePortrait( bInc );
				}
				else if( Mi->ClassWidget == SelectedWidget )
				{
					Mi->ToggleClass( bInc );
				}
				else if( Mi->EditableAttributesWidget == SelectedWidget )
				{
					const eg_uint AttrIndex = Mi->EditableAttributesWidget->GetSelectedIndex();
					if( m_BaseAList.IsValidIndex( AttrIndex ) )
					{
						Mi->ChangeAttr( m_BaseAList[AttrIndex] , bInc ? 1 : -1 );
					}
				}
			}
		}
		return true;
	}

	if( InputType == eg_menuinput_t::BUTTON_DOWN && Mi && Mi->EditableAttributesWidget == SelectedWidget )
	{
		if( Mi->EditableAttributesWidget->GetSelectedIndex() == (m_BaseAList.Len()-1) )
		{
			SetFocusedWidget( m_BeginGameButton , 0 , true );
			return true;
		}
	}

	if( InputType == eg_menuinput_t::BUTTON_UP && m_BeginGameButton == SelectedWidget )
	{
		if( m_PartyCreateInfos.IsValidIndex( m_SelectedPartyMember ) )
		{
			SetFocusedWidget( m_PartyCreateInfos[m_SelectedPartyMember].EditableAttributesWidget , m_BaseAList.LenAs<eg_uint>()-1 , true );
			return true;
		}
	}

	return Super::OnInput( InputType );
}

void ExPartyCreateMenu::OnInputCmds( const struct egLockstepCmds& Cmds )
{
	if( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) || Cmds.WasMenuPressed( CMDA_MENU_PREV_PAGE ) )
	{
		const eg_bool bNext = Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE );

		if( m_PartyCreateInfos.HasItems() )
		{
			m_SelectedPartyMember = ( m_SelectedPartyMember + m_PartyCreateInfos.LenAs<eg_int>() + (bNext ? 1 : -1) ) % m_PartyCreateInfos.LenAs<eg_int>();
		}
		else
		{
			m_SelectedPartyMember = 0;
		}
		RefreshHighlightedPartyMember();
		MoveSelectionToNewPartyMember();
	}
}

void ExPartyCreateMenu::OnFocusedWidgetChanged( EGUiWidget* NewFocusedWidget , EGUiWidget* OldFocusedWidget )
{
	Super::OnFocusedWidgetChanged( NewFocusedWidget , OldFocusedWidget );

	if( exPartyCreateMenuMemberInfo* Info = GetMemberInfoForWidget( NewFocusedWidget ) )
	{
		m_SelectedPartyMember = Info->PartyIndex;
	}

	RefreshHighlightedPartyMember();
	RefreshHintText();
}

void ExPartyCreateMenu::HandleHighlighterHovered( ExPartyCreateMenuHighlighter* Highlighter )
{
	eg_int HoveredPartyMember = -1;
	for( eg_int i=0; i<m_PartyCreateInfos.LenAs<eg_int>(); i++ )
	{
		if( m_PartyCreateInfos[i].HighlightWidget == Highlighter )
		{
			HoveredPartyMember = i;
		}
	}

	if( m_PartyCreateInfos.IsValidIndex(HoveredPartyMember) && (HoveredPartyMember != m_SelectedPartyMember || GetFocusedWidget() == m_BeginGameButton) )
	{
		m_SelectedPartyMember = HoveredPartyMember;
		RefreshHighlightedPartyMember();
		MoveSelectionToNewPartyMember();
	}
}

exPartyCreateMenuMemberInfo* ExPartyCreateMenu::GetMemberInfoForWidget( EGObject* Widget )
{
	for( exPartyCreateMenuMemberInfo& Info : m_PartyCreateInfos )
	{
		if( Info.PortraitTextWidget == Widget || Info.NameWidget == Widget || Info.ClassWidget == Widget || Info.EditableAttributesWidget == Widget )
		{
			return &Info;
		}
	}

	return nullptr;
}

eg_int ExPartyCreateMenu::GetIndexOfPortrait( eg_string_crc PortraitId ) const
{
	for( eg_int i=0; i<m_PortraitList.LenAs<eg_int>(); i++ )
	{
		if( PortraitId == m_PortraitList[i] )
		{
			return i;
		}
	}

	assert( false );
	return 0;
}

eg_int ExPartyCreateMenu::GetIndexOfClass( ex_class_t ClassType ) const
{
	for( eg_int i=0; i<m_ClassList.LenAs<eg_int>(); i++ )
	{
		if( ClassType == m_ClassList[i] )
		{
			return i;
		}
	}

	assert( false );
	return 0;
}

void ExPartyCreateMenu::UpdateAttriubte( exPartyCreateMenuMemberInfo& Mi , EGUiGridWidgetItem* GridItem , eg_int ListIndex , EGArray<ex_attr_t>& List )
{
	if( GridItem && List.IsValidIndex( ListIndex ) )
	{
		const ex_attr_t Type = List[ListIndex];

		GridItem->SetText( eg_crc("NameText") , EGFormat( L"|SC(EX_ATTR)|{0}|RC()|" , ExCore_GetAttributeName( Type )) );

		const eg_loc_text ValueText = EGFormat( Mi.AttrPointPool < 0 ? L"|SC(RED)|{0}|RC()|" : L"{0}" , static_cast<eg_int>(Mi.PartyMemberData.GetAttrValue( Type , ex_fighter_get_attr_t::BaseValue )) );

		GridItem->SetText( eg_crc("ValueText") , ValueText );
	}
}

void ExPartyCreateMenu::OnEditableAttributeChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
{
	if( CellInfo.GridItem )
	{
		if( CellInfo.IsNewlySelected() )
		{
			CellInfo.GridItem->RunEvent( eg_crc("SelectArrow") );
			RefreshHintText();
		}
		if( CellInfo.IsNewlyDeselected() )
		{
			CellInfo.GridItem->RunEvent( eg_crc("Deselect") );
		}

		for( exPartyCreateMenuMemberInfo& Mi : m_PartyCreateInfos )
		{
			if( Mi.EditableAttributesWidget == CellInfo.Owner )
			{
				UpdateAttriubte( Mi , CellInfo.GridItem , CellInfo.GridIndex , m_BaseAList );	
				break;
			}
		}
	}
}

void ExPartyCreateMenu::OnEditableAttributeClicked( egUiGridWidgetCellClickedInfo2& CellInfo )
{
	const eg_real HIT_MIN = .36f;

	if( (CellInfo.HitPoint.x >= HIT_MIN || !CellInfo.bFromMouse) && m_BaseAList.IsValidIndex( CellInfo.GridIndex ) )
	{
		if( exPartyCreateMenuMemberInfo* Mi = GetMemberInfoForWidget( CellInfo.Owner ) )
		{
			ex_attr_t AttrType = m_BaseAList[CellInfo.GridIndex];
			eg_bool bInc = !CellInfo.bFromMouse || EGMath_GetMappedRangeValue( CellInfo.HitPoint.x , eg_vec2( HIT_MIN , 1.f ) , eg_vec2(0.f,1.f) ) > .5f;
			Mi->ChangeAttr(  AttrType , bInc ? 1 : -1 );
		}
	}
}

void ExPartyCreateMenu::OnPropertyButtonClicked( const egUIWidgetEventInfo& WidgetInfo )
{
	if( WidgetInfo.bFromMouse )
	{
		exPartyCreateMenuMemberInfo* Mi = GetMemberInfoForWidget( WidgetInfo.Widget );
		if( Mi )
		{
			if( Mi->PortraitTextWidget == WidgetInfo.Widget )
			{
				Mi->TogglePortrait( WidgetInfo.HitPoint.x >= .5f );
			}
			else if( Mi->ClassWidget == WidgetInfo.Widget )
			{
				Mi->ToggleClass( WidgetInfo.HitPoint.x >= .5f );
			}
		}
	}
}

void ExPartyCreateMenu::OnBeginGameClicked( const egUIWidgetEventInfo& Info )
{
	unused( Info );

	eg_bool bNotAllPointsAllocated = false;
	eg_bool bNameMissing = false;
	eg_bool bTooManPointsAllocated = false;

	for( exPartyCreateMenuMemberInfo& Mi : m_PartyCreateInfos )
	{
		if( Mi.AttrPointPool < 0 )
		{
			bTooManPointsAllocated = true;
		}

		if( Mi.NameWidget && !Mi.NameWidget->HasText() )
		{
			bNameMissing = true;
		}

		if(  Mi.AttrPointPool != 0 )
		{
			bNotAllPointsAllocated = true;
		}
	}

	if( bTooManPointsAllocated )
	{
		ExDialogMenu_PushDialogMenu( GetOwnerClient() , exMessageDialogParms( eg_loc("PartyCreateMenuTooManyPointsAllocated","Too many attribute points have been used on a character. You must reduce the number of points used in order to create the character.") ) );
	}
	else if( bNameMissing )
	{
		ExDialogMenu_PushDialogMenu( GetOwnerClient() , exMessageDialogParms( eg_loc("PartyCreateMenuNoName","A character name is empty. A character must have a name to be created.") ) );
	}
	else if( bNotAllPointsAllocated )
	{
		ExDialogMenu_PushDialogMenu( GetOwnerClient() , exYesNoDialogParms( eg_loc("PartyCreateMenuNotAllPointsAllocated","Some attribute points have not been allocated. This may increase the difficulty of the game. Are you sure you want to continue? (You can always create new party members and swap them by visiting the Inn.)") , this , eg_crc("DoCreation") , true ) );
	}
	else
	{
		ExDialogMenu_PushDialogMenu( GetOwnerClient() , exYesNoDialogParms( eg_loc("PartyCreateMenuConfirmCreation","Is this the party you want to begin the game with? (You can always create new party members and swap them by visiting the Inn.)") , this , eg_crc("DoCreation") , false ) );
	}
}

void ExPartyCreateMenu::CommitChangesAndLeaveMenu()
{
	ExGame* Game = GetGame();

	if( Game )
	{
		for( eg_int i=0; i<m_PartyCreateInfos.LenAs<eg_int>(); i++ )
		{
			exPartyCreateMenuMemberInfo& Mi = m_PartyCreateInfos[i];

			// Commit any data to the party member.
			Mi.PartyMemberData.SetAttackType( ex_attack_t::NONE );
			if( Mi.NameWidget )
			{
				Mi.NameWidget->GetText( Mi.PartyMemberData.LocName , countof(Mi.PartyMemberData.LocName) );
			}

			// Send changes to server
			Game->ClientUpdateRosterCharacterByPartyIndex( i , Mi.PartyMemberData );
		}

		Game->SDK_RunServerEvent( eg_crc("ServerNotifyEvent") , eg_crc("PartyCreated") );
	}

	assert( IsActive() );
	if( IsActive() )
	{
		MenuStack_Pop();
	}
}

void ExPartyCreateMenu::OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice )
{
	if( ListenerParm == eg_crc("DoCreation") && Choice == eg_crc("Yes") )
	{
		CommitChangesAndLeaveMenu();
	}
}

void ExPartyCreateMenu::RefreshHighlightedPartyMember()
{
	for( eg_int i=0; i<m_PartyCreateInfos.LenAs<eg_int>(); i++ )
	{
		if( m_PartyCreateInfos[i].HighlightWidget )
		{
			m_PartyCreateInfos[i].HighlightWidget->SetHighlightActive( i == m_SelectedPartyMember && GetFocusedWidget() != m_BeginGameButton );
		}
	}
}

void ExPartyCreateMenu::MoveSelectionToNewPartyMember()
{
	EGUiWidget* FocusedWidget = GetFocusedWidget();

	exPartyCreateMenuMemberInfo* SourceMi = GetMemberInfoForWidget( FocusedWidget );
	exPartyCreateMenuMemberInfo* DestMi = m_PartyCreateInfos.IsValidIndex( m_SelectedPartyMember ) ? &m_PartyCreateInfos[m_SelectedPartyMember] : nullptr;

	if( DestMi )
	{
		if( SourceMi && FocusedWidget )
		{
			if( FocusedWidget == SourceMi->NameWidget )
			{
				SetFocusedWidget( DestMi->NameWidget , 0 , true );
			}
			else if( FocusedWidget == SourceMi->PortraitTextWidget )
			{
				SetFocusedWidget( DestMi->PortraitTextWidget , 0 , true );
			}
			else if( FocusedWidget == SourceMi->ClassWidget )
			{
				SetFocusedWidget( DestMi->ClassWidget , 0 , true );
			}
			else if( FocusedWidget == SourceMi->EditableAttributesWidget )
			{
				eg_uint SelectedIndex = SourceMi->EditableAttributesWidget->GetSelectedIndex();
				SetFocusedWidget( DestMi->EditableAttributesWidget , SelectedIndex , true );
			}
		}
		else
		{
			SetFocusedWidget( DestMi->NameWidget , 0 , true );
		}
	}
}

void ExPartyCreateMenu::RefreshHintText()
{
	ClearHints();

	eg_bool bIsLeftRightWidget = false;

	eg_loc_text PrimaryHint;
	
	if( m_HelpText )
	{
		eg_loc_text HelpText;

		EGUiWidget* FocusedWidget = GetFocusedWidget();
		if( FocusedWidget )
		{
			exPartyCreateMenuMemberInfo* Mi = GetMemberInfoForWidget( FocusedWidget );
			if( Mi )
			{
				if( Mi->PortraitTextWidget == FocusedWidget )
				{
					HelpText = eg_loc_text(ExCore_GetHelpText( ex_character_help_text_t::Portrait ));
					bIsLeftRightWidget = true;
				}
				else if( Mi->NameWidget == FocusedWidget )
				{
					HelpText = eg_loc_text(ExCore_GetHelpText( ex_character_help_text_t::Name ));
					PrimaryHint = eg_loc_text(eg_loc("PartyCreateMenuEditTextHint","Edit Text"));
				}
				else if( Mi->ClassWidget == FocusedWidget )
				{
					HelpText = eg_loc_text(ExCore_GetClassDesc( Mi->PartyMemberData.GetClass() ));
					bIsLeftRightWidget = true;
				}
				else if( Mi->EditableAttributesWidget == FocusedWidget )
				{
					const eg_int SelectedIndex = Mi->EditableAttributesWidget->GetSelectedIndex();
					const ex_attr_t AttrType = m_BaseAList.IsValidIndex(SelectedIndex) ? m_BaseAList[SelectedIndex] : ex_attr_t::XP_;
					if( AttrType != ex_attr_t::XP_ )
					{
						const exClassDefaults Defaults( Mi->PartyMemberData.GetClass() );

						HelpText = EGFormat( 
							Defaults.GetMin(AttrType) == Defaults.GetMax(AttrType) ? eg_loc("PartyCreateMenu_AttrDesc_SingleRange","|SC(EX_ATTR)|{0}|RC()|: {1} |SC(EX_CLASS)|{2}|RC()| Limit: {3}") : eg_loc("PartyCreateMenu_AttrDesc","|SC(EX_ATTR)|{0}|RC()|: {1} |SC(EX_CLASS)|{2}|RC()| Limit: {3}-{4}")
							, ExCore_GetAttributeLongName( AttrType )
							, ExCore_GetAttributeDesc( AttrType )
							, ExCore_GetClassName( Mi->PartyMemberData.GetClass() )
							, Defaults.GetMin( AttrType )
							, Defaults.GetMax( AttrType ) );

						bIsLeftRightWidget = true;
					}
				}
			}
			else
			{
				// TODO: Begin Game Button... etc.
				if( m_BeginGameButton == FocusedWidget )
				{
					PrimaryHint = eg_loc_text(eg_loc("PartyCreateMenuBeginGameHint","Begin Exploring"));
				}
			}
		}

		m_HelpText->SetText( CT_Clear , HelpText );
	}

	if( PrimaryHint.GetLen() > 0 )
	{
		AddHint( CMDA_MENU_PRIMARY , PrimaryHint );
	}

	if( bIsLeftRightWidget )
	{
		AddHint( CMDA_EX_MENU_LEFTRIGHT , eg_loc_text(eg_loc("PartyCreateMenuChangeValueHint","Change Value")) );
	}

	AddHint( CMDA_MENU_PREV_PAGE , CT_Clear );
	AddHint( CMDA_MENU_NEXT_PAGE , eg_loc_text(eg_loc("PartyCreateMenuChangeCharacterHint","Switch Character")) );
}

//
// exPartyCreateMenuMemberInfo
//

void exPartyCreateMenuMemberInfo::RefreshView()
{
	if( PortraitWidget )
	{
		PortraitWidget->SetTexture( CT_Clear , CT_Clear , *ExPortraits::Get().FindInfo( PartyMemberData.PortraitId ).Filename.FullPath );
	}

	if( PortraitTextWidget && OwnerMenu )
	{
		PortraitTextWidget->SetText( eg_crc("Text") , EGFormat( eg_loc("PartyCreateMenuPortraitText","Portrait {0}") , OwnerMenu->GetIndexOfPortrait( PartyMemberData.PortraitId ) +1 ) );
	}

	if( ClassWidget )
	{
		ClassWidget->SetText( eg_crc("Text") , eg_loc_text(ExCore_GetClassName( PartyMemberData.GetClass() )) );
	}

	if( EditableAttributesWidget )
	{
		EditableAttributesWidget->RefreshGridWidget( OwnerMenu ? OwnerMenu->m_BaseAList.LenAs<eg_uint>() : 0 );
	}

	if( PointsToAllocateWidget )
	{
		const ex_attr_value MaxAllocationPoints = ExGlobalData::Get().GetCharacterCreateAllocationPoints();
		const eg_loc_text AllocText = EGFormat( AttrPointPool >= 0 ? eg_loc("PartyCreateMenuAttrPool","Unused Points: {0}") : eg_loc("PartyCreateMenuAttrPoolFull","Unused Points: |SC(RED)|{0}|RC()|") , AttrPointPool );
		PointsToAllocateWidget->SetText( CT_Clear , AllocText );
	}

	if( StartingInfoWidget )
	{
		eg_string_crc FormatStr = eg_loc("PartyCreateMenuStartingInfoFormatText","|SC(EX_H2)|Equipment|RC()|\n{0:EQUIPMENT}\n|SC(EX_H2)|Attributes|RC()|\n|SC(EX_ATTR)|HP|RC()| {0:ATTR:HP}\n|SC(EX_ATTR)|MP|RC()| {0:ATTR:MP}\n|SC(EX_ATTR)|DMG|RC()| {0:ATTR:DMG_}\n|SC(EX_ATTR)|DEF|RC()| {0:ATTR:DEF_}\n|SC(EX_ATTR)|AGR|RC()| {0:ATTR:AGR}");

		StartingInfoWidget->SetText( CT_Clear , EGFormat( FormatStr , PartyMemberData ) );
	}
}

void exPartyCreateMenuMemberInfo::TogglePortrait( eg_bool bNext )
{
	if( OwnerMenu && OwnerMenu->m_PortraitList.Len() > 0 )
	{
		eg_int PortraitIndex = OwnerMenu->GetIndexOfPortrait( PartyMemberData.PortraitId );
		PortraitIndex = ( PortraitIndex + OwnerMenu->m_PortraitList.LenAs<eg_int>() + (bNext ? 1 : -1) )%OwnerMenu->m_PortraitList.LenAs<eg_int>();
		PartyMemberData.PortraitId = OwnerMenu->m_PortraitList[PortraitIndex];
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		RefreshView();
	}
}

void exPartyCreateMenuMemberInfo::ToggleClass( eg_bool bNext )
{
	if( OwnerMenu && OwnerMenu->m_ClassList.Len() > 0 )
	{
		eg_int ClassIndex = OwnerMenu->GetIndexOfClass( PartyMemberData.GetClass() );
		ClassIndex = ( ClassIndex + OwnerMenu->m_ClassList.LenAs<eg_int>() + (bNext ? 1 : -1) )%OwnerMenu->m_ClassList.LenAs<eg_int>();

		const ex_class_t NewClass = OwnerMenu->m_ClassList[ClassIndex];

		const exClassDefaults Defaults( NewClass );

		ResetToClassDefaults( NewClass );

		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		RefreshView();
		OwnerMenu->RefreshHintText();
	}
}

void exPartyCreateMenuMemberInfo::ChangeAttr( ex_attr_t AttrType , eg_int Offset )
{
	const exClassDefaults Defaults( PartyMemberData.GetClass() );

	const ex_attr_value CurValue = PartyMemberData.GetAttrValue( AttrType , ex_fighter_get_attr_t::BaseValue );
	const ex_attr_value NewValue = CurValue + Offset;

	if( EG_IsBetween<ex_attr_value>( NewValue , Defaults.GetMin( AttrType ) , Defaults.GetMax( AttrType ) ) )
	{
		DoAttrChange( AttrType , Offset );
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		RefreshView();
	}
}

void exPartyCreateMenuMemberInfo::DoAttrChange( ex_attr_t Type , eg_int Delta ) // Does not make sure that the pool can handle this or that this is in range.
{
	AttrPointPool -= Delta;
	PartyMemberData.SetAttrValue( Type , PartyMemberData.GetAttrValue( Type , ex_fighter_get_attr_t::BaseValue ) + Delta );
	PartyMemberData.RestoreAllHP();
	PartyMemberData.RestoreAllMP();
	PartyMemberData.ResolveEquipmentBoosts();
}

void exPartyCreateMenuMemberInfo::ResetToClassDefaults( ex_class_t NewClass )
{
	const exClassDefaults Defaults( NewClass );

	auto ResolveClassChange = [this,&Defaults]( ex_attr_t Type , ex_attr_value Def , ex_attr_value Min , ex_attr_value Max ) -> void
	{
		unused( Min , Max );

		PartyMemberData.SetAttrValue( Type , Def );
	};

	for( ex_attr_t AttrType : OwnerMenu->m_BaseAList )
	{
		ResolveClassChange( AttrType , Defaults.GetDef( AttrType ) , Defaults.GetMin( AttrType ) , Defaults.GetMax( AttrType ) );
	}

	PartyMemberData.SetAttrValue( ex_attr_t::BHP , Defaults.GetDef( ex_attr_t::BHP ) );
	PartyMemberData.SetAttrValue( ex_attr_t::BMP , Defaults.GetDef( ex_attr_t::BMP ) );
	PartyMemberData.SetAttrValue( ex_attr_t::AGR , Defaults.GetDef( ex_attr_t::AGR ) );
	PartyMemberData.InitAs( NewClass , true );
	PartyMemberData.InitDefaultItemsAndSkills();
	PartyMemberData.ResolveEquipmentBoosts();

	RefreshAttrPointPool();
}

void exPartyCreateMenuMemberInfo::RefreshAttrPointPool()
{
	AttrPointPool = ExGlobalData::Get().GetCharacterCreateAllocationPoints();

	if( OwnerMenu )
	{
		for( ex_attr_t AttrType : OwnerMenu->m_BaseAList )
		{
			AttrPointPool -= PartyMemberData.GetAttrValue( AttrType , ex_fighter_get_attr_t::BaseValue );
		}
	}
}

void ExPartyCreateMenuHighlighter::SetHighlightActive( eg_bool bNewValue )
{
	m_EntObj.SetPalette( eg_vec4(1.f,1.f,1.f,bNewValue?1.f:0.f) );
}

eg_bool ExPartyCreateMenuHighlighter::OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured )
{
	unused( WidgetHitPoint , bIsMouseCaptured );

	HandleHovered();
	
	return true;
}

eg_bool ExPartyCreateMenuHighlighter::OnMouseMovedOn( const eg_vec2& WidgetHitPoint )
{
	unused( WidgetHitPoint );

	HandleHovered();
	
	return true;
}

void ExPartyCreateMenuHighlighter::HandleHovered()
{
	ExPartyCreateMenu* OwnerPartyCreate = EGCast<ExPartyCreateMenu>( GetOwnerMenu() );
	if( OwnerPartyCreate )
	{
		OwnerPartyCreate->HandleHighlighterHovered( this );
	}
}
