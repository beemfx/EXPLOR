// (c) 2017 Beem Media

#include <base.hlsli>

////////////////////////////////////////////////////////////////////////////////
//
// Font - Uses only alhpa chanel from texture, and color from vertex. In this
// way a font can have an alpha chanel for aliasing and be any color.
//
////////////////////////////////////////////////////////////////////////////////

egv_vert_simple VS_Font(egv_vert_simple IN) __export( vs , "default_font" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP); //Final position of vertex.
	return OUT;
}

float4 PS_Font(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "default_font" )
{
	return float4(IN.Color0.rgb, IN.Color0.a*EG_Sample( 0 , IN.Tex0 ).a);
}

egv_vert_simple VS_FontPaletteColor(egv_vert_simple IN) __export( vs , "default_font_palette_color" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP); //Final position of vertex.
	OUT.Color0 = EG_GetPalette( 0 )*IN.Color0.a;
	return OUT;
}

float4 PS_FontPaletteColor(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "default_font_palette_color" )
{
	return float4(IN.Color0.rgb, IN.Color0.a*EG_Sample( 0 , IN.Tex0 ).a);
}

egv_vert_simple VS_FontGlyph(egv_vert_simple IN) __export( vs , "default_font_glyph" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP); //Final position of vertex.
	return OUT;
}

float4 PS_FontGlyph(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "default_font_glyph" )
{
	return EG_Sample( 0 , IN.Tex0 )*IN.Color0;
}

egv_vert_simple VS_FontGlyphPaletteColor(egv_vert_simple IN) __export( vs , "default_font_glyph_palette_color" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP); //Final position of vertex.
	OUT.Color0 = EG_GetPalette( 0 )*IN.Color0.a;
	return OUT;
}

float4 PS_FontGlyphPaletteColor(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "default_font_glyph_palette_color" )
{
	return float4(IN.Color0.rgb , IN.Color0.a * EG_Sample( 0 , IN.Tex0 ).a);
}