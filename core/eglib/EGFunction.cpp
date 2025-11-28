/******************************************************************************
File: Function.h
Purpose: See header.

(c) 2011 Beem Software
******************************************************************************/
#include "EGFunction.h"
#include "EGRandom.h"
#include "EGXMLBase.h"

static_assert( sizeof(eg_bool) == 1 , "Invalid size" );
static_assert( sizeof(eg_byte) == 1 , "Invalid size" );
static_assert( sizeof(eg_uintptr_t) == sizeof(void*) , "Invalid size" );
static_assert( sizeof(eg_intptr_t) == sizeof(void*) , "Invalid size" );
static_assert( sizeof(eg_size_t) == sizeof(void*) , "Invalid size" );

static_assert( sizeof(eg_real32) == 4 , "Invalid size" );
static_assert( sizeof(eg_real64) == 8 , "Invalid size" );

static_assert( sizeof(eg_uint8)  == 1 , "Invalid size" );
static_assert( sizeof(eg_uint16) == 2 , "Invalid size" );
static_assert( sizeof(eg_uint32) == 4 , "Invalid size" );
static_assert( sizeof(eg_uint64) == 8 , "Invalid size" );

static_assert( sizeof(eg_int8)  == 1 , "Invalid size" );
static_assert( sizeof(eg_int16) == 2 , "Invalid size" );
static_assert( sizeof(eg_int32) == 4 , "Invalid size" );
static_assert( sizeof(eg_int64) == 8 , "Invalid size" );

static_assert( sizeof(eg_char8) == 1 , "Invalid size" );
static_assert( sizeof(eg_char16) == 2 , "Invalid size" );

static_assert( sizeof(eg_flags) == 4 , "Invalid size" );

static_assert( (sizeof(egv_vert_mesh)%EG_ALIGNMENT) == 0 , "egv_vert_mesh size must be aligned" );

static_assert( sizeof(eg_mat) == 16*sizeof(eg_real) , "Invalid size" );
static_assert( sizeof(eg_vec4) == 4*sizeof(eg_real) , "Invalid size" );
static_assert( sizeof(eg_vec3) == 3*sizeof(eg_real) , "Invalid size" );
static_assert( sizeof(eg_vec2) == 2*sizeof(eg_real) , "Invalid size" );

egTextNodeTagInfo EGTextNodeTag_FromXmlTag( const EGXmlAttrGetter& AttGet )
{
	egTextNodeTagInfo Out;

	Out.Id = AttGet.GetString( "id" );
	Out.Bone = AttGet.GetUInt( "bone" );
	Out.LocText = AttGet.GetString( "text" , ("Sample Text") );
	Out.LocTextEnus = AttGet.GetString( "text_enus" , "" );
	Out.TextContext = AttGet.GetString( "text_context" , "" );

	//Get the font
	{
		eg_string_big StrFont = AttGet.GetString( "font" );
		StrFont.ConvertToLower();
		Out.Font = StrFont;
	}

	//Get the dimensions width height line_height
	{
		eg_string_big StrDims = AttGet.GetString( "dims" , "1 .1 .1" );
		eg_real Dims[3];
		EGString_GetFloatList( StrDims , StrDims.Len() , &Dims , countof(Dims) );
		Out.NodeWidth = Dims[0];
		Out.NodeHeight = Dims[1];
		Out.LineHeight = Dims[2];
	}

	//Color:
	{
		eg_string_big ColorStr = AttGet.GetString( "color" , "1 1 1 1" );
		eg_color c;
		EGString_GetFloatList( ColorStr , ColorStr.Len() , &c, 4, false );
		Out.Color = c;
	}

	//Drop Shadow Color:
	{
		eg_string_big ColorStr = AttGet.GetString( "drop_shadow_color" , "0 0 0 1" );
		eg_color c;
		EGString_GetFloatList( ColorStr , ColorStr.Len() , &c, 4, false );
		Out.DropShadowColor = c;
	}

	//Drop Shadow Offset:
	{
		eg_string_big Str = AttGet.GetString( "drop_shadow_offset" , "0 0" );
		eg_vec2 v2;
		EGString_GetFloatList( Str , Str.Len() , &v2, 2, false );
		Out.DropShadowOffset = v2;
	}

	Out.bShadowIsBorder = AttGet.GetBool( "drop_border" , false );

	//Text Justification:
	{
		Out.Justify = EGTextNodeTag_StringToJustify(AttGet.GetString( "justify" ));
	}

	return Out;
}

