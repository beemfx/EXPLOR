// (c) 2017 Beem Media

#pragma once

#include "EGWnd.h"
#include "EGWndBase.h"

class EGWndAppWindow : public EGWndBase
{
private:

	static eg_cpstr16 CLASS_NAME;

public:

	EGWndAppWindow( eg_cpstr16 AppName );
	virtual ~EGWndAppWindow() override;

	virtual void OnAppStart( const EGArray<eg_cpstr16>& Args ){ unused( Args ); }
	virtual void OnAppUpdate( eg_real DeltaTime ){ unused( DeltaTime ); }
	virtual eg_bool IsDone() const { return false; }

	eg_int GetAppHeight() const;
	eg_int GetAppWidth() const;

protected:

	void SetAppWidth( eg_int DesiredWidth );
	void SetAppHeight( eg_int DesiredHeight );

public:

	static void InitClass( HINSTANCE hInst , HMODULE hResModule , eg_cpstr16 Icon , eg_cpstr16 IconSm );
	static void DeinitClass( HINSTANCE hInst );
};
