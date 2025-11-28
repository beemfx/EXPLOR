#include "EGMenu.h"
#include "EGParse.h"
#include "EGUiWidget.h"
#include "EGUiGridWidget.h"
#include "EGUiButtonWidget.h"
#include "EGEngine.h"
#include "EGSettings2.h"

struct egSettingsMenuItem
{
	EGSettingsVar* Var;
	eg_string_crc NameStr;
	eg_cpstr ValueStr;
	eg_bool bNeedsVidRestart:1;
	eg_bool bApplyImmediately:1;
};

class EGGameSettingsMenu: public EGMenu
{
	EG_CLASS_BODY( EGGameSettingsMenu , EGMenu )

private:

	EGArray<egSettingsMenuItem> m_SettingsItems;
	bool                        m_bNeedsVidRestart = false;

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		m_SettingsItems.Clear();
		EGArray<EGSettingsVar*> GlobalSettings;
		EGSettingsVar::GetAllSettingsMatchingFlags( GlobalSettings , EGS_F_EDITABLE | EGS_F_DEBUG_EDITABLE );

		for( EGSettingsVar* Var : GlobalSettings )
		{
			if( Var )
			{
				egSettingsMenuItem NewItem;
				NewItem.Var = Var;
				NewItem.NameStr = Var->GetDisplayName();
				NewItem.ValueStr = nullptr;
				NewItem.bApplyImmediately = true;
				NewItem.bNeedsVidRestart = Var->GetFlags().IsSet( EGS_F_NEEDSVRESTART );
				m_SettingsItems.Append( NewItem );
			}
		}

		EGUiGridWidget* SettingsItems = GetWidget<EGUiGridWidget>( eg_crc("SettingsItems") );
		if( SettingsItems )
		{
			SettingsItems->OnItemPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
			SettingsItems->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );

			SettingsItems->RefreshGridWidget( m_SettingsItems.LenAs<eg_uint>() );
			SetFocusedWidget( SettingsItems , 0 , false );
		}

		EGUiButtonWidget* CancelButton = GetWidget<EGUiButtonWidget>( eg_crc("CancelButton") );
		if( CancelButton )
		{
			CancelButton->OnPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
		}

		EGUiButtonWidget* SaveAndExitButton = GetWidget<EGUiButtonWidget>( eg_crc("SaveAndExitButton") );
		if( SaveAndExitButton )
		{
			SaveAndExitButton->OnPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
		}
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		switch( InputType )
		{
			case eg_menuinput_t::BUTTON_BACK:
			{
				SaveAndExit();
			} return true;

			case eg_menuinput_t::BUTTON_RIGHT:
			{
				EGUiGridWidget* SettingsList = GetWidget<EGUiGridWidget>( eg_crc( "SettingsItems" ) );
				if( SettingsList && SettingsList == GetFocusedWidget() )
				{
					eg_uint Index = SettingsList->GetSelectedIndex();
					ChangeAtIndex( Index , true );
					SettingsList->RunEvent( eg_crc( "ClickSound" ) );
					SettingsList->RefreshGridWidget( m_SettingsItems.LenAs<eg_int>() );
				}
			} return true;

			case eg_menuinput_t::BUTTON_LEFT:
			{
				EGUiGridWidget* SettingsList = GetWidget<EGUiGridWidget>( eg_crc( "SettingsItems" ) );
				if( SettingsList && SettingsList == GetFocusedWidget() )
				{
					eg_uint Index = SettingsList->GetSelectedIndex();
					ChangeAtIndex( Index , false );
					SettingsList->RunEvent( eg_crc( "ClickSound" ) );
					SettingsList->RefreshGridWidget( m_SettingsItems.LenAs<eg_int>() );
				}
			} return true;

			default: break;
		}

		return Super::OnInput( InputType );
	}

	void OnObjPressed( const egUIWidgetEventInfo& Info )
	{
		switch_crc( Info.WidgetId )
		{
			case_crc( "SettingsItems" ):
			{
				//EGLogf( eg_log_t::General , "Clicked setting %u at <%f,%f>" , Info.Index , Info.HitPoint.x , Info.HitPoint.y );

				// We only care about clicking on the right half:
				if( m_SettingsItems.IsValidIndex( Info.GridIndex ) )
				{
					const egSettingsMenuItem& SettingItem = m_SettingsItems[Info.GridIndex];

					const eg_real MinClick = .5f;
					if( Info.HitPoint.x >= MinClick )
					{
						eg_real AdjClickPos = EGMath_GetMappedRangeValue( Info.HitPoint.x , eg_vec2( MinClick , 1.f ) , eg_vec2( -1.f , 1.f ) );
						ChangeAtIndex( Info.GridIndex , AdjClickPos >= 0.f );
						if( Info.GridWidgetOwner )
						{
							Info.GridWidgetOwner->RunEvent( eg_crc( "ClickSound" ) );
							Info.GridWidgetOwner->RefreshGridWidget( m_SettingsItems.LenAs<eg_uint>() );
						}
					}
				}
			} break;
			case_crc( "CancelButton" ):
			{
				MenuStack_Pop();
			} break;
			case_crc( "SaveAndExitButton" ):
			{
				SaveAndExit();
			} break;
		}
	}

	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		if( eg_crc("SettingsItems") == ItemInfo.WidgetId && ItemInfo.Widget && ItemInfo.GridWidgetOwner )
		{
			ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

			if( m_SettingsItems.IsValidIndex( ItemInfo.GridIndex ) )
			{
				const egSettingsMenuItem& SettingItem = m_SettingsItems[ItemInfo.GridIndex];
				ItemInfo.Widget->SetText( eg_crc( "NameText" ) , eg_loc_text( SettingItem.NameStr ) );

				if( SettingItem.Var )
				{
					eg_loc_text ValueText = SettingItem.Var->ToLocText();
					ItemInfo.Widget->SetText( eg_crc( "ValueText" ) , ValueText );
				}
			}
		}
	}

	void SaveAndExit()
	{
		Engine_QueMsg( "engine.SaveConfig()" );
		if( m_bNeedsVidRestart )
		{
			Engine_QueMsg( "engine.VidRestart()" );
		}
		MenuStack_Pop();
	}

	void ChangeAtIndex( eg_uint Index , eg_bool bInc )
	{
		if( m_SettingsItems.IsValidIndex( Index ) )
		{
			const egSettingsMenuItem& SettingItem = m_SettingsItems[Index];

			if( SettingItem.bNeedsVidRestart )
			{
				m_bNeedsVidRestart = true;
			}

			if( SettingItem.Var )
			{
				if( bInc )
				{
					SettingItem.Var->Inc();
				}
				else
				{
					SettingItem.Var->Dec();
				}
			}
		}
		else
		{
			assert( false ); // Index out of range!
		}
	}
};

EG_CLASS_DECL( EGGameSettingsMenu )
