// (c) 2016 Beem Media. All Rights Reserved.

#include "ExCharacterCreateMenu.h"
#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "EGTextFormat.h"
#include "ExPortraits.h"
#include "ExCharacterPortraitWidget.h"
#include "EGRandom.h"
#include "ExUiSounds.h"
#include "ExKeyboardMenu.h"
#include "ExTextEditWidget.h"
#include "EGUiGridWidget.h"
#include "ExGlobalData.h"
#include "ExAchievements.h"


static eg_uint ExCharacterCreateMenu_RosterIndex = ExRoster::ROSTER_SIZE;

void ExCharacterCreateMenu_OpenCreateMenu( class EGClient* Client , eg_uint RosterIndex )
{
	ExCharacterCreateMenu_RosterIndex = RosterIndex;
	Client->SDK_PushMenu( eg_crc("CharacterCreateMenu") );
	ExCharacterCreateMenu_RosterIndex = ExRoster::ROSTER_SIZE;
}

class ExCharacterCreateMenu: public ExMenu , public IExDialogListener , public IExKeyboardMenuCb
{
	EG_CLASS_BODY( ExCharacterCreateMenu , ExMenu )

private:
	typedef EGArray<ex_attr_t> ExAttrTypeArray;

	enum class ex_sel_thing
	{
		NONE,
		NAME,
		CLASS,
		PORTRAIT,
		ATTRS,
		CREATE_BUTTON,
	};
private:
	eg_uint                   m_RosterIndex;
	ExFighter                 m_CreateCharacter;
	eg_size_t                 m_PortraitIndex;
	eg_size_t                 m_ClassIndex;
	ex_attr_value             m_AttrPointPool;
	EGUiWidget*               m_BaseAttributes;
	EGUiTextWidget*           m_StartingInfoWidget;
	ExCharacterPortraitWidget2* m_PortraitWidget2;
	ex_sel_thing              m_SelectedThing;

	EGArray<eg_string_crc>    m_PortraitList;
	EGArray<ex_class_t>       m_ClassList;

	EGArray<ex_attr_t>        m_BaseAList;
	EGArray<ex_attr_t>        m_CompAList;

public:
	ExCharacterCreateMenu(): ExMenu(), m_CreateCharacter( CT_Default )
	{
		SetIgnoreAltNav( true );
	}

	virtual void Refresh() override final
	{
		m_CreateCharacter.PortraitId = m_PortraitList[m_PortraitIndex];
		
		RefreshName();

		EGUiButtonWidget* ClassText = GetWidget<EGUiButtonWidget>( eg_crc("ClassText") );
		if( ClassText )
		{
			ClassText->SetFocusEvents( eg_crc("SelectArrow") , eg_crc("Deselect") );
			ClassText->SetText( eg_crc("Text") , eg_loc_text(ExCore_GetClassName( m_CreateCharacter.GetClass() ) ) );
		}

		EGUiButtonWidget* PortraitText = GetWidget<EGUiButtonWidget>( eg_crc("PortraitText") );
		if( PortraitText )
		{
			PortraitText->SetFocusEvents( eg_crc("SelectArrow") , eg_crc("Deselect") );
			PortraitText->SetText( eg_crc("Text") , EGFormat( eg_loc("CharacterCreatePortraitText","Portrait {0}") , (m_PortraitIndex+1) ) );
		}

		EGUiWidget* PointsToAllocateText = GetWidget( eg_crc("PointsToAllocateText") );
		if( PointsToAllocateText )
		{
			const ex_attr_value MaxAllocationPoints = ExGlobalData::Get().GetCharacterCreateAllocationPoints();
			PointsToAllocateText->SetText( eg_crc("") , EGFormat( m_AttrPointPool >= 0 ? eg_loc("AttrPool","Attribute Points Allocated: {0}/{1}") : eg_loc("AttrPoolFull","Attribute Points Allocated: |SetColor(RED)|{0}/{1}|RestoreColor()|") , MaxAllocationPoints - m_AttrPointPool , MaxAllocationPoints ) );
		}

		RefreshGrids();
		RefreshButtons();

		m_PortraitWidget2 = EGCast<ExCharacterPortraitWidget2>( GetWidget( eg_crc("CharacterPortrait") ) );
		if( m_PortraitWidget2 )
		{
			m_PortraitWidget2->InitPortrait( &m_CreateCharacter , 0 , nullptr );
		}

		RefreshStartingAttributes();
	}

