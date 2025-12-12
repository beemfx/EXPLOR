// (c) 2016 Beem Media. All Rights Reserved.

#include "ExKeyboardMenu.h"
#include "ExMenu.h"
#include "ExUiSounds.h"
#include "EGKbCharHandler.h"
#include "EGUiGridWidget.h"
#include "EGUiButtonWidget.h"

struct exKeyboardMenuCreate
{
	const eg_loc_char* ValidChars;
	const eg_loc_char* InitialString;
	IExKeyboardMenuCb* CbInterface;
	eg_size_t          MaxChars;

	exKeyboardMenuCreate( eg_ctor_t Ct )
	{
		if( CT_Clear == Ct )
		{
			ValidChars = nullptr;
			CbInterface = nullptr;
			MaxChars = 0;
		}
	}
};

static exKeyboardMenuCreate ExKeyboardMenu_CreateInfo( CT_Clear );

void ExKeyboardMenu_PushMenu( class EGClient* Owner , const eg_loc_char* ValidChars , IExKeyboardMenuCb* CbInterface , eg_size_t MaxChars , const eg_loc_char* InitialString )
{
	ExKeyboardMenu_CreateInfo.ValidChars = ValidChars;
	ExKeyboardMenu_CreateInfo.CbInterface = CbInterface;
	ExKeyboardMenu_CreateInfo.MaxChars = MaxChars;
	ExKeyboardMenu_CreateInfo.InitialString = InitialString;
	Owner->SDK_PushMenu( eg_crc("KeyboardMenu") );
	ExKeyboardMenu_CreateInfo = exKeyboardMenuCreate( CT_Clear );
}

class ExKeyboardMenu: public ExMenu , public EGKbCharHandler::IListener
{
	EG_CLASS_BODY( ExKeyboardMenu , ExMenu )

private:
	
	IExKeyboardMenuCb*   m_CbInterface;
	eg_size_t            m_MaxChars;
	EGArray<eg_loc_char> m_ValidChars;
	EGArray<eg_loc_char> m_CurString;
	eg_real              m_BlinkTime;
	eg_bool              m_bBlinkerOn:1;

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		SetIgnoreAltNav( true );

		m_CbInterface = ExKeyboardMenu_CreateInfo.CbInterface;
		m_MaxChars = ExKeyboardMenu_CreateInfo.MaxChars;
		const eg_loc_char* ValidChars = ExKeyboardMenu_CreateInfo.ValidChars;
		if( ValidChars )
		{
			while( *ValidChars )
			{
				m_ValidChars.Append( *ValidChars );
				ValidChars++;
			}
		}

		const eg_loc_char* InitialString = ExKeyboardMenu_CreateInfo.InitialString;
		if( InitialString )
		{
			while( *InitialString )
			{
				m_CurString.Append( *InitialString );
				InitialString++;
			}
		}

		RefreshText();

		EGUiGridWidget* Keys = GetWidget<EGUiGridWidget>( eg_crc("Keys") );
		if( Keys )
		{
			Keys->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
			Keys->OnItemPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
			Keys->RefreshGridWidget( m_ValidChars.LenAs<eg_uint>() );
		}

