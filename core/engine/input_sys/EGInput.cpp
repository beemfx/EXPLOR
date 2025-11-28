///////////////////////////////////////////////////////////////////////////////
// EGInput 
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////

#include "EGInput.h"
#include "EGWindowsAPI.h"
#include "EGInputKbmDevice.h"
#include "EGFileData.h"
#include "EGEngineConfig.h"
#include "EGGlobalConfig.h"
#include "EGDebugText.h"

static EGSettingsReal InputConfig_CursorSenitivity( "InputConfig.CursorSenitivity" , eg_loc("CursorSenitivity","Cursor Sensitivity") , 0.05f , EGS_F_USER_SAVED );
static EGSettingsClampedReal InputConfig_MouseAxisSensitivity( "InputConfig.MouseAxisSensitivity" , eg_loc("MouseAxisSensitivy","Mouse Axis Sensitivity") , 1.f , EGS_F_USER_SAVED|EGS_F_EDITABLE , .5f , 10.f , .1f );
static EGSettingsClampedReal InputConfig_GamepadAxisSensitivity( "InputConfig.GamepadAxisSensitivity" , eg_loc("GamepadAxisSensitivy","Gamepad Axis Sensitivity") , 1.f , EGS_F_USER_SAVED|EGS_F_EDITABLE , .5f , 10.f , .1f );
static EGSettingsClampedInt InputConfig_MouseSmoothSamples( "InputConfig.MouseSmoothSamples" , eg_loc("MouseSmoothing","Mouse Smoothing") , 1 , EGS_F_USER_SAVED|EGS_F_EDITABLE , 1 , EGForceSampler::MAX_SAMPLES-1 , 1 );
static EGSettingsClampedInt InputConfig_GamepadSmoothSamples( "InputConfig.GamepadSmoothSamples" , eg_loc("GamepadSmoothing","Gamepad Smoothing") , 1 , EGS_F_USER_SAVED|EGS_F_EDITABLE , 1 , EGForceSampler::MAX_SAMPLES-1 , 1 );

EGInput EGInput::s_Input;

EGInput::EGInput()
: m_CharBuffer()
, m_fAspect(1.0f)
{	
	
}

void EGInput::SDK_RegisterInput( eg_cmd_t Cmd , eg_cpstr StrName , eg_cpstr DefaultGpButton , eg_cpstr DefaultKbButton )
{
	ButtonNames_SetCmdName( Cmd , StrName );

	eg_gp_btn GpButton = InputButtons_StringToGpButton(DefaultGpButton);
	eg_kbm_btn KbButton = InputButtons_StringToKbmButton(DefaultKbButton);
	m_DefaultBindings.Bind( Cmd , GpButton , KbButton );
}

void EGInput::InitMouse()
{
	m_Kbm.MouseAxis.x=0.0f;
	m_Kbm.MouseAxis.y=0.0f;
	//Start mouse int he center of the screen.
	m_Kbm.MousePos.x=0.0f;
	m_Kbm.MousePos.y=0.0f;
}

EGInput::~EGInput()
{
	
}

