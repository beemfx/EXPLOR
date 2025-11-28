///////////////////////////////////////////////////////////////////////////////
// InputKbmDevice - Interface for dealing with the mouse an keyboard device.
// It's a bit complicated since the input comes through the windows message
// system (which may be on a different thread), so input must be delivered in
// chunks. Currently the devices are attached to the render thread.
//
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////

//The functions are called from the thread that owns the window:
void InputKbmDevice_WindowThread_Init( HWND hwnd );
void InputKbmDevice_WindowThread_Deinit( HWND hwnd );
void InputKbmDevice_WindowThread_Update( eg_real DeltaTime , HWND hwnd );
void InputKbmDevice_WindowThread_HandleMsg( HWND hwnd , UINT uMsg , WPARAM wParam , LPARAM lParam );
void WindowThread_HandleWindowPositionOrSizeChanged();

//The functions are called form whatever thread owns input:
void InputKbmDevice_SetMouseMode( enum INPUT_MOUSE_MODE Mode );
void InputKbmDevice_GetStateAndReset( struct egRawKbmData* InputDataOut );