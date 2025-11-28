// (c) 2015 Beem Media. All Rights Reserved.

#include "EGInputButtons.h"
#include "EGDebugText.h"
#include "EGMutex.h"
#include "EGWindowsAPI.h"
#include "EGInputKbmDevice.h"
#include "EGInputKbmTables.h"

static const eg_log_channel LogKbmDevice( eg_crc("KbmDevice") );

static class EGInputKbmDevice
{
private:

	HWND             m_Window = NULL;
	EGMutex          m_Lock;
	INPUT_MOUSE_MODE m_MouseMode;
	egRawKbmData     m_RawData;
	eg_kbm_btn       m_KeyMap[256];
	POINT            m_LastCursorPos;
	eg_bool			  m_Inited:1;
	eg_bool          m_Active:1;
	eg_bool          m_bIsCapturing:1;

public:
	EGInputKbmDevice()
	: m_Inited( false )
	, m_MouseMode( INPUT_MOUSE_MODE_CURSOR )
	, m_Active(false) //Until told otherwise.
	, m_bIsCapturing(false)
	, m_RawData( CT_Clear )
	{
		zero( &m_RawData );
		zero( &m_KeyMap );
		zero( &m_LastCursorPos );
	}

	~EGInputKbmDevice()
	{
		assert( !m_Inited ); //Never deinited?
	}

	void WindowThread_Init( HWND hwnd )
	{
		EGFunctionLock Lock( &m_Lock );

		if( m_Inited )
		{
			assert( false ); //Should only Init the device once.
			return;
		}

		m_Window = hwnd;

		// Registre RID
		{
			static const USHORT HID_USAGE_PAGE_GENERIC  = 0x01;
			static const USHORT HID_USAGE_GENERIC_MOUSE = 0x02;
	
			RAWINPUTDEVICE Rid[1];
			Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
			Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
			Rid[0].dwFlags = RIDEV_INPUTSINK;   
			Rid[0].hwndTarget = hwnd;
			RegisterRawInputDevices( Rid , countof(Rid) , sizeof(Rid[0]) );
		}

		//Initialize the KeyMap.
		zero( &m_KeyMap );
		for( eg_uint KeyIndex=0; KeyIndex<countof(InputKbmDevice_KbSysToKeyMap); KeyIndex++ )
		{
			const egInputKbSysToKeyMap& Item = InputKbmDevice_KbSysToKeyMap[KeyIndex];
			if( 0 != Item.SysKey )
			{
				assert( Item.SysKey < countof(m_KeyMap) );
				m_KeyMap[Item.SysKey] = Item.GameKey;
			}
		}

		m_MouseMode = INPUT_MOUSE_MODE_CURSOR;
		m_RawData.RawMouseDelta = eg_ivec2(0,0);
		m_RawData.RawMouseDeltaNumSamples = 0;
		m_RawData.MousePos       = eg_vec2(0,0);
		for( eg_uint i=0; i<countof(m_RawData.KbData.Pressed); i++ )
		{
			m_RawData.KbData.Pressed[i].Clear();
		}
		for( eg_uint i=0; i<countof(m_RawData.KbData.Released); i++ )
		{
			m_RawData.KbData.Released[i].Clear();
		}

		m_RawData.KbData.CharBufferCount = 0;
		for(eg_uint i=0; i<egInputKbmData::CBUFFER_SIZE; i++)
		{
			m_RawData.KbData.CharBuffer[i]=0;
		}

		m_Inited = true;
	}

	void WindowThread_Deinit( HWND hwnd )
	{
		unused( hwnd );

		EGFunctionLock Lock( &m_Lock );

		if( !m_Inited )
		{
			assert( false ); //Not initialized.
			return;
		}

		m_Active = false;
		WindowThread_HandleCaptureChanged();

		assert( m_Window == hwnd );
		m_Window = NULL;

		m_Inited = false;
	}

