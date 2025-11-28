
#if defined( IN_GAME_DECL )
	#define STRUCT_BEGIN( name ) struct name {
	#define STRUCT_END };
	#define DECL_STRUCT( type , var ) struct var;
	#define DECL_VEC4( var ) eg_vec4 var;
	#define DECL_REAL( var ) eg_real var; eg_real var_##PADDING[3];
	#define DECL_MATRIX( var ) eg_matrix var;
	#define DECL_COLOR( var ) eg_color var;
	#define DECL_NCOLOR( var ) eg_color32 var;
#elif defined( SHADER_BUILD_DECL3 ) || defined( SHADER_BUILD_DECL5 )
	#define STRUCT_BEGIN( name ) Line = EGString_Format( "struct %s\r\n{\r\n" , #name ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define STRUCT_END Line = EGString_Format( "};\r\n\r\n" ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_STRUCT( type , var ) Line = EGString_Format( "\t%s %s;\r\n" , #type , #var ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_VEC4( var ) Line = EGString_Format( "\tfloat4 %s;\r\n" , #var ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#if defined( SHADER_BUILD_DECL3 )
		#define DECL_REAL( var ) Line = EGString_Format( "\tfloat1 %s; float1 %s_PADDING[3];\r\n" , #var , #var ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#elif defined( SHADER_BUILD_DECL5 ) // Shader Compiler 5 always adds padding, so we don't add it ourselves.
		#define DECL_REAL( var ) Line = EGString_Format( "\tfloat1 %s;\r\n" , #var ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#endif
	#define DECL_MATRIX( var ) Line = EGString_Format( "\tfloat4x4 %s;\r\n" , #var ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_COLOR( var ) Line = EGString_Format( "\tfloat4 %s;\r\n" , #var ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
	#define DECL_NCOLOR( var ) Line = EGString_Format( "\tuint %s;\r\n" , #var ); fwrite( Line.String() , sizeof(eg_char) , Line.Len() , File );
#else
	#pragma __ERROR__( "Invalid declaration type specified!" )

	#define STRUCT_BEGIN( name )
	#define STRUCT_END
	#define DECL_STRUCT( type , var )
	#define DECL_VEC4( var )
	#define DECL_REAL( var )
	#define DECL_MATRIX( var )
	#define DECL_COLOR( var )
	#define DECL_NCOLOR( var )
#endif



//Light struct (all lights are treated as direction lights, the point light
//component is computing in the game engine).
STRUCT_BEGIN( EGLight )
	DECL_VEC4( Dir )
	DECL_VEC4( Pos )
	DECL_COLOR( Color )
	DECL_VEC4( OnOff_RangeSq_Intensity_FallofRadiusSq ) //x - on or off , y - range

	#if defined( IN_GAME_DECL )

	EGLight() = default;

	EGLight( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			Dir = eg_vec4( 0.f , 1.f , 0.f , 0.f);
			Pos = eg_vec4( 0.f , 0.f , 0.f , 1.f );
			Color = eg_color( 1.f , 1.f , 1.f , 1.f );
			OnOff_RangeSq_Intensity_FallofRadiusSq = eg_vec4( 0.f , 0.f , 0.f , 0.f );
		}
	}

	void SetEnabled( eg_bool bEnabled ){ OnOff_RangeSq_Intensity_FallofRadiusSq.x = bEnabled ? 1.f : 0.f; }
	eg_bool IsEnabled()const{ return OnOff_RangeSq_Intensity_FallofRadiusSq.x > 0; }

	void SetRangeSq( eg_real RangeSq , eg_real FalloffRadiusSq ){ OnOff_RangeSq_Intensity_FallofRadiusSq.y = RangeSq; OnOff_RangeSq_Intensity_FallofRadiusSq.w = FalloffRadiusSq; }
	eg_real GetRangeSq() const { return OnOff_RangeSq_Intensity_FallofRadiusSq.y; }
	eg_real GetFalloffRadiusSq() const { return OnOff_RangeSq_Intensity_FallofRadiusSq.w; }

	void SetIntensity( eg_real Intensity ){ OnOff_RangeSq_Intensity_FallofRadiusSq.z = Intensity; }
	eg_real GetIntensity() const { return OnOff_RangeSq_Intensity_FallofRadiusSq.z; }

	#endif

STRUCT_END

//Material struct. An imitation of D3D's FFP material.
STRUCT_BEGIN( EGMaterial )
	DECL_COLOR( Diffuse )  // Diffuse color RGBA
	DECL_COLOR( Ambient )  // Ambient color RGB
	DECL_COLOR( Specular ) // Specular 'shininess'
	DECL_COLOR( Emissive ) // Emissive color RGB
	DECL_REAL( Power )     // Sharpness if specular highlight
STRUCT_END




#undef STRUCT_BEGIN
#undef STRUCT_END
#undef DECL_STRUCT
#undef DECL_VEC4
#undef DECL_REAL
#undef DECL_MATRIX
#undef DECL_COLOR
#undef DECL_NCOLOR
