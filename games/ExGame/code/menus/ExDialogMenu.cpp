// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "ExBgComponent.h"
#include "EGUiGridWidget.h"
#include "EGInput.h"
#include "ExGlobalData.h"

static const exDialogParms* ExDialogMenu_NextParms = nullptr;
static const exKeyBindDialogParms* ExDialogMenu_KeyBindDialogParms = nullptr;

void ExDialogMenu_PushDialogMenu( class EGClient* Client, const exDialogParms& Parms )
{
	assert( nullptr == ExDialogMenu_NextParms && Client ); // A dialog should not push another dialog...
	ExDialogMenu_NextParms = &Parms;
	if( Client )
	{
		Client->SDK_PushMenu( eg_crc("Dialog") );
	}
	ExDialogMenu_NextParms = nullptr;
}


void ExDialogMenu_PushKeyBindDialog( class EGClient* Client , const exKeyBindDialogParms& Parms )
{
	assert( nullptr == ExDialogMenu_KeyBindDialogParms && Client ); // A dialog should not push another dialog...
	ExDialogMenu_KeyBindDialogParms = &Parms;
	if( Client )
	{
		Client->SDK_PushMenu( eg_crc("KeyBindDialog") );
	}
	ExDialogMenu_KeyBindDialogParms = nullptr;
}

class ExDialog : public ExMenu
{
	EG_CLASS_BODY( ExDialog , ExMenu )

private:

	EGArray<eg_string_crc> m_Choices;
	IExDialogListener*     m_Listener;
	eg_string_crc          m_ListenerParm;
	eg_string_crc          m_RemoteEventOnClose;
	ExDialogDelegate       m_DelegateCb;
	ExKeyBindDialogDelegate m_KeyBindCb;
	EGUiGridWidget*        m_ChoiceList;
	eg_bool m_bIsKeyBindDialog = false;
	EGArray<eg_kbm_btn> m_KbButtonsPressed;
	EGArray<EGArray<eg_gp_btn>> m_GpButtonsPressed;

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		m_RemoteEventOnClose = CT_Clear;
		m_DelegateCb.Clear();
		m_KeyBindCb.Clear();
		m_bIsKeyBindDialog = false;

		m_ChoiceList = GetWidget<EGUiGridWidget>( eg_crc("ChoiceList") );

		if( ExDialogMenu_NextParms )
		{
			if( !ExDialogMenu_NextParms->RemoteEventOnClose.IsNull() )
			{
				m_RemoteEventOnClose = ExDialogMenu_NextParms->RemoteEventOnClose;
			}
			m_Listener = ExDialogMenu_NextParms->DlgListener;
			m_ListenerParm = ExDialogMenu_NextParms->DlgListenerParm;
			m_DelegateCb = ExDialogMenu_NextParms->DelegateCb;
			m_Choices = ExDialogMenu_NextParms->Choices;
			if( m_Choices.Len() == 0 )
			{
				m_Choices.Append( eg_crc("Okay") );
			}
			EGUiWidget* TextHeader = GetWidget( eg_crc("TextHeader") );
			EGUiWidget* TextBody = GetWidget( eg_crc("TextBody") );
			if( TextHeader && TextBody )
			{
				TextHeader->SetText( eg_crc("") , ExDialogMenu_NextParms->HeaderText );
				TextHeader->SetPalette( 0 , eg_color( ExGlobalData::Get().GetUiColors().DefaultDialogHeaderColor ).ToVec4() );
				TextBody->SetText( eg_crc("") , ExDialogMenu_NextParms->BodyText );
				TextBody->SetPalette( 0 , eg_color( ExGlobalData::Get().GetUiColors().DefaultDialogBodyColor ).ToVec4() );
				TextHeader->SetVisible( false );
				TextBody->SetVisible( false );
			}

			if( m_ChoiceList )
			{
				m_ChoiceList->OnItemChangedDelegate.Bind( this , &ThisClass::OnChoiceItemChanged );
				m_ChoiceList->OnItemPressedDelegate.Bind( this , &ThisClass::OnChoiceItemClicked );
				m_ChoiceList->RefreshGridWidget( m_Choices.LenAs<eg_uint>() );
			}

			DoReveal( static_cast<eg_uint>(m_Choices.Len()) , ExDialogMenu_NextParms->bMuteIntro );
			SetFocusedWidget( eg_crc("ChoiceList") , ExDialogMenu_NextParms->InitialChoice , false );
		}
		else if( ExDialogMenu_KeyBindDialogParms )
		{
			m_Listener = nullptr;
			m_ListenerParm = CT_Clear;
			m_DelegateCb.Clear();
			m_Choices.Clear();
			m_bIsKeyBindDialog = true;
			m_KeyBindCb = ExDialogMenu_KeyBindDialogParms->DelegateCb;

			EGUiWidget* TextHeader = GetWidget( eg_crc("TextHeader") );
			EGUiWidget* TextBody = GetWidget( eg_crc("TextBody") );
			if( TextHeader && TextBody )
			{
				TextHeader->SetText( eg_crc("") , ExDialogMenu_KeyBindDialogParms->HeaderText );
				TextHeader->SetPalette( 0 , eg_color(ExColor_HeaderYellow).ToVec4() );
				TextBody->SetText( eg_crc("") , ExDialogMenu_KeyBindDialogParms->BodyText );
				TextBody->SetPalette( 0 , eg_color::White.ToVec4() );
				TextHeader->SetVisible( false );
				TextBody->SetVisible( false );
			}

			DoReveal( 0 , false );
			SetFocusedWidget( CT_Clear , 0 , false );
		}
		else
		{
			assert( false ); // Dialog should only be pushed with ExDialogMenu_PushDialogMenu
			MenuStack_Pop();
		}
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( m_bIsKeyBindDialog )
		{
			// Don't want any processing of input if this is key bind dialog.
			return true;
		}

		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			if( m_Choices.Len() <= 1 && m_Choices.IsValidIndex(0) )
			{
				MenuStack_Pop();
				if( m_Listener )
				{
					m_Listener->OnDialogClosed( m_ListenerParm, m_Choices[0] );
				}
				if( GetOwnerClient() && !m_RemoteEventOnClose.IsNull() )
				{
					GetOwnerClient()->SDK_RunServerEvent( egRemoteEvent( m_RemoteEventOnClose , 0 ) );
					GetOwnerClient()->SDK_RunClientEvent( egRemoteEvent( m_RemoteEventOnClose , 0 ) );
				}
				m_DelegateCb.ExecuteIfBound( m_ListenerParm , m_Choices[0] );
			}

			return true;
		}

		return false;
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		if( m_bIsKeyBindDialog )
		{
			// Process raw keyboard
			{
				const EGInput::egKbmDeviceInfo& KbDevice = EGInput::Get().GetRawKbmDeviceInfo();
				const egInputKbmData& KbmData = KbDevice.Data;
				for( eg_size_t i = 0; i < eg_kbm_btn::KBM_BTN_COUNT; i++ )
				{
					eg_kbm_btn Btn = EG_To<eg_kbm_btn>(i);
					if( KbmData.IsReleased( Btn ) && m_KbButtonsPressed.Contains( Btn ) )
					{
						m_KbButtonsPressed.DeleteByItem( Btn );
						m_KeyBindCb.ExecuteIfBound( Btn , eg_gp_btn::GP_BTN_NONE );
					}

					if( KbmData.IsPressed( Btn ) )
					{
						m_KbButtonsPressed.AppendUnique( Btn );
					}
				}
			}

			// Process raw gamepad
			{
				const eg_size_t NumGamepads = EGInput::Get().GetNumGpDevices();
				m_GpButtonsPressed.Resize( NumGamepads );
				for( eg_int GpIndex=0; GpIndex<m_GpButtonsPressed.Len(); GpIndex++ )
				{
					const EGInput::egGpDeviceInfo& GpDeviceInfo = EGInput::Get().GetRawGpDeviceInfo( GpIndex );
					const EGGpDevice& GpDevice = GpDeviceInfo.Gp;
					EGArray<eg_gp_btn>& GpKeyArray = m_GpButtonsPressed[GpIndex];

					for( eg_size_t i = 0; i < eg_gp_btn::GP_BTN_COUNT; i++ )
					{
						eg_gp_btn Btn = EG_To<eg_gp_btn>(i);
						if( !GpDevice.IsHeld( Btn ) && GpKeyArray.Contains( Btn ) )
						{
							GpKeyArray.DeleteByItem( Btn );
							m_KeyBindCb.ExecuteIfBound( eg_kbm_btn::KBM_BTN_NONE , Btn );
						}

						if( GpDevice.IsHeld( Btn ) && !GpKeyArray.Contains( Btn ) )
						{
							GpKeyArray.AppendUnique( Btn );
						}
					}
				}
			}
		}
	}

	void OnChoiceItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

		if( m_Choices.IsValidIndex(ItemInfo.GridIndex) )
		{
			ItemInfo.Widget->SetText( eg_crc("LabelText") , eg_loc_text(m_Choices[ItemInfo.GridIndex]) );
		}
	}

	void OnChoiceItemClicked( const egUIWidgetEventInfo& Info )
	{
		EGClient* Client = GetOwnerClient();
		if( nullptr == Client )
		{
			assert( false ); // This menu must be associated with a client.
			return;
		}

		if( m_Choices.IsValidIndex( Info.GridIndex ) )
		{
			PlaySoundEvent( ExUiSounds::ex_sound_e::ConfirmDialogChoice );
			MenuStack_Pop();
			if( m_Listener )
			{
				m_Listener->OnDialogClosed( m_ListenerParm , m_Choices[Info.GridIndex] );
			}
			if( !m_RemoteEventOnClose.IsNull() )
			{
				Client->SDK_RunServerEvent( egRemoteEvent( m_RemoteEventOnClose , m_Choices[Info.GridIndex] ) );
				Client->SDK_RunClientEvent( egRemoteEvent( m_RemoteEventOnClose , m_Choices[Info.GridIndex] ) );
			}
			m_DelegateCb.ExecuteIfBound( m_ListenerParm , m_Choices[Info.GridIndex] );
		}
	}
};

EG_CLASS_DECL( ExDialog )
