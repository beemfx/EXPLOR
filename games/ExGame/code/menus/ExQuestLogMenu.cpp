// (c) 2016 Beem Media

#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "ExQuests.h"
#include "ExQuestItems.h"
#include "EGTextFormat.h"
#include "EGUiGridWidget.h"
#include "EGUiGridWidget2.h"
#include "ExGlobalData.h"

static const eg_real ExQuestLogMenu_PartyOptionsClickOffset = .5f;

class ExQuestLogMenu : public ExMenu
{
	EG_CLASS_BODY( ExQuestLogMenu , ExMenu )

private:

	enum class ex_party_option_t
	{
		EncounterDisposition,
		FrontLineSize,
	};

private:

	EGArray<exQuestInfo> m_ActiveQuests;
	EGArray<exQuestItemInfo>  m_QuestInventoryItems;
	EGArray<ex_party_option_t> m_PartyOptions;
	EGUiGridWidget2* m_PartyOptionsWidget;
	EGUiTextWidget* m_QuestDescriptionWidget;
	EGUiGridWidget2* m_QuestInventoryItemsWidget;
	EGUiGridWidget* m_QuestsWidget;
	const eg_bool m_bHidesHud = true;

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		m_QuestDescriptionWidget = GetWidget<EGUiTextWidget>( eg_crc("QuestDescriptionText") );
		m_QuestInventoryItemsWidget = GetWidget<EGUiGridWidget2>( eg_crc("QuestInventoryItemsList") );

		m_ActiveQuests.Clear();
		ExQuests::Get().GetActiveQuests( GetGame() , m_ActiveQuests );

		m_QuestInventoryItems.Clear();
		ExQuestItems::Get().GetOwnedItems( GetGame() , m_QuestInventoryItems );

		m_PartyOptions.Clear();
		m_PartyOptions.Append( ex_party_option_t::EncounterDisposition );
		m_PartyOptions.Append( ex_party_option_t::FrontLineSize );

		if( m_bHidesHud )
		{
			HideHUD();
		}

		if( m_QuestsWidget = GetWidget<EGUiGridWidget>( eg_crc("QuestItems") ) )
		{
			m_QuestsWidget->OnItemChangedDelegate.Bind( this , &ThisClass::OnQuestItemChanged );
			m_QuestsWidget->RefreshGridWidget( m_ActiveQuests.LenAs<eg_uint>() );
			SetFocusedWidget( m_QuestsWidget , 0 , false );
		}

		if( m_QuestInventoryItemsWidget )
		{
			const eg_int VisibleRows = m_QuestInventoryItemsWidget->GetNumVisibleRows();
			const eg_int VisibleColumns = m_QuestInventoryItemsWidget->GetNumVisibleColumns();
			const eg_int MinVisibleCells = VisibleRows*VisibleColumns;

			eg_int NumBagSlots = EG_Max<eg_uint>( MinVisibleCells , m_QuestInventoryItems.LenAs<eg_uint>() );
			if( NumBagSlots > MinVisibleCells )
			{
				// We always want a multiple of the number of columns
				for( eg_int i=0; i<VisibleColumns; i++ )
				{
					if( (NumBagSlots%VisibleColumns) != 0 )
					{
						NumBagSlots++;
					}
				}
			}

			m_QuestInventoryItemsWidget->CellChangedDelegate.Bind( this , &ThisClass::OnQuestInventoryItemChanged );
			m_QuestInventoryItemsWidget->CellSelectionChangedDelegate.Bind( this , &ThisClass::OnQuestInventoryItemSelected );
			m_QuestInventoryItemsWidget->RefreshGridWidget( NumBagSlots );
			m_QuestInventoryItemsWidget->SetEnabled( true );
		}

		if( m_PartyOptionsWidget = GetWidget<EGUiGridWidget2>( eg_crc("PartyOptions") ) )
		{
			m_PartyOptionsWidget->CellChangedDelegate.Bind( this , &ThisClass::OnPartyOptionItemChanged );
			m_PartyOptionsWidget->CellClickedDelegate.Bind( this , &ThisClass::OnPartyOptionItemClicked);
			m_PartyOptionsWidget->CellSelectionChangedDelegate.Bind( this , &ThisClass::OnPartyOptionSelected );
			m_PartyOptionsWidget->RefreshGridWidget( m_PartyOptions.LenAs<eg_uint>() );
		}

		OnEncounterDispositionChanged();
		if( GetGame() )
		{
			GetGame()->OnClientDispositionChangedDelegate.AddUnique( this , &ThisClass::OnEncounterDispositionChanged );
		}

		RefreshHints();
		DoReveal();
	}

	virtual void OnDeinit() override final
	{
		if( GetGame() )
		{
			GetGame()->OnClientDispositionChangedDelegate.RemoveAll( this );
		}

		if( m_bHidesHud )
		{
			ShowHUD( false );
		}
		
		Super::OnDeinit();
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override
	{
		Super::OnInputCmds( Cmds );

		if( HandlePossibleMenuToggle( ExMenu::ex_toggle_menu_t::QuestLog , Cmds ) )
		{
			return;
		}

		if( Cmds.WasMenuPressed( CMDA_MENU_BTN3 ) )
		{
			if( m_QuestsWidget && m_QuestsWidget == GetFocusedWidget() )
			{
				SetFocusedWidget( m_PartyOptionsWidget , 0 , true );
			}
			else if( m_PartyOptionsWidget && m_PartyOptionsWidget == GetFocusedWidget() )
			{
				SetFocusedWidget( m_QuestInventoryItemsWidget , 0 , true );
			}
			else if( m_QuestInventoryItemsWidget && m_QuestInventoryItemsWidget == GetFocusedWidget() )
			{
				SetFocusedWidget( m_QuestsWidget , 0 , true );
			}
		}
	}

	void RefreshHints()
	{
		ClearHints();

		eg_string_crc NextSectionHint = CT_Clear;

		if( m_QuestsWidget && m_QuestsWidget == GetFocusedWidget() )
		{
			NextSectionHint = eg_loc("QuestLogMenuSwitchToDispositionHint","Edit Disposition");
		}
		else if( m_PartyOptionsWidget && m_PartyOptionsWidget == GetFocusedWidget() )
		{
			NextSectionHint = eg_loc("QuestLogMenuSwitchToQuestInventoryItemsHint","View Quest Items");
			if( m_PartyOptions.IsValidIndex( m_PartyOptionsWidget->GetSelectedIndex() ) )
			{
				const ex_party_option_t SelectedPartyOption = m_PartyOptions[m_PartyOptionsWidget->GetSelectedIndex()];

				switch( SelectedPartyOption )
				{
					case ex_party_option_t::EncounterDisposition:
						AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("QuestLogMenuChangeBoolHint","Change Value")) );
						break;
					case ex_party_option_t::FrontLineSize:
						AddHint( CMDA_EX_MENU_LEFTRIGHT , eg_loc_text(eg_loc("QuestLogMenuChangeToggleHint","Change Value")) );
						break;
				}
			}
		}
		else if( m_QuestInventoryItemsWidget && m_QuestInventoryItemsWidget == GetFocusedWidget() )
		{
			NextSectionHint = eg_loc("QuestLogMenuSwitchToQuestsHint","View Quests Progress");
		}


		if( NextSectionHint.IsNotNull() )
		{
			AddHint( CMDA_MENU_BTN3 , eg_loc_text(NextSectionHint) );
		}
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_crc("BackOutText")) );
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}

		if( m_PartyOptionsWidget && m_PartyOptionsWidget == GetFocusedWidget() )
		{
			const eg_int SelectedIndex = m_PartyOptionsWidget->GetSelectedIndex();
			if( InputType == eg_menuinput_t::BUTTON_RIGHT || InputType == eg_menuinput_t::BUTTON_LEFT )
			{
				if( m_PartyOptions.IsValidIndex( SelectedIndex ) )
				{
					if( m_PartyOptions[SelectedIndex] == ex_party_option_t::FrontLineSize )
					{
						ToggleDisposition( m_PartyOptions[SelectedIndex] , InputType == eg_menuinput_t::BUTTON_RIGHT );
					}
				}
				return true;
			}
		}

		return false;
	}

	void OnQuestItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

		if( ItemInfo.Widget )
		{
			eg_loc_text LocText = EGFormat( eg_loc("txtTmpQuest" , "Quest {0}") , ItemInfo.GridIndex );
			eg_string_crc QuestStateEvent = CT_Clear;

			if( m_ActiveQuests.IsValidIndex( ItemInfo.GridIndex ) )
			{
				const exQuestInfo& Quest = m_ActiveQuests[ItemInfo.GridIndex];
				LocText = eg_loc_text( Quest.QuestName.Text );
				switch( Quest.GetStageStatus( GetGame() ) )
				{
				case ex_stage_status::NotStarted:
				case ex_stage_status::InProgress:
					QuestStateEvent = eg_crc("SetUnchecked");
					break;
				case ex_stage_status::Complete:
					QuestStateEvent = eg_crc("SetChecked");
					break;
				case ex_stage_status::Failed:
					QuestStateEvent = eg_crc("SetDisabled");
					break;
				}
				

				if( m_QuestDescriptionWidget )
				{
					if( ItemInfo.IsNewlySelected() )
					{
						m_QuestDescriptionWidget->SetText( CT_Clear , eg_loc_text(Quest.GetStageDesc( GetGame() )) );
					}
				}
			}

			ItemInfo.Widget->SetText( eg_crc("LabelText") , LocText );
			ItemInfo.Widget->RunEvent( QuestStateEvent );
		}		
	}

	void OnQuestInventoryItemChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
	{
		if( CellInfo.GridItem )
		{
			CellInfo.GridItem->RunEvent( CellInfo.bIsSelected ? eg_crc("SelectSilent") : eg_crc("Deselect") );

			if( m_QuestInventoryItems.IsValidIndex( CellInfo.GridIndex ) )
			{
				CellInfo.GridItem->RunEvent( eg_crc("SetStateIcon") );
				// CellInfo.GridItem->SetText( eg_crc("ItemName") , eg_loc_text( EGFormat( L"{0:NAME}" , &m_QuestItems[CellInfo.GridIndex] ) ));
				CellInfo.GridItem->SetTexture( eg_crc("Icon") , eg_crc("Texture") , *m_QuestInventoryItems[CellInfo.GridIndex].IconPath.FullPath );
			}
			else
			{
				CellInfo.GridItem->RunEvent( eg_crc("SetStateEmpty") );
				// CellInfo.GridItem->SetText( eg_crc("ItemName") , eg_loc_text( eg_loc("QuestLogMenuNoItemsText" , "No Quest Items") ) );	
				// CellInfo.GridItem->SetTexture( eg_crc("Icon") , eg_crc("Texture") , "/egdata/textures/T_EG_Transparent" );
			}
		}		
	}

	virtual void OnFocusedWidgetChanged( EGUiWidget* NewFocusedWidget , EGUiWidget* OldFocusedWidget ) override
	{
		Super::OnFocusedWidgetChanged( NewFocusedWidget , OldFocusedWidget );

		RefreshHints();
	}

	void OnQuestInventoryItemSelected( EGUiGridWidget2* GridOwner , eg_uint CellIndex )
	{
		unused( GridOwner );

		if( CellIndex != EGUiGrid2::INDEX_NONE )
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );

			if( m_QuestDescriptionWidget )
			{
				if( m_QuestInventoryItems.IsValidIndex( CellIndex ) )
				{
					m_QuestDescriptionWidget->SetText( CT_Clear , EGFormat( L"{0:NAME}\n{0:DESC}" , &m_QuestInventoryItems[CellIndex] ) );
				}
				else
				{
					m_QuestDescriptionWidget->SetText( CT_Clear , eg_loc_text( eg_loc("QuestLogMenuNoQuestItemText","No Item") ) );
				}
			}
		}

		RefreshHints();
	}

	void OnPartyOptionItemChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
	{
		if( CellInfo.GridItem )
		{
			if( m_PartyOptions.IsValidIndex( CellInfo.GridIndex) && GetGame() )
			{
				const ex_party_option_t Type = m_PartyOptions[CellInfo.GridIndex];

				switch( Type )
				{

				case ex_party_option_t::EncounterDisposition:
				{
					CellInfo.GridItem->SetText( eg_crc("NameText") , eg_loc_text(eg_loc("ThrillSeekerText","Thrill Seeker") ) );
					const eg_bool bChecked = GetGame()->GetEncounterDisposition() == ex_encounter_disposition::FirstProbabilitySet;
					CellInfo.GridItem->SetText( eg_crc("ValueText") , CT_Clear );
					CellInfo.GridItem->RunEvent( CellInfo.bIsSelected ? eg_crc("SelectBox") : eg_crc("Deselect") );
					CellInfo.GridItem->RunEvent( bChecked ? eg_crc("SetChecked") : eg_crc("SetUnchecked") );
				} break;

				case ex_party_option_t::FrontLineSize:
				{
					CellInfo.GridItem->SetText( eg_crc("NameText") , eg_loc_text(eg_loc("FrontLineText","Front Line Ranks") ) );
					CellInfo.GridItem->SetText( eg_crc("ValueText") , EGFormat( L"{0}" , GetGame()->GetFrontLineSize() ) );
					CellInfo.GridItem->RunEvent( CellInfo.bIsSelected ? eg_crc("SelectArrow") : eg_crc("Deselect") );
					CellInfo.GridItem->RunEvent( eg_crc("SetNoCheck") );
				} break;

				}
			}
		}

	}

	void OnPartyOptionItemClicked( egUiGridWidgetCellClickedInfo2& CellInfo )
	{
		if( m_PartyOptions.IsValidIndex( CellInfo.GridIndex) )
		{
			if( CellInfo.bFromMouse )
			{
				const eg_real MinClick = ExQuestLogMenu_PartyOptionsClickOffset;
				if( CellInfo.HitPoint.x >= MinClick )
				{
					const eg_real AdjClickPos = EGMath_GetMappedRangeValue( CellInfo.HitPoint.x , eg_vec2( MinClick , 1.f ) , eg_vec2( -1.f , 1.f ) );
					ToggleDisposition( m_PartyOptions[CellInfo.GridIndex] , AdjClickPos >= 0.f  );
				}
			}
			else
			{
				ToggleDisposition( m_PartyOptions[CellInfo.GridIndex] , true );
			}
		}
	}

	void OnPartyOptionSelected( EGUiGridWidget2* GridOwner , eg_uint CellIndex )
	{
		unused( GridOwner );

		if( CellIndex != EGUiGrid2::INDEX_NONE )
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );

			if( m_PartyOptions.IsValidIndex( CellIndex ) && m_QuestDescriptionWidget )
			{
				switch( m_PartyOptions[CellIndex] )
				{
					case ex_party_option_t::EncounterDisposition:
						m_QuestDescriptionWidget->SetText( CT_Clear , eg_loc_text( eg_loc("ThrillSeekerDescription","When Thrill Seeker mode is active the party will have random combat encounters at an increased rate. This may be useful if the party does not have enough XP or Gold to make progress in lower levels of the dungeon.") ) );
						break;
					case ex_party_option_t::FrontLineSize:
						m_QuestDescriptionWidget->SetText( CT_Clear , eg_loc_text( eg_loc("FrontLineDescription","The number of party members that will be on the front lines at the start of combat. These party members will be able to use melee attacks but are also vulnerable to the same.") ) );
						break;
				}
			}

			RefreshHints();
		}
	}

	void ToggleDisposition( ex_party_option_t Type , eg_bool bInc )
	{
		switch( Type )
		{
			case ex_party_option_t::EncounterDisposition:
			{
				RunServerEvent( egRemoteEvent( eg_crc("ServerNotifyEvent") , eg_crc("ToggleEncounterDisposition") ) );
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
			} break;

			case ex_party_option_t::FrontLineSize:
			{
				if( GetGame() )
				{
					const eg_int MIN_FL_SIZE = ExGlobalData::Get().GetCombatPlayerTeamFrontLineMinSize();
					const eg_int MAX_FL_SIZE = ExRoster::PARTY_SIZE;

					const eg_int CurrentFrontLineSize = GetGame() ? GetGame()->GetFrontLineSize() : 0;

					if( (bInc && CurrentFrontLineSize < MAX_FL_SIZE) || (!bInc && CurrentFrontLineSize > MIN_FL_SIZE ) )
					{
						RunServerEvent( egRemoteEvent( eg_crc("ServerNotifyEvent") , bInc ? eg_crc("ToggleFrontLineSizeUp") : eg_crc("ToggleFrontLineSizeDown") ) );
						ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
					}
				}
			} break;
		}
	}

	void OnEncounterDispositionChanged()
	{
		if( m_PartyOptionsWidget )
		{
			m_PartyOptionsWidget->RefreshGridWidget( m_PartyOptions.LenAs<eg_uint>() );
		}
	}
};

EG_CLASS_DECL( ExQuestLogMenu )

class ExQuestLogMenuDispositionGridWidget : public EGUiGridWidget2
{
	EG_CLASS_BODY( ExQuestLogMenuDispositionGridWidget , EGUiGridWidget2 )

	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured ) override
	{
		const eg_bool bClickInRange = EG_IsBetween( WidgetHitPoint.x , ExQuestLogMenu_PartyOptionsClickOffset , 1.f );
		if( !bClickInRange )
		{
			return false;
		}

		return Super::OnMouseMovedOver( WidgetHitPoint , bIsMouseCaptured );
	}
};

EG_CLASS_DECL( ExQuestLogMenuDispositionGridWidget )
