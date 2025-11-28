// (c) 2017 Beem Media

#include "EGWndDebugLogPanel.h"

EG_CLASS_DECL( EGWndDebugLogPanelListener )

EGWndDebugLogPanel::EGWndDebugLogPanel( EGWndPanel* InParent )
: Super( InParent , EGWnd_GetFontSize( egw_font_t::CONSOLE ) )
{
	m_Listener.SetDebugPanel( this );
	m_LogDispatcher.Delegate.AddUnique( &m_Listener , &EGWndDebugLogPanelListener::OnLog );
	EGLog_AddDispatcher( &m_LogDispatcher );
	m_WheelScrollAmount = 1;
	m_bFavorBottom = true;
	SetWndTimer( UPDATE_TIMER_ID , 100 );
	UpdateScroll();
}

EGWndDebugLogPanel::~EGWndDebugLogPanel()
{
	EGLog_RemoveDispatcher( &m_LogDispatcher );
	m_LogDispatcher.Delegate.RemoveAll( &m_Listener );
}

eg_size_t EGWndDebugLogPanel::GetNumItems() const
{
	return m_Lines.Len();
}

void EGWndDebugLogPanel::OnPaint( HDC hdc )
{
	RECT rcWnd = GetViewRect();

	FillRect( hdc , &rcWnd , EGWnd_GetBrush( egw_color_t::BG_CONSOLE ) );
	
	eg_int LineHeight = m_ItemSize;

	// SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
	SetFont( hdc , EGWnd_GetFont( egw_font_t::CONSOLE ) );
	SetBkColor( hdc , EGWnd_GetColor( egw_color_t::BG_CONSOLE ) );
	SetBkMode( hdc , TRANSPARENT );

	for( eg_int i=0; i<static_cast<eg_int>(m_Lines.Len()); i++ )
	{
		eg_int Top = LineHeight*i;
		Top -= m_ScrollPos;
		eg_int Bottom = Top + LineHeight;
		if( 0 <= Bottom && Top <= rcWnd.bottom )
		{
			SetTextColor( hdc, RGB( m_Lines[i].Color.R , m_Lines[i].Color.G , m_Lines[i].Color.B ) );
			TextOutA( hdc , 2 , Top , m_Lines[i].Line , m_Lines[i].Line.Len() );
		}
	}
}

void EGWndDebugLogPanel::OnWmTimer( eg_uint64 TimerId )
{
	Super::OnWmTimer( TimerId );

	if( TimerId == UPDATE_TIMER_ID )
	{
		m_LogDispatcher.DispatchLogs();

		if( m_LinesAddedSinceUpdate > 0 )
		{
			UpdateScroll();
			SetScroll( GetScroll() + m_LinesAddedSinceUpdate*m_ItemSize );
			FullRedraw();
			m_LinesAddedSinceUpdate = 0;
		}
	}
}

void EGWndDebugLogPanel::InsertLine( eg_log_channel Channel , eg_cpstr Line )
{
	eg_color32 Color = EGLog_GetColorForChannel( Channel );
	eg_string_small Name = EGLog_GetNameForChannel( Channel );

	eg_string_big FinalLine = EGString_Format( "[%s] %s" , Name.String() , Line );

	m_Lines.InsertLastAndOverwriteFirstIfFull( egLogItem( FinalLine , Color ) );
	m_LinesAddedSinceUpdate++;
}

void EGWndDebugLogPanelListener::OnLog( eg_log_channel Channel, const eg_string_base& LogText )
{
	if( m_DebugPanel )
	{
		m_DebugPanel->InsertLine( Channel, LogText );
	}
}