		RefreshHints();
		DoReveal();
	}

	virtual void OnActivate() override final
	{
		Super::OnActivate();
		EGKbCharHandler::Get().AddListener( this , true );
		SetFocusedWidget( eg_crc("Keys") , 0 , false );
	}

	virtual void OnDeactivate() override final
	{
		Super::OnDeactivate();
		EGKbCharHandler::Get().RemoveListener( this );
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );
		m_BlinkTime += DeltaTime;
		if( m_BlinkTime > .4f )
		{
			m_BlinkTime = 0.f;
			m_bBlinkerOn = !m_bBlinkerOn;
			RefreshText();
		}
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			CommitChanges();
			return true;
		}

		return false;
	}

	virtual eg_bool HandleTypedChar( eg_char16 Char ) override final
	{
		eg_bool bHandled = false;

		if( Char == '\b' )
		{
			// Handled by OnInputCmds
			bHandled = false;
		}
		else if( m_CurString.Len() < m_MaxChars )
		{
			auto IsValidChar = [this]( eg_char16 Char ) -> eg_bool
			{
				for( eg_size_t i=0; i<m_ValidChars.Len(); i++ )
				{
					if( Char == m_ValidChars[i] )
					{
						return true;
					}
				}

				return false;
			};

			if( IsValidChar( Char ) )
			{
				m_CurString.Append( Char );
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
				RefreshText();
				bHandled = true;
			}
		}

		return bHandled;
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds )
	{
		if( Cmds.WasPressedOrRepeated( CMDA_KBMENU_BKSP ) )
		{
			EraseChar();
		}
	}

	void EraseChar()
	{
		if( m_CurString.Len() > 0 )
		{
			m_CurString.DeleteByIndex( m_CurString.Len()-1 );
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
			RefreshText();
		}
	}

	void GetCurString( eg_d_string16& Out )
	{
		Out.Clear();
		EGArray<eg_loc_char> TempArray = m_CurString;
		TempArray.Append( '\0' );
		Out = TempArray.GetArray();
	}

	void RefreshText()
	{
		if( EGUiTextWidget* EnteredText = GetWidget<EGUiTextWidget>( eg_crc("EnteredText") ) )
		{
			eg_d_string16 LocText;
			GetCurString( LocText );
			if( m_bBlinkerOn )
			{
				LocText.Append( L"_" );
			}
			EnteredText->SetText( CT_Clear , eg_loc_text( *LocText ) );
		}

		if( EGUiTextWidget* LimitTextWidget = GetWidget<EGUiTextWidget>( eg_crc("LimitText") ) )
		{
			const eg_loc_text LimitText = EGFormat( eg_loc("KeyboardMenuLimitText","{0}/{1} Characters") , m_CurString.LenAs<eg_int>() , EG_To<eg_int>(m_MaxChars) );
			LimitTextWidget->SetText( CT_Clear , LimitText );
		}


		RefreshHints();
	}

	void CommitChanges()
	{
		eg_d_string16 LocText;
		GetCurString( LocText );
		if( m_CbInterface )
		{
			m_CbInterface->OnTextFromKeyboardMenu( *LocText );
		}
		MenuStack_Pop();
	}

	void OnObjPressed( const egUIWidgetEventInfo& Info )
	{
		switch_crc( Info.WidgetId )
		{
			case_crc("ContinueButton"):
			{
				CommitChanges();
			} break;

			case_crc("Keys"):
			{
				if( m_ValidChars.IsValidIndex( Info.GridIndex ) )
				{
					HandleTypedChar( m_ValidChars[Info.GridIndex] );
				}
			} break;

			case_crc("EraseButton"):
			{
				EraseChar();
			} break;
		}
	}

	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );
		
		if( ItemInfo.WidgetId == eg_crc("Keys") )
		{
			if( m_ValidChars.IsValidIndex( ItemInfo.GridIndex ) )
			{
				if( m_ValidChars[ItemInfo.GridIndex] == ' ' )
				{
					ItemInfo.Widget->SetText( eg_crc("KeyText") , eg_loc_text(eg_loc("KeyboardMenuSpaceBarKey","SPC")) );
				}
				else
				{
					const eg_loc_char KeyStr[] = { m_ValidChars[ItemInfo.GridIndex] , '\0' };
					ItemInfo.Widget->SetText( eg_crc("KeyText") , eg_loc_text( KeyStr ) );
				}
			}
		}
	}

	void RefreshHints()
	{
		ClearHints();
		if( m_CurString.Len() > 0 )
		{
			AddHint( CMDA_KBMENU_BKSP , eg_loc_text(eg_loc("KeyboardMenuEraseCharacter","Erase Character")) );
		}
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("KeybaordMenuBackOut","Back")) );
	}
};

EG_CLASS_DECL( ExKeyboardMenu )