	void WindowThread_Update( eg_real DeltaTime , HWND hwnd )
	{
		EGFunctionLock Lock( &m_Lock );

		unused( DeltaTime );

		if( INPUT_MOUSE_MODE_DELTA == m_MouseMode && m_Active )
		{
			RECT rcWnd;
			GetClientRect( hwnd , &rcWnd );
			eg_int Width = rcWnd.right - rcWnd.left;
			eg_int Height = rcWnd.bottom - rcWnd.top;
			eg_int CenterX = rcWnd.left + Width/2;
			eg_int CenterY = rcWnd.top + Height/2;
	
			POINT ScreenPos = { CenterX , CenterY };
			ClientToScreen( hwnd , &ScreenPos );
			SetCursorPos( ScreenPos.x , ScreenPos.y );
		}
		else if( INPUT_MOUSE_MODE_CURSOR == m_MouseMode && m_Active )
		{
			POINT ScreenPos = { 0 , 0 };
			GetCursorPos( &ScreenPos );
			ScreenToClient( hwnd , &ScreenPos );

			RECT rcWnd;
			GetClientRect(hwnd, &rcWnd);
			eg_int Width = rcWnd.right - rcWnd.left;
			eg_int Height = rcWnd.bottom - rcWnd.top;

			eg_real xPercent = static_cast<eg_real>(ScreenPos.x)/static_cast<eg_real>(Width);
			eg_real yPercent = static_cast<eg_real>(ScreenPos.y)/static_cast<eg_real>(Height);
			m_RawData.MousePos = eg_vec2(xPercent,yPercent);
		}

		WindowThread_HandleCaptureChanged();
	}

	void WindowThread_HandleCaptureChanged()
	{
		const eg_bool bShouldBeCapturing = m_Active && m_MouseMode == INPUT_MOUSE_MODE_DELTA;
		const eg_bool bIsCurrenltyCapturing = m_bIsCapturing;

		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Capture: {0} Should Capture: {1}" , m_bIsCapturing , bShouldBeCapturing ) );

		if( bShouldBeCapturing != bIsCurrenltyCapturing )
		{
			if( bShouldBeCapturing )
			{
				m_bIsCapturing = bShouldBeCapturing;
				WindowThread_HandleWindowPositionOrSizeChanged();

			}
			else
			{
				m_bIsCapturing = bShouldBeCapturing;
				ClipCursor( NULL );
			}
		}
	}

	void WindowThread_HandleWindowPositionOrSizeChanged()
	{
		if( m_bIsCapturing )
		{
			RECT RcWindow;
			if( GetClientRect( m_Window , &RcWindow ) )
			{
				ClientToScreen( m_Window , reinterpret_cast<POINT*>(&RcWindow.left) ); // convert top-left
				ClientToScreen( m_Window , reinterpret_cast<POINT*>(&RcWindow.right) ); // convert bottom-right
				ClipCursor( &RcWindow );
			}
		}
	}

