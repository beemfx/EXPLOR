#if defined( IN_GAME_DECL )
	#define DECL_MATRIX( var , type ) eg_mat m_##var;
	#define DECL_VECTOR( var , type ) eg_vec4 m_##var;
	#define DECL_IVECTOR( var , type ) eg_ivec4 m_##var;
	#define DECL_2FLOATS( var1 , var2 , type ) eg_real m_##var1; eg_real m_##var2; eg_real F_PAD##var1##var2[2];
	#define DECL_LIGHT_ARRAY( var , count ) EGLight m_##var[count];
	#define DECL_MATERIAL( var ) EGMaterial m_##var;
	#define DECL_MATRIX_ARRAY( var , count ) eg_mat m_##var[count];
	#define BUFFER_START( name , reg )
	#define BUFFER_END
#endif

#if defined( SHADER_BUILD_DECL3 )
	#define DECL_MATRIX( var , type ) Line = EGString_Format( "float4x4 g_"#var" : register( %s , c%u ) : %s;\r\n" , sShaderType , Offset , #type ); Offset+=4; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_VECTOR( var , type ) Line = EGString_Format( "float4 g_"#var" : register( %s , c%u ) : %s;\r\n" , sShaderType , Offset , #type ); Offset+=1; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_IVECTOR( var , type ) Line = EGString_Format( "int4 g_"#var" : register( %s , c%u ) : %s;\r\n" , sShaderType , Offset , #type ); Offset+=1; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_2FLOATS( var1 , var2 , type ) Line = EGString_Format( "float4 g_"#var1#var2" : register( %s , c%u ) : %s;\r\n" , sShaderType , Offset , #type ); Offset+=1; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_LIGHT_ARRAY( var , count ) Line = EGString_Format( "EGLight g_"#var"[%u] : register( %s , c%u );\r\n" , count , sShaderType , Offset ); Offset+=LIGHT_REG_SIZE*(count); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_MATERIAL( var ) Line = EGString_Format( "EGMaterial g_"#var" : register( %s , c%u );\r\n" , sShaderType , Offset ); Offset+=MATERIAL_SIZE; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_MATRIX_ARRAY( var , count ) Line = EGString_Format( "float4x4 g_"#var"[%u] : register( %s , c%u );\r\n" , count , sShaderType , Offset ); Offset+=4*(count); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define BUFFER_START( name , reg )
	#define BUFFER_END

	Offset = 0;
#endif

#if defined( SHADER_BUILD_DECL5 )
	#define DECL_MATRIX( var , type ) Line = EGString_Format( "\tfloat4x4 g_"#var" : packoffset( c%u ) : %s;\r\n" , Offset , #type ); Offset+=4; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_VECTOR( var , type ) Line = EGString_Format( "\tfloat4 g_"#var" : packoffset( c%u ) : %s;\r\n" , Offset , #type ); Offset+=1; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_IVECTOR( var , type ) Line = EGString_Format( "\tint4 g_"#var" : packoffset( c%u ) : %s;\r\n" , Offset , #type ); Offset+=1; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_2FLOATS( var1 , var2 , type ) Line = EGString_Format( "\tfloat4 g_"#var1#var2" : packoffset( c%u ) : %s;\r\n" , Offset , #type ); Offset+=1; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_LIGHT_ARRAY( var , count ) Line = EGString_Format( "\tEGLight g_"#var"[%u] : packoffset( c%u );\r\n" , count , Offset ); Offset+=LIGHT_REG_SIZE*(count); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_MATERIAL( var ) Line = EGString_Format( "\tEGMaterial g_"#var" : packoffset( c%u );\r\n" , Offset ); Offset+=MATERIAL_SIZE; fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_MATRIX_ARRAY( var , count ) Line = EGString_Format( "\tfloat4x4 g_"#var"[%u] : packoffset( c%u );\r\n" , count , Offset ); Offset+=4*(count); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define BUFFER_START( name , reg ) Line = EGString_Format( "cbuffer "#name" : register( b"#reg" )\r\n{\r\n" ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File ); Offset = 0;
	#define BUFFER_END Line = EGString_Format( "}\r\n\r\n" ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );

#endif


#if defined( VS_CONSTS )
/******************************
*** Vertex Shader Registers ***
******************************/

BUFFER_START( VsConsts , 0 )
//We provide all 6 combinations of trasnform matrices so the shader never needs
//to compute one of these explicitely.
	DECL_MATRIX( mW , WORLD )
	DECL_MATRIX( mV , VIEW )
	DECL_MATRIX( mP , PROJECTION )
	DECL_MATRIX( mWVP , WORLDVIEWPROJECTION )
	DECL_MATRIX( mWV , WORLDVIEW )
	DECL_MATRIX( mVP , VIEWPROJECTION )
//Camera position in world space (mesh and map only).
	DECL_VECTOR( v4Camera , CAMERA )
	DECL_VECTOR( AmbientLight , AMBIENT )
//Time of the current object (meshes only).
	DECL_2FLOATS( fTime , fOneMinuteCycleTime , TIME )
	DECL_VECTOR( ScaleVec , SCALE )
//Lights and materials (meshes only).
	DECL_LIGHT_ARRAY( Lights , RENDERER_MAX_LIGHTS )
	DECL_MATERIAL( Material )
//Misc. registers (matches g_ps_reg*) (special cases only).
	DECL_VECTOR( vs_reg0 , REG0 )
	DECL_VECTOR( vs_Palette0 , PALETTE0 )
	DECL_VECTOR( vs_Palette1 , PALETTE1 )
	DECL_VECTOR( vs_GamePalette0 , GAMEPALETTE0 )
	DECL_VECTOR( vs_GamePalette1 , GAMEPALETTE1 )
	DECL_VECTOR( vs_GamePalette2 , GAMEPALETTE2 )
	DECL_IVECTOR( vs_EntInts0 , ENTINTS0 )
BUFFER_END

BUFFER_START( VsBones , 1 )
//Up to 32 bones are supported (meshes only).
	DECL_MATRIX_ARRAY( mBoneM , RENDERER_MAX_BONES+1 )
BUFFER_END
#endif

#if defined( PS_CONSTS )
/******************************************
*** Pixel Shader Registers and Samplers ***
******************************************/
BUFFER_START( PsConsts , 0 )
//Misc. registers (matches g_vs_reg*) (special cases only).
	DECL_VECTOR( ps_reg0 , REG0 )
	DECL_VECTOR( ps_Palette0 , PALETTE0 )
	DECL_VECTOR( ps_Palette1 , PALETTE1 )
	DECL_VECTOR( ps_GamePalette0 , GAMEPALETTE0 )
	DECL_VECTOR( ps_GamePalette1 , GAMEPALETTE1 )
	DECL_VECTOR( ps_GamePalette2 , GAMEPALETTE2 )
	DECL_IVECTOR( ps_EntInts0 , ENTINTS0 )
	DECL_MATRIX( ps_PrevVP , PREVVIEWPROJECTION )
	DECL_MATRIX( ps_InvVP , INVERSEVIEWPROJECTION )
BUFFER_END
#endif


#undef DECL_MATRIX_ARRAY
#undef DECL_MATERIAL
#undef DECL_LIGHT_ARRAY
#undef DECL_2FLOATS
#undef DECL_MATRIX
#undef DECL_VECTOR
#undef DECL_IVECTOR
#undef BUFFER_START
#undef BUFFER_END