// (c) 2016 Beem Media

#include "EGMake.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#include "EGFileData.h"
#include "EGTGAWriter.h"
#include "EGWindowsAPI.h"
#include "EGLibFile.h"
#include "EGExtProcess.h"
#include "EGPath2.h"

///////////////////////////////////////////////////////////////////////////////
static const eg_uint FIRST_CHAR = ' ';
static const eg_uint LAST_CHAR  = '~';
static const eg_char16 ADDITIONAL_CHARS[] = { /*(c)*/L'\xA9' };
static const eg_uint NUM_CHARS = LAST_CHAR - FIRST_CHAR + 1 + countof(ADDITIONAL_CHARS);
static const eg_uint TEXTURE_CHAR_PADDING=4;
#if defined(__DEBUG__)
static const eg_bool DEBUG_LEAVE_TEMP_FILES = false;
#else
static const eg_bool DEBUG_LEAVE_TEMP_FILES = false;
#endif


static struct SCharInfo
{
	eg_char16 c;
	eg_uint Left;
	eg_uint Top;
	eg_uint Width;
	eg_uint Height;
	eg_real XAdvance;
	eg_real YAdvance;
	eg_int  XOffset;
	eg_int  YOffset;
}
g_CI[NUM_CHARS];

eg_string_big g_strFontName("Unknown");

static eg_bool g_bHasWarnedAboutTooBig = false;

static void EGR_Font_WarnTooBig()
{
	if( !g_bHasWarnedAboutTooBig )
	{
		g_bHasWarnedAboutTooBig = true;

		EGLogf( eg_log_t::Warning , "Warning: Font is too big for this texture, some characters may not render correctly." );
	}
}

static void EGR_Font_DrawBox( EGTgaWriter& TGAFile , eg_uint x , eg_uint y , eg_uint width , eg_uint height )
{
	EGTgaWriter::egPixel WhitePixel;
	WhitePixel.a = 255;
	WhitePixel.r = 255;
	WhitePixel.g = 255;
	WhitePixel.b = 255;

	// Top:
	for( eg_uint d=0; d<width; d++ )
	{
		TGAFile.WritePixel( x+d , y , WhitePixel );
	}

	// Bottom
	for( eg_uint d=0; d<width; d++ )
	{
		TGAFile.WritePixel( x+d , y+height , WhitePixel );
	}

	// Left
	for( eg_uint d=0; d<height; d++ )
	{
		TGAFile.WritePixel( x , y+d , WhitePixel );
	}

	for( eg_uint d=0; d<height; d++ )
	{
		TGAFile.WritePixel( x+width , y+d , WhitePixel );
	}
}