	void WindowThread_HandleMsg( HWND hwnd , UINT uMsg , WPARAM wParam , LPARAM lParam )
	{
		EGFunctionLock Lock( &m_Lock );

		switch(uMsg)
		{
			case WM_CANCELMODE:
			{
			} break;
			case WM_CAPTURECHANGED:
			{
			} break;
			case WM_ACTIVATE:
			{
				switch( wParam )
				{
					default:
					case WA_ACTIVE:
					case WA_CLICKACTIVE:
						if( !m_Active )
						{
							SetFocus( hwnd );
							m_Active = true;
						}
						break;
					case WA_INACTIVE: 
						if( m_Active )
						{
							m_Active = false;
							//Treat everything as being released.
							for( eg_uint i = 0; i < KBM_BTN_COUNT; i++ )
							{
								eg_kbm_btn Button = static_cast<eg_kbm_btn>(i);
								m_RawData.KbData.SetReleased( Button , true );
							}
						}
						break;
				}

			} break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				if( m_Active )
				{
					eg_kbm_btn Key = SysKeyToGameKey( wParam , lParam );
					if( KBM_BTN_NONE != Key )
					{
						m_RawData.KbData.SetPressed( Key , true );
					}
				}
			} break;
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				if( m_Active )
				{
					eg_kbm_btn Key = SysKeyToGameKey( wParam , lParam );
					if( KBM_BTN_NONE != Key )
					{
						m_RawData.KbData.SetReleased( Key , true );
					}
				}
			} break;
			case WM_CHAR:
				if( m_Active )
				{
					if(VK_ESCAPE != wParam)
					{
						m_RawData.KbData.OnChar((eg_char)wParam);
					}
				}
				break;
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				break;
			case WM_MOUSEMOVE:
			{
				const int xPos = GET_X_LPARAM(lParam);
				const int yPos = GET_Y_LPARAM(lParam);

				// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Mouse Move: {0} {1}" , xPos , yPos ) );

			} break;
			case WM_INPUT:
			{
				if( m_Active )
				{
					RAWINPUT raw;
					UINT dwSize = sizeof(raw);
					GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));
					if( raw.header.dwType == RIM_TYPEMOUSE ) 
					{ 
						m_RawData.RawMouseDelta += eg_ivec2( raw.data.mouse.lLastX , raw.data.mouse.lLastY );
						m_RawData.RawMouseDeltaNumSamples++;

						#define HANDLE_MOUSE_BUTTON( _id_  ) \
						if( 0 != (raw.data.mouse.usButtonFlags&RI_MOUSE_BUTTON_ ## _id_ ## _DOWN) ) \
						{ \
							m_RawData.KbData.SetPressed( KBM_BTN_MOUSE_ ## _id_ , true ); \
						} \
						if( 0 != (raw.data.mouse.usButtonFlags&RI_MOUSE_BUTTON_ ## _id_ ## _UP) ) \
						{ \
							m_RawData.KbData.SetReleased( KBM_BTN_MOUSE_ ## _id_ , true ); \
						}
				
						HANDLE_MOUSE_BUTTON(1)
						HANDLE_MOUSE_BUTTON(2)
						HANDLE_MOUSE_BUTTON(3)
						HANDLE_MOUSE_BUTTON(4)
						HANDLE_MOUSE_BUTTON(5)

						#undef HANDLE_MOUSE_BUTTON

						if( 0 != (raw.data.mouse.usButtonFlags&RI_MOUSE_WHEEL) )
						{
							SHORT MouseDelta = raw.data.mouse.usButtonData;
							//For the mouse wheel we send both pressed and released events.
							if( MouseDelta < 0 )
							{
								m_RawData.KbData.SetPressed( KBM_BTN_MOUSE_W_DOWN , true );
								//m_RawData.KbData.SetReleased( KBM_BTN_MOUSE_W_DOWN , true ); //Release handled in a special way
							}
							else if( MouseDelta > 0 )
							{
								m_RawData.KbData.SetPressed( KBM_BTN_MOUSE_W_UP , true );
								//m_RawData.KbData.SetReleased( KBM_BTN_MOUSE_W_UP , true ); //Release handled in a special way
							}
						}
					}	
				} 
			} break;
		}
	}

	void SetMouseMode( INPUT_MOUSE_MODE Mode )
	{
		EGFunctionLock Lock( &m_Lock );

		eg_bool Changed = Mode != m_MouseMode;
		if( Changed )
		{
			if( m_MouseMode == INPUT_MOUSE_MODE_CURSOR )
			{
				GetCursorPos( &m_LastCursorPos );
			}
			else if( m_MouseMode == INPUT_MOUSE_MODE_DELTA )
			{
				SetCursorPos( m_LastCursorPos.x , m_LastCursorPos.y );
			}

			m_MouseMode = Mode;
			m_RawData.RawMouseDelta = eg_ivec2(0,0);
			m_RawData.RawMouseDeltaNumSamples = 0;

		}
	}

	void GetStateAndReset( egRawKbmData* InputDataOut )
	{
		EGFunctionLock Lock( &m_Lock );

		assert( m_Inited );

		//Since the mouse wheel always has the press and release on the same
		//frame we delay it a frame so that the event goes through.
		eg_bool MouseWheelUpPressed = m_RawData.KbData.IsPressed( KBM_BTN_MOUSE_W_UP );
		eg_bool MouseWheelDownPressed = m_RawData.KbData.IsPressed( KBM_BTN_MOUSE_W_DOWN );

		m_RawData.bHadInput = false;

		for( eg_size_t i=0; i<countof(m_RawData.KbData.Pressed); i++ )
		{
			if( m_RawData.KbData.Pressed[i] != eg_flags() )
			{
				m_RawData.bHadInput = true;
				break;
			}

			if( m_RawData.KbData.Released[i] != eg_flags() )
			{
				m_RawData.bHadInput = true;
				break;
			}
		}

		if( m_RawData.RawMouseDeltaNumSamples > 0 )
		{
			m_RawData.bHadInput = true;
		}

		*InputDataOut = m_RawData;
	
		ClearState();

		if( MouseWheelUpPressed )
		{
			m_RawData.KbData.SetReleased( KBM_BTN_MOUSE_W_UP , true );
		}

		if( MouseWheelDownPressed )
		{
			m_RawData.KbData.SetReleased( KBM_BTN_MOUSE_W_DOWN , true );
		}
	}

private:
	void ClearState( void )
	{
		m_RawData.RawMouseDelta = eg_ivec2(0,0);
		m_RawData.RawMouseDeltaNumSamples = 0;
		//m_RawData.MouseData.Pos       = eg_vec2(0,0);

		for( eg_uint i=0; i<countof(m_RawData.KbData.Pressed); i++ )
		{
			m_RawData.KbData.Pressed[i].Clear();
		}
		for( eg_uint i=0; i<countof(m_RawData.KbData.Released); i++ )
		{
			m_RawData.KbData.Released[i].Clear();
		}

		m_RawData.bHadInput = false;
		m_RawData.KbData.CharBufferCount = 0;
	}

	eg_kbm_btn SysKeyToGameKey( WPARAM wParam , LPARAM lParam )
	{
		if( 0 <= wParam && wParam < countof(m_KeyMap) )
		{
			const BYTE VkCode = static_cast<BYTE>(wParam);
			const UINT ScanCode = (lParam & 0x00FF0000) >> 16;
			const UINT ExtKey = (lParam & 0x01000000) >> 24;

			if( VkCode == VK_SHIFT )
			{
				const UINT MappedKey = MapVirtualKeyW( ScanCode , MAPVK_VSC_TO_VK_EX );
				if( MappedKey == VK_LSHIFT )
				{
					return eg_kbm_btn::KBM_BTN_LSHIFT;
				}
				else if( MappedKey == VK_RSHIFT )
				{
					return eg_kbm_btn::KBM_BTN_RSHIFT;
				}
			}
			else if( VkCode == VK_CONTROL )
			{
				if( ExtKey != 0 )
				{
					return eg_kbm_btn::KBM_BTN_RCONTROL;
				}
				else
				{
					return eg_kbm_btn::KBM_BTN_LCONTROL;
				}
			}
			else if( VkCode == VK_MENU )
			{
				if( ExtKey != 0 )
				{
					return eg_kbm_btn::KBM_BTN_RMENU;
				}
				else
				{
					return eg_kbm_btn::KBM_BTN_LMENU;
				}
			}
			else if( VkCode == VK_RETURN )
			{
				if( ExtKey != 0 )
				{
					return eg_kbm_btn::KBM_BTN_NUMPADENTER;
				}
				else if( ExtKey != 0 )
				{
					return eg_kbm_btn::KBM_BTN_RETURN;
				}
			}

			return m_KeyMap[VkCode];
		}

		assert( false ); // Key out of range.
		return eg_kbm_btn::KBM_BTN_NONE;
	}


} KbmDevice;

void InputKbmDevice_WindowThread_Init( HWND hwnd ){ KbmDevice.WindowThread_Init( hwnd ); }
void InputKbmDevice_WindowThread_Deinit( HWND hwnd ){ KbmDevice.WindowThread_Deinit( hwnd ); }
void InputKbmDevice_WindowThread_Update( eg_real DeltaTime , HWND hwnd ){ KbmDevice.WindowThread_Update( DeltaTime , hwnd ); }
void InputKbmDevice_WindowThread_HandleMsg( HWND hwnd , UINT uMsg , WPARAM wParam , LPARAM lParam ){ KbmDevice.WindowThread_HandleMsg( hwnd , uMsg , wParam , lParam ); }

void WindowThread_HandleWindowPositionOrSizeChanged() { KbmDevice.WindowThread_HandleWindowPositionOrSizeChanged(); }

void InputKbmDevice_SetMouseMode( enum INPUT_MOUSE_MODE Mode ){ KbmDevice.SetMouseMode( Mode ); }
void InputKbmDevice_GetStateAndReset( struct egRawKbmData* InputDataOut ){ KbmDevice.GetStateAndReset( InputDataOut ); }
