// (c) 2017 Beem Media

#include "EGLpkFileListPanel.h"
#include "fs_sys2/fs_lpk.h"

EGLpkFileListPanel::EGLpkFileListPanel( EGWndPanel* Parent )
: Super( Parent , EGWnd_GetFontSize( egw_font_t::DEFAULT ) )
{
	m_ItemSize = EGWnd_GetFontSize( egw_font_t::DEFAULT );

	SetScroll( 0 );
	UpdateScroll();
	FullRedraw();
}

EGLpkFileListPanel::~EGLpkFileListPanel()
{

}

void EGLpkFileListPanel::OnArchiveUpdated( CLArchive& Archive )
{
	m_FileList.Clear();

	for( fs_dword i=0; i<Archive.GetNumFiles(); i++ )
	{
		const LPK_FILE_INFO* Info = Archive.GetFileInfo( i );
		if( Info )
		{
			egFileInfo NewFileInfo;
			NewFileInfo.Filename = Info->szFilename;
			NewFileInfo.nOffset = Info->nOffset;
			NewFileInfo.nSize = Info->nSize;
			NewFileInfo.nCmpSize = Info->nCmpSize;
			NewFileInfo.bIsCompressed = Info->nType == LPK_ADD_ZLIBCMP;
			m_FileList.Append( NewFileInfo );
		}
	}

	UpdateScroll();
	SetScroll( 0 );
}

eg_size_t EGLpkFileListPanel::GetNumItems() const
{
	return m_FileList.Len();
}

LRESULT EGLpkFileListPanel::WndProc( UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch( Msg )
	{
	case WM_VSCROLL:
	{
		ScrollProc( LOWORD( wParam ) );
	} return 0;
	case WM_SIZING:
	case WM_SIZE:
	{
		UpdateScroll();
	} return 0;
	case WM_MOUSEWHEEL:
	{
		eg_int zDelta = GET_WHEEL_DELTA_WPARAM( wParam );
		ScrollProc( zDelta < 0 ? SB_LINEDOWN : SB_LINEUP );
	} break;
	case WM_MOUSEMOVE:
	{
		eg_int xPos = GET_X_LPARAM(lParam); 
		eg_int yPos = GET_Y_LPARAM(lParam); 

		TrackMouseLeave();

		SetHoveredItem( ClientPosToItemIndex( eg_ivec2(xPos,yPos) ) );
	} return 0;
	}

	return Super::WndProc( Msg , wParam , lParam );
}

void EGLpkFileListPanel::OnPaint( HDC hdc )
{
	RECT rc = GetViewRect();
	rc.top -= m_ScrollPos;

	SetBkMode( hdc , TRANSPARENT );

	auto DrawTextAndAdvanceY = [&hdc,&rc,this]( const egFileInfo& FileInfo , DWORD Format , eg_bool bSelected ) -> void
	{
		SetTextColor( hdc , EGWnd_GetColor( bSelected ? egw_color_t::FG_EDIT : egw_color_t::FG_STATIC ) );
		SetFont( hdc , bSelected ? m_SelectedItemFont : m_ItemFont );

		if( bSelected )
		{
			SelectObject( hdc , static_cast<HGDIOBJ>(EGWnd_GetBrush( egw_color_t::BG_EDIT ) ));
			Rectangle( hdc , rc.left , rc.top , rc.right , rc.top + m_ItemSize); 
		}

		RECT rcLineSize = rc;
		rcLineSize.left += 5;
		rcLineSize.right = FILENAME_WIDTH;
		DrawTextA( hdc , FileInfo.Filename , -1 , &rcLineSize , Format|DT_SINGLELINE );

		eg_int CmpPct = (eg_int)(100.0f-static_cast<eg_real>(FileInfo.nCmpSize)/static_cast<eg_real>(FileInfo.nSize)*100.0f);

		auto FormatSize =[]( eg_size_t Size ) -> eg_string_big
		{
			eg_string_big  Out = "";

			if( Size < 1024 )
			{
				Out = EGString_Format( "%u B" , EG_To<eg_uint32>(Size) );
			}
			else if( Size < 1024*1024 )
			{
				Out = EGString_Format( "%u K" , EG_To<eg_uint32>(Size/1024) );
			}
			else if( Size < 1024*1024*1024 )
			{
				Out = EGString_Format( "%u M" , EG_To<eg_uint32>(Size/(1024*1024)) );
			}
			else
			{
				Out = EGString_Format( "%u G" , EG_To<eg_uint32>(Size/(1024*1024*1024)) );
			}
			return Out;
		};

		rcLineSize.left = rcLineSize.right;
		rcLineSize.right = rcLineSize.left + COLUMN_WIDTH;
		OffsetRect( &rcLineSize , COLUMN_WIDTH , 0);
		DrawTextA( hdc , EGString_Format( "%u" , FileInfo.nOffset ) , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
		OffsetRect( &rcLineSize , COLUMN_WIDTH , 0);
		DrawTextA( hdc , FormatSize( FileInfo.nSize ) , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
		OffsetRect( &rcLineSize , COLUMN_WIDTH , 0);
		DrawTextA( hdc , FormatSize( FileInfo.nCmpSize ) , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
		OffsetRect( &rcLineSize , COLUMN_WIDTH , 0);
		DrawTextA( hdc , EGString_Format( "%d%%" , CmpPct ) , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
		OffsetRect( &rcLineSize , COLUMN_WIDTH , 0);
		DrawTextA( hdc , EGString_Format( "%s" , FileInfo.bIsCompressed ? "ZLIB" : "UNCMP" ) , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
		rc.top += m_ItemSize;
	};

	SetFont( hdc , m_ItemFont );
	for( eg_size_t i=0; i<m_FileList.Len(); i++ )
	{
		DrawTextAndAdvanceY( m_FileList[i] , 0 , i==m_HoveredItem );
	}
}

void EGLpkFileListPanel::OnDrawBg( HDC hdc )
{
	FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
}

void EGLpkFileListPanel::OnWmMouseLeave()
{
	Super::OnWmMouseLeave();

	SetHoveredItem( -1 );
}

EGLpkFileListPanelHeader::EGLpkFileListPanelHeader( EGWndPanel* Parent ) 
: EGWndPanel( Parent, eg_panel_size_t::Fixed , EGWndPanel::GetHeaderSize() )
{
	FullRedraw();
}

void EGLpkFileListPanelHeader::OnPaint( HDC hdc )
{
	DWORD Format = 0;

	RECT rc = GetViewRect();

	SetBkMode( hdc , TRANSPARENT );

	SetFont( hdc , EGWnd_GetFont(egw_font_t::DEFAULT_BOLD) );
	SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_EDIT ) );

	RECT rcLineSize = rc;
	rcLineSize.left += 5;
	rcLineSize.right = EGLpkFileListPanel::FILENAME_WIDTH;
	DrawTextA( hdc , "File Name" , -1 , &rcLineSize , Format|DT_SINGLELINE );

	rcLineSize.left = rcLineSize.right;
	rcLineSize.right = rcLineSize.left + EGLpkFileListPanel::COLUMN_WIDTH;
	OffsetRect( &rcLineSize , EGLpkFileListPanel::COLUMN_WIDTH , 0);
	DrawTextA( hdc , "Offset" , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
	OffsetRect( &rcLineSize , EGLpkFileListPanel::COLUMN_WIDTH , 0);
	DrawTextA( hdc , "Size" , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
	OffsetRect( &rcLineSize , EGLpkFileListPanel::COLUMN_WIDTH , 0);
	DrawTextA( hdc , "Cmp Size" , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
	OffsetRect( &rcLineSize , EGLpkFileListPanel::COLUMN_WIDTH , 0);
	DrawTextA( hdc , "Cmp Percent" , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
	OffsetRect( &rcLineSize , EGLpkFileListPanel::COLUMN_WIDTH , 0);
	DrawTextA( hdc , "Cmp Type" , -1 , &rcLineSize , Format|DT_SINGLELINE|DT_RIGHT );
}

void EGLpkFileListPanelHeader::OnDrawBg( HDC hdc )
{
	FillRect( hdc, &GetViewRect(), EGWnd_GetBrush( egw_color_t::BG_EDIT ) );
}