static void EGR_Font_GenerateTexture_GenerateChar(EGTgaWriter& TGAFile, ::FT_Face& Face, eg_uint& x, eg_uint& y, eg_char16 c, eg_uint CharHeight, eg_int& MaxHeightThisRow, eg_int& TotalHeight)
{
	unused(CharHeight);

	eg_uint nGlyphI = ::FT_Get_Char_Index(Face, c);
	::FT_Load_Glyph(Face, nGlyphI, FT_LOAD_DEFAULT);
	//Insure that a bitmap exists
	::FT_Render_Glyph(Face->glyph, FT_RENDER_MODE_NORMAL);
	//This Reference Will Make Accessing The Bitmap Easier.
	const FT_Bitmap& bm=Face->glyph->bitmap;

	auto SetImageBox = [&bm,&x,&y]( eg_recti& rcBox )
	{
		rcBox.left   = x;
		rcBox.right  = rcBox.left + bm.width + TEXTURE_CHAR_PADDING*2;
		rcBox.top    = y;
		rcBox.bottom = rcBox.top + bm.rows + TEXTURE_CHAR_PADDING*2;
	};

	eg_recti rcImageBox;
	SetImageBox( rcImageBox );

	//First check to see if we need to go to the next row.
	if(x + rcImageBox.GetWidth() >= (TGAFile.GetWidth()))
	{
		x = 0;
		y += MaxHeightThisRow;
		TotalHeight += MaxHeightThisRow;
		MaxHeightThisRow = 0;

		SetImageBox( rcImageBox );
	}

	MaxHeightThisRow = EG_Max( MaxHeightThisRow , rcImageBox.GetHeight() );

	if( rcImageBox.bottom > EG_To<eg_int>(TGAFile.GetHeight()) )
	{
		EGR_Font_WarnTooBig();
	}

	EGR_Font_DrawBox( TGAFile , rcImageBox.left , rcImageBox.top , rcImageBox.GetWidth() , rcImageBox.GetHeight() );

	//Now we draw the bitmap to the render target.	
	{	
		eg_uint BmStartX = rcImageBox.left + TEXTURE_CHAR_PADDING;
		eg_uint BmStartY = rcImageBox.top + TEXTURE_CHAR_PADDING;

		for(eg_uint bmx=0; bmx<bm.width; bmx++)
		{
			for(eg_uint bmy=0; bmy<bm.rows; bmy++)
			{
				eg_byte Intensity = bm.buffer[bmx + bmy*bm.pitch];
				EGTgaWriter::egPixel p;
				p.a = Intensity;
				p.r = Intensity;
				p.g = Intensity;
				p.b = Intensity;
				TGAFile.WritePixel( bmx + BmStartX , bmy + BmStartY , p );
			}
		}
	}

	eg_int CharIndex = 0;
	if( EG_IsBetween<eg_char16>( c , FIRST_CHAR , LAST_CHAR ) )
	{
		CharIndex = c - FIRST_CHAR;
	}
	else
	{
		eg_int OffsetInAdditional = 0;
		for( eg_int i=0; i<countof(ADDITIONAL_CHARS); i++ )
		{
			if( ADDITIONAL_CHARS[i] == c )
			{
				OffsetInAdditional = i;
			}
		}
		CharIndex = LAST_CHAR - FIRST_CHAR + 1 + OffsetInAdditional;
	}

	//We now setup the texture coordinates for the character.
	SCharInfo& CI = g_CI[CharIndex];
	CI.c = c;
	CI.Left = rcImageBox.left+TEXTURE_CHAR_PADDING;
	CI.Top  = rcImageBox.top+TEXTURE_CHAR_PADDING;
	CI.Width = rcImageBox.GetWidth()-TEXTURE_CHAR_PADDING*2;
	CI.Height = rcImageBox.GetHeight()-TEXTURE_CHAR_PADDING*2;

	// Not sure why advance needs to be divided by 64 but it seems to be the case regardless of the size of the characters.
	CI.XAdvance = Face->glyph->advance.x / EG_To<eg_real>(64);
	CI.YAdvance = Face->glyph->advance.y / EG_To<eg_real>(64);
	CI.XOffset = Face->glyph->bitmap_left;
	CI.YOffset = Face->glyph->bitmap_top;

	//We now update the x.
	x += rcImageBox.GetWidth();
}

static void EGR_Font_GenerateTexture_GenerateCharacters( EGTgaWriter& TGAFile , FT_Face& Face , eg_uint CharHeight , eg_int& TotalHeight )
{
	::FT_Set_Pixel_Sizes( Face , 0 , CharHeight );

	eg_uint y=0;
	eg_uint x=0;
	eg_int MaxHeightThisRow = 0;
	for(eg_char c = FIRST_CHAR; c<= LAST_CHAR; c++)
	{
		EGR_Font_GenerateTexture_GenerateChar(TGAFile, Face, x, y, c, CharHeight, MaxHeightThisRow, TotalHeight);
	}
	for( eg_char16 c : ADDITIONAL_CHARS )
	{
		EGR_Font_GenerateTexture_GenerateChar(TGAFile, Face, x, y, c, CharHeight, MaxHeightThisRow, TotalHeight);
	}

	TotalHeight += MaxHeightThisRow;
}

