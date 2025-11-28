#pragma once

#include "EGRendererTypes.h"
#include "EGXMLBase.h"
#include "EGDataLibrary.h"

class EGFontBase
{
public:
	struct egTextFormat
	{
		eg_aabb  aabbWindow;
		eg_real  LineHeight;
		eg_color Color;
		eg_flags Flags;
	};

	struct egDrawResult
	{
		eg_size_t CharsDrawn;
		eg_real   XOffset;
	};


public:
	EG_DECLARE_FLAG( F_DRAW_I , 0 );
	EG_DECLARE_FLAG( F_DRAW_SUP , 1 );
	EG_DECLARE_FLAG( F_DRAW_SUB , 2 );
	egDrawResult AppendText( const eg_loc_char LocText[] , eg_size_t LocTextLength , eg_real x , eg_real y , const egTextFormat& Format , egv_vert_simple* VertsOut , eg_size_t VertsOutSize );
	egv_material GetMaterial()const{ return m_txFont; }

	//Flags.
	EG_DECLARE_FLAG(F_VALID   , 0);
	EG_DECLARE_FLAG(F_XYMODE  , 1);
	EG_DECLARE_FLAG(F_BOXMODE , 2);

	struct egCharInfo
	{
		eg_loc_char LocChar;
		eg_real fLeft;
		eg_real fRight;
		eg_real fTop;
		eg_real fBottom;

		eg_vec2 Dims;
		eg_vec2 Offset;
		eg_vec2 Advance;
	};

public:
	EGFontBase( eg_loc_char FirstChar , eg_loc_char LastChar , eg_loc_char UnkChar )
	: m_flags(0)
	, m_txFont(EGV_MATERIAL_NULL)
	, m_BaseYOffset(0)
	, m_CharScale(0)
	, m_CI( CT_Clear )
	, m_FirstChar( FirstChar )
	, m_LastChar( LastChar )
	, m_UnkChar( UnkChar )
	, m_NextAdditionalChar( 0 )
	{

	}

protected:
	egv_material m_txFont;
	eg_real      m_CharScale;
	eg_real      m_BaseYOffset;
	eg_flags     m_flags;
	eg_string    m_sTex;
	eg_loc_char  m_FirstChar;
	eg_loc_char  m_LastChar;
	eg_loc_char  m_UnkChar;
	eg_int       m_NextAdditionalChar;
	//Information for all of the characters.
	EGFixedArray<egCharInfo,160> m_CI;
};

class EGFont : public EGFontBase , private IXmlBase
{
private:
	friend class EGFontMgr;
	EGFont(eg_cpstr strFont);
	~EGFont();

private:
	//Constants.
	static const eg_loc_char FIRST_CHAR = ' ';
	static const eg_loc_char LAST_CHAR  = '~';
	static const eg_loc_char UNK_CHAR = 'X';

/******************
*** XML Loading ***
******************/
private:
	eg_uint m_nTextureWidth;
	eg_uint m_nTextureHeight;
	eg_uint m_FontSize;
	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& atts) override final;
	virtual eg_cpstr XMLObjName()const{ return ("Font"); }
};

//
// EGInputMapFont
//
struct egGlphyFontCharInfo
{
	eg_string_crc Id;
	eg_uint       Index;
	eg_string_crc Button;

	egGlphyFontCharInfo()
	: Id( CT_Clear )
	, Index( 0 )
	, Button( CT_Clear )
	{

	}

	egGlphyFontCharInfo( eg_ctor_t Ct )
	: Id( Ct )
	, Button( Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			Index = 0;
		}
	}
};

class EGGlyphFont: public EGFontBase , public EGDataLibrary<egGlphyFontCharInfo>
{
public:
	EGGlyphFont(): EGFontBase( 0 , 0 , 0 ), m_nTextureWidth(0) , m_nTextureHeight(0){ }

	void Init( eg_cpstr Filename );
	void Deinit();

	eg_loc_char GlyphNameToChar( eg_string_crc Name ) const;

private:
	eg_string m_GlyphFile;
	eg_uint   m_nTextureWidth;
	eg_uint   m_nTextureHeight;
private:
	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& atts) override final;
	virtual void PoplateItemFromTag( egGlphyFontCharInfo& Info , const EGXmlAttrGetter& AttGet ) const override final;
	virtual eg_cpstr GetLibraryName() const override final { return "EGInputMapFont"; }
	virtual eg_cpstr GetItemTag() const override final { return "glyph"; }
};

///////////////////////////////////////////////////////////////////////////////
//
// Font Manager
//
///////////////////////////////////////////////////////////////////////////////

#include "EGStringMap.h"

class EGFontMgr
{
public:
	static const eg_uint MAX_FONTS=10;
public:
	static EGFontMgr* Get(){ return &s_FontMgr; }
public:
	EGFontMgr()
	: m_FontMap( CT_Clear )
	, m_GlyphFont()
	{
	}

	void Init();
	void Deinit();

	EGFontBase* GetFont( eg_string_crc FontId ) const;
	EGGlyphFont* GetGlyphFont();
	void QueryFonts( EGArray<eg_string_crc>& Out );
	void SetDefaultFont( eg_string_crc FontId );
private:
	void Init_AddFont( eg_cpstr Filename );
private:
	EGStringCrcMapFixedSize<EGFont*, MAX_FONTS> m_FontMap;
	EGGlyphFont m_GlyphFont;
private:
	static void EnumerateFontFiles( eg_cpstr16 Filename , void* Data );
	static EGFontMgr s_FontMgr;
};