// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"
#include "EGUiGridWidgetItem.h"
#include "EGUiGridWidget2.h"

class EGClient;
class EGUiGridWidget2;
struct egUiGridWidgetCellChangedInfo2;
struct egUiGridWidgetCellClickedInfo2;
class ExChoiceWidgetMenu;
class ExSettingsMenuGridWidget;

enum class ex_settings_menu_page
{
	Game,
	Graphics,
	Audio,
	Debug,
};

enum class ex_settings_menu_item_t
{
	Default ,
	Bool ,
	Slider ,
	DropDown ,
};

enum class ex_settings_menu_special_t
{
	None ,
	ScreenMode ,
};

struct exSettingsMenuItem
{
	EGSettingsVar* Var;
	eg_string_crc NameStr;
	eg_cpstr ValueStr;
	ex_settings_menu_item_t Type;
	ex_settings_menu_special_t SpecialType;
	eg_string_crc DescText;
	eg_bool bNeedsVidRestart:1;
	eg_bool bApplyImmediately:1;
};

class ExSettingsMenu : public ExMenu
{
	EG_CLASS_BODY( ExSettingsMenu , ExMenu )

public:

	static void OpenMenu( EGClient* Client , ex_settings_menu_page Type );

private:

	ExSettingsMenuGridWidget*   m_SettingsWidget;
	EGUiWidget*                 m_DescBgWidget;
	EGUiTextWidget*             m_DescWidget;
	EGArray<exSettingsMenuItem> m_SettingsItems;
	bool                        m_bNeedsVidRestart = false;
	eg_real                     m_CountdownTilInputForVidRestart = 0.f;
	EGSettingsVar*              m_SettingForContext = nullptr;
	ex_settings_menu_special_t  m_SettingForContextSpecialType = ex_settings_menu_special_t::None;

public:

	void InitAs( ex_settings_menu_page InType );

private:

	virtual void OnRevealComplete() override;

	virtual void OnInit() override final;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final;
	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override;

	void OnSettingsItemCellClicked( egUiGridWidgetCellClickedInfo2& CellInfo );
	void OnSettingsItemCellChanged( egUiGridWidgetCellChangedInfo2& CellInfo );
	void OnSettingsItemCellSelected( EGUiGridWidget2* GridOwner , eg_uint CellIndex );
	void ChangeAtIndex( eg_uint Index , eg_bool bInc );
	void SaveAndExit();
	void OpenContextMenu( eg_uint Index );
	void OnContextMenuChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex );
	void RefreshHints();

	static void GetFullScreenModeTexts( EGArray<eg_loc_text>& Out );
	static eg_int GetFullScreenMode();
	static void SetFullScreenMode( eg_int ModeIndex );
};


class ExSettingsMenuItemWidget : public EGUiGridWidgetItem
{
	EG_CLASS_BODY( ExSettingsMenuItemWidget , EGUiGridWidgetItem )
};

class ExSettingsMenuGridWidget : public EGUiGridWidget2
{
	EG_CLASS_BODY( ExSettingsMenuGridWidget , EGUiGridWidget2 )

	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured ) override;
};