	void RefreshName()
	{
		ExTextEditWidget* NameText = EGCast<ExTextEditWidget>(GetWidget(eg_crc("NameText")));
		if( NameText )
		{
			NameText->GetText( m_CreateCharacter.LocName , countof(m_CreateCharacter.LocName) );
		}

		if( m_PortraitWidget2 )
		{
			m_PortraitWidget2->Refresh();
		}
	}

	virtual void OnTextFromKeyboardMenu( const eg_loc_char* String ) override final
	{
		EGString_Copy( m_CreateCharacter.LocName , String , countof(m_CreateCharacter.LocName) );
	}

	void DoAttrChange( ex_attr_t Type , eg_int Delta ) // Does not make sure that the pool can handle this or that this is in range.
	{
		m_AttrPointPool -= Delta;
		m_CreateCharacter.SetAttrValue( Type , m_CreateCharacter.GetAttrValue( Type ) + Delta );
		m_CreateCharacter.RestoreAllHP();
		m_CreateCharacter.RestoreAllMP();
	}


	void OnClassChanged()
	{
		ex_class_t Class = m_ClassList[m_ClassIndex];
		const exClassDefaults Defaults( Class );

		auto ResolveClassChange = [this]( ex_attr_t Type , ex_attr_value Def , ex_attr_value Min , ex_attr_value Max ) -> void
		{
			unused( Min , Max );

			m_CreateCharacter.SetAttrValue( Type , Def );
		};

		for( ex_attr_t AttrType : m_BaseAList )
		{
			ResolveClassChange( AttrType , Defaults.GetDef( AttrType ) , Defaults.GetMin( AttrType ) , Defaults.GetMax( AttrType ) );
		}

		m_CreateCharacter.SetAttrValue( ex_attr_t::BHP , Defaults.GetDef( ex_attr_t::BHP ) );
		m_CreateCharacter.SetAttrValue( ex_attr_t::BMP , Defaults.GetDef( ex_attr_t::BMP ) );
		m_CreateCharacter.SetAttrValue( ex_attr_t::AGR , Defaults.GetDef( ex_attr_t::AGR ) );
		m_CreateCharacter.InitAs( Class , true );
		m_CreateCharacter.InitDefaultItemsAndSkills();
		m_CreateCharacter.ResolveEquipmentBoosts();

		RefreshAttrPointPool();

		Refresh();
		UpdateHelpText();
		RefreshStartingAttributes();
	}

	void RefreshAttrPointPool()
	{
		m_AttrPointPool = ExGlobalData::Get().GetCharacterCreateAllocationPoints();

		for( ex_attr_t AttrType : m_BaseAList )
		{
			m_AttrPointPool -= m_CreateCharacter.GetAttrValue( AttrType , ex_fighter_get_attr_t::BaseValue );
		}
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		RefreshName();
	}

