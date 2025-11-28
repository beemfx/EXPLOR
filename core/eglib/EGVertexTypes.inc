
#if defined( IN_GAME_DECL )
	#define STRUCT_BEGIN( name ) struct name {
	#define STRUCT_END };
	#define DECL_VEC4( var , h ) eg_vec4 var;
	#define DECL_VEC2( var , h ) eg_vec2 var;
	#define DECL_REAL( var , h) eg_real var;
	#define DECL_MATRIX( var , h ) eg_matrix var;
	#define DECL_COLOR( var , h ) eg_color var;
	#define DECL_NCOLOR( var , h ) eg_color32 var;
	#define DECL_UINT( var , h ) eg_uint var;
#elif defined( SHADER_BUILD_DECL3 ) || defined( SHADER_BUILD_DECL5 )
	#define STRUCT_BEGIN( name ) Line = EGString_Format( "struct %s\r\n{\r\n" , #name ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define STRUCT_END Line = EGString_Format( "};\r\n\r\n" ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_VEC4( var , h ) Line = EGString_Format( "\tfloat4 %s : %s;\r\n" , #var , #h ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_VEC2( var , h ) Line = EGString_Format( "\tfloat2 %s : %s;\r\n" , #var , #h ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_REAL( var , h ) Line = EGString_Format( "\tfloat1 %s : %s;\r\n" , #var , #h ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_MATRIX( var , h ) Line = EGString_Format( "\tfloat4x4 %s : %s;\r\n" , #var , #h ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_COLOR( var , h ) Line = EGString_Format( "\tfloat4 %s : %s;\r\n" , #var , #h ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_NCOLOR( var , h ) Line = EGString_Format( "\tuint %s : %s;\r\n" , #var , #h ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_UINT( var , h ) Line = EGString_Format( "\tuint %s : %s;\r\n" , #var , #h ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
#else
	#pragma __ERROR__( "Invalid declaration type specified!" )

	#define STRUCT_BEGIN( name )
	#define STRUCT_END
	#define DECL_VEC4( var , h )
	#define DECL_VEC2( var , h )
	#define DECL_REAL( var , h)
	#define DECL_MATRIX( var , h )
	#define DECL_COLOR( var , h )
	#define DECL_NCOLOR( var , h )
	#define DECL_UINT( var , h )
#endif


STRUCT_BEGIN( egv_vert_mesh )
	DECL_VEC4( Pos , EG_POSITION )
	DECL_VEC4( Norm , NORMAL )
	DECL_VEC4( Tan , TANGENT )
	DECL_VEC2( Tex0 , TEXCOORD0 )
	DECL_VEC2( Tex1 , TEXCOORD1 )
	DECL_COLOR( Color0 , COLOR0 )
	DECL_UINT( Bone0 , BLENDINDICES0 )
	DECL_REAL( Weight0 , BLENDWEIGHT0 )
	DECL_UINT( Bone1 , BLENDINDICES1 )
	DECL_REAL( Weight1 , BLENDWEIGHT1 )
STRUCT_END

STRUCT_BEGIN( egv_vert_simple )
	DECL_VEC4( Pos , EG_POSITION )
	DECL_VEC2( Tex0 , TEXCOORD0 )
	DECL_COLOR( Color0 , COLOR0 )
STRUCT_END

STRUCT_BEGIN( egv_vert_terrain )
	DECL_VEC4( Pos , EG_POSITION )
	DECL_VEC4( Norm , NORMAL )
	DECL_VEC4( Tan , TANGENT )
	DECL_VEC2( Tex0 , TEXCOORD0 )
	DECL_VEC2( Tex1 , TEXCOORD1 )
STRUCT_END


#undef STRUCT_BEGIN
#undef STRUCT_END
#undef DECL_VEC4
#undef DECL_VEC2
#undef DECL_REAL
#undef DECL_MATRIX
#undef DECL_COLOR
#undef DECL_NCOLOR
#undef DECL_UINT