void EGInput::Init()
{
	ButtonNames_Init();

	m_CharBuffer.Clear();
	for( eg_uint i=0; i<countof(m_GpInfos); i++ )
	{
		m_GpInfos[i].Init( i );
	}

	m_Kbm.bHadInputThisFrame = false;

	InitMouse();

	//Default input table:

	static const struct egDefaultBinding
	{
		eg_cmd_t Action;
		eg_cpstr Name;
		eg_cpstr GpButtonStr;
		eg_cpstr KbButtonStr;
	}
	DefaultMap[] =
	{
		{ CMDA_MENU_PRIMARY      , "EG_MENU_PRIMARY"      , "GP_A"       , "KBM_RETURN"       },
		{ CMDA_MENU_BACK         , "EG_MENU_BACK"         , "GP_B"       , "KBM_ESCAPE"       },
		{ CMDA_MENU_LMB          , "EG_MENU_LMB"          , ""           , "KBM_MOUSE_1"      },
		{ CMDA_MENU_RMB          , "EG_MENU_RMB"          , ""           , "KBM_MOUSE_2"      },
		{ CMDA_MENU_UP           , "EG_MENU_UP"           , "GP_DUP"     , "KBM_UP"           },
		{ CMDA_MENU_DOWN         , "EG_MENU_DOWN"         , "GP_DDOWN"   , "KBM_DOWN"         },
		{ CMDA_MENU_LEFT         , "EG_MENU_LEFT"         , "GP_DLEFT"   , "KBM_LEFT"         },
		{ CMDA_MENU_RIGHT        , "EG_MENU_RIGHT"        , "GP_DRIGHT"  , "KBM_RIGHT"        },
		{ CMDA_MENU_SCRLUP       , "EG_MENU_SCRLUP"       , ""           , "KBM_MOUSE_W_UP"   },
		{ CMDA_MENU_SCRLDOWN     , "EG_MENU_SCRLDOWN"     , ""           , "KBM_MOUSE_W_DOWN" },
		{ CMDA_MENU_BTN2	       , "EG_MENU_BTN2"         , "GP_X"       , "KBM_SPACE"        },
		{ CMDA_MENU_BTN3	       , "EG_MENU_BTN3"         , "GP_Y"       , "KBM_LSHIFT"       },
		{ CMDA_MENU_NEXT_PAGE    , "EG_MENU_NEXT_PAGE"    , "GP_R1"      , "KBM_RBRACKET"     },
		{ CMDA_MENU_PREV_PAGE    , "EG_MENU_PREV_PAGE"    , "GP_L1"      , "KBM_LBRACKET"     },
		{ CMDA_MENU_NEXT_SUBPAGE , "EG_MENU_NEXT_SUBPAGE" , "GP_R2"      , "KBM_PERIOD"       },
		{ CMDA_MENU_PREV_SUBPAGE , "EG_MENU_PREV_SUBPAGE" , "GP_L2"      , "KBM_COMMA"        },
		{ CMDA_MENU_PRIMARY2     , "EG_MENU_PRIMARY2"     , ""           , "" },
		{ CMDA_MENU_BACK2        , "EG_MENU_BACK2"        , ""           , "" },
		{ CMDA_MENU_UP2          , "EG_MENU_UP2"          , "GP_LUP"     , "KBM_W" },
		{ CMDA_MENU_DOWN2        , "EG_MENU_DOWN2"        , "GP_LDOWN"   , "KBM_S" },
		{ CMDA_MENU_LEFT2        , "EG_MENU_LEFT2"        , "GP_LLEFT"   , "KBM_A" },
		{ CMDA_MENU_RIGHT2       , "EG_MENU_RIGHT2"       , "GP_LRIGHT"  , "KBM_D" },
	};

	for( eg_uint i=0; i<countof(DefaultMap); i++ )
	{
		SDK_RegisterInput( DefaultMap[i].Action , DefaultMap[i].Name, DefaultMap[i].GpButtonStr , DefaultMap[i].KbButtonStr );
	}

	EGGpDevice::InitDevices();
}

void EGInput::Deinit()
{
	for( eg_uint i=0; i<countof(m_GpInfos); i++ )
	{
		m_GpInfos[i].Deinit();
	}
	EGGpDevice::DeinitDevices();
}

eg_bool EGInput::WasThereInputThisFrameFor( eg_input_device_id DeviceId ) const
{
	eg_bool bOut = false;

	switch( DeviceId )
	{
		case eg_input_device_id::COUNT:
		case eg_input_device_id::NONE: bOut = false; break;
		case eg_input_device_id::KBM: bOut = m_Kbm.bHadInputThisFrame; break;
		case eg_input_device_id::GP1: bOut = m_GpInfos[0].Gp.HadInputThisFrame(); break;
		case eg_input_device_id::GP2: bOut = m_GpInfos[1].Gp.HadInputThisFrame(); break;
		case eg_input_device_id::GP3: bOut = m_GpInfos[2].Gp.HadInputThisFrame(); break;
		case eg_input_device_id::GP4: bOut = m_GpInfos[3].Gp.HadInputThisFrame(); break;
	}

	return bOut;
}

void EGInput::Bind( EGInputBindings& Bindings, eg_cpstr StrAction, eg_cpstr StrGp, eg_cpstr StrKb ) const
{
	eg_cmd_t Action   = ButtonNames_StringToAction( StrAction );
	eg_gp_btn   GpButton = InputButtons_StringToGpButton( StrGp );
	eg_kbm_btn  KbButton = InputButtons_StringToKbmButton( StrKb );

	if( CMDA_UNK == Action || (GpButton == GP_BTN_NONE && KbButton == GP_BTN_NONE) )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": %s was not mapped to an action!" , StrAction );
		// assert( false ); //Can't bind to nothing.
		return;
	}

	Bindings.Bind( Action , GpButton , KbButton );
}