	virtual void OnInit() override final
	{
		Super::OnInit();

		ExTextEditWidget* NameText = EGCast<ExTextEditWidget>( GetWidget( eg_crc( "NameText" ) ) );
		if( NameText )
		{
			NameText->InitWidget( EX_CHARACTER_NAME_ALLOWED_CHARS , countof(m_CreateCharacter.LocName)-1 );
		}
		m_RosterIndex = ExCharacterCreateMenu_RosterIndex;

		m_StartingInfoWidget = GetWidget<EGUiTextWidget>( eg_crc("StartingInfo") );

		//
		// Create a random character to start...
		//
		EGRandom Rng( CT_Default );

		m_BaseAList.Append( ex_attr_t::STR );
		m_BaseAList.Append( ex_attr_t::MAG );
		m_BaseAList.Append( ex_attr_t::END );
		//m_BaseAList.Append( ex_attr_t::ACC );
		m_BaseAList.Append( ex_attr_t::SPD );
		// m_BaseAList.Append( ex_attr_t::AGR );

		m_CompAList.Append( ex_attr_t::HP );
		m_CompAList.Append( ex_attr_t::MP );
		m_CompAList.Append( ex_attr_t::DMG_ );

		// m_CompAList.Append( ex_attr_t::PDMG );
		// m_CompAList.Append( ex_attr_t::FDMG );
		// m_CompAList.Append( ex_attr_t::WDMG );
		// m_CompAList.Append( ex_attr_t::EDMG );
		// m_CompAList.Append( ex_attr_t::ADMG );

		m_CompAList.Append( ex_attr_t::DEF_ );
		// m_CompAList.Append( ex_attr_t::PDEF );
		// m_CompAList.Append( ex_attr_t::FDEF );
		// m_CompAList.Append( ex_attr_t::WDEF );
		// m_CompAList.Append( ex_attr_t::EDEF );
		// m_CompAList.Append( ex_attr_t::ADEF );

		m_CompAList.Append( ex_attr_t::AGR );

		// m_CompAList.Append( ex_attr_t::HIT );
		// m_CompAList.Append( ex_attr_t::DG );
		// m_CompAList.Append( ex_attr_t::ATK );

		m_ClassList.Append( ex_class_t::Warrior );
		m_ClassList.Append( ex_class_t::Thief );
		m_ClassList.Append( ex_class_t::Mage );
		m_ClassList.Append( ex_class_t::Cleric );

		ExPortraits::Get().GetPlayerPortraitIds( m_PortraitList );
		if( m_PortraitList.Len() == 0 ) // Make sure there is at least one.
		{
			assert( false ); // No portraits loaded?
			m_PortraitList.Append( eg_crc("") );
		}

		m_PortraitIndex = static_cast<eg_uint>(Rng.GetRandomRangeI( 0 , static_cast<eg_int>(m_PortraitList.Len())-1 ));
		m_ClassIndex = static_cast<eg_uint>(Rng.GetRandomRangeI( 0 , static_cast<eg_int>(m_ClassList.Len())-1 ));
		EGString_Copy( m_CreateCharacter.LocName , L"" , countof(m_CreateCharacter.LocName) );

		m_BaseAttributes = GetWidget( eg_crc("BaseAttributes") );

		SetFocusedWidget( eg_crc("NameText") , 0 , false );

		// Initialize the attribute pool
		m_AttrPointPool = ExGlobalData::Get().GetCharacterCreateAllocationPoints();
		for( ex_attr_t AttrType : m_BaseAList )
		{
			m_CreateCharacter.SetAttrValue( AttrType , 0 );
		}

		OnClassChanged();

		RefreshHints();
		DoReveal();
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		switch( InputType )
		{
		case eg_menuinput_t::BUTTON_BACK:
		{
			MenuStack_Pop();
		} break;
		case eg_menuinput_t::BUTTON_RIGHT:
		{
			switch( m_SelectedThing )
			{
			case ex_sel_thing::NONE: break;
			case ex_sel_thing::NAME: break;
			case ex_sel_thing::CREATE_BUTTON: break;
			case ex_sel_thing::CLASS:
			{
				ChangeClass( 1 );
			} break;
			case ex_sel_thing::PORTRAIT:
			{
				ChangePortrait( 1 );
			} break;
			case ex_sel_thing::ATTRS:
			{
				EGUiWidget* SelObj = GetFocusedWidget();
				if( SelObj )
				{
					ex_attr_t AttrType = m_BaseAList[SelObj->GetSelectedIndex()];
					ChangeAttr( AttrType, 1 );
				}
			} break;
			}
		} break;
		case eg_menuinput_t::BUTTON_LEFT:
		{
			switch( m_SelectedThing )
			{
			case ex_sel_thing::NONE: break;
			case ex_sel_thing::NAME: break;
			case ex_sel_thing::CREATE_BUTTON: break;
			case ex_sel_thing::CLASS:
			{
				ChangeClass( -1 );
			} break;
			case ex_sel_thing::PORTRAIT:
			{
				ChangePortrait( -1 );
			} break;
			case ex_sel_thing::ATTRS:
			{
				EGUiWidget* SelObj = GetFocusedWidget();
				if( SelObj )
				{
					ex_attr_t AttrType = m_BaseAList[SelObj->GetSelectedIndex()];
					ChangeAttr( AttrType, -1 );
				}
			} break;
			}
		} break;
		default: return false;
		}

		return true;
	}

