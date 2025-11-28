///////////////////////////////////////////////////////////////////////////////
// EGInput - This class manages all the input on the client system. The main
// thing it does is get an egLockstepCmds struct for any given device or for
// the single player device (which catenates input from all devices). Basically
// it's mean to take any possible input data, and convert it into a single
// structure that doesn't care where that input came from.
//
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "EGBoolList.h"
#include "EGInputButtons.h"
#include "EGInputGpDevice.h"
#include "EGInputBindings.h"
#include "EGInputTypes.h"
#include "EGCircleArray.h"
#include "EGInputCmdState.h"
#include "EGForceSampler.h"

enum class eg_input_smooth_t
{
	None,
	TimeBased,
	SampleBased,
};

class EGInput : public ISdkInputRegistrar
{
public:
	
	struct egKbmDeviceInfo
	{
		//Button state:
		egInputKbmData Data;
		egInputKbmData DataPressedMeansHeld;
		eg_vec2        MouseAxis;
		eg_vec2        MousePos;
		eg_bool        bHadInputThisFrame;
	};

	struct egGpDeviceInfo
	{
		EGGpDevice Gp;

		void Init( eg_uint Id ){ Gp.Init( Id ); }
		void Deinit(){ }
	};


public:

	static EGInput& Get(){ return s_Input; }

private:

	EGInput();
	~EGInput();
	static EGInput s_Input;

public:

	void Init();
	void Deinit();

	eg_bool WasThereInputThisFrameFor( eg_input_device_id DeviceId ) const;

	void Update( eg_real DeltaTime , eg_bool WantCursorPos , eg_real Aspect );
	void WriteBindings( const EGInputBindings& InputBindings , class EGFileData& fout , eg_cpstr strFormat )const;
	virtual void SDK_RegisterInput( eg_cmd_t Cmd , eg_cpstr StrName , eg_cpstr DefaultGpButton , eg_cpstr DefaultKbButton ) override final;
	void Bind( EGInputBindings& Bindings , eg_cpstr StrAction , eg_cpstr StrGp , eg_cpstr StrKb ) const;
	
	egLockstepCmds UpdateCmds( eg_input_device_id DeviceId , const EGInputBindings& Bindings , eg_real DeltaTime , EGInputCmdState& CmdList, eg_real AspectRatio ) const;
	eg_char GetChar();

	const EGInputBindings& GetDefaultBindings() const { return m_DefaultBindings; }

	eg_string_crc InputActionToKey( const EGInputBindings& Bindings , eg_string_crc Action , eg_bool bGP );
	eg_cpstr InputActionToKeyStr( const EGInputBindings& Bindings , eg_string_crc Action , eg_bool bGP );
	eg_cpstr InputCmdToName( eg_cmd_t Cmd );

	eg_size_t GetNumGpDevices() const { return countof(m_GpInfos); }
	const egGpDeviceInfo& GetRawGpDeviceInfo( eg_size_t GpIndex ) const { return m_GpInfos[GpIndex]; }
	const egKbmDeviceInfo& GetRawKbmDeviceInfo() const { return m_Kbm; }

private:

	static const eg_uint INPUT_BUFFER_SIZE = 256;
	static const eg_uint CBUFFER_SIZE=10;

	static const eg_uint INPUTBUTTONS_MAX_NAME_LENGTH = 24;
	typedef eg_string_fixed_size<INPUTBUTTONS_MAX_NAME_LENGTH> eg_cmd_name;
	struct egCmdNameMap
	{
		eg_string_crc CrcName;
		eg_cmd_name   StrName;
	};

	typedef EGCircleArray<eg_char,CBUFFER_SIZE> EGCharBuffer;

private:

	egGpDeviceInfo     m_GpInfos[4];
	egKbmDeviceInfo    m_Kbm;
	eg_real            m_fAspect;
	eg_input_smooth_t  m_SmoothType = eg_input_smooth_t::SampleBased;
	egCmdNameMap       m_CmdNames[CMD_MAX];
	EGInputBindings    m_DefaultBindings;
	EGCharBuffer       m_CharBuffer; // Keyboard input from the OS API for smooth typing in text boxes.
	EGForceSampler     m_MouseSmoothSamples;
	EGForceSampler     m_GpRSSmoothSamples[4];
	EGForceSampler     m_GpLSSmoothSamples[4];

private:

	void InitMouse();
	eg_bool IsMousePosFromOS()const;
	void SetMousePosFromScreenPosPercent(eg_real PercentXFromRight, eg_real PercentYFromTop);
	void UpdateAxisDrivenCursorPos( const eg_ivec2& MouseDelta , eg_bool WantCursorPos );
	void ProcessRawMouseDelta( const egRawKbmData& RawData , eg_real DeltaTime );
	void OnChar(eg_char c);
	void WriteBindingsForDevice( const EGInputBindings& Bindings , class EGFileData& fout , eg_cpstr strFormat )const;

	void ButtonNames_Init();
	void ButtonNames_SetCmdName( eg_cmd_t Cmd , eg_cpstr Name );
	eg_cmd_t ButtonNames_StringToAction( eg_cpstr Str )const;
	eg_cmd_t ButtonNames_StringCrcToAction( eg_string_crc Crc )const;
	eg_cpstr ButtonNames_ActionToString( eg_cmd_t Action )const;
};
