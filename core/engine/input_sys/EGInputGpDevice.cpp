///////////////////////////////////////////////////////////////////////////////
// EGGpDevice 
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////
#include "EGInputGpDevice.h"
#include "EGWindowsAPI.h"
#include "EGSettings2Types.h"
#include <XInput.h>

#include "EGDebugText.h"

static EGSettingsReal EGGpDevice_AxisButtonThreshold( "EGGpDevice.AxisButtonThreshold" , CT_Clear , .5f , EGS_F_USER_SAVED|EGS_F_EDITABLE );
static EGSettingsReal EGGpDevice_TriggerButtonThreshold( "EGGpDevice.TriggerButtonThreshold" , CT_Clear , .5f , EGS_F_USER_SAVED|EGS_F_EDITABLE );

void EGGpDevice::InitDevices()
{
// There's no version information in the XInput header but
// XINPUT_DEVSUBTYPE_ARCADE_PAD is unique to 1.4, so we can at least explain
// what is going on. The game will work fine with newer versions since the api
// hasn't changed, but it might not be compatible with older operating systems.
#if defined(XINPUT_DEVSUBTYPE_ARCADE_PAD)
	#pragma __NOTICE__("EXPLOR was originally built with xinput1_3.dll, but is being built with " XINPUT_DLL_A)
#else
	XInputEnable( TRUE );
#endif
}

void EGGpDevice::DeinitDevices()
{
#if defined(XINPUT_DEVSUBTYPE_ARCADE_PAD)
	// We already warned.
#else
	XInputEnable( FALSE );
#endif
}

EGGpDevice::EGGpDevice()
: m_Id( 0 )
, m_Connected( false )
, m_BtnState( 0 )
, m_bHadInputThisFrame( false )
, m_RSmoothAxis(0.f,0.f)
, m_LSmoothAxis(0.f,0.f)
, m_RAxis(0.f,0.f)
, m_LAxis(0.f,0.f)
{

}

EGGpDevice::~EGGpDevice()
{
}

void EGGpDevice::Init( eg_uint Id )
{
	m_Id = Id;
	assert( 0 <= Id && Id < XUSER_MAX_COUNT ); //Invalid Gamepad Id.
}

