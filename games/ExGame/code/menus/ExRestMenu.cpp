// (c) 2017 Beem Media

#include "ExMenu.h"
#include "EGUiImageWidget.h"
#include "ExCharacterPortraitWidget.h"
#include "EGUiGridWidget.h"
#include "ExGlobalData.h"
#include "EGMenuStack.h"
#include "ExCombatMenu.h"

class ExRestMenu : public ExMenu
{
	EG_CLASS_BODY( ExRestMenu , ExMenu )

private:
	
	EGUiMeshWidget*  m_BgFade;
	eg_real          m_FadeTime;
	eg_uint          m_RestHours;
	EGUiTextWidget*  m_WarningLine1Text;
	EGUiTextWidget*  m_WarningLine2Text;
	EGUiTextWidget*  m_AmbushTextWidget;
	EGArray<EGUiTextWidget*> m_FadeWidgets;
	eg_bool              m_bAmbush = false;
	eg_bool				   m_bPlayedFadeOut = false;
	eg_bool              m_bPartyIsStarving = false;

public:

	void SetAmbush()
	{
		m_bAmbush = true;
		m_FadeTime = 0.f;
		if( m_BgFade )
		{
			m_BgFade->RunEvent( eg_crc("FadeRestAmbush") );
		}
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::Ambush );
	}

	virtual void OnInit() override final
	{
		Super::OnInit();

		HideHUD();

		m_RestHours = GetGame()->GetStartingRestHours();

		m_BgFade = GetWidget<EGUiMeshWidget>( eg_crc("BgFade") );
		m_WarningLine1Text = GetWidget<EGUiTextWidget>( eg_crc("WarningLine1Text") );
		m_WarningLine2Text = GetWidget<EGUiTextWidget>( eg_crc("WarningLine2Text") );
		m_AmbushTextWidget = GetWidget<EGUiTextWidget>( eg_crc("AmbushText") );

		m_FadeWidgets.Append( GetWidget<EGUiTextWidget>( eg_crc("RestingText") ) );
		m_FadeWidgets.Append( GetWidget<EGUiTextWidget>( eg_crc("InfoText") ) );
		m_FadeWidgets.Append( GetWidget<EGUiTextWidget>( eg_crc("FoodText") ) );
		m_FadeWidgets.Append( m_WarningLine1Text );
		m_FadeWidgets.Append( m_WarningLine2Text );

		EGUiGridWidget* CharacterPortraits = GetWidget<EGUiGridWidget>( eg_crc("CharacterPortraits") );
		if( CharacterPortraits )
		{
			CharacterPortraits->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
			CharacterPortraits->RefreshGridWidget( ExRoster::PARTY_SIZE );
		}

		m_bPartyIsStarving = GetGame()->GetGameVar( eg_crc("PartyFood") ).as_int() <= 0;

		m_bPlayedFadeOut = false;
		if( m_BgFade )
		{
			m_BgFade->RunEvent( eg_crc("FadeRestIn") );
		}
	}

	virtual void OnDeinit() override final
	{
		ShowHUD( true );

		Super::OnDeinit();
	}
	
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		if( m_AmbushTextWidget )
		{
			m_AmbushTextWidget->SetVisible( m_bAmbush );
		}

		if( m_bAmbush )
		{
			const eg_real AMBUSH_TIME = 1.f;
			const eg_vec2 AMBUSH_FADEOUT_RANGE( .9f , 1.f );

			if( m_FadeTime >= AMBUSH_TIME )
			{
				ExCombatMenu* CombatMenu = EGCast<ExCombatMenu>(MenuStack_SwitchTo( eg_crc("CombatMenu") ));
				if( CombatMenu )
				{
					CombatMenu->ApplyAmbush();
				}
			}

			eg_real AmbushTextAlpha = 1.f;
			if( m_FadeTime < AMBUSH_FADEOUT_RANGE.x )
			{
				AmbushTextAlpha = 1.f;
			}
			else if( EG_IsBetween( m_FadeTime , AMBUSH_FADEOUT_RANGE.x , AMBUSH_FADEOUT_RANGE.y ) )
			{
				AmbushTextAlpha = EGMath_GetMappedCubicRangeValueNormalized( m_FadeTime , AMBUSH_FADEOUT_RANGE , eg_vec2(1.f,0.f) );
				AmbushTextAlpha = EG_Clamp( AmbushTextAlpha , 0.f , 1.f );
			}
			else
			{
				AmbushTextAlpha = 0.f;
			}

			if( m_AmbushTextWidget )
			{
				m_AmbushTextWidget->SetColorAlpha( AmbushTextAlpha );
			}
		}
		else
		{
			if( !GetGame()->IsResting() )
			{
				MenuStack_Pop();
			}
		}

		eg_real AlphaValue = 0.f;

		if( m_bAmbush )
		{
			const eg_real AMBUSH_FADE_OUT_TIME = .5f;
			if( m_FadeTime < AMBUSH_FADE_OUT_TIME )
			{
				AlphaValue = EGMath_GetMappedCubicRangeValueNormalized( m_FadeTime , eg_vec2(0,AMBUSH_FADE_OUT_TIME) , eg_vec2(1.,0.f) );
				AlphaValue = EG_Clamp( AlphaValue , 0.f , 1.f );
			}
			else
			{
				AlphaValue = 0.f;
			}
		}
		else
		{
			const eg_real TOTAL_REST_TIME = m_RestHours*ExGlobalData::Get().GetRestTimePerHour();
			const eg_real TIME_TO_DARK = .5f;
			const eg_real TIME_TO_LIGHT = .5f;
			const eg_real DARK_TIME_END = TOTAL_REST_TIME - TIME_TO_LIGHT;
			
			if( m_FadeTime < TIME_TO_DARK )
			{
				AlphaValue = EGMath_GetMappedCubicRangeValueNormalized( m_FadeTime , eg_vec2(0.f,TIME_TO_DARK) , eg_vec2(0.f,1.f) );
			}
			else if( m_FadeTime < DARK_TIME_END )
			{
				AlphaValue = 1.f;
			}
			else if( m_FadeTime < TOTAL_REST_TIME )
			{
				AlphaValue = EGMath_GetMappedCubicRangeValueNormalized( m_FadeTime , eg_vec2(DARK_TIME_END,TOTAL_REST_TIME) , eg_vec2(1.f,0.f) );

				if( !m_bPlayedFadeOut )
				{
					m_bPlayedFadeOut = true;
					if( m_BgFade )
					{
						m_BgFade->RunEvent( eg_crc("FadeRestOut") );
					}
				}
			}
			else
			{
				AlphaValue = 0.f;
			}
		}
		
		if( m_WarningLine1Text )
		{
			m_WarningLine1Text->SetVisible( m_bPartyIsStarving );
		}

		if( m_WarningLine2Text )
		{
			m_WarningLine2Text->SetVisible( m_bPartyIsStarving );
		}

		for( EGUiTextWidget* Widget : m_FadeWidgets )
		{
			if( Widget )
			{
				Widget->SetColorAlpha( AlphaValue );
			}
		}

		m_FadeTime += DeltaTime;
	}

	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

		if( ItemInfo.WidgetId == eg_crc("CharacterPortraits") )
		{
			if( EG_IsBetween<eg_size_t>( ItemInfo.GridIndex , 0 , 6 ) )
			{
				ExCharacterPortraitWidget2* PortraitWidget = EGCast<ExCharacterPortraitWidget2>(ItemInfo.Widget);
				if( PortraitWidget )
				{
					PortraitWidget->SetAnimateBars( true );
					PortraitWidget->InitPortrait( nullptr , ItemInfo.GridIndex , GetGame() );
				}
			}
		}
	}

};

EG_CLASS_DECL( ExRestMenu )

eg_bool ExRestMenu_IsOpen( EGClient* Client )
{
	EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
	return MenuStack && nullptr != MenuStack->FindMenuByClass<ExRestMenu>();
}

void ExRestMenu_SetAmbush( EGClient* Client )
{
	EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
	ExRestMenu* RestMenu = MenuStack ? MenuStack->FindMenuByClass<ExRestMenu>() : nullptr;
	if( RestMenu )
	{
		RestMenu->SetAmbush();
	}
}
