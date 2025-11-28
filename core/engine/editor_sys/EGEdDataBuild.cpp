// (c) 2018 Beem Media

#if defined( __EGEDITOR__ )

#include "EGEdDataBuild.h"
#include "EGWndAppWindow.h"
#include "EGWndPanel.h"
#include "EGWndDebugLogPanel.h"
#include "EGWndPanelPropEditor.h"
#include "EGWndHelpers.h"
#include "EGResourceLib.h"
#include "EGEngineApp.h"
#include "EGThread.h"
#include "EGThreadProc.h"
#include "EGDataBuilder.h"
#include "../tools/EGEdResLib/resource.h"

class EGEdDataBuildWorkerProc : public EGObject , public IThreadProc
{
	EG_CLASS_BODY( EGEdDataBuildWorkerProc , EGObject )

private:
	
	eg_d_string m_GameName = "";
	EGDataBuilder m_DataBuilder;
	mutable EGMutex m_Mutex;
	eg_bool m_bIsDone = false;
	eg_bool m_bIsCancelled = false;
	eg_real m_TimeRunning = 0.f;

public:

	void Init( eg_cpstr GameNameIn )
	{
		m_GameName = GameNameIn;
	}

	virtual void OnStart() override
	{
		m_TimeRunning = 0.f;
		SetIsDone( false );
		SetCanceled( false );

		m_DataBuilder.ExecuteFirstStep( *m_GameName );
	}

	virtual void Update( eg_real DeltaTime ) override
	{
		m_TimeRunning += DeltaTime;

		if( m_DataBuilder.IsDone() )
		{
			if( m_TimeRunning > 1.f ) // Force to run for at least 1 seconds.
			{
				SetIsDone( true );
			}
		}
		else
		{
			m_DataBuilder.ExecuteNextStep( DeltaTime );
		}

		if( IsCanceled() )
		{
			SetIsDone( true );
		}
	}

	virtual void OnStop() override
	{
		if( !IsDone() )
		{
			EGLogf( eg_log_t::Warning , "The build job was not completed." );
		}
	}

	void SetIsDone( eg_bool bNewValue )
	{
		EGFunctionLock Lock( &m_Mutex );

		m_bIsDone = bNewValue;
	}

	void SetCanceled( eg_bool bNewValue )
	{
		EGFunctionLock Lock( &m_Mutex );
		m_bIsCancelled = bNewValue;
	}

	void CancelBuild()
	{
		EGFunctionLock Lock( &m_Mutex );
		m_bIsCancelled = true;
		EGLogf( eg_log_t::Warning , "The build job was canceled." );
	}

	eg_bool IsDone() const
	{
		EGFunctionLock Lock( &m_Mutex );
		return m_bIsDone;
	}

	eg_bool IsCanceled() const
	{
		EGFunctionLock Lock( &m_Mutex );
		return m_bIsCancelled;
	}
};

EG_CLASS_DECL( EGEdDataBuildWorkerProc )

class EGEdDataBuildWnd;

static const eg_char16 EGEdDataBuild_Title[] = L"EG Data Build";

class EGEdDataBuild : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	EGEdDataBuildWnd*const m_App = nullptr;
	eg_d_string m_DisplayText = "";

public:

	EGEdDataBuild( EGWndPanel* Parent , EGEdDataBuildWnd* App )
	: Super( Parent , eg_panel_size_t::Fixed , 100 ) 
	, m_App(App)
	{
	 
	}

	virtual void OnDrawBg( HDC hdc ) override
	{
		FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
	}

	virtual void OnPaint( HDC hdc ) override
	{
		SetFont( hdc , EGWnd_GetFont( egw_font_t::DEFAULT ) );
		SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
		SetBkMode( hdc , TRANSPARENT );

		RECT rcView = GetViewRect();

		DrawTextA( hdc , *m_DisplayText , -1 , &rcView , DT_SINGLELINE|DT_CENTER|DT_VCENTER );
	}

	void SetDisplayText( eg_cpstr NewValue )
	{
		m_DisplayText = NewValue;
		FullRedraw();
	}
};

class EGEdDataBuildWnd : public EGWndAppWindow
{
	EG_DECL_SUPER( EGWndAppWindow )

private:

	static const eg_uint64 UPDATE_TIMER_ID = 1;

private:

	EGEdDataBuild*           m_DataBuildPanel = nullptr;
	EGWndPanel               m_MainPanel;
	EGThread                 m_WorkerThread;
	EGEdDataBuildWorkerProc* m_WorkerProc = nullptr;
	int                      m_nShowCmd;
	eg_int                   m_BuildingElipses = 0;
	eg_bool                  m_bIsDone:1;
	eg_bool                  m_bAutoClose:1;

public:

	EGEdDataBuildWnd( int nCmdShow , eg_cpstr GameName , eg_bool bAutoCloseIn )
	: EGWndAppWindow( EGEdDataBuild_Title )
	, m_MainPanel( egWndPanelAppContainer( this ) )
	, m_WorkerThread( EGThread::egInitParms( "Builder" , EGThread::eg_init_t::LowPriority ) )
	, m_nShowCmd( nCmdShow )
	, m_bIsDone( false )
	, m_bAutoClose( bAutoCloseIn )
	{
		EGWndPanel* MainContainer = m_MainPanel.AddContainer( eg_panel_fill::Horizontal );
		EGWndPanel* MidPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		MidPane->SetFillSize( eg_panel_size_t::Auto , 0 );

		// Mid Pane
		{
			MidPane->AddChild<EGWndPanelHeader>( "Data Builder" );
			m_DataBuildPanel = MidPane->AddChild<EGEdDataBuild>( this );
			MidPane->AddChild<EGWndPanelHeader>( "Log" );
			EGWndDebugLogPanel* DebugLogPanel = MidPane->AddChild<EGWndDebugLogPanel>();
			DebugLogPanel->SetFillSize( eg_panel_size_t::Auto , 0 );
		}

		eg_ivec2 DesktopRes = EGWnd_GetDesktopResolution();

		SetAppWidth( DesktopRes.x/2 );
		SetAppHeight( DesktopRes.y/2 );

		m_MainPanel.HandleResize();

		SetWndTimer( UPDATE_TIMER_ID , 333 );

		m_WorkerProc = EGNewObject<EGEdDataBuildWorkerProc>( eg_mem_pool::System );
		if( m_WorkerProc )
		{
			m_WorkerProc->Init( GameName );
			m_WorkerThread.RegisterProc( m_WorkerProc );
			m_WorkerThread.Start();
		}
	}

	~EGEdDataBuildWnd()
	{
		if( m_WorkerThread.GetState() == EGThread::STATE_RUNNING )
		{
			m_WorkerThread.Stop();
		}

		if( m_WorkerProc )
		{
			m_WorkerThread.UnregisterProc( m_WorkerProc );
			EGDeleteObject( m_WorkerProc );
			m_WorkerProc = nullptr;
		}
	}

	virtual void OnAppStart( const EGArray<eg_cpstr16>& Args ) override final
	{
		Super::OnAppStart( Args );

		if( !m_bIsDone )
		{
			ShowWindow( GetWnd() , m_nShowCmd );
			UpdateWindow( GetWnd() );
			UpdateHeaderText();
		}
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }

private:

	virtual void OnWmActivateApp( eg_bool bActivating , DWORD ThreadId ) override
	{
		Super::OnWmActivateApp( bActivating , ThreadId );
	}

	virtual void OnWmClose() override
	{
		Super::OnWmClose();

		HandleQuitRequest();
	}

	virtual void OnWmGetMinMaxInfo( MINMAXINFO* MinMaxInfo ) override
	{
		MinMaxInfo->ptMinTrackSize.x = EGWndPanel::GetPane3MidMinWidth();
		MinMaxInfo->ptMinTrackSize.y = EGWndPanel::GetMainWndMinHeight();
	}

	virtual void OnWmSize( const eg_ivec2& NewClientSize )
	{
		Super::OnWmSize( NewClientSize );

		m_MainPanel.HandleResize();
	}

	virtual void OnWmTimer( eg_uint64 TimerId ) override
	{
		Super::OnWmTimer( TimerId );

		if( TimerId == UPDATE_TIMER_ID )
		{
			if( m_DataBuildPanel )
			{
				m_BuildingElipses++;
				m_BuildingElipses %= 4;
				eg_string BuildingText( CT_Clear );
				for( eg_int i=0; i<m_BuildingElipses; i++ )
				{
					BuildingText += '.';
				}
				BuildingText += "Building";
				for( eg_int i=0; i<m_BuildingElipses; i++ )
				{
					BuildingText += '.';
				}
				m_DataBuildPanel->SetDisplayText( BuildingText );
			}

			if( m_WorkerProc && m_WorkerProc->IsDone() && m_WorkerThread.GetState() == EGThread::STATE_RUNNING )
			{
				m_WorkerThread.Stop();
				if( m_bAutoClose )
				{
					m_bIsDone = true;
				}
			}
		}
	}

	void UpdateHeaderText()
	{
		SetWindowTextW( m_hwnd , EGEdDataBuild_Title );
	}

	void HandleQuitRequest()
	{
		if( m_WorkerProc && !m_WorkerProc->IsDone() )
		{
			if( IDYES == MessageBoxW( GetWnd() , L"The build job is not completed yet, are you sure you want to cancel?" , EGEdDataBuild_Title , MB_YESNO ) )
			{
				m_WorkerProc->CancelBuild();
			}
		}
		else
		{
			m_bIsDone = true;
		}
	}
};

int EGEdDataBuild_Run( eg_cpstr16 Filename, const struct egSdkEngineInitParms& EngineParms )
{
	unused( Filename , EngineParms );

	eg_bool bAutoClose = true; // May want a command line to auto close.

	static const egWndAppInfo AppInfo( GetModuleHandleW( NULL ) , EGResourceLib_GetLibrary() , SW_SHOWNORMAL , false , MAKEINTRESOURCEW(IDI_APPICON) , MAKEINTRESOURCEW(IDI_APPICON) , nullptr );
	EGWnd_RunApp( AppInfo , [&EngineParms,&bAutoClose](int nCmdShow)->EGWndAppWindow*{ return new EGEdDataBuildWnd( nCmdShow , EngineParms.GameName , bAutoClose ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } ); 
	return 0;
}

#endif
