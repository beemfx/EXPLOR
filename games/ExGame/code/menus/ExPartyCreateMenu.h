// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"
#include "ExFighter.h"
#include "ExDialogMenu.h"
#include "EGUiImageWidget.h"

class ExTextEditWidget;
class EGUiGridWidget2;
class EGUiImageWidget;
class EGUiButtonWidget;
class ExPartyCreateMenu;
struct egUiGridWidgetCellChangedInfo2;
struct egUiGridWidgetCellClickedInfo2;
class EGUiGridWidgetItem;
class ExPartyCreateMenuHighlighter;

struct exPartyCreateMenuMemberInfo
{
	ExFighter PartyMemberData = CT_Clear;
	eg_int AttrPointPool = 0;
	eg_int PartyIndex = 0;

	ExPartyCreateMenuHighlighter* HighlightWidget = nullptr;
	EGUiImageWidget* PortraitWidget = nullptr;
	EGUiButtonWidget* PortraitTextWidget = nullptr;
	EGUiWidget* NameBgWidget = nullptr;
	ExTextEditWidget* NameWidget = nullptr;
	EGUiButtonWidget* ClassWidget = nullptr;
	EGUiGridWidget2* EditableAttributesWidget = nullptr;
	EGUiTextWidget* PointsToAllocateWidget = nullptr;
	EGUiTextWidget* StartingInfoWidget = nullptr;

	EGWeakPtr<ExPartyCreateMenu> OwnerMenu;

	void RefreshView();
	void TogglePortrait( eg_bool bNext );
	void ToggleClass( eg_bool bNext );
	void ChangeAttr( ex_attr_t AttrType , eg_int Offset );
	void DoAttrChange( ex_attr_t Type , eg_int Delta ); // Does not make sure that the pool can handle this or that this is in range.
	void ResetToClassDefaults( ex_class_t NewClass );
	void RefreshAttrPointPool();
};

class ExPartyCreateMenu : public ExMenu , public IExDialogListener
{
	EG_CLASS_BODY( ExPartyCreateMenu , ExMenu )

friend struct exPartyCreateMenuMemberInfo;

private:

	EGArray<exPartyCreateMenuMemberInfo> m_PartyCreateInfos;

	EGArray<eg_string_crc> m_PortraitList;
	EGArray<ex_class_t> m_ClassList;

	EGArray<ex_attr_t> m_BaseAList;
	EGArray<ex_attr_t> m_CompAList;

	EGUiTextWidget* m_HelpText = nullptr;
	EGUiButtonWidget* m_BeginGameButton = nullptr;

	eg_int m_SelectedPartyMember = 0;

private:

	virtual void OnInit() override;
	virtual void OnDeinit() override;
	virtual void OnActivate() override;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override;
	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override;
	virtual void OnFocusedWidgetChanged( EGUiWidget* NewFocusedWidget , EGUiWidget* OldFocusedWidget ) override;

public:

	void HandleHighlighterHovered( ExPartyCreateMenuHighlighter* Highlighter );

private:

	exPartyCreateMenuMemberInfo* GetMemberInfoForWidget( EGObject* Widget );
	eg_int GetIndexOfPortrait( eg_string_crc PortraitId ) const;
	eg_int GetIndexOfClass( ex_class_t ClassType ) const;

	void UpdateAttriubte( exPartyCreateMenuMemberInfo& Mi , EGUiGridWidgetItem* GridItem , eg_int ListIndex , EGArray<ex_attr_t>& List );
	void OnEditableAttributeChanged( egUiGridWidgetCellChangedInfo2& CellInfo );
	void OnEditableAttributeClicked( egUiGridWidgetCellClickedInfo2& CellInfo );

	void OnPropertyButtonClicked( const egUIWidgetEventInfo& WidgetInfo );

	void OnBeginGameClicked( const egUIWidgetEventInfo& Info );
	void CommitChangesAndLeaveMenu();
	virtual void OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice ) override;

	void RefreshHighlightedPartyMember();
	void MoveSelectionToNewPartyMember();
	void RefreshHintText();
};

class ExPartyCreateMenuHighlighter : public EGUiImageWidget
{
	EG_CLASS_BODY( ExPartyCreateMenuHighlighter , EGUiImageWidget )

public:

	void SetHighlightActive( eg_bool bNewValue );

	virtual eg_bool IsWidget() const override { return true; }
	virtual eg_bool IsFocusable() const override { return true; }
	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured ) override; // Called if the mouse moved over the widget ( xPos and yPos are relative the dimensions of the widget in [0,1]x[0,1] if the widget is captured the value may be out of that range.
	virtual eg_bool OnMouseMovedOn( const eg_vec2& WidgetHitPoint ) override;

	void HandleHovered();
};