eg_string EGTextNodeTag_JustifyToString( eg_text_align Justify )
{
	eg_string JustifyString = "LEFT";
	switch( Justify  )
	{
	case eg_text_align::CENTER: JustifyString = "CENTER"; break;
	case eg_text_align::LEFT: JustifyString = "LEFT"; break;
	case eg_text_align::RIGHT: JustifyString = "RIGHT"; break;
	}
	return JustifyString;
}

eg_text_align EGTextNodeTag_StringToJustify( eg_cpstr Str )
{
	eg_text_align Out = eg_text_align::LEFT;

	eg_string JustifyString = Str;
	JustifyString.ConvertToUpper();

	switch_crc( eg_string_crc(JustifyString) )
	{
	case_crc( "LEFT" )  : Out = eg_text_align::LEFT; break;
	case_crc( "CENTER" ): Out = eg_text_align::CENTER; break;
	case_crc( "RIGHT" ) : Out = eg_text_align::RIGHT; break;
	}
	return Out;
}

eg_string_big EGTextNodeTag_ToXmlTag( const egTextNodeTagInfo& Info , const eg_transform& Pose , eg_cpstr ExtraTag )
{
	eg_string JustifyString = EGTextNodeTag_JustifyToString( Info.Justify );

	eg_string_big Text = Info.LocText;
	eg_string::ToXmlFriendly( Info.LocText , Text );

	eg_string_big EnusText;
	eg_string::ToXmlFriendly( Info.LocTextEnus , EnusText );

	eg_string_big TextContext;
	eg_string_big Font;

	eg_string_big::ToXmlFriendly( Info.TextContext , TextContext );
	eg_string_big::ToXmlFriendly( Info.Font , Font );

	eg_string_big TagOpen = EGString_Format( "\t<textnode %s text=\"%s\" text_enus=\"%s\" text_context=\"%s\" font=\"%s\" dims=\"%g %g %g\" color=\"%g %g %g %g\" justify=\"%s\" bone=\"%u\" drop_shadow_color=\"%g %g %g %g\" drop_shadow_offset=\"%g %g\" drop_border=\"%s\" >\r\n" 
		, ExtraTag
		, Text.String() , EnusText.String() , TextContext.String() , Font.String()
		, Info.NodeWidth , Info.NodeHeight , Info.LineHeight
		, Info.Color.r , Info.Color.g , Info.Color.b , Info.Color.a
		, JustifyString.String()
		, Info.Bone
		, Info.DropShadowColor.r , Info.DropShadowColor.g , Info.DropShadowColor.b , Info.DropShadowColor.a 
		, Info.DropShadowOffset.x , Info.DropShadowOffset.y
		, (Info.bShadowIsBorder ? "true" : "false") );
	eg_string_big Pos = EGString_Format( "\t\t<position type=\"QUAT_TRANS\">%g %g %g %g  %g %g %g %g</position>\r\n" 
		, Pose.GetRotation().x , Pose.GetRotation().y , Pose.GetRotation().z , Pose.GetRotation().w 
		, Pose.GetPosition().x , Pose.GetPosition().y , Pose.GetPosition().z , Pose.GetPosition().w );
	eg_string_big TagEnd = ( "\t</textnode>\r\n" );


	return EGString_Format( "%s%s%s" , TagOpen.String() , Pos.String() , TagEnd.String() );
}

#if defined( __DEBUG__ )
#include "EGFunction.inl"
#endif

#if defined( __WIN32__ ) || defined( __WIN64__ )
#include "EGWindowsAPI.h"
void EG_ToolsAssert( bool Success , const char* Function , const char* File , unsigned int Line )
{
	if( Success )return;

	eg_string_big Msg = EGString_Format( "Assert failed in %s:\n%s(%u)" , Function , File , Line );
	EGLogf( eg_log_t::Warning , "Warning: %s" , Msg.String() );
	//Only show the message box once.
	static bool HasAsserted = false;
	if( HasAsserted )return;
	MessageBoxA( nullptr , Msg , "EG Tools" , MB_OK|MB_ICONWARNING );

#if defined(__DEBUG__)
	__debugbreak();
#endif

	HasAsserted = true;
}
#endif