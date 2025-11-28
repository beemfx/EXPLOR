#include "EGFont.h"
#include "EGLocalize.h"
#include "EGRenderer.h"
#include "EGEngineConfig.h"
#include "EGCrcDb.h"
#include "EGInputGlyphs.h"
#include <fs_sys2/fs_sys2.h>

EGFont::EGFont(eg_cpstr strFile)
: EGFontBase( FIRST_CHAR , LAST_CHAR , UNK_CHAR )
, m_nTextureWidth(0)
, m_nTextureHeight(0)
, m_FontSize(0)
{	
	m_NextAdditionalChar = 0;

	XMLLoad(strFile);

	// We want the font to fit exactly inside a text node, so 
	// we're going to find the largest extents to the top and bottom
	// and treat the bottom most extent as the baseline.
	eg_real TopMost = 0.f;
	eg_real BottomMost = 0.f;
	for( eg_uint i=0; i<m_CI.Len(); i++ )
	{
		const egCharInfo& CI = m_CI[i];

		eg_real Top = CI.Offset.y;
		eg_real Bottom = Top - m_CI[i].Dims.y;

		TopMost = EG_Max( TopMost , Top );
		BottomMost = EG_Min( BottomMost , Bottom );
	}

	m_CharScale = static_cast<eg_real>(EG_Abs(TopMost - BottomMost));
	m_BaseYOffset = -BottomMost;

	m_txFont = EGRenderer::Get().CreateMaterialFromTextureFile(m_sTex);
	m_flags.Set(F_VALID);
}

EGFont::~EGFont()
{
	if(!m_flags.IsSet(F_VALID))return;

	EGRenderer::Get().DestroyMaterial(m_txFont);
	m_txFont = EGV_MATERIAL_NULL;
	m_flags.UnSet(F_VALID);
}

EGFont::egDrawResult EGFontBase::AppendText( const eg_loc_char LocText[] , eg_size_t LocTextLength , eg_real x , eg_real y , const egTextFormat& Format , egv_vert_simple* VertsOut , eg_size_t VertsOutSize )
{
	//Setup the vertexes
	eg_real XPos = x/Format.LineHeight;
	
	const eg_real I_SIZE = 0.15f*m_CharScale;

	const eg_real fTopAdj = Format.Flags.IsSet(F_DRAW_I) ?  I_SIZE : 0;
	const eg_real fBotAdj = Format.Flags.IsSet(F_DRAW_I) ? -I_SIZE : 0;
	const eg_real fScaleAdj = Format.Flags.IsSet(F_DRAW_SUB) || Format.Flags.IsSet(F_DRAW_SUP) ? .4f : 1.f;

	eg_mat mTranslate;
	mTranslate = eg_mat::I;
	mTranslate *= eg_mat::BuildTranslation( eg_vec3( 0 , -(y/Format.LineHeight) , 0 ) );
	mTranslate *= eg_mat::BuildScaling( Format.LineHeight );
	mTranslate *= eg_mat::BuildTranslation( eg_vec3( Format.aabbWindow.Min.x , Format.aabbWindow.Max.y - Format.LineHeight , 0 ) );
	if( Format.Flags.IsSet(F_DRAW_SUP) )
	{
		mTranslate.TranslateThis( 0.f , Format.LineHeight*.7f , 0.f );
	}
	if( Format.Flags.IsSet(F_DRAW_SUB) )
	{
		mTranslate.TranslateThis( 0.f , -Format.LineHeight*.1f , 0.f );
	}

	eg_uint nChars=0;
	for(eg_uint i=0; i<(VertsOutSize/6) && i < LocTextLength; i++, nChars++)
	{
		//Find the vertexes for this letter.
		egv_vert_simple* pVerts = &VertsOut[i*6];

		eg_loc_char c = LocText[i];
		eg_int CharIndex = 0;
		if( EG_IsBetween<eg_loc_char>( c , m_FirstChar , m_LastChar ) )
		{
			CharIndex = c - m_FirstChar;
		}
		else
		{
			// Search for character...
			for( eg_int i=c - m_LastChar + 1; i<m_CI.Len(); i++ )
			{
				if( c == m_CI[i].LocChar )
				{
					CharIndex = i;
					break;
				}
			}
		}

		const egCharInfo& CI = m_CI[CharIndex];

		//Get the width of the character.
		const eg_real fWidth  = CI.Dims.x*m_CharScale*fScaleAdj;
		const eg_real fHeight = CI.Dims.y*m_CharScale*fScaleAdj;

		const eg_real xBase = XPos + CI.Offset.x*m_CharScale*fScaleAdj;
		const eg_real yBase = (m_BaseYOffset + CI.Offset.y)*m_CharScale*fScaleAdj;

		eg_vec4 TopLeft( xBase+fTopAdj , yBase , 0.0f , 1.0f );
		eg_vec4 BottomLeft  = TopLeft + eg_vec4(-fTopAdj+fBotAdj,-fHeight,0.f,0.f);
		eg_vec4 TopRight    = TopLeft + eg_vec4(fWidth,0.f,0.f,0.f);
		eg_vec4 BottomRight = BottomLeft + eg_vec4(fWidth,0.f,0.f,0.f);

		pVerts[0].Pos    = BottomLeft;
		pVerts[0].Tex0   = eg_vec2(CI.fLeft, CI.fBottom);
		pVerts[0].Color0 = Format.Color;

		pVerts[1].Pos    = TopLeft;
		pVerts[1].Tex0   = eg_vec2(CI.fLeft, CI.fTop);
		pVerts[1].Color0 = Format.Color;
			
		pVerts[2].Pos    = BottomRight;
		pVerts[2].Tex0   = eg_vec2(CI.fRight, CI.fBottom);
		pVerts[2].Color0 = Format.Color;

		pVerts[3].Pos    = BottomRight;
		pVerts[3].Tex0   = eg_vec2(CI.fRight, CI.fBottom);
		pVerts[3].Color0 = Format.Color;

		pVerts[4].Pos    = TopLeft;
		pVerts[4].Tex0   = eg_vec2(CI.fLeft, CI.fTop);
		pVerts[4].Color0 = Format.Color;

		pVerts[5].Pos    = TopRight;
		pVerts[5].Tex0   = eg_vec2(CI.fRight, CI.fTop);
		pVerts[5].Color0 = Format.Color;

		XPos += CI.Advance.x*m_CharScale*fScaleAdj;

		for( eg_uint i=0; i<6;i++ )
		{
			pVerts[i].Pos *= mTranslate;
		}

	}

	egDrawResult Result;
	Result.CharsDrawn = nChars;
	Result.XOffset = XPos*Format.LineHeight;
	return Result;
}


///////////////////////////////////////////////////////////////////////////////
/// XML
///////////////////////////////////////////////////////////////////////////////

void EGFont::OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& Getter)
{
	if("efont"==Tag)
	{	
		eg_uint nMinChar = Getter.GetString( "min_char" ).ToUIntFromHex();
		eg_uint nMaxChar = Getter.GetString( "max_char" ).ToUIntFromHex();

		m_sTex = Getter.GetString( "tex" );
		m_nTextureWidth  = Getter.GetUInt( "tex_width" );
		m_nTextureHeight = Getter.GetUInt( "tex_height" );
		m_FontSize = Getter.GetUInt( "font_size" );

		//SetFromTag(m_strFilename, atts);
		m_sTex.MakeThisFilenameRelativeTo(GetXmlFilename());
		assert(nMinChar == FIRST_CHAR);
		assert(nMaxChar == LAST_CHAR);
	}
	else if("char"==Tag)
	{
		eg_uint u = 0;

		eg_uint Pos[2];
		eg_uint Dims[2];
		eg_real Advance[2];
		eg_int Offset[2];
		
		u = Getter.GetString("u").ToUIntFromHex();
		EGString_GetIntList( Getter.GetString("pos" , "0 0").String() , 0 , &Pos , countof(Pos) );
		EGString_GetIntList( Getter.GetString("dims" , "0 0").String() , 0 , &Dims , countof(Dims) );
		EGString_GetFloatList( Getter.GetString("advance" , "0 0").String() , 0 , &Advance , countof(Advance) );
		EGString_GetIntList( Getter.GetString("offset" , "0 0").String() , 0 , &Offset , countof(Offset) );

		auto PopulateCharInfo = [&Offset,&Dims,&Pos,&Advance,&u,this]( egCharInfo& CI ) -> void
		{
			// Position within texture
			CI.LocChar    = EG_To<eg_loc_char>(u);
			CI.fLeft   = (static_cast<eg_real>(Pos[0]-1) )/m_nTextureWidth;
			CI.fRight  = (static_cast<eg_real>(Pos[0]+Dims[0]+1) )/m_nTextureWidth;
			CI.fTop    = (static_cast<eg_real>(Pos[1]-1) )/m_nTextureHeight;
			CI.fBottom = (static_cast<eg_real>(Pos[1]+Dims[1]+1) )/m_nTextureHeight;

			const eg_real DrawScale = 1.f/m_FontSize;

			CI.Advance.x = static_cast<eg_real>(Advance[0])*DrawScale;
			CI.Advance.y = static_cast<eg_real>(Advance[1])*DrawScale;

			CI.Offset.x = static_cast<eg_real>(Offset[0])*DrawScale;
			CI.Offset.y = static_cast<eg_real>(Offset[1])*DrawScale;

			CI.Dims.x = static_cast<eg_real>(Dims[0])*DrawScale;
			CI.Dims.y = static_cast<eg_real>(Dims[1])*DrawScale;
		};

		if(FIRST_CHAR <= u && u<=LAST_CHAR)
		{
			eg_uint Index = u - FIRST_CHAR;
			m_CI.ExtendToAtLeast( Index+1 );
			egCharInfo& CI = m_CI[Index];
			PopulateCharInfo( CI );
			
		}
		else
		{
			eg_int TargetIndex = LAST_CHAR - FIRST_CHAR + 1 + m_NextAdditionalChar;
			m_NextAdditionalChar++;
			m_CI.ExtendToAtLeast( TargetIndex + 1 );
			if( m_CI.IsValidIndex( TargetIndex ) )
			{
				egCharInfo& CI = m_CI[TargetIndex];
				PopulateCharInfo( CI );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Font Manager
//
///////////////////////////////////////////////////////////////////////////////
EGFontMgr EGFontMgr::s_FontMgr;

void EGFontMgr::Init()
{
	FS_EnumerateFilesOfType( L"efont" , EnumerateFontFiles , this );
	EGFontBase* DefaultFont = m_FontMap.Get( CON_FONT_ID );
	assert( nullptr != DefaultFont ); //No default font! Uh oh. Data may not have been deleted.
	m_FontMap.SetNotFoundItem( static_cast<EGFont*>(DefaultFont) );

	m_GlyphFont.Init( "/egdata/font/ButtonFont.xml" );
}

void EGFontMgr::Init_AddFont( eg_cpstr Filename )
{
	if( m_FontMap.IsFull() )
	{
		assert( false );
		EGLogf( eg_log_t::Error ,  __FUNCTION__ ": Too many fonts loaded." );
		return;
	}
	EGFont* NewFont = new ( eg_mem_pool::System ) EGFont( Filename );
	if( nullptr == NewFont )
	{
		assert( false );
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Out of memory?" );
		return;		
	}

	//Create the string_crc for this font.
	eg_string StrFontname = Filename;
	StrFontname.ConvertToLower();
	StrFontname.SetToFilenameFromPathNoExt( StrFontname );
	EGCrcDb::AddAndSaveIfInTool( StrFontname );
	m_FontMap.Insert( eg_string_crc(StrFontname) , NewFont );
}

void EGFontMgr::Deinit()
{
	m_GlyphFont.Deinit();

	for( eg_uint i=0; i<m_FontMap.Len(); i++ )
	{
		EGFont* Font = m_FontMap.GetByIndex( i );
		if( Font )
		{
			delete Font;
		}
	}
	m_FontMap.Clear();
}

EGFontBase* EGFontMgr::GetFont( eg_string_crc FontId ) const
{
	return m_FontMap.Get( FontId );
}

EGGlyphFont* EGFontMgr::GetGlyphFont()
{
	return &m_GlyphFont;
}

void EGFontMgr::QueryFonts( EGArray<eg_string_crc>& Out )
{
	for( eg_size_t i=0; i<m_FontMap.Len(); i++ )
	{
		eg_string_crc Crc = m_FontMap.GetKeyByIndex( i );
		if( !Out.Contains( Crc ) && Crc != eg_string_crc( CT_Clear ) )
		{
			Out.Append( Crc );
		}
	}
}

void EGFontMgr::SetDefaultFont( eg_string_crc FontId )
{
	if( m_FontMap.Contains( FontId ) )
	{
		EGFontBase* DefaultFont = m_FontMap.Get( FontId );
		assert( nullptr != DefaultFont ); //No default font! Uh oh. Data may not have been deleted.
		m_FontMap.SetNotFoundItem( static_cast<EGFont*>(DefaultFont) );
	}
}

void EGFontMgr::EnumerateFontFiles( eg_cpstr16 Filename, void* Data )
{
	EGFontMgr* _this = reinterpret_cast<EGFontMgr*>(Data);
	_this->Init_AddFont( EGString_ToMultibyte(Filename) );
}

void EGGlyphFont::Init( eg_cpstr Filename )
{
	EGDataLibrary<egGlphyFontCharInfo>::SetWarnAboutDuplicateIds( false );

	EGDataLibrary<egGlphyFontCharInfo>::Init( Filename );

	m_FirstChar = 1;
	m_LastChar = m_Infos.LenAs<eg_loc_char>();

	m_txFont = EGRenderer::Get().CreateMaterialFromTextureFile( m_GlyphFile );

	m_CharScale = 1.f;
	m_BaseYOffset = 0.f;

	const eg_uint TX_WIDTH = m_nTextureWidth;
	const eg_uint TX_HEIGHT = m_nTextureHeight;
	const eg_uint GRID_WIDTH = 9;

	const eg_uint ITEM_WIDTH = 100;
	const eg_uint ITEM_HEIGHT = 100;

	const eg_uint GLYPH_PAD = 4;

	m_CI.Resize( m_Infos.LenAs<eg_uint>() );
	for( eg_uint i=0; i<m_Infos.LenAs<eg_uint>(); i++ )
	{
		egCharInfo& CI = m_CI[i];

		eg_uint Row = i/GRID_WIDTH;
		eg_uint Column = i%GRID_WIDTH;

		CI.fLeft   = static_cast<eg_real>(Column*(ITEM_WIDTH+GLYPH_PAD*2) + GLYPH_PAD );
		CI.fRight  = static_cast<eg_real>(Column*(ITEM_WIDTH+GLYPH_PAD*2) + GLYPH_PAD + ITEM_WIDTH );
		CI.fTop    = static_cast<eg_real>(Row*(ITEM_HEIGHT+GLYPH_PAD*2) + GLYPH_PAD );
		CI.fBottom = static_cast<eg_real>(Row*(ITEM_HEIGHT+GLYPH_PAD*2) + GLYPH_PAD + ITEM_HEIGHT );

		CI.fLeft /= TX_WIDTH;
		CI.fRight /= TX_WIDTH;
		CI.fTop /= TX_HEIGHT;
		CI.fBottom /= TX_HEIGHT;

		const eg_real DrawScale = 1.f/64.f;

		CI.Dims    = eg_vec2( 63.f*DrawScale , 63.f*DrawScale );
		CI.Offset  = eg_vec2( 0.f*DrawScale , 64.f*DrawScale );
		CI.Advance = eg_vec2( 64.f*DrawScale , 0.f*DrawScale );
	}
}

void EGGlyphFont::Deinit()
{
	EGRenderer::Get().DestroyMaterial( m_txFont );
	m_txFont = EGV_MATERIAL_NULL;

	EGDataLibrary<egGlphyFontCharInfo>::Deinit();
}

eg_loc_char EGGlyphFont::GlyphNameToChar( eg_string_crc Name ) const
{
	for( eg_size_t i=0; i<m_Infos.Len(); i++ )
	{
		if( m_Infos[i].Button == Name )
		{
			return eg_loc_char(i+1);
		}
	}

	return '\0';
}

void EGGlyphFont::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& AttGet )
{
	EGDataLibrary<egGlphyFontCharInfo>::OnTag( Tag , AttGet );

	if( Tag == "glyphs" )
	{
		m_GlyphFile = AttGet.GetString( "texture" );
		m_GlyphFile.MakeThisFilenameRelativeTo( GetXmlFilename() );
		eg_int Size[2];
		AttGet.GetIntArray( "size" , Size , countof(Size) , false );
		m_nTextureWidth = Size[0];
		m_nTextureHeight = Size[1];
	}
}

void EGGlyphFont::PoplateItemFromTag( egGlphyFontCharInfo& Info, const EGXmlAttrGetter& AttGet ) const
{
	Info.Index = AttGet.GetUInt( "id" );
	Info.Button = eg_string_crc(AttGet.GetString( "button" ));
}