	void RefreshHints()
	{
		ClearHints();

		eg_bool bIsLeftRightWidget = false;
		eg_bool bIsTextEditWidget = false;
		eg_bool bCreateCharacter = false;

		switch( m_SelectedThing )
		{
		case ex_sel_thing::NONE:
			break;
		case ex_sel_thing::NAME:
			bIsTextEditWidget = true;
			break;
		case ex_sel_thing::CLASS:
		case ex_sel_thing::PORTRAIT:
		case ex_sel_thing::ATTRS:
			bIsLeftRightWidget = true;
			break;
		case ex_sel_thing::CREATE_BUTTON:
			bCreateCharacter = true;
			break;
		}

		if( bIsTextEditWidget )
		{
			AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("CharacterCreateMenuEditTextHint","Edit Text")) );
		}
		if( bCreateCharacter )
		{
			AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("CharacterCreateMenuCreateHint","Create Character")) );
		}
		if( bIsLeftRightWidget )
		{
			AddHint( CMDA_EX_MENU_LEFTRIGHT , eg_loc_text(eg_loc("CharacterCreateMenuChangeValueHint","Change Value")) );
		}
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("CharacterCreateMenuBackOut","Cancel")) );
	}

	void ChangePortrait( eg_int Offset )
	{
		m_PortraitIndex = (m_PortraitIndex + m_PortraitList.Len() + Offset)%m_PortraitList.Len();
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		Refresh();
		UpdateHelpText();
	}

	void ChangeClass( eg_int Offset )
	{
		m_ClassIndex = (m_ClassIndex + m_ClassList.Len() + Offset)%m_ClassList.Len();
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		OnClassChanged();
		Refresh();
	}

	void ChangeAttr( ex_attr_t AttrType , eg_int Offset )
	{
		const exClassDefaults Defaults( m_CreateCharacter.GetClass() );

		ex_attr_value CurValue = m_CreateCharacter.GetAttrValue( AttrType );
		ex_attr_value NewValue = CurValue + Offset;

		if( EG_IsBetween<ex_attr_value>( NewValue , Defaults.GetMin( AttrType ) , Defaults.GetMax( AttrType ) ) )
		{
			DoAttrChange( AttrType , Offset );
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
			Refresh();	
		}
	}

	virtual void OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice ) override final
	{
		switch_crc( ListenerParm )
		{
			case_crc("DoCreation"):
			{
				if( Choice == eg_crc("Yes") )
				{
					DoCreation();
				}
			} break;
		}
	}

	void DoCreation()
	{
		exRosterCharacterCreationData NewCharacter;
		NewCharacter.RosterIndex = m_RosterIndex;
		NewCharacter.Character = m_CreateCharacter;
		ExGame* Game = GetGame();
		if( Game )
		{
			Game->ClientCreateRosterCharacter( m_RosterIndex , m_CreateCharacter );
		}
		ExAchievements::Get().UnlockAchievement( eg_crc("ACH_CREATECHARACTER") );
		MenuStack_Pop();
	}

	void OnObjPressed( const egUIWidgetEventInfo& Info )
	{
		switch_crc( Info.WidgetId )
		{
			case_crc("CreateButton"):
			{
				eg_bool bCanCreate = true;

				if( bCanCreate && m_AttrPointPool < 0 )
				{
					ExDialogMenu_PushDialogMenu( GetOwnerClient() , exMessageDialogParms( eg_loc("CreateMsg_TooManyPointsAllocated","You have too many attribute points allocated to create this character.") ) );
					bCanCreate = false;
				}

				if( bCanCreate && m_CreateCharacter.LocName[0] == '\0' )
				{
					ExDialogMenu_PushDialogMenu( GetOwnerClient() , exMessageDialogParms( eg_loc("CreateMsg_NoName","Your character must have a name.") ) );
					bCanCreate = false;
				}

				if( bCanCreate && m_AttrPointPool != 0 )
				{
					ExDialogMenu_PushDialogMenu( GetOwnerClient() , exYesNoDialogParms( eg_loc("CreateMsg_NotAllPointsAllocated","You have not allocated all attribute points, are you sure you want to create this character?") , this , eg_crc("DoCreation") , true ) );
					bCanCreate = false;
				}

				if( bCanCreate )
				{
					DoCreation();
				}
			} break;
			case_crc("CancelButton"):
			{
				MenuStack_Pop();
			} break;
			case_crc("PortraitText"):
			{
				eg_bool bInc = Info.HitPoint.x >= .5f && Info.bFromMouse;
				ChangePortrait( bInc ? 1 : -1 );	
			} break;
			case_crc("ClassText"):
			{
				eg_bool bInc = Info.HitPoint.x >= .5f && Info.bFromMouse;
				ChangeClass( bInc ? 1 : -1 );
			} break;
			case_crc("BaseAttributes"):
			{
				const eg_real HIT_MIN = .36f;
				if( Info.HitPoint.x >= HIT_MIN && Info.bFromMouse)
				{
					ex_attr_t AttrType = m_BaseAList[Info.GridIndex];
					eg_bool bInc = !Info.bFromMouse ||EGMath_GetMappedRangeValue( Info.HitPoint.x , eg_vec2( HIT_MIN , 1.f ) , eg_vec2(0.f,1.f) ) > .5f;
					ChangeAttr(  AttrType , bInc ? 1 : -1 );
				}
			} break;
		}
	}


	ex_sel_thing GetSelectedThingByCrcId( eg_string_crc CrcId )
	{
		switch_crc( CrcId )
		{
			case_crc("PortraitText"): return ex_sel_thing::PORTRAIT;
			case_crc("ClassText"): return ex_sel_thing::CLASS;
			case_crc("NameText"): return ex_sel_thing::NAME;
			case_crc("BaseAttributes"): return ex_sel_thing::ATTRS;
			case_crc("CreateButton"): return ex_sel_thing::CREATE_BUTTON;
		}

		return ex_sel_thing::NONE;
	}

	virtual void OnFocusedWidgetChanged( EGUiWidget* NewFocusedWidget , EGUiWidget* OldFocusedWidget ) override
	{
		Super::OnFocusedWidgetChanged( NewFocusedWidget , OldFocusedWidget );
		m_SelectedThing = GetSelectedThingByCrcId( NewFocusedWidget ? NewFocusedWidget->GetId() : CT_Clear );

		UpdateHelpText();
		RefreshName();
		RefreshHints();
	}

	void UpdateHelpText()
	{
		EGUiWidget* HelpText = GetWidget( eg_crc("HelpText") );

		if( HelpText )
		{
			switch( m_SelectedThing )
			{
			case ex_sel_thing::ATTRS:
				// Set in OnGridItemChanged
				break;
			case ex_sel_thing::NONE: 
				HelpText->SetText( eg_crc("") , eg_loc_text( CT_Clear ) ); 
				break;
			case ex_sel_thing::NAME: 
				HelpText->SetText( eg_crc("") , eg_loc_text( ExCore_GetHelpText( ex_character_help_text_t::Name ) ) ); 
				break;
			case ex_sel_thing::PORTRAIT:
			{
				if( GetGame() )
				{
					const ExRoster& Roster = GetGame()->GetRoster();
					EGArray<const ExFighter*> SamePortraits;
					Roster.GetCharactersWithPortrait( m_CreateCharacter.PortraitId , SamePortraits );
					eg_loc_text PortraitHelpText( ExCore_GetHelpText( ex_character_help_text_t::Portrait ) );
					eg_loc_text FinalHelpText;
					if( SamePortraits.Len() == 1 && SamePortraits[0] )
					{
						eg_loc_text SharedText = EGFormat( eg_loc("CharacterCreateMenuCharacterSharesThis","|SC(EX_WARN)|Notice|RC()|: {0:NAME} shares this portrait.") , *SamePortraits[0] );
						FinalHelpText = EGFormat( L"{0}\n\n{1}" , PortraitHelpText , SharedText );
					}
					else if( SamePortraits.Len() > 1 )
					{
						eg_loc_text SharedText = EGFormat( eg_loc("CharacterCreateMenuCharacterMultipleSharesThis","|SC(EX_WARN)|Notice|RC()|: {0} characters share this portrait.") , SamePortraits.LenAs<eg_int>() );
						FinalHelpText = EGFormat( L"{0}\n\n{1}" , PortraitHelpText , SharedText );
					}
					else
					{
						FinalHelpText = PortraitHelpText;
					}
					HelpText->SetText( CT_Clear , FinalHelpText );
				}
			} break;
			case ex_sel_thing::CLASS:
				HelpText->SetText( eg_crc("") , eg_loc_text( ExCore_GetClassDesc( m_ClassList[m_ClassIndex] ) ) );
				break;
			case ex_sel_thing::CREATE_BUTTON:
				HelpText->SetText( CT_Clear , eg_loc_text( ExCore_GetHelpText( ex_character_help_text_t::CreateCharacter ) ) );
				break;
			}
		}
	}

	void RefreshStartingAttributes()
	{
		if( m_StartingInfoWidget )
		{
			m_CreateCharacter.SetAttackType( ex_attack_t::MELEE );
			eg_string_crc FormatStr = eg_loc("CharacterCreateMenuStartingInfoFormatText","|SC(EX_H2)|Equipment|RC()|\n{0:EQUIPMENT}\n|SC(EX_H2)|Attributes|RC()|\n|SC(EX_ATTR)|HP|RC()| {0:ATTR:HP}\n|SC(EX_ATTR)|MP|RC()| {0:ATTR:MP}\n|SC(EX_ATTR)|DMG|RC()| {0:ATTR:DMG_}\n|SC(EX_ATTR)|DEF|RC()| {0:ATTR:DEF_}\n|SC(EX_ATTR)|AGR|RC()| {0:ATTR:AGR}");
			m_StartingInfoWidget->SetText( CT_Clear , EGFormat( FormatStr , m_CreateCharacter ) );
			m_CreateCharacter.SetAttackType( ex_attack_t::NONE );
		}
	}

	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

		switch_crc(ItemInfo.WidgetId)
		{
			case_crc("BaseAttributes"):
			{
				const EGArray<ex_attr_t>& List = (eg_crc("BaseAttributes") == ItemInfo.WidgetId) ? m_BaseAList : m_CompAList;
				ex_attr_t Type = List[ItemInfo.GridIndex];
				ItemInfo.Widget->SetText( eg_crc("NameText") , EGFormat( L"|SC(EX_ATTR)|{0}|RC()|" , ExCore_GetAttributeName( Type )) );
				if( Type == ex_attr_t::HP )
				{
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( L"{0}/{1}" , static_cast<eg_int>(m_CreateCharacter.GetHP()) , static_cast<eg_int>(m_CreateCharacter.GetAttrValue( Type )) ) );
				}
				else if( Type == ex_attr_t::MP )
				{
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( L"{0}/{1}" , static_cast<eg_int>(m_CreateCharacter.GetMP()) , static_cast<eg_int>(m_CreateCharacter.GetAttrValue( Type )) ) );
				}
				else
				{
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( L"{0}" , static_cast<eg_int>(m_CreateCharacter.GetAttrValue( Type )) ) );
				}

				if( ItemInfo.IsNewlySelected() )
				{
					ItemInfo.Widget->RunEvent( eg_crc("SelectArrow") );
				}
				else if( ItemInfo.IsNewlyDeselected() )
				{
					ItemInfo.Widget->RunEvent( eg_crc("Deselect") );
				}

				if( ItemInfo.IsNewlySelected() )
				{
					EGUiWidget* HelpText = GetWidget( eg_crc("HelpText") );
					if( HelpText )
					{
						ex_class_t Class = m_CreateCharacter.GetClass();
						const exClassDefaults Defaults( Class );
						eg_loc_text LocText = EGFormat( 
							Defaults.GetMin(Type) == Defaults.GetMax(Type) ? eg_loc("CharacterCreateMenu_AttrDesc_SingleRange","|SC(EX_ATTR)|{0}|RC()|: {1}\n\n{2} Limit: {3}") : eg_loc("CharacterCreateMenu_AttrDesc","|SC(EX_ATTR)|{0}|RC()|: {1}\n\n{2} Limit: {3}-{4}")
							, ExCore_GetAttributeLongName( Type )
							, ExCore_GetAttributeDesc( Type )
							, ExCore_GetClassName( Class )
							, Defaults.GetMin( Type )
							, Defaults.GetMax( Type ) );
						HelpText->SetText( eg_crc("") , LocText );
					}
				}
			} break;
		}
	}

	void RefreshGrids()
	{
		auto RefreshGridWidget = [this]( eg_string_crc GridId , eg_uint Size ) -> void
		{
			EGUiGridWidget* GridWidget = GetWidget<EGUiGridWidget>( GridId );
			if( GridWidget )
			{
				GridWidget->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
				GridWidget->OnItemPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
				GridWidget->RefreshGridWidget( Size );
			}
		};

		RefreshGridWidget( eg_crc("BaseAttributes") , m_BaseAList.LenAs<eg_uint>() );
	}

	void RefreshButtons()
	{
		auto RefreshButton = [this]( eg_string_crc WidgetId ) -> void
		{
			EGUiButtonWidget* ButtonWidget = GetWidget<EGUiButtonWidget>( WidgetId );
			if( ButtonWidget )
			{
				ButtonWidget->OnPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
			}
		};

		RefreshButton( eg_crc("CreateButton") );
		RefreshButton( eg_crc("CancelButton") );
		RefreshButton( eg_crc("PortraitText") );
		RefreshButton( eg_crc("ClassText") );
		RefreshButton( eg_crc("BaseAttributes") );
	}
};

EG_CLASS_DECL( ExCharacterCreateMenu )
