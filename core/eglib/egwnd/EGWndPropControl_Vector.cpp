// (c) 2017 Beem Media

#include "EGWndPropControl_Vector.h"

EGWndPropControl_Vector::EGWndPropControl_Vector( IWndPropControlOwner* Parent, const egPropControlInfo& Info , eg_int Size , egwnd_prop_control_vector_t Type )
: Super( Parent , Info )
, m_Size( Size )
, m_Type( Type )
, m_WndEditX( this )
, m_WndEditY( this )
, m_WndEditZ( this )
, m_WndEditW( this )
{
	m_Height = FONT_SIZE*4;

	assert( EG_IsBetween<eg_uint>( m_Size , 2 , 4 ) );

	EGWndTextNode* Nodes[] = { &m_WndEditX , &m_WndEditY , &m_WndEditZ , &m_WndEditW };

	for( eg_int i=0; i<countof(Nodes); i++ )
	{
		Nodes[i]->SetVisible( i < m_Size );
		Nodes[i]->SetFont( m_Font );
		Nodes[i]->SetMultiline( false );
	}

	RefreshFromSourceData();
}

LRESULT EGWndPropControl_Vector::WndProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	case WM_EG_COMMIT_ON_FOCUS_LOST:
	case WM_EG_COMMIT:
	{
		CommitChangesToSourceData( reinterpret_cast<HWND>(lParam) );
		break;
	}
	}

	return Super::WndProc( message , wParam , lParam );
}

void EGWndPropControl_Vector::OnWmSize( const eg_ivec2& NewClientSize )
{
	Super::OnWmSize( NewClientSize );

	const eg_int EditWidth = GetEditWidth();
	const eg_int Padding = GetPadding();

	EGWndTextNode* Nodes[] = { &m_WndEditX , &m_WndEditY , &m_WndEditZ , &m_WndEditW };

	for( eg_int i=0; i<countof(Nodes); i++ )
	{
		Nodes[i]->SetWndPos( Padding + EditWidth*i , FONT_SIZE*2 , EditWidth , FONT_SIZE + 8 );
	}
}

void EGWndPropControl_Vector::OnPaint( HDC hdc )
{
	Super::OnPaint( hdc );

	static const eg_cpstr16 Labels[] = { L"X" , L"Y" , L"Z" , L"W" };

	const eg_int EditWidth = GetEditWidth();
	const eg_int Padding = GetPadding();

	// SetFont( hdc , EGWnd_GetFont( egw_font_t::DEFAULT_BOLD ) );
	// SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );

	for( eg_int i=0; i<m_Size; i++ )
	{
		TextOutW( hdc , Padding + EditWidth*i , FONT_SIZE , Labels[i] , 1 );
	}
}

void EGWndPropControl_Vector::CommitChangesToSourceData( HWND CommitControl )
{
	unused( CommitControl );

	auto ProcessAsVec2 = [this]() -> void
	{
		if( m_ControlInfo.Editor )
		{
			if( m_Type == egwnd_prop_control_vector_t::Real )
			{
				eg_vec2* AsVec2 = reinterpret_cast<eg_vec2*>(m_ControlInfo.Editor->GetData());
				eg_vec2 OldVec = *AsVec2;

				eg_vec2 NewVec;
				NewVec.x = m_WndEditX.GetText().ToFloat();
				NewVec.y = m_WndEditY.GetText().ToFloat();

				if( OldVec != NewVec )
				{
					*AsVec2 = NewVec;
					m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
					RefreshFromSourceData();
				}
			}
			else
			{
				eg_ivec2* AsVec2 = reinterpret_cast<eg_ivec2*>(m_ControlInfo.Editor->GetData());
				eg_ivec2 OldVec = *AsVec2;

				eg_ivec2 NewVec;
				NewVec.x = m_WndEditX.GetText().ToInt();
				NewVec.y = m_WndEditY.GetText().ToInt();

				if( OldVec != NewVec )
				{
					*AsVec2 = NewVec;
					m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
					RefreshFromSourceData();
				}
			}
		}
	};

	auto ProcessAsVec3 = [this]() -> void
	{
		if( m_ControlInfo.Editor )
		{
			if( m_Type == egwnd_prop_control_vector_t::Real )
			{
				eg_vec3* AsVec3 = reinterpret_cast<eg_vec3*>( m_ControlInfo.Editor->GetData() );
				eg_vec3 OldVec = *AsVec3;

				eg_vec3 NewVec;
				NewVec.x = m_WndEditX.GetText().ToFloat();
				NewVec.y = m_WndEditY.GetText().ToFloat();
				NewVec.z = m_WndEditZ.GetText().ToFloat();

				if( OldVec != NewVec )
				{
					*AsVec3 = NewVec;
					m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
					RefreshFromSourceData();
				}
			}
			else
			{
				eg_ivec3* AsVec3 = reinterpret_cast<eg_ivec3*>( m_ControlInfo.Editor->GetData() );
				eg_ivec3 OldVec = *AsVec3;

				eg_ivec3 NewVec;
				NewVec.x = m_WndEditX.GetText().ToInt();
				NewVec.y = m_WndEditY.GetText().ToInt();
				NewVec.z = m_WndEditZ.GetText().ToInt();

				if( OldVec != NewVec )
				{
					*AsVec3 = NewVec;
					m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
					RefreshFromSourceData();
				}
			}
		}
	};

	auto ProcessAsVec4 = [this]() -> void
	{
		if( m_ControlInfo.Editor )
		{
			if( m_Type == egwnd_prop_control_vector_t::Real )
			{
				eg_vec4* AsVec4 = reinterpret_cast<eg_vec4*>( m_ControlInfo.Editor->GetData() );
				eg_vec4 OldVec = *AsVec4;

				eg_vec4 NewVec;
				NewVec.x = m_WndEditX.GetText().ToFloat();
				NewVec.y = m_WndEditY.GetText().ToFloat();
				NewVec.z = m_WndEditZ.GetText().ToFloat();
				NewVec.w = m_WndEditW.GetText().ToFloat();

				if( OldVec != NewVec )
				{
					*AsVec4 = NewVec;
					m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
					RefreshFromSourceData();
				}
			}
			else
			{
				eg_ivec4* AsVec4 = reinterpret_cast<eg_ivec4*>( m_ControlInfo.Editor->GetData() );
				eg_ivec4 OldVec = *AsVec4;

				eg_ivec4 NewVec;
				NewVec.x = m_WndEditX.GetText().ToInt();
				NewVec.y = m_WndEditY.GetText().ToInt();
				NewVec.z = m_WndEditZ.GetText().ToInt();
				NewVec.w = m_WndEditW.GetText().ToInt();

				if( OldVec != NewVec )
				{
					*AsVec4 = NewVec;
					m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
					RefreshFromSourceData();
				}
			}
		}
	};

	switch( m_Size )
	{
		case 2:
			ProcessAsVec2();
			break;
		case 3:
			ProcessAsVec3();
			break;
		case 4:
			ProcessAsVec4();
			break;
		default:
			assert( false );
			break;
	}
}

void EGWndPropControl_Vector::RefreshFromSourceData()
{
	eg_vec4 Vec4( CT_Default );
	eg_ivec4 IntVec4( CT_Default );

	switch( m_Size )
	{
		case 2:
		{
			if( m_Type == egwnd_prop_control_vector_t::Real )
			{
				eg_vec2 Vec2 = m_ControlInfo.Editor ? *reinterpret_cast<const eg_vec2*>(m_ControlInfo.Editor->GetData()) : CT_Clear;
				Vec4 = eg_vec4( Vec2.x , Vec2.y , 0.f , 0.f );
			}
			else
			{
				eg_ivec2 Vec2 = m_ControlInfo.Editor ? *reinterpret_cast<const eg_ivec2*>(m_ControlInfo.Editor->GetData()) : CT_Clear;
				IntVec4 = eg_ivec4( Vec2.x , Vec2.y , 0 , 0 );
			}
		} break;
		case 3:
		{
			if( m_Type == egwnd_prop_control_vector_t::Real )
			{
				eg_vec3 Vec3 = m_ControlInfo.Editor ? *reinterpret_cast<const eg_vec3*>(m_ControlInfo.Editor->GetData()) : CT_Clear;
				Vec4 = eg_vec4( Vec3.x , Vec3.y , Vec3.z , 0.f );
			}
			else
			{
				eg_ivec3 Vec3 = m_ControlInfo.Editor ? *reinterpret_cast<const eg_ivec3*>(m_ControlInfo.Editor->GetData()) : CT_Clear;
				IntVec4 = eg_ivec4( Vec3.x , Vec3.y , Vec3.z , 0 );
			}
		} break;
		case 4:
		{
			if( m_Type == egwnd_prop_control_vector_t::Real )
			{
				Vec4 = m_ControlInfo.Editor ? *reinterpret_cast<const eg_vec4*>(m_ControlInfo.Editor->GetData()) : CT_Clear;
			}
			else
			{
				IntVec4 = m_ControlInfo.Editor ? *reinterpret_cast<const eg_ivec4*>(m_ControlInfo.Editor->GetData()) : CT_Clear;
			}
		} break;
		default:
			assert( false );
			break;
	}

	if( m_Type == egwnd_prop_control_vector_t::Real )
	{
		m_WndEditX.SetText( EGString_Format("%g",Vec4.x) );
		m_WndEditY.SetText( EGString_Format("%g",Vec4.y) );
		m_WndEditZ.SetText( EGString_Format("%g",Vec4.z) );
		m_WndEditW.SetText( EGString_Format("%g",Vec4.w) );
	}
	else
	{
		m_WndEditX.SetText( EGString_Format("%d",IntVec4.x) );
		m_WndEditY.SetText( EGString_Format("%d",IntVec4.y) );
		m_WndEditZ.SetText( EGString_Format("%d",IntVec4.z) );
		m_WndEditW.SetText( EGString_Format("%d",IntVec4.w) );
	}
}

eg_int EGWndPropControl_Vector::GetEditWidth() const
{
	eg_int EditWidthAdj = EG_Clamp<eg_int>( GetViewRect().right , 150 , 300 );

	return ( EditWidthAdj - GetPadding() ) / m_Size;
}

eg_int EGWndPropControl_Vector::GetPadding() const
{
	return 5;
}
