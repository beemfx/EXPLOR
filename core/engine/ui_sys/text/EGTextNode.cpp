// (c) 2015 Beem Media

#include "EGTextNode.h"
#include "EGRenderer.h"
#include "EGFont.h"
#include "EGTextFormat.h"
#include "EGLocalize.h"
#include "EGParse.h"
#include "EGInputGlyphs.h"

eg_int EGTextNode::g_UseStencilIgnore = 0;
eg_int EGTextNode::g_UseDepthIgnore = 0;
EGStringCrcMapFixedSize<eg_color32,EGTextNode::MAX_REGISTERED_COLORS> EGTextNode::s_ColorMap( CT_Clear , eg_color32(255,255,255) );

void EGTextNode::SetUseStencilIgnore( eg_bool bUse )
{
	if( bUse )
	{
		g_UseStencilIgnore++;
	}
	else
	{
		assert( g_UseStencilIgnore > 0 ); // Use/Stop using mismatch!
		g_UseStencilIgnore--;
	}
}

void EGTextNode::SetUseDepthIgnore( eg_bool bUse )
{
	if( bUse )
	{
		g_UseDepthIgnore++;
	}
	else
	{
		assert( g_UseDepthIgnore > 0 ); // Use/Stop using mismatch!
		g_UseDepthIgnore--;
	}
}

EGTextNode::EGTextNode( eg_ctor_t Ct )
: m_CharFont( nullptr )
, m_GlpyhFont( nullptr )
, m_LastX(0)
, m_LastY(0)
, m_ColorStackPos(0)
, m_Align(eg_text_align::LEFT)
, m_LastLineStart(0)
, m_LastGlyphLineStart(0)
, m_AllowFormatting(true)
, m_AllowStyles(true)
, m_bSingleLine(false)
, m_bWordWrapEnabled(true)
{
	assert( Ct == CT_Default );
	unused( Ct );

	zero( &m_TextFormat );
	zero( &m_ColorStack );
}

EGTextNode::~EGTextNode()
{

}

void EGTextNode::PushColor( const eg_color& Color )
{
	if( m_ColorStackPos < countof(m_ColorStack) )
	{
		m_ColorStack[m_ColorStackPos] = eg_color32(Color);
		m_ColorStackPos++;
	}
	else
	{
		assert( false ); //May need a bigger color stack.
	}
	m_TextFormat.Color = Color;
}

void EGTextNode::PopColor()
{
	if( m_ColorStackPos > 1 )
	{
		m_ColorStackPos--;
	}
	else
	{
		assert(false); //You can't pop more colors than you pushed.
	}
	m_TextFormat.Color = eg_color(m_ColorStack[m_ColorStackPos-1]);
}

void EGTextNode::SetColor( const eg_color& Color )
{
	m_ColorStackPos = 0;
	PushColor( Color );
}

void EGTextNode::SetupNode( class EGFontBase* Font , eg_real Width , eg_real Height , eg_real LineHeight )
{
	ClearText();

	m_TextFormat.aabbWindow.Min = eg_vec4( -Width*0.5f , -Height*0.5f , 0 , 1 );
	m_TextFormat.aabbWindow.Max = eg_vec4(  Width*0.5f ,  Height*0.5f , 0 , 1 );
	m_TextFormat.LineHeight = LineHeight;
	m_TextFormat.Color = eg_color(1,1,1,1);
	m_TextFormat.Flags = 0;
	m_Align = eg_text_align::LEFT;
	m_LastLineStart = 0;
	m_LastGlyphLineStart = 0;
	m_bSingleLine = ::EG_IsEqualEps( Height , LineHeight );

	m_CharFont = Font;
	m_GlpyhFont = EGFontMgr::Get()->GetGlyphFont();

	SetColor( eg_color(1,1,1,1) );
}

void EGTextNode::ClearText()
{
	m_VGlyphs.Clear( false );
	m_VChars.Clear( false );
	m_Text.Clear( false );
	m_LastY = 0;
	m_LastX = 0;
	m_LastLineStart = 0;
	m_LastGlyphLineStart = 0;
}

void EGTextNode::ClearAndFreeMem()
{
	ClearText();

	m_VGlyphs.Clear();
	m_VChars.Clear();
	m_Text.Clear();
	m_DrawnChars.Clear();
	m_HandleStyleDrawnChars.Clear();
}

void EGTextNode::FormatText_AddWord_HandleJustify( eg_size_t LastChar , eg_size_t LastGlyph )
{
	if( eg_text_align::LEFT == m_Align)return;

	eg_size_t FirstVertex = m_LastLineStart*6;
	eg_size_t LastVertex  = LastChar*6;//m_VChars.Len();
	eg_size_t FirstGlyphVertex = m_LastGlyphLineStart*6;
	eg_size_t LastGlyphVertex = LastGlyph*6;//m_VGlyphs.Len();

	const eg_real WindowWidth = (m_TextFormat.aabbWindow.Max.x - m_TextFormat.aabbWindow.Min.x);

	eg_real Adj = 0;
	switch( m_Align )
	{
		case eg_text_align::LEFT  : Adj = 0; break;
		case eg_text_align::RIGHT : Adj = WindowWidth-m_LastX; break;
		case eg_text_align::CENTER: Adj = (WindowWidth-m_LastX)*0.5f; break;
	}

	for( eg_size_t i=FirstVertex; i<LastVertex; i++)
	{
		assert( i < m_VChars.Len() );
		m_VChars[i].Pos.x += Adj;
	}

	for( eg_size_t i=FirstGlyphVertex; i<LastGlyphVertex; i++)
	{
		assert( i < m_VGlyphs.Len() );
		m_VGlyphs[i].Pos.x += Adj;
	}

	m_LastLineStart = m_VChars.Len()/6;
	m_LastGlyphLineStart = m_VGlyphs.Len()/6;
}

void EGTextNode::FormatText_AddWord( const eg_loc_char Text[] , eg_size_t Length , eg_bool NoSpace )
{
	m_DrawnChars.Resize( Length*6 );

	auto FormatBySection = [this,&Text,&Length]() -> eg_real
	{
		eg_real EndPos = m_LastX;
		EGFont::egDrawResult DrawRes;
		eg_size_t SectionStart = 0;
		for( eg_size_t i=0; i<Length; i++ )
		{
			if( m_AllowStyles && STYLE_START_CHAR == Text[i] )
			{
				DrawRes = m_CharFont->AppendText( &Text[SectionStart] , i-SectionStart, EndPos , m_LastY , m_TextFormat , m_DrawnChars.GetArray() , m_DrawnChars.Len() );
				m_VChars.Append( m_DrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
				EndPos = DrawRes.XOffset;
				eg_size_t CharsRead = FormatText_HandleStyle( &Text[i] , Length-i, EndPos );
				i+=CharsRead;
				SectionStart = i+1;
			}
		}

		DrawRes = m_CharFont->AppendText( &Text[SectionStart] , Length-SectionStart, EndPos , m_LastY , m_TextFormat , m_DrawnChars.GetArray() , m_DrawnChars.Len() );
		m_VChars.Append( m_DrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
		EndPos = DrawRes.XOffset;

		return EndPos;
	};

	eg_size_t StartChar = m_VChars.Len()/6;
	eg_size_t StartGlyph = m_VGlyphs.Len()/6;
	eg_real StartPos = m_LastX;
	eg_real EndPos = FormatBySection();
	const eg_real WindowWidth = (m_TextFormat.aabbWindow.Max.x - m_TextFormat.aabbWindow.Min.x);

	if( m_bWordWrapEnabled && !m_bSingleLine && ( EndPos > WindowWidth && (m_VChars.Len()/6 > 0 || m_VGlyphs.Len()/6 > 0) ) ) // If too wide and if there is already something there, wrap.
	{
		FormatText_AddWord_HandleJustify( StartChar , StartGlyph );
		m_LastLineStart = StartChar;
		m_LastGlyphLineStart = StartGlyph;
		m_LastX = 0;
		m_LastY += m_TextFormat.LineHeight;
		eg_real Width = EndPos - StartPos;
		EndPos = Width;
		eg_vec4 AdjVec(-StartPos,-m_TextFormat.LineHeight,0,0);
		for( eg_size_t i=(StartChar*6); i<m_VChars.Len(); i++ )
		{
			m_VChars[i].Pos += AdjVec;
		}
		for( eg_size_t i=(StartGlyph*6); i<m_VGlyphs.Len(); i++ )
		{
			m_VGlyphs[i].Pos += AdjVec;
		}
	}

	m_LastX = EndPos;

	if( !NoSpace )
	{
		EGFont::egDrawResult DrawRes = m_CharFont->AppendText( &WORD_SEPARATOR , 1, m_LastX , m_LastY, m_TextFormat , m_DrawnChars.GetArray() , m_DrawnChars.Len() );
		m_LastX = DrawRes.XOffset;
		m_VChars.Append( m_DrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
	}
}

eg_size_t EGTextNode::FormatText_HandleStyle( const eg_loc_char Text[] , eg_size_t Length, eg_real& EndPos )
{
	const eg_loc_char START_CHAR = STYLE_START_CHAR;
	const eg_loc_char END_CHAR = STYLE_END_CHAR;

	m_HandleStyleDrawnChars.Resize( Length*6 );
	
	eg_size_t OutLen=0;

	eg_string FormatString;
	for( eg_uint i=1; 0 == OutLen && i<Length; i++ )
	{
		if( Text[i] != END_CHAR )
		{
			FormatString.Append( static_cast<eg_char>(Text[i]) ); // We're casting to eg_char here, but no style code should use a wide char.
		}
		else
		{
			OutLen = i;
		}
	}

	if(0 == OutLen ){ return 0;} //| has no matching |.
	
	eg_string_crc FormatCrc = eg_string_crc(FormatString);
	eg_string Parms;

	egParseFuncInfo Info;
	EGParse_ParseFunction( FormatString , &Info );

	eg_bool bIsFunction = false;
	EGPARSE_RESULT ParseRes = EGPARSE_OKAY;

	for( eg_uint i=0; i<FormatString.Len(); i++ )
	{
		if( '(' == FormatString[i] )
		{
			bIsFunction = true;
			break;
		}
	}

	if( bIsFunction )
	{
		ParseRes = EGParse_ParseFunction( FormatString , &Info );
	}

	if( bIsFunction )
	{
		if( EGPARSE_OKAY == ParseRes )
		{
			switch_crc( eg_string_crc(Info.FunctionName) )
			{
				case_crc("Glyph"):
				{
					if( Info.NumParms > 0 && m_GlpyhFont )
					{
						eg_string_crc ButtonCrc = eg_string_crc(Info.Parms[0]);
						eg_string_crc KeyCrc = EGInputGlyphs_InputActionToKey( ButtonCrc );
						eg_loc_char GlyphChar = m_GlpyhFont->GlyphNameToChar( KeyCrc );
						if( GlyphChar != '\0' )
						{
							EGFont::egTextFormat GlyphFormat = m_TextFormat;
							GlyphFormat.Color = eg_color(1,1,1,1);
							EGFont::egDrawResult DrawRes = m_GlpyhFont->AppendText( &GlyphChar , 1, EndPos , m_LastY , GlyphFormat , m_HandleStyleDrawnChars.GetArray() , m_HandleStyleDrawnChars.Len() );
							m_VGlyphs.Append( m_HandleStyleDrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
							EndPos = DrawRes.XOffset;
						}
						else
						{
							eg_d_string KeyStr = EGInputGlyphs_InputActionToKeyStr( ButtonCrc );
							eg_d_string16 RawName = KeyStr.Len() > 0 ? EGSFormat16( L"[{0}]" , *KeyStr ) : L"-";
							EGFont::egDrawResult DrawRes = m_CharFont->AppendText( *RawName , RawName.Len() , EndPos , m_LastY , m_TextFormat , m_HandleStyleDrawnChars.GetArray() , m_HandleStyleDrawnChars.Len() );
							m_VChars.Append( m_HandleStyleDrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
							EndPos = DrawRes.XOffset;
						}
					}
				} break;
				case_crc("ButtonGlyph"):
				{
					eg_string_crc KeyCrc = eg_string_crc(Info.Parms[0]);
					eg_loc_char GlyphChar = m_GlpyhFont->GlyphNameToChar( KeyCrc );
					if( GlyphChar != '\0' )
					{
						EGFont::egTextFormat GlyphFormat = m_TextFormat;
						GlyphFormat.Color = eg_color(1,1,1,1);
						EGFont::egDrawResult DrawRes = m_GlpyhFont->AppendText( &GlyphChar , 1, EndPos , m_LastY , GlyphFormat , m_HandleStyleDrawnChars.GetArray() , m_HandleStyleDrawnChars.Len() );
						m_VGlyphs.Append( m_HandleStyleDrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
						EndPos = DrawRes.XOffset;
					}
					else
					{
						eg_d_string16 RawName = EGString_StrLen( Info.Parms[0] ) > 0 ? EGSFormat16( L"[{0}]" , Info.Parms[0] ) : L"-";
						EGFont::egDrawResult DrawRes = m_CharFont->AppendText( *RawName , RawName.Len() , EndPos , m_LastY , m_TextFormat , m_HandleStyleDrawnChars.GetArray() , m_HandleStyleDrawnChars.Len() );
						m_VChars.Append( m_HandleStyleDrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
						EndPos = DrawRes.XOffset;
					}
				} break;
				case_crc("SC"):
				case_crc("SetColor"):
				{
					eg_color32 Color( m_TextFormat.Color );
					if( Info.NumParms == 1 )
					{
						eg_uint8 Alpha = Color.A;
						eg_string_crc ColorName( Info.Parms[0] );
						Color = s_ColorMap[ColorName];
						Color.A = Alpha;
					}
					else if( Info.NumParms == 3 )
					{
						Color.R = static_cast<eg_uint8>(eg_string(Info.Parms[0]).ToUInt());
						Color.G = static_cast<eg_uint8>(eg_string(Info.Parms[1]).ToUInt());
						Color.B = static_cast<eg_uint8>(eg_string(Info.Parms[2]).ToUInt());
					}
					else if( Info.NumParms == 4 )
					{
						Color.A = static_cast<eg_uint8>(eg_string(Info.Parms[0]).ToUInt());
						Color.R = static_cast<eg_uint8>(eg_string(Info.Parms[1]).ToUInt());
						Color.G = static_cast<eg_uint8>(eg_string(Info.Parms[2]).ToUInt());
						Color.B = static_cast<eg_uint8>(eg_string(Info.Parms[3]).ToUInt());
					}
					PushColor( eg_color(Color) );
				} break;
				case_crc("RC"):
				case_crc("RestoreColor"):
				{
					this->PopColor();
				} break;
			}
		}
	}
	else if( eg_crc("I") == FormatCrc )
	{
		m_TextFormat.Flags.Toggle( EGFont::F_DRAW_I );
	}
	else if( eg_crc("SUP") == FormatCrc )
	{
		m_TextFormat.Flags.Toggle( EGFont::F_DRAW_SUP );
	}
	else if( eg_crc("SUB") == FormatCrc )
	{
		m_TextFormat.Flags.Toggle( EGFont::F_DRAW_SUB );
	}
	else if( eg_crc("BAR") == FormatCrc )
	{
		eg_loc_char BarChar = L'|';
		EGFont::egDrawResult DrawRes = m_CharFont->AppendText( &BarChar , 1, EndPos , m_LastY , m_TextFormat , m_HandleStyleDrawnChars.GetArray() , m_HandleStyleDrawnChars.Len() );
		m_VChars.Append( m_HandleStyleDrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
		EndPos = DrawRes.XOffset;
	}
	else if( eg_crc("COPY") == FormatCrc )
	{
		eg_loc_char CopyChar = L'\xA9';
		EGFont::egDrawResult DrawRes = m_CharFont->AppendText( &CopyChar , 1, EndPos , m_LastY , m_TextFormat , m_HandleStyleDrawnChars.GetArray() , m_HandleStyleDrawnChars.Len() );
		m_VChars.Append( m_HandleStyleDrawnChars.GetArray() , DrawRes.CharsDrawn*6 );
		EndPos = DrawRes.XOffset;
	}

	return OutLen;
}

void EGTextNode::FormatText_SquishSingleLine()
{
	assert( m_bSingleLine );
	const eg_real WindowWidth = (m_TextFormat.aabbWindow.Max.x - m_TextFormat.aabbWindow.Min.x);
	assert( m_LastX > WindowWidth );
	assert( WindowWidth > 0.f );

	// This is just EGMath_GetMappedRangeValue, but optimized.
	const eg_real SquishFactor = WindowWidth/m_LastX;
	const eg_real LeftSide = m_TextFormat.aabbWindow.Min.x;

	auto TransformPoint =[&SquishFactor,&LeftSide]( eg_vec4& Point ) -> void
	{
		Point.x = LeftSide + (Point.x - LeftSide)*SquishFactor;
	};

	for( eg_size_t i=0; i<m_VChars.Len(); i++ )
	{
		TransformPoint( m_VChars[i].Pos );
	}
	for( eg_size_t i=0; i<m_VGlyphs.Len(); i++ )
	{
		TransformPoint( m_VGlyphs[i].Pos );
	}
	m_LastX = WindowWidth;
}

void EGTextNode::FormatText_SquishHeight()
{
	const eg_real TextHeight = (m_LastY+m_TextFormat.LineHeight);

	assert( !m_bSingleLine );
	const eg_real WindowHeight = m_TextFormat.aabbWindow.GetHeight();
	assert( TextHeight > WindowHeight );
	assert( WindowHeight > 0.f );

	// This is just EGMath_GetMappedRangeValue, but optimized.
	const eg_real SquishFactor = WindowHeight/TextHeight;
	const eg_real TopSide = m_TextFormat.aabbWindow.Min.y;
	const eg_real TopSquished = TopSide*SquishFactor;
	const eg_real TopAdj = TopSquished - TopSide;

	const eg_real LineOffset = TopAdj;

	auto TransformPoint =[&SquishFactor,&TopSide,&LineOffset]( eg_vec4& Point ) -> void
	{
		Point.y = Point.y*SquishFactor + LineOffset;
	};

	for( eg_size_t i=0; i<m_VChars.Len(); i++ )
	{
		TransformPoint( m_VChars[i].Pos );
	}
	for( eg_size_t i=0; i<m_VGlyphs.Len(); i++ )
	{
		TransformPoint( m_VGlyphs[i].Pos );
	}
	m_LastY = WindowHeight - m_TextFormat.LineHeight;
}

void EGTextNode::SetText( const eg_loc_text& LocText )
{
	FormatText( LocText.GetString() , LocText.GetLen() );
}

void EGTextNode::FormatText( const eg_loc_char RawText[] , eg_size_t RawLength )
{
	ClearText();

	m_Text.Resize( RawLength );
	for( eg_size_t i=0; i<m_Text.Len(); i++ )
	{
		m_Text[i] = RawText[i];
	}

	//We're going to do this one word at a time in case we need to wrap.
	eg_size_t WordStart = 0;
	eg_bool bInFormatSection = false;
	for( eg_size_t i=0; i<m_Text.Len(); i++ )
	{
		if( !bInFormatSection )
		{
			if( '\n' == m_Text[i] )
			{
				FormatText_AddWord( &m_Text[WordStart] , i-WordStart , true );
				m_LastX = 0;
				m_LastY += m_TextFormat.LineHeight;
				WordStart = i+1;
			}
			else if( '\r' == m_Text[i] )
			{
				FormatText_AddWord( &m_Text[WordStart] , i-WordStart , false );
				WordStart = i+1;
			}
			else if( WORD_SEPARATOR == m_Text[i] )
			{
				FormatText_AddWord( &m_Text[WordStart] , i-WordStart , false );
				WordStart = i+1;
			}
		}
		else if( m_AllowFormatting && STYLE_START_CHAR == m_Text[i] )
		{
			bInFormatSection = !bInFormatSection;
		}
	}
	//Add the last word.
	assert( !bInFormatSection ); // Missing ending | for format, the text will not look right.
	if( m_Text.Len() > WordStart )
	{
		FormatText_AddWord( &m_Text[WordStart] , m_Text.Len() - WordStart , true );
	}
	if( m_bSingleLine )
	{
		const eg_real WindowWidth = (m_TextFormat.aabbWindow.Max.x - m_TextFormat.aabbWindow.Min.x);
		if( m_LastX > WindowWidth )
		{
			FormatText_SquishSingleLine();
		}
	}
	if( !m_bSingleLine )
	{
		const eg_real WindowHeight = m_TextFormat.aabbWindow.GetHeight();
		if( (m_LastY+m_TextFormat.LineHeight) > WindowHeight )
		{
			FormatText_SquishHeight();
		}
	}
	FormatText_AddWord_HandleJustify(m_VChars.Len()/6,m_VGlyphs.Len()/6);
}

void EGTextNode::Draw()
{
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::FONT );
	if( g_UseStencilIgnore > 0 )
	{
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONZERO );
	}
	else if( g_UseDepthIgnore > 0 )
	{
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
	}
	else
	{
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF );
	}
	MainDisplayList->SetMaterial( m_CharFont->GetMaterial() );
	MainDisplayList->DrawRawTris( m_VChars.GetArray() , static_cast<eg_uint>(m_VChars.Len()/3) );

	if( m_VGlyphs.Len() > 0 )
	{
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::FONT_GLYPH );
		MainDisplayList->SetMaterial( m_GlpyhFont->GetMaterial() );
		MainDisplayList->DrawRawTris( m_VGlyphs.GetArray() , static_cast<eg_uint>(m_VGlyphs.Len()/3) );
		MainDisplayList->PopDefaultShader();
	}

	MainDisplayList->PopDepthStencilState();
	MainDisplayList->PopDefaultShader();
}

void EGTextNode::DrawShadow( const eg_color& Color )
{
	MainDisplayList->SetVec4( eg_rv4_t::ENT_PALETTE_0 , Color.ToVec4() );
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::FONT_SHADOW );
	MainDisplayList->PushDepthStencilState( g_UseStencilIgnore == 0 ? eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF : eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONZERO );
	MainDisplayList->SetMaterial( m_CharFont->GetMaterial() );
	MainDisplayList->DrawRawTris( m_VChars.GetArray() , static_cast<eg_uint>(m_VChars.Len()/3) );

	if( m_VGlyphs.Len() > 0 )
	{
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::FONT_GLYPH_SHADOW );
		MainDisplayList->SetMaterial( m_GlpyhFont->GetMaterial() );
		MainDisplayList->DrawRawTris( m_VGlyphs.GetArray() , static_cast<eg_uint>(m_VGlyphs.Len()/3) );
		MainDisplayList->PopDefaultShader();
	}

	MainDisplayList->PopDepthStencilState();
	MainDisplayList->PopDefaultShader();
}

void EGTextNode::DrawDebugBorder()
{
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::FONT );
	MainDisplayList->PushDepthStencilState( g_UseStencilIgnore == 0 ? eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF : eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONZERO );
	MainDisplayList->SetMaterial( m_CharFont->GetMaterial() );

	MainDisplayList->PushDefaultShader( eg_defaultshader_t::COLOR );
	MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
	MainDisplayList->DrawAABB( m_TextFormat.aabbWindow , eg_color(eg_color32(255,255,0)) );
	eg_aabb TextSize = m_TextFormat.aabbWindow;
	TextSize.Min.x = m_TextFormat.aabbWindow.Min.x;
	if( m_Align == eg_text_align::CENTER )
	{
		TextSize.Min.x += (m_TextFormat.aabbWindow.GetWidth()-m_LastX)*.5f;
	}
	else if( m_Align == eg_text_align::RIGHT )
	{
		TextSize.Min.x += (m_TextFormat.aabbWindow.GetWidth()-m_LastX);
	}
	TextSize.Max.x = TextSize.Min.x + m_LastX;
	MainDisplayList->DrawAABB( TextSize , eg_color(eg_color32(255,0,255)) );
	MainDisplayList->PopDefaultShader();
	MainDisplayList->PopDepthStencilState();

	MainDisplayList->PopDepthStencilState();
	MainDisplayList->PopDefaultShader();
}

eg_vec2 EGTextNode::GetFontShadowOffset() const
{
	// TODO: This value should be stored in the font data asset...
	const eg_real ShadowOffsetFactor = .07f;
	const eg_real ShadowOffset = m_TextFormat.LineHeight * ShadowOffsetFactor;
	return eg_vec2( ShadowOffset , -ShadowOffset );
}

void EGTextNode::RegisterDefaultColors()
{
	RegisterColor( eg_crc( "RED" ) , eg_color32( 255 , 0 , 0 ) );
	RegisterColor( eg_crc( "YELLOW" ) , eg_color32( 255 , 255 , 0 ) );
	RegisterColor( eg_crc( "GREEN" ) , eg_color32( 0 , 255 , 0 ) );
}

void EGTextNode::RegisterColor( eg_string_crc Crc , eg_color32 Color )
{
	if( s_ColorMap.Contains( Crc ) )
	{
		s_ColorMap[Crc] = Color;
	}
	else if( !s_ColorMap.IsFull() )
	{
		s_ColorMap.Insert( Crc , Color );
	}
	else
	{
		assert( false ); // Too many colors registered.
	}
}