void EGInput::WriteBindings( const EGInputBindings& InputBindings , EGFileData& fout , eg_cpstr strFormat )const
{
	WriteBindingsForDevice( InputBindings , fout , strFormat );
}

void EGInput::WriteBindingsForDevice( const EGInputBindings& Bindings , class EGFileData& fout , eg_cpstr strFormat )const
{
	for( eg_uint i=0; i<CMD_MAX; i++ )
	{
		eg_cmd_t Action = static_cast<eg_cmd_t>(i);

		if( CMDA_UNK == Action )continue;

		eg_gp_btn GpButton = Bindings.ActionToGp( Action );
		eg_kbm_btn KbButton = Bindings.ActionToKb( Action );
		eg_cpstr ActionString = ButtonNames_ActionToString( Action );
		eg_cpstr GpString = InputButtons_GpButtonToString( GpButton );
		eg_cpstr KbString = InputButtons_KbmButtonToString( KbButton );
		eg_string strOut = EGString_Format( strFormat , ActionString , GpString , KbString );
		eg_char8 strMB[1024];
		strOut.CopyTo( strMB, countof(strMB) );
		// Only write the command if it had a binding
		if( EGString_StrLen( ActionString ) > 0 )
		{
			fout.Write( strMB , strOut.Len()*sizeof(strMB[0]) );
		}
	}
}

void EGInput::OnChar(eg_char c)
{
	m_CharBuffer.InsertLastAndOverwriteFirstIfFull( c );
}

eg_char EGInput::GetChar()
{
	eg_char c;
	if( m_CharBuffer.Len() > 0 )
	{
		c = m_CharBuffer[0];
		m_CharBuffer.RemoveFirst();
	}
	else
	{
		c = 0;
	}
	return c;
}

eg_string_crc EGInput::InputActionToKey( const EGInputBindings& Bindings , eg_string_crc Action, eg_bool bGP )
{
	eg_cmd_t Cmd = ButtonNames_StringCrcToAction( Action );

	return Cmd != CMDA_UNK ? Bindings.InputActionToKey( Cmd , bGP ) : Action;
}

eg_cpstr EGInput::InputActionToKeyStr( const EGInputBindings& Bindings , eg_string_crc Action , eg_bool bGP )
{
	eg_cmd_t Cmd = ButtonNames_StringCrcToAction( Action );

	return Cmd != CMDA_UNK ? Bindings.InputActionToKeyStr( Cmd , bGP ) : "";
}

eg_cpstr EGInput::InputCmdToName( eg_cmd_t Cmd )
{
	return ButtonNames_ActionToString( Cmd );
}

void EGInput::SetMousePosFromScreenPosPercent( eg_real PercentXFromRight, eg_real PercentYFromTop )
{
	m_Kbm.MousePos.x =-1.0f + 2.0f*(PercentXFromRight);
	m_Kbm.MousePos.y = 1.0f - 2.0f*(PercentYFromTop);

	// m_Kbm.MousePos.x=EG_Clamp<eg_real>(m_Kbm.MousePos.x, -1.0f, 1.0f);
	// m_Kbm.MousePos.y=EG_Clamp<eg_real>(m_Kbm.MousePos.y, -1.0f, 1.0f);
}

void EGInput::Update( eg_real DeltaTime , eg_bool WantCursorPos , eg_real Aspect )
{
	m_Kbm.bHadInputThisFrame = false;

	m_fAspect = Aspect;
	INPUT_MOUSE_MODE MouseMode = INPUT_MOUSE_MODE_DELTA;
	if( WantCursorPos )
	{
		if( IsMousePosFromOS() )
		{
			MouseMode = INPUT_MOUSE_MODE_CURSOR;
		}
	}
	InputKbmDevice_SetMouseMode( MouseMode );

	egRawKbmData PltInputData( CT_Clear );
	InputKbmDevice_GetStateAndReset( &PltInputData );
	m_Kbm.bHadInputThisFrame = PltInputData.bHadInput;
	//Process typed characters.
	//Always clear out the character buffer, just in case no window
	//decided to get input.
	m_CharBuffer.Clear();
	for( eg_uint i=0; i<PltInputData.KbData.CharBufferCount; i++ )
	{
		OnChar( PltInputData.KbData.CharBuffer[i] );
	}

	//Compute the keyboard state.
	m_Kbm.Data = PltInputData.KbData;
	for( eg_uint i=0; i<countof(m_Kbm.Data.Pressed); i++ )
	{
		m_Kbm.DataPressedMeansHeld.Pressed[i].Set( m_Kbm.Data.Pressed[i] );
		m_Kbm.DataPressedMeansHeld.Pressed[i].UnSet( m_Kbm.Data.Released[i] );
	}
	if( IsMousePosFromOS() )
	{
		SetMousePosFromScreenPosPercent( PltInputData.MousePos.x , PltInputData.MousePos.y );
	}

	for( eg_uint i=0; i<countof(m_GpInfos); i++ )
	{
		EGGpDevice& GpDevice = m_GpInfos[i].Gp;
		GpDevice.Update();

		m_GpLSSmoothSamples[i].SetMaxSamples( InputConfig_GamepadSmoothSamples.GetValue() );
		m_GpLSSmoothSamples[i].SetMaxAge( 0.f );
		m_GpLSSmoothSamples[i].Update( eg_vec3( GpDevice.GetLAxis( false ) , 0.f ) , DeltaTime );
		const eg_vec2 LSampled = m_GpLSSmoothSamples[i].GetSmoothValue().XY_ToVec2();

		m_GpRSSmoothSamples[i].SetMaxSamples( InputConfig_GamepadSmoothSamples.GetValue() );
		m_GpRSSmoothSamples[i].SetMaxAge( 0.f );
		m_GpRSSmoothSamples[i].Update( eg_vec3( GpDevice.GetRAxis( false ) , 0.f ) , DeltaTime );
		const eg_vec2 RSampled = m_GpRSSmoothSamples[i].GetSmoothValue().XY_ToVec2();
		GpDevice.SetLSmoothAxis( LSampled );
		GpDevice.SetRSmoothAxis( RSampled );
	}

	UpdateAxisDrivenCursorPos( PltInputData.RawMouseDelta , WantCursorPos );
	ProcessRawMouseDelta( PltInputData , DeltaTime );
}

eg_bool EGInput::IsMousePosFromOS()const
{
	return true;
}

void EGInput::UpdateAxisDrivenCursorPos( const eg_ivec2& MouseDelta , eg_bool WantCursorPos )
{
	const eg_real MouseMoveX = EG_To<eg_real>(MouseDelta.x);
	const eg_real MouseMoveY = EG_To<eg_real>(MouseDelta.y);
	
	//The sensitivity of the mouse in the y direction is affected by
	//the aspect ratio that way on wider screens it won't seem like the
	//mouse is moving faster when going left and right, vs up and down.
	
	//We update the mouse position if the input target is not the game:
	if( !IsMousePosFromOS() && (WantCursorPos) )
	{
		const eg_real fCurseSense = InputConfig_CursorSenitivity.GetValue();

		m_Kbm.MousePos.x+=MouseMoveX*fCurseSense;
		//The mouse range is limited on both axises from -1 to 1 with (0, 0)
		//being the center of the screen and (1, 1) being the upper right
		//part of the screen.
		m_Kbm.MousePos.x=EG_Clamp<eg_real>(m_Kbm.MousePos.x, -1.0f, 1.0f);
		m_Kbm.MousePos.y=EG_Clamp<eg_real>(m_Kbm.MousePos.y, -1.0f, 1.0f);
	}
}