void EGGpDevice::Update()
{
	if( !(0<= m_Id && m_Id < XUSER_MAX_COUNT ) )
	{
		return;
	}

	XINPUT_STATE State;
	zero( &State );
	DWORD Res = XInputGetState( m_Id , &State );

	m_Connected = ERROR_SUCCESS == Res;

	m_RAxis = CT_Clear;
	m_LAxis = CT_Clear;
	m_BtnState.Clear();

	if( m_Connected )
	{
		//Get button states:
		//Regular buttons:
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_DPAD_UP)        ){ m_BtnState.Set( ButtonToFlag( GP_BTN_DUP          ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_DPAD_DOWN)      ){ m_BtnState.Set( ButtonToFlag( GP_BTN_DDOWN        ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_DPAD_LEFT)      ){ m_BtnState.Set( ButtonToFlag( GP_BTN_DLEFT        ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_DPAD_RIGHT)     ){ m_BtnState.Set( ButtonToFlag( GP_BTN_DRIGHT       ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_START)          ){ m_BtnState.Set( ButtonToFlag( GP_BTN_START            ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_BACK)           ){ m_BtnState.Set( ButtonToFlag( GP_BTN_BACK             ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_LEFT_THUMB)     ){ m_BtnState.Set( ButtonToFlag( GP_BTN_L3       ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_RIGHT_THUMB)    ){ m_BtnState.Set( ButtonToFlag( GP_BTN_R3      ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_LEFT_SHOULDER)  ){ m_BtnState.Set( ButtonToFlag( GP_BTN_L1    ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_RIGHT_SHOULDER) ){ m_BtnState.Set( ButtonToFlag( GP_BTN_R1   ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_A)              ){ m_BtnState.Set( ButtonToFlag( GP_BTN_A                ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_B)              ){ m_BtnState.Set( ButtonToFlag( GP_BTN_B                ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_X)              ){ m_BtnState.Set( ButtonToFlag( GP_BTN_X                ) ); }
		if( 0 != (State.Gamepad.wButtons&XINPUT_GAMEPAD_Y)              ){ m_BtnState.Set( ButtonToFlag( GP_BTN_Y                ) ); }

		auto CalculateAxis = []( eg_int AxisIn , eg_int DeadZoneIn ) -> eg_real
		{
			eg_real Out = 0.f;

			if( AxisIn > DeadZoneIn )
			{
				Out = EGMath_GetMappedRangeValue( EG_To<eg_real>(AxisIn) , eg_vec2(EG_To<eg_real>(DeadZoneIn),32767.f) , eg_vec2(0.f,1.f) );
			}
			else if( AxisIn < -DeadZoneIn )
			{
				Out = EGMath_GetMappedRangeValue( EG_To<eg_real>(AxisIn) , eg_vec2(-32768.f,EG_To<eg_real>(-DeadZoneIn)) , eg_vec2(-1.f,0.f) );
			}

			return Out;
		};

		auto CalculateTriggerAxis = []( eg_uint8 AxisIn , eg_int DeadZoneIn ) -> eg_real
		{
			eg_real Out = 0.f;

			if( AxisIn > DeadZoneIn )
			{
				Out = EGMath_GetMappedRangeValue( EG_To<eg_real>(AxisIn) , eg_vec2(EG_To<eg_real>(DeadZoneIn),256.f) , eg_vec2(0.f,1.f) );
			}

			return Out;
		};

		m_RAxis.x = CalculateAxis( State.Gamepad.sThumbRX , XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
		m_RAxis.y = CalculateAxis( State.Gamepad.sThumbRY , XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );

		m_LAxis.x = CalculateAxis( State.Gamepad.sThumbLX , XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
		m_LAxis.y = CalculateAxis( State.Gamepad.sThumbLY , XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );

		//Left stick buttons:
		const eg_real AxisButtonPressThreshold = EGGpDevice_AxisButtonThreshold.GetValue();
		if( m_LAxis.x>  AxisButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_LRIGHT ) ); }
		if( m_LAxis.x < -AxisButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_LLEFT ) ); }
		if( m_LAxis.y >  AxisButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_LUP ) ); }
		if( m_LAxis.y < -AxisButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_LDOWN ) ); }
		//Right stick buttons:
		if( m_RAxis.x>  AxisButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_RRIGHT ) ); }
		if( m_RAxis.x < -AxisButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_RLEFT ) ); }
		if( m_RAxis.y >  AxisButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_RUP ) ); }
		if( m_RAxis.y < -AxisButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_RDOWN ) ); }

		const eg_real LeftTriggerAxis = CalculateTriggerAxis( State.Gamepad.bLeftTrigger , XINPUT_GAMEPAD_TRIGGER_THRESHOLD );
		const eg_real RightTriggerAxis = CalculateTriggerAxis( State.Gamepad.bRightTrigger , XINPUT_GAMEPAD_TRIGGER_THRESHOLD );

		//Trigger buttons
		const eg_real TriggerButtonPressThreshold = EGGpDevice_TriggerButtonThreshold.GetValue();
		if( LeftTriggerAxis > TriggerButtonPressThreshold  ){ m_BtnState.Set( ButtonToFlag( GP_BTN_L2 ) ); }
		if( RightTriggerAxis > TriggerButtonPressThreshold ){ m_BtnState.Set( ButtonToFlag( GP_BTN_R2 ) ); }

		m_bHadInputThisFrame = m_BtnState != eg_flags() || m_RAxis.x != 0 || m_RAxis.y != 0 || m_LAxis.x != 0 || m_LAxis.y != 0;
	}
}

eg_bool EGGpDevice::IsHeld( eg_gp_btn Btn )const
{
	return m_BtnState.IsSet( ButtonToFlag( Btn ) );
}

eg_flag EGGpDevice::ButtonToFlag( eg_gp_btn Btn )
{
	return (1<<static_cast<eg_uint>(Btn));
}