struct egGenerateTextureRes
{
	eg_bool Success;
	eg_uint FontSize;
};
static egGenerateTextureRes EGR_Font_GenerateTexture(EGTgaWriter& TGAFile, EGFileData& FontFile)
{
	egGenerateTextureRes ResOut = { false , 0 };

	::FT_Library FTLib = 0;
	eg_uint nErr = 0;
	//We now attempt to open the library.
	nErr = ::FT_Init_FreeType(&FTLib);
	if(!nErr)
	{
		//Now attempt to create the font face.
		::FT_Face Face = 0;
		nErr = ::FT_New_Memory_Face(
			FTLib, 
			FontFile.GetDataAs<const eg_byte>(), EG_To<FT_Long>(FontFile.GetSize()), 
			0, 
			&Face);

		if(!nErr)
		{
			g_strFontName = Face->family_name;
			g_strFontName += " ";
			g_strFontName += Face->style_name;

			//Set the font size.
			//::FT_Set_Char_Size(Face, 0, FONT_SIZE, RT_SIZE, RT_SIZE);

			const eg_uint CharHeight = TGAFile.GetHeight()/16;
			ResOut.FontSize = CharHeight;
			eg_int TotalHeight = 0;
			EGR_Font_GenerateTexture_GenerateCharacters( TGAFile , Face , CharHeight , TotalHeight );

			::FT_Done_Face(Face);
			Face = 0;
		}
		else
		{
			::EGLogf(eg_log_t::Error, ("egresource.font Error: Could not load font, possibly not a valid file."));
		}
	}
	else
	{
		//For some reason the library could not be created.
		::EGLogf(eg_log_t::Error, ("egresource.font Error: Could not create the FreeType library."));
		ResOut.Success = false;
		ResOut.FontSize = 0;
		return ResOut;
	}



	::FT_Done_FreeType(FTLib);
	FTLib = 0;
	ResOut.Success = (0 == nErr);
	return ResOut;
}

static eg_bool EGR_Font_TGAToDDS( eg_cpstr strOut , eg_uint MipLevels )
{
	// texconv doesn't like for the directory to end with a \ since it does wierd things with the quotes.
	eg_d_string OutputDir = *EGPath2_BreakPath( EGMake_GetOutputPath(FINFO_DIR) ).GetDirectory( '\\' );
	if( OutputDir.Len() > 0 && OutputDir[OutputDir.Len()-1] == '\\' )
	{
		OutputDir.ClampEnd( 1 );
	}

	// In the past we had -pow2 option but it tends to downscale the image if it's not already pow2 so we took it out (modern video cards don't seem to care).
	eg_d_string InputPath = *EGPath2_CleanPath( strOut , '\\' );

	//We want an alpha only.

	eg_d_string strCmd = EGSFormat8( "texconv.exe -nologo -srgb -y -fl 9.3 -ft DDS -f A8_UNORM -m {0} -if CUBIC_DITHER -o \"{1}\" \"{2}\"", MipLevels , *OutputDir , *InputPath );
	eg_bool bRes = EGExtProcess_Run( *strCmd , nullptr );
	if(!bRes)EGLogf(eg_log_t::Error, ("egresource.font Error: Couldn't create DDS from TGA."));

	//Get rid of TGA.
	if( !DEBUG_LEAVE_TEMP_FILES )
	{
		bRes = bRes && ::DeleteFileA(strOut);
		if(!bRes)EGLogf(eg_log_t::Error, ("egresource.font Error: Couldn't delete temporary TGA."));
	}

	return bRes;
}

eg_bool EGMakeRes_efont()
{
	//Load up the font and generate the texture:
	EGFileData FontFile( eg_file_data_init_t::HasOwnMemory );
	eg_bool bFontOpened = EGLibFile_OpenFile( EGString_ToWide(EGMake_GetInputPath()) , eg_lib_file_t::OS , FontFile );
	if( !bFontOpened )
	{
		return false;
	}

	struct egTextureFile
	{
		const eg_uint Width;
		const eg_uint Height;
		EGTgaWriter*  TGAWriter;
		egGenerateTextureRes Res;

		~egTextureFile()
		{
			if( TGAWriter )
			{
				delete TGAWriter;
			}
		}
	};

	static const eg_int MipLevelsToGenerate = 2;

	egTextureFile TextureCreator = { 512 , 1024 , nullptr , { false , 0 } };
	
	{
		TextureCreator.TGAWriter = new EGTgaWriter(TextureCreator.Width,TextureCreator.Height);
		if( TextureCreator.TGAWriter )
		{
			EGTgaWriter::egPixel TransparentBlack = eg_color32(0,0,0,0);
			TextureCreator.TGAWriter->Clear(TransparentBlack);
			// Generate the font texture
			TextureCreator.Res = EGR_Font_GenerateTexture( *TextureCreator.TGAWriter , FontFile );
			if( TextureCreator.Res.Success )
			{
				eg_d_string sFileName( ::EGMake_GetOutputPath(FINFO_NOEXT_FILE) );
				sFileName.Append( ".tga" );
				eg_bool bSucc = EGMake_WriteOutputFile( *sFileName, TextureCreator.TGAWriter->GetTGA() , TextureCreator.TGAWriter->GetSize() );
				bSucc = bSucc && EGR_Font_TGAToDDS( *sFileName , MipLevelsToGenerate );
				if(!bSucc)
				{
					EGLogf( eg_log_t::Error , "Font Builder: Was not able to write TGA." );
					return false;
				}
			}
			else
			{
				EGLogf( eg_log_t::Error , "Font Builder: Was not able to generate font." );
			}
		}
		else
		{
			EGLogf( eg_log_t::Error , "Font Builder: Out of memory?" );
		}
	}

	//Now write the xml:

	EGFileData XmlFile( eg_file_data_init_t::HasOwnMemory );

	#define WRITE { XmlFile.Write(strLine.String(), strLine.Len()*sizeof(eg_char)); }

	eg_cpstr UTF_SIZE = 1==sizeof(eg_char) ? ("8") : ("16");

	eg_string_big strLine;
	strLine = EGString_Format( ("<?xml version=\"1.0\" encoding=\"utf-%s\"?>\n"), UTF_SIZE );
	WRITE

	strLine = EGString_Format( ("<efont name=\"%s\" font_size=\"%u\" min_char=\"0x%04X\" max_char=\"0x%04X\" tex=\"%s\" tex_width=\"%u\" tex_height=\"%u\">\n"), g_strFontName.String() , TextureCreator.Res.FontSize , FIRST_CHAR, LAST_CHAR, ::EGMake_GetOutputPath(FINFO_SHORT_NOEXT), TextureCreator.TGAWriter->GetWidth(), TextureCreator.TGAWriter->GetHeight());
	WRITE


	for(eg_uint i=0; i<NUM_CHARS; i++)
	{
		const SCharInfo& CI = g_CI[i];
		strLine = EGString_Format( ("\t<char u=\"0x%04X\" pos=\"%u %u\" dims=\"%u %u\" advance=\"%g %g\" offset=\"%i %i\"/>\n"), CI.c, CI.Left, CI.Top, CI.Width, CI.Height , CI.XAdvance , CI.YAdvance , CI.XOffset , CI.YOffset );
		WRITE
	}

	strLine = EGString_Format( ("</efont>") );
	WRITE

	#undef WRITE

	eg_bool bSucc = false;

	//Write the font description:
	{
		eg_string_big sFileName = ::EGMake_GetOutputPath(FINFO_NOEXT_FILE);
		sFileName.Append( (".efont") );
		bSucc = EGMake_WriteOutputFile( sFileName, XmlFile.GetDataAs<eg_byte>(), XmlFile.Tell());
	}
	eg_string_big sOutput( ("") );
	sOutput.Append( EGString_Format( ("%s.dds,%s.efont"), ::EGMake_GetOutputPath(FINFO_SHORT_NOEXT), ::EGMake_GetOutputPath(FINFO_SHORT_NOEXT)) );
	::EGMake_SetActualOutputFile(sOutput);

	return bSucc;
}