void EGInput::ProcessRawMouseDelta( const egRawKbmData& RawData , eg_real DeltaTime )
{
	static const eg_real RAW_MOUSE_SAMPLE_BASE_MULT = 1.f/2000.f; // We want the mouse movement to be close to game-pad (which is -1 to 1) for any kind of reasonable mouse movement.
	static const eg_real RAW_MOUSE_MAX_AGE = .25f;
	
	const eg_ivec2& RawMouseDelta = RawData.RawMouseDelta;
	const eg_uint& NumRawMouseSamples = RawData.RawMouseDeltaNumSamples;
	if( NumRawMouseSamples > 0 )
	{
		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Raw Mouse: {0} {1} ({2})" , RawMouseDelta.x , RawMouseDelta.y , NumRawMouseSamples ) );
	}

	if( 0 == NumRawMouseSamples ){ assert( RawMouseDelta == eg_ivec2(0,0) ); }
	m_MouseSmoothSamples.SetMaxSamples( InputConfig_MouseSmoothSamples.GetValue() );
	m_MouseSmoothSamples.SetMaxAge( RAW_MOUSE_MAX_AGE );
	m_MouseSmoothSamples.Update( eg_vec3( EG_To<eg_real>(RawMouseDelta.x) , EG_To<eg_real>(RawMouseDelta.y) , 0.f ) , DeltaTime );
	const eg_vec2 AveragedDelta = m_MouseSmoothSamples.GetSmoothValue().XY_ToVec2();
	const eg_real AxisSensitivity = InputConfig_MouseAxisSensitivity.GetValue()*(1.f/DeltaTime)*RAW_MOUSE_SAMPLE_BASE_MULT; // We divide by DeltaTime so that the mouse axis can be treated the same as the gamepad and get multiplied by DeltaTime in player input ticks.
	m_Kbm.MouseAxis.x=AveragedDelta.x*AxisSensitivity;
	m_Kbm.MouseAxis.y=AveragedDelta.y*AxisSensitivity;

	// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Mouse Axis: {0} {1}" , m_Kbm.MouseAxis.x , m_Kbm.MouseAxis.y ) );
}

egLockstepCmds EGInput::UpdateCmds( eg_input_device_id DeviceId , const EGInputBindings& Bindings , eg_real DeltaTime , EGInputCmdState& CmdList, eg_real AspectRatio ) const
{
	egLockstepCmds Out;
	Out.Clear();

	auto HandleCmdState = []( eg_cmd_t Action , eg_bool Active , EGInputCmdState* CmdList , egLockstepCmds* LockstepCmds ) -> void
	{
		if( Active )
		{
			//If the button wasn't down, set as pressed.
			CmdList->SetPressed( Action , !CmdList->IsActive(Action) );


			CmdList->SetReleased( Action , false );	
			CmdList->SetActive( Action , true );
			CmdList->SetWillDeactivate( Action , true );

		}
		else
		{
			//If the button was already down, set the deactivated flags...
			CmdList->SetReleased( Action , CmdList->IsActive(Action) );

			CmdList->SetActive( Action , false );
			CmdList->SetPressed( Action , false );
			CmdList->SetWillDeactivate( Action , false );
		}

		LockstepCmds->SetIsActive( Action , CmdList->IsActive(Action) );
		LockstepCmds->SetWasPressed( Action , CmdList->IsPressed(Action) );
		LockstepCmds->SetWasReleased( Action , CmdList->IsReleased(Action) );
		LockstepCmds->SetWasRepeated( Action , CmdList->IsRepeated(Action) );
	};

	auto ObtainInputForKbm = [&Bindings,&DeltaTime,&CmdList,&Out,&HandleCmdState]( const egKbmDeviceInfo& Kbm ) -> void
	{
		for(eg_uint i=0; i<CMD_MAX; i++)
		{
			eg_kbm_btn Btn = Bindings.ActionToKb(static_cast<eg_cmd_t>(i));
			eg_bool nState = Kbm.DataPressedMeansHeld.IsPressed( Btn );

			eg_cmd_t Action = static_cast<eg_cmd_t>(i);

			HandleCmdState( Action , nState , &CmdList , &Out );
			Out.m_Axis1.x = Kbm.MouseAxis.x;
			Out.m_Axis1.y = -Kbm.MouseAxis.y;
			Out.m_Axis2 = CT_Clear; // TODO: Could use arrow keys or something...
		}
	};

	auto ObtainInputForGamepad = [this,&Bindings,&DeltaTime,&CmdList,&Out,&HandleCmdState]( eg_uint Index ) -> void
	{
		const egGpDeviceInfo& GpInfo = m_GpInfos[Index];

		for(eg_uint i=0; i<CMD_MAX; i++)
		{
			eg_gp_btn Btn = Bindings.ActionToGp(static_cast<eg_cmd_t>(i));
			eg_bool nState = GpInfo.Gp.IsHeld( Btn );

			eg_cmd_t Action = static_cast<eg_cmd_t>(i);

			HandleCmdState( Action , nState , &CmdList , &Out );
		}

		const eg_real AxisSensitivity = InputConfig_GamepadAxisSensitivity.GetValue();

		Out.m_Axis1 = GpInfo.Gp.GetRAxis( true )*AxisSensitivity;
		Out.m_Axis2 = GpInfo.Gp.GetLAxis( true )*AxisSensitivity;

		// EGLogToScreen::Get().Log( this , __LINE__ + GpInfo.Gp.GetId() , 1.f , *EGSFormat8( "GP Axis[{0}]: {1} {2}" , GpInfo.Gp.GetId() , Out.m_Axis[0] , Out.m_Axis[1] ) );
	};

	for(eg_uint i=0; i<CMD_MAX; i++)
	{
		CmdList.UpdateRepeat( i , DeltaTime );
	}

	switch( DeviceId )
	{
		case eg_input_device_id::COUNT:
		case eg_input_device_id::NONE:
			break;
		case eg_input_device_id::KBM:
			ObtainInputForKbm( m_Kbm );
			break;
		case eg_input_device_id::GP1:
			// Keyboard and first gamepad:
			ObtainInputForGamepad( 0 );
			break;
		case eg_input_device_id::GP2:
			// 2nd Gamepad
			ObtainInputForGamepad( 1 );
			break;
		case eg_input_device_id::GP3:
			// 2nd Gamepad
			ObtainInputForGamepad( 2 );
			break;
		case eg_input_device_id::GP4:
			// 2nd Gamepad
			ObtainInputForGamepad( 3 );
			break;
	}

	Out.m_MousePos.x = m_Kbm.MousePos.x * AspectRatio;
	Out.m_MousePos.y = m_Kbm.MousePos.y;

	return Out;
}

void EGInput::ButtonNames_Init()
{
	for( eg_uint i=0; i<countof(m_CmdNames); i++ )
	{
		egCmdNameMap& Item = m_CmdNames[i];

		Item.CrcName = eg_crc("");
		Item.StrName = "";
	}
}

void EGInput::ButtonNames_SetCmdName( eg_cmd_t Cmd , eg_cpstr Name )
{
	if( 1 <= Cmd && Cmd < countof(m_CmdNames) && EGString_StrLen( Name ) > 0 )
	{
		// Verify that this command names is not already used.
		for( eg_uint i=0; i<countof(m_CmdNames); i++ )
		{
			if( EGString_EqualsI( Name , m_CmdNames[i].StrName.String() ) )
			{
				assert( false ); // This command name was already used.
				EGLogf( eg_log_t::Error , __FUNCTION__ ": The command %s won't be bound properly." , Name );
			}
		}
		
		egCmdNameMap& Item = m_CmdNames[Cmd];

		assert( Item.StrName.Len() == 0 ); // This command was already registered... Not good, please change the id...
		assert( EGString_StrLen(Name) < INPUTBUTTONS_MAX_NAME_LENGTH ); // This name is too long, it won't be entered correctly.
		Item.StrName = Name;
		Item.CrcName = eg_string_crc(Name);
	}
	else
	{
		assert( false ); // Zero should not be registered as it is the nil command, and commands must be less than CMD_MAX.
	}
}


eg_cmd_t EGInput::ButtonNames_StringToAction( eg_cpstr Str )const
{
	eg_string_crc CrcStr = eg_string_crc(Str);

	for( eg_uint i=0; i<countof(m_CmdNames); i++ )
	{
		const egCmdNameMap& Item = m_CmdNames[i];
		if( Item.CrcName == CrcStr )
		{
			return i;
		}
	}
	// assert( false ); // A command with this name was not found...
	return CMDA_UNK;
}

eg_cmd_t EGInput::ButtonNames_StringCrcToAction( eg_string_crc Crc ) const
{
	eg_cmd_t Out = CMDA_UNK;

	for( eg_uint i=0; i<countof(m_CmdNames); i++ )
	{
		const egCmdNameMap& Item = m_CmdNames[i];
		if( Item.CrcName == Crc )
		{
			return i;
		}
		
	}
	return Out;
}

eg_cpstr EGInput::ButtonNames_ActionToString( eg_cmd_t Action )const
{
	eg_uint Index = Action;
	if( 1 <= Index && Index < countof(m_CmdNames) )
	{
		return m_CmdNames[Index].StrName.String();
	}
	//TODO:
	assert( false );
	return "";
}
