// (c) 2021 Beem Media. All Rights Reserved.

#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "EGUiGridWidget2.h"
#include "EGInput.h"
#include "EGEngine.h"

class ExControlSettingsGridWidget : public EGUiGridWidget2
{
	EG_CLASS_BODY( ExControlSettingsGridWidget , EGUiGridWidget2 )

public:
	
	EGDelegate<void,eg_int> ColumnHoveredDelegate;
	EGDelegate<void,eg_int> ColumnClickedDelegate;

protected:
	
	const eg_vec2 LeftSideRange = eg_vec2(.5f,.7f);
	const eg_vec2 RightSideRange = eg_vec2(.78f,1.f);

	eg_int m_ColumnClickedOn = -1;

protected:
	
	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured ) override
	{
		if( EG_IsBetween( WidgetHitPoint.x , LeftSideRange.x , LeftSideRange.y ) )
		{
			ColumnHoveredDelegate.ExecuteIfBound( 0 );
			return Super::OnMouseMovedOver( WidgetHitPoint , bIsMouseCaptured );
		}
		else if( EG_IsBetween( WidgetHitPoint.x , RightSideRange.x , RightSideRange.y ) )
		{
			ColumnHoveredDelegate.ExecuteIfBound( 1 );
			return Super::OnMouseMovedOver( WidgetHitPoint , bIsMouseCaptured );
		}

		return false;
	}

	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint ) override
	{
		if( EG_IsBetween( WidgetHitPoint.x , LeftSideRange.x , LeftSideRange.y ) )
		{
			m_ColumnClickedOn = 0;
			ColumnHoveredDelegate.ExecuteIfBound( 0 );
			return Super::OnMousePressed( WidgetHitPoint );
		}
		else if( EG_IsBetween( WidgetHitPoint.x , RightSideRange.x , RightSideRange.y ) )
		{
			m_ColumnClickedOn = 1;
			ColumnHoveredDelegate.ExecuteIfBound( 1 );
			return Super::OnMousePressed( WidgetHitPoint );
		}

		return false;
	}

	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* WidgetReleasedOn ) override
	{
		eg_int ColumnReleasedOn = -1;

		if( EG_IsBetween( WidgetHitPoint.x , LeftSideRange.x , LeftSideRange.y ) )
		{
			ColumnReleasedOn = 0;
			ColumnHoveredDelegate.ExecuteIfBound( 0 );
		}
		else if( EG_IsBetween( WidgetHitPoint.x , RightSideRange.x , RightSideRange.y ) )
		{
			ColumnReleasedOn = 1;
			ColumnHoveredDelegate.ExecuteIfBound( 1 );
		}

		if( ColumnReleasedOn == m_ColumnClickedOn && ColumnReleasedOn != -1 )
		{
			return Super::OnMouseReleased( WidgetHitPoint , WidgetReleasedOn );
		}

		if( m_Owner && m_Owner->GetMouseCapture() == this )
		{
			m_Owner->EndMouseCapture();
		}
		return true;
	}
};

EG_CLASS_DECL( ExControlSettingsGridWidget )

class ExControlsSettingsMenu : public ExMenu
{
	EG_CLASS_BODY( ExControlsSettingsMenu , ExMenu )

protected:
	
	enum class ex_control_item_t
	{
		None ,
		Binding ,
		Toggle ,
	};

	enum class ex_rebind_context
	{
		None ,
		Gameplay ,
		MenusDefault ,
		EngineDebug ,
	};

	enum class ex_warn_t
	{
		Safe ,
		Warn ,
	};

	struct exControlItem
	{
		ex_control_item_t ItemType = ex_control_item_t::None;
		eg_loc_text LabelText;
		eg_cmd_t Cmd = CMDA_UNK;
		ex_rebind_context RebindContext = ex_rebind_context::None;
		ex_rebind_context AssociatedContext = ex_rebind_context::None;
		eg_bool bLocked = false;
		eg_bool bDisplayed = true;
		ex_warn_t WarnType = ex_warn_t::Safe;
	};

protected:
	
	ExControlSettingsGridWidget* m_BindListWidget = nullptr;
	eg_bool m_bMuted = false;
	EGArray<exControlItem> m_AllControlItems;
	EGArray<eg_int> m_DisplayedControlItems;
	eg_int m_ColumnSelected = 0;
	eg_cmd_t m_RebindTargetCmd = CMDA_UNK;
	ex_rebind_context m_RebindTargetContext = ex_rebind_context::None;
	ex_rebind_context m_RebindTargetAssociatedContext = ex_rebind_context::None;
	eg_bool m_bRebindTargetIsGamepad = false;

protected:
	
	virtual void OnInit() override
	{
		Super::OnInit();

		BuildControlItemList();

		if( EGUiTextWidget* TitleText = GetWidget<EGUiTextWidget>( eg_crc("TitleText") ) )
		{
			TitleText->SetText( CT_Clear , eg_loc_text(eg_loc("ControlsSettingsMenuTitle","Controls")) );
		}

		if( m_BindListWidget = GetWidget<ExControlSettingsGridWidget>( eg_crc("BindList") ) )
		{
			m_BindListWidget->CellChangedDelegate.Bind( this , &ThisClass::OnBindListCellChanged );
			m_BindListWidget->CellClickedDelegate.Bind( this , &ThisClass::OnBindListCellClicked );
			m_BindListWidget->ColumnHoveredDelegate.Bind( this , &ThisClass::OnBindListColumnHovered );
			m_BindListWidget->CellSelectionChangedDelegate.Bind( this , &ThisClass::OnBindlistCellSelected );
		}

		UpdateHints();
		DoReveal();
	}

	virtual void OnDeinit() override
	{
		Engine_QueMsg( "engine.SaveConfig()" );

		Super::OnDeinit();
	}

	virtual void OnRevealComplete() override
	{
		Super::OnRevealComplete();

		if( m_BindListWidget )
		{
			RefreshGrid();
			m_bMuted = true;
			SetFocusedWidget( m_BindListWidget , 0 , false );
			m_bMuted = false;
		}
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override
	{
		if( Cmds.WasMenuPressed( CMDA_MENU_BTN3 ) )
		{
			if( EGClient* Client = GetClientOwner() )
			{
				Client->GetInputBindings() = EGInput::Get().GetDefaultBindings();
				RefreshGrid();
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::ConfirmDialogChoice );
			}
		}
		
		if( Cmds.WasMenuPressed( CMDA_MENU_DELETE ) )
		{
			if( const exControlItem* SelectedControlItem = GetSelectedControlItem() )
			{
				if( SelectedControlItem->ItemType == ex_control_item_t::Binding )
				{
					if( SelectedControlItem->bLocked )
					{
						ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( EGFormat( eg_loc("ControlSettingsMenuCannotUnbind","The binding for \"{0}\" cannot be changed.") , SelectedControlItem->LabelText ) ) );
						ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
					}
					else
					{
						if( EGClient* Client = GetClientOwner() )
						{
							if( m_ColumnSelected == 0 )
							{
								Client->GetInputBindings().Bind( SelectedControlItem->Cmd , GP_BTN_COUNT );
							}
							else
							{
								Client->GetInputBindings().Bind( SelectedControlItem->Cmd , KBM_BTN_COUNT );
							}

							RefreshGrid();
							ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::ConfirmDialogChoice );
						}
					}
				}
			}
		}
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}
		else if( InputType == eg_menuinput_t::BUTTON_LEFT )
		{
			if( m_ColumnSelected > 0 )
			{
				if( const exControlItem* SelectedControlItem = GetSelectedControlItem() )
				{
					if( SelectedControlItem->ItemType == ex_control_item_t::Binding )
					{
						ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
						m_ColumnSelected--;
						RefreshGrid();
						UpdateHints();
					}
				}
			}
			return true;
		}
		else if( InputType == eg_menuinput_t::BUTTON_RIGHT )
		{
			if( m_ColumnSelected < 1 )
			{
				if( const exControlItem* SelectedControlItem = GetSelectedControlItem() )
				{
					if( SelectedControlItem->ItemType == ex_control_item_t::Binding )
					{
						ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
						m_ColumnSelected++;
						RefreshGrid();
						UpdateHints();
					}
				}
			}
			return true;
		}

		return Super::OnInput( InputType );
	}

	eg_bool IsBindAllowed( ex_rebind_context RebindContext , ex_rebind_context AssociatedContext , eg_kbm_btn KbButton , eg_gp_btn GpButton , eg_loc_text* ReasonOut , eg_bool* bAssocatedIssueOut ) const
	{
		if( bAssocatedIssueOut )
		{
			*bAssocatedIssueOut = false;
		}

		if( KbButton == KBM_BTN_MOUSE_1 )
		{
			if( ReasonOut )
			{
				*ReasonOut = eg_loc_text(eg_loc("ControlSettingsMenuGameSystemUsage","Menu: Interact"));
			}
			return false;
		}

		if( KbButton == KBM_BTN_NUMLOCK || KbButton == KBM_BTN_RWIN || KbButton == KBM_BTN_LWIN || KbButton == KBM_BTN_SYSRQ )
		{
			if( ReasonOut )
			{
				*ReasonOut = eg_loc_text(eg_loc("ControlSettingsMenuOSUsage","Operating System Functions"));
			}
			return false;
		}

		if( const EGClient* Client = const_cast<ExControlsSettingsMenu*>(this)->GetClientOwner() )
		{
			const EGInputBindings& Bindings = Client->GetInputBindings();
			for( const exControlItem& Item : m_AllControlItems )
			{
				if( Item.RebindContext == RebindContext )
				{
					if( Item.bLocked )
					{
						if( KbButton != KBM_BTN_NONE )
						{
							if( KbButton == Bindings.ActionToKb( Item.Cmd ) )
							{
								if( ReasonOut )
								{
									*ReasonOut = Item.LabelText;
								}
								return false;
							}
						}

						if( GpButton != GP_BTN_NONE )
						{
							if( GpButton == Bindings.ActionToGp( Item.Cmd ) )
							{
								if( ReasonOut )
								{
									*ReasonOut = Item.LabelText;
								}
								return false;
							}
						}
					}
				}

				if( AssociatedContext != ex_rebind_context::None && Item.RebindContext == AssociatedContext )
				{
					// If an associated context has the key bound, we can't bind this.
					if( KbButton != KBM_BTN_NONE )
					{
						if( KbButton == Bindings.ActionToKb( Item.Cmd ) )
						{
							if( ReasonOut )
							{
								*ReasonOut = Item.LabelText;
							}
							if( bAssocatedIssueOut )
							{
								*bAssocatedIssueOut = true;
							}
							return false;
						}
					}

					if( GpButton != GP_BTN_NONE )
					{
						if( GpButton == Bindings.ActionToGp( Item.Cmd ) )
						{
							if( ReasonOut )
							{
								*ReasonOut = Item.LabelText;
							}
							if( bAssocatedIssueOut )
							{
								*bAssocatedIssueOut = true;
							}
							return false;
						}
					}
				}
			}
		}
		return true;
	}

	void ClearAllBindsTo( ex_rebind_context Context , eg_kbm_btn KbButton , eg_gp_btn GpButton )
	{
		if( EGClient* Client = GetClientOwner() )
		{
			EGInputBindings& Bindings = Client->GetInputBindings();

			for( exControlItem& Item : m_AllControlItems )
			{
				if( Item.RebindContext == Context || Item.AssociatedContext == Context )
				{
					if( KbButton != KBM_BTN_NONE )
					{
						if( Bindings.ActionToKb( Item.Cmd ) == KbButton )
						{
							Bindings.Bind( Item.Cmd , KBM_BTN_COUNT );
						}
					}

					if( GpButton != GP_BTN_NONE )
					{
						if( Bindings.ActionToGp( Item.Cmd ) == GpButton )
						{
							Bindings.Bind( Item.Cmd , GP_BTN_COUNT );
						}
					}
				}
			}
		}
	}

	void OnKeyBindButtonPressed( eg_kbm_btn KbButton , eg_gp_btn GpButton )
	{
		if( KbButton == KBM_BTN_ESCAPE || GpButton == GP_BTN_START )
		{
			MenuStack_Pop();
			return;
		}

		auto ShowNotAllowedMessage = [this,&KbButton,&GpButton]() -> void
		{
			eg_loc_text CannotBindReason;
			eg_bool bAssocatedIssue = false;
			if( !IsBindAllowed( m_RebindTargetContext , m_RebindTargetAssociatedContext , KbButton , GpButton , &CannotBindReason , &bAssocatedIssue ) )
			{
				MenuStack_Pop();
				eg_loc_text CannotRebindText;
				if( EGClient* Client = GetClientOwner() )
				{
					const EGInputBindings& Bindings = Client->GetInputBindings();
					for( const exControlItem& Item : m_AllControlItems )
					{
						if( Item.Cmd == m_RebindTargetCmd )
						{
							if( bAssocatedIssue )
							{
								CannotRebindText = EGFormat( eg_loc("ControlSettingsCannotRebindsAssociatedWithReason","\"{0}\" cannot be bound to |ButtonGlyph({1})|. That button is already bound to \"{2}\"") , Item.LabelText , *eg_d_string16(m_ColumnSelected == 0 ? InputButtons_GpButtonToString( GpButton ) : InputButtons_KbmButtonToString( KbButton )) , CannotBindReason );
							}
							else
							{
								if( CannotBindReason.GetLen() > 0 )
								{
									CannotRebindText = EGFormat( eg_loc("ControlSettingsCannotRebindsWithReason","\"{0}\" cannot be bound to |ButtonGlyph({1})|. That button is permanently bound to \"{2}\"") , Item.LabelText , *eg_d_string16(m_ColumnSelected == 0 ? InputButtons_GpButtonToString( GpButton ) : InputButtons_KbmButtonToString( KbButton )) , CannotBindReason );
								}
								else
								{
									CannotRebindText = EGFormat( eg_loc("ControlSettingsCannotRebinds","\"{0}\" cannot be bound to |ButtonGlyph({1})|. That button cannot be bound.") , Item.LabelText , *eg_d_string16(m_ColumnSelected == 0 ? InputButtons_GpButtonToString( GpButton ) : InputButtons_KbmButtonToString( KbButton )) );
								}
							}
						}
					}
				}
				ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( CannotRebindText ) );
				PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
			}
		};

		if( KbButton != KBM_BTN_NONE && !m_bRebindTargetIsGamepad )
		{
			if( IsBindAllowed( m_RebindTargetContext , m_RebindTargetAssociatedContext , KbButton , GpButton , nullptr , nullptr ) )
			{
				ClearAllBindsTo( m_RebindTargetContext , KbButton , GpButton );

				if( EGClient* Client = GetClientOwner() )
				{
					EGInputBindings& Bindings = Client->GetInputBindings();
					Bindings.Bind( m_RebindTargetCmd , KbButton );
					RefreshGrid();
				}
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::ConfirmDialogChoice );
				MenuStack_Pop();
			}
			else
			{
				ShowNotAllowedMessage();
			}
		}
		else if( GpButton != GP_BTN_NONE && m_bRebindTargetIsGamepad )
		{
			if( IsBindAllowed( m_RebindTargetContext , m_RebindTargetAssociatedContext , KbButton , GpButton , nullptr , nullptr ) )
			{
				ClearAllBindsTo( m_RebindTargetContext , KbButton , GpButton );

				if( EGClient* Client = GetClientOwner() )
				{
					EGInputBindings& Bindings = Client->GetInputBindings();
					Bindings.Bind( m_RebindTargetCmd , GpButton );
					RefreshGrid();
				}
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::ConfirmDialogChoice );
				MenuStack_Pop();
			}
			else
			{
				ShowNotAllowedMessage();
			}
		}
	}

	void OnBindListColumnHovered( eg_int HoveredColumn )
	{
		if( m_ColumnSelected != HoveredColumn )
		{
			m_ColumnSelected = HoveredColumn;
			RefreshGrid();
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
		}

		UpdateHints();
	}

	void OnBindlistCellSelected( EGUiGridWidget2* GridWiget , eg_uint CellIndex )
	{
		unused( GridWiget );

		if( !m_bMuted && m_DisplayedControlItems.IsValidIndex( CellIndex ) )
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
		}

		UpdateHints();
	}

	void OnBindListCellChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
	{
		if( CellInfo.GridItem )
		{
			if( const exControlItem* SourceItemPtr = GetDisplayedControlItem( CellInfo.GridIndex ) )
			{
				const exControlItem& SourceItem = *SourceItemPtr;

				if( SourceItem.bLocked )
				{
					CellInfo.GridItem->SetText( eg_crc("NameText") , EGFormat( L"|SC(EX_DISABLED_OPTION)|{0}|RC()|" , SourceItem.LabelText ) );
				}
				else
				{
					CellInfo.GridItem->SetText( eg_crc("NameText") , SourceItem.LabelText );
				}

				switch( SourceItem.ItemType )
				{
				case ex_control_item_t::None:
					CellInfo.GridItem->SetText( eg_crc("ValueToggleText") , CT_Clear );
					CellInfo.GridItem->SetText( eg_crc("Value0Text") , CT_Clear );
					CellInfo.GridItem->SetText( eg_crc("Value1Text") , CT_Clear );
					break;
				case ex_control_item_t::Binding:
				{
					eg_d_string16 GpButton;
					eg_d_string16 KbmButton;
					if( EGClient* Client = GetClientOwner() )
					{
						const EGInputBindings& Bindings = Client->GetInputBindings();
						KbmButton = Bindings.InputActionToKeyStr( SourceItem.Cmd , false );
						GpButton = Bindings.InputActionToKeyStr( SourceItem.Cmd , true );
					}

					CellInfo.GridItem->SetText( eg_crc("ValueToggleText") , CT_Clear );
					CellInfo.GridItem->SetText( eg_crc("Value0Text") , eg_loc_text(*EGSFormat16( L"|ButtonGlyph({0})|" , *GpButton)) );
					CellInfo.GridItem->SetText( eg_crc("Value1Text") , eg_loc_text(*EGSFormat16( L"|ButtonGlyph({0})|" , *KbmButton)) );
				} break;
				case ex_control_item_t::Toggle:
					CellInfo.GridItem->SetText( eg_crc("ValueToggleText") , CT_Clear );
					CellInfo.GridItem->SetText( eg_crc("Value0Text") , CT_Clear );
					CellInfo.GridItem->SetText( eg_crc("Value1Text") , CT_Clear );
					break;
				}
				

				if( CellInfo.bIsSelected)
				{
					CellInfo.GridItem->RunEvent( SourceItem.ItemType == ex_control_item_t::Binding ? m_ColumnSelected == 0 ? eg_crc("Select0") : m_ColumnSelected == 1 ? eg_crc("Select1") : eg_crc("SelectToggle") : eg_crc("SelectToggle") );
				}
				else
				{
					CellInfo.GridItem->RunEvent( eg_crc("Deselect") );
				}
			}
		}
	}

	void OnBindListCellClicked( egUiGridWidgetCellClickedInfo2& ClickInfo )
	{
		if( const exControlItem* SourceItemPtr = GetDisplayedControlItem( ClickInfo.GridIndex ) )
		{
			const exControlItem& SourceItem = *SourceItemPtr;

			if( SourceItem.ItemType == ex_control_item_t::Binding )
			{
				if( SourceItem.bLocked )
				{
					ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( EGFormat( eg_loc("ControlsSettingsMenuCannotRebind","The binding for \"{0}\" cannot be changed.") , SourceItem.LabelText ) ) );
					PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
				}
				else
				{
					exKeyBindDialogParms KeyBindParms;
					KeyBindParms.DelegateCb.Bind( this , &ThisClass::OnKeyBindButtonPressed );
					KeyBindParms.HeaderText = eg_loc_text(eg_loc("KeyBindDialogHeader","Input Bind"));

					if( m_ColumnSelected == 0 )
					{
						KeyBindParms.BodyText = EGFormat( eg_loc("KeyBindGamepadBodyText","Press a gamepad button to bind to \"{0}\".\n\n|ButtonGlyph(KBM_ESCAPE)|/|ButtonGlyph(GP_START)| Cancel") , SourceItem.LabelText );
					}
					else if( m_ColumnSelected == 1 )
					{
						KeyBindParms.BodyText = EGFormat(eg_loc("KeyBindKeyboardBodyText","Press a keyboard or mouse button to bind to \"{0}\".\n\n|ButtonGlyph(KBM_ESCAPE)|/|ButtonGlyph(GP_START)| Cancel") , SourceItem.LabelText );
					}

					m_RebindTargetCmd = SourceItem.Cmd;
					m_RebindTargetContext = SourceItem.RebindContext;
					m_RebindTargetAssociatedContext = SourceItem.AssociatedContext;
					m_bRebindTargetIsGamepad = m_ColumnSelected == 0;
					ExDialogMenu_PushKeyBindDialog( GetClientOwner() , KeyBindParms );
				}
			}
		}
	}

	void BuildControlItemList()
	{
		const eg_bool bDisplayIfDebug = EX_CHEATS_ENABLED;

		const exControlItem MasterList[] =
		{
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveForward","Move Forward")) , CMDA_FORWARD1 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveBack","Move Backward")) , CMDA_BACK1 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveStrafeLeft","Strafe Left")) , CMDA_STRAFELEFT1 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveStrafeRight","Strafe Right")) , CMDA_STRAFERIGHT1 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveTurnLeft","Turn Left")) , CMDA_TURNLEFT1 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveTurnRight","Turn Right")) , CMDA_TURNRIGHT1 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,

			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlNextPartyMember","Next Party Member")) , CMDA_GAME_NEXTPMEMBER , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlPrevPartyMember","Previous Party Member")) , CMDA_GAME_PREVPMEMBER , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,

			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlWaitTurn","Wait")) , CMDA_WAITTURN , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,

			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlPartyOptions","Character Options")) , CMDA_CONTEXTMENU , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlQuestLog","View Quest Log")) , CMDA_QUESTLOG , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Warn } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlInventoryMenu","View Inventory")) , CMDA_QUICK_INVENTORY , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Warn } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMap","View Map")) , CMDA_MAPMENU , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Warn } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlCharacter","View Character")) , CMDA_VIEWCHARMENU , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Warn } ,

			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveForwardAlt","Move Forward (Alternate)")) , CMDA_FORWARD2 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveBackAlt","Move Backward (Alternate)")) , CMDA_BACK2 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveStrafeLeftAlt","Strafe Left (Alternate)")) , CMDA_STRAFELEFT2 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveStrafeRightAlt","Strafe Right (Alternate)")) , CMDA_STRAFERIGHT2 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveTurnLeftAlt","Turn Left (Alternate)")) , CMDA_TURNLEFT2 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMoveTurnRightAlt","Turn Right (Alternate)")) , CMDA_TURNRIGHT2 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlChangePartyMember","Next Party Member (Alternate)")) , CMDA_GAME_NEXTPMEMBER2 , ex_rebind_context::Gameplay , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,

			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlGameMenu","Open Game Menu")) , CMDA_GAMEMENU , ex_rebind_context::Gameplay , ex_rebind_context::None , true , true , ex_warn_t::Safe } ,
	
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuSelect","Menu: Select")) , CMDA_MENU_PRIMARY , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuBack","Menu: Back/Close/Cancel")) , CMDA_MENU_BACK , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuAltSelect","Menu: Select (Alternate)")) , CMDA_MENU_PRIMARY2 , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuAltBack","Menu: Back/Close/Cancel (Alternate)")) , CMDA_MENU_BACK2 , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,

			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuUp","Menu: Navigate Up")) , CMDA_MENU_UP , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuDown","Menu: Navigate Down")) , CMDA_MENU_DOWN , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuLeft","Menu: Navigate Left")) , CMDA_MENU_LEFT , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuRight","Menu: Navigate Right")) , CMDA_MENU_RIGHT , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuAltUp","Menu: Navigate Up (Alternate)")) , CMDA_MENU_UP2 , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuAltDown","Menu: Navigate Down (Alternate)")) , CMDA_MENU_DOWN2 , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuAltLeft","Menu: Navigate Left (Alternate)")) , CMDA_MENU_LEFT2 , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuAltRight","Menu: Navigate Right (Alternate)")) , CMDA_MENU_RIGHT2 , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuLMB","Menu: Interact")) , CMDA_MENU_LMB , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuRMB","Menu: Back/Close/Cancel")) , CMDA_MENU_RMB , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuScrollUp","Menu: Scroll Up")) , CMDA_MENU_SCRLUP , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuScrollDown","Menu: Scroll Down")) , CMDA_MENU_SCRLDOWN , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuExtra1","Menu: Contextual Command 1")) , CMDA_MENU_BTN2 , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuExtra2","Menu: Contextual Command 2")) , CMDA_MENU_BTN3 , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuNextPage","Menu: Next Party Member")) , CMDA_MENU_NEXT_PAGE , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuPrevPage","Menu: Previous Party Member")) , CMDA_MENU_PREV_PAGE , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , true , ex_warn_t::Safe } ,
			// { ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuNextSubPage","Menu: Next Sub-Page")) , CMDA_MENU_NEXT_SUBPAGE , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , false , ex_warn_t::Safe } ,
			// { ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuPrevSubPage","Menu: Previous Sub-Page")) , CMDA_MENU_PREV_SUBPAGE , ex_rebind_context::MenusDefault , ex_rebind_context::None , false , false , ex_warn_t::Safe } ,

			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuBackspace","Menu: Backspace")) , CMDA_KBMENU_BKSP , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuDelete","Menu: Delete")) , CMDA_MENU_DELETE , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuCompare","Menu: Compare")) , CMDA_MENU_COMPARE , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlMenuClose","Menu: Close")) , CMDA_GAMEMENU_CLOSE , ex_rebind_context::MenusDefault , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,

			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlConToggle","Menu: Console Toggle")) , CMDA_SYS_CONTOGGLE , ex_rebind_context::EngineDebug , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlConPageUp","Menu: Console Scroll Up")) , CMDA_SYS_CONPAGEUP , ex_rebind_context::EngineDebug , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlConPageDown","Menu: Console PageDown")) , CMDA_SYS_CONPAGEDOWN , ex_rebind_context::EngineDebug , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlConHistory","Menu: Console History")) , CMDA_SYS_CONHISTORY , ex_rebind_context::EngineDebug , ex_rebind_context::None , true , false , ex_warn_t::Safe } ,
		
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlDebugQuickSave","Debug: Quick Save")) , CMDA_QSAVE , ex_rebind_context::Gameplay , ex_rebind_context::None , false , bDisplayIfDebug , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlDebugQuickLoad","Debug: Quick Load")) , CMDA_QLOAD , ex_rebind_context::Gameplay , ex_rebind_context::None , false , bDisplayIfDebug , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlDebugNext","Debug: Inc Day Time")) , CMDA_DEBUG_NEXT , ex_rebind_context::Gameplay , ex_rebind_context::None , false , bDisplayIfDebug , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlDebugPrev","Debug: Dec Day Time")) , CMDA_DEBUG_PREV , ex_rebind_context::Gameplay , ex_rebind_context::None , false , bDisplayIfDebug , ex_warn_t::Safe } ,
			{ ex_control_item_t::Binding , eg_loc_text(eg_loc("ControlDebugSpeed","Debug: Inc/Dec Day Time Adjustment")) , CMDA_DEBUG_SPEED , ex_rebind_context::Gameplay , ex_rebind_context::None , false , bDisplayIfDebug , ex_warn_t::Safe } ,
		};

		m_AllControlItems.Append( MasterList , countof(MasterList) );

		for( eg_int i=0; i<m_AllControlItems.LenAs<eg_int>(); i++ )
		{
			if( !m_AllControlItems[i].bDisplayed )
			{
				assert( m_AllControlItems[i].bLocked ); // Any non display control items should be locked.
			}
			if( m_AllControlItems[i].bDisplayed )
			{
				m_DisplayedControlItems.Append( i );
			}
		}
	}

	const exControlItem* GetDisplayedControlItem( eg_int DisplayIndex ) const
	{
		return m_DisplayedControlItems.IsValidIndex( DisplayIndex ) && m_AllControlItems.IsValidIndex( m_DisplayedControlItems[DisplayIndex] ) ? &m_AllControlItems[m_DisplayedControlItems[DisplayIndex]] : nullptr;
	}

	const exControlItem* GetSelectedControlItem() const
	{
		const eg_int SelectedIndex = m_BindListWidget ? m_BindListWidget->GetSelectedIndex() : -1;
		return GetDisplayedControlItem( SelectedIndex );
	}

	void RefreshGrid()
	{
		if( m_BindListWidget )
		{
			m_BindListWidget->RefreshGridWidget( m_DisplayedControlItems.LenAs<eg_uint>() );
		}
	}

	void UpdateHints()
	{
		ClearHints();
		if( const exControlItem* SelectedControl = GetSelectedControlItem() )
		{
			if( !SelectedControl->bLocked )
			{
				AddHint( CMDA_MENU_PRIMARY, eg_loc_text(eg_loc("ControlsSettingsBindKey","Change Binding")) );
				AddHint( CMDA_MENU_DELETE , eg_loc_text(eg_loc("ControlsSettingsUnbindKey","Unbind")) );
			}
		}
		AddHint( CMDA_MENU_BTN3 , eg_loc_text(eg_loc("ControlsSettingsResetToDefault","Reset to Defaults")) );
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("ControlsSettingsMenuBack","Back")) );
	}
};

EG_CLASS_DECL( ExControlsSettingsMenu )
