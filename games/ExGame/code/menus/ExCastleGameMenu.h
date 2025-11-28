// (c) 2021 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"
#include "EGSmRuntime.h"

class ExTextRevealWidget;
class EGUiGridWidget2;
struct egUiGridWidgetCellChangedInfo2;
struct egUiGridWidgetCellClickedInfo2;

class ExCastleGameMenu : public ExMenu , public ISmRuntimeHandler
{
	EG_CLASS_BODY( ExCastleGameMenu , ExMenu )

protected:
	
	enum class ex_castle_s
	{
		None ,
		Running ,
		WaitingForChoice ,
	};

protected:
	
	EGUiTextWidget* m_TitleTextWidget = nullptr;
	ExTextRevealWidget* m_StoryTextWidget = nullptr;
	EGUiGridWidget2* m_ChoicesWidget = nullptr;
	egsmState m_SmState;
	EGArray<eg_string_crc> m_CurrentChoices;
	ex_castle_s m_CastleState = ex_castle_s::None;
	eg_real m_InputCountdown;
	eg_bool m_bTextFullyRevealed = false;
	const eg_real m_InputCountdownDuration = .1f;
	eg_bool m_bIsPlayingDialogSound = false;

protected:
	
	virtual void OnInit() override;
	virtual void OnDeinit() override;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override;
	virtual void OnRevealComplete() override;

	virtual egsmNativeRes SmOnNativeEvent( eg_string_crc EventName , const EGArray<egsm_var>& Parms , const EGArray<eg_string_crc>& Branches ) override;

	void OnChoicesCellChanged( egUiGridWidgetCellChangedInfo2& CellInfo );
	void OnChoicesCellClicked( egUiGridWidgetCellClickedInfo2& CellInfo );
};
