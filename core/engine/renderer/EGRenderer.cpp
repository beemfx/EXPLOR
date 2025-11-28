// (c) 2011 Beem Media

#include "EGRenderer.h"
#include "EGRenderThread.h"
#include "EGDisplayList.h"
#include "EGWorkerThreads.h"
#include "EGTextFormat.h"

class EGDisplayList* MainDisplayList = nullptr;


EGRendererShaderQualityConfigVar EGRenderer_ShaderQuality( "EGRenderer_ShaderQuality" , eg_loc("ShaderQuality","Shader Quality") , SHADER_QUALITY_VERY_HIGH , EGS_F_SYS_SAVED|EGS_F_EDITABLE|EGS_F_NEEDSVRESTART  );
EGRendererScreenResConfigVar EGRenderer_ScreenRes( "EGRenderer_ScreenRes" , eg_loc("ScreenRes","Resolution") , eg_ivec2(0,0) , EGS_F_SYS_SAVED|EGS_F_EDITABLE|EGS_F_NEEDSVRESTART );

EGRenderer EGRenderer::s_Renderer; 

eg_bool EGRenderer::Init( eg_string Driver )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread( true );
	Renderer->Start( Driver );
	Renderer->WaitForInitialization();
	return Renderer->IsInitialized();
}

void EGRenderer::Deinit( void )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread( true );
	Renderer->Stop();
}

void EGRenderer::SetDrawSplash( eg_bool DrawSplash )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	Renderer->SetDrawSplash( DrawSplash );
}

egRendererSpecs EGRenderer::GetSpecs()
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer ? Renderer->GetSpecs()  : egRendererSpecs();
}

void EGRenderer::ResetDisplay( void )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	if( Renderer )	Renderer->ResetDisplay();
}

void EGRenderer::BeginFrame( void )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	assert( nullptr == MainDisplayList );
	MainDisplayList = Renderer->BeginFrame_MainThread();
}

void EGRenderer::EndFrame( void )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	assert( nullptr != MainDisplayList );
	Renderer->EndFrame_MainThread( MainDisplayList );
	MainDisplayList = nullptr;
}

egv_material EGRenderer::CreateMaterial( const EGMaterialDef* Def , eg_cpstr SharedId )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->CreateMaterial( Def , SharedId );
}

egv_material EGRenderer::CreateMaterialFromTextureFile( eg_cpstr strFile )
{
	egv_material mtrl = EGV_MATERIAL_NULL;
	EGMaterialDef MatDef;
	eg_string s(strFile);
	s.CopyTo(MatDef.m_strTex[0], EG_MAX_PATH);
	eg_string SharedId = EGString_Format( "TxOnly(%s)" , strFile );
	return EGRenderer::CreateMaterial( &MatDef , SharedId );
}

void EGRenderer::DestroyMaterial( egv_material Mtrl )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->ReleaseMaterial( Mtrl );
}

egv_vbuffer EGRenderer::CreateVB( eg_string_crc VertexType , eg_uint nNumVerts , const egv_vert_mesh* pVerts )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->CreateVB( VertexType , nNumVerts , pVerts );
}

void EGRenderer::DestroyVB( egv_vbuffer Buffer )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->ReleaseVB( Buffer );
}

egv_ibuffer EGRenderer::CreateIB( eg_uint NumIndexes , const egv_index* Indexes )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->CreateIB( NumIndexes , Indexes );
}

void EGRenderer::DestroyIB( egv_ibuffer Buffer )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->ReleaseIB( Buffer );
}

egv_rtarget EGRenderer::CreateRenderTarget( eg_uint Width , eg_uint Height )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->CreateRenderTarget( Width , Height );
}

void EGRenderer::DestroyRenderTarget( egv_rtarget RenderTarget )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->DestroyRenderTarget( RenderTarget );
}

EGVDynamicBuffer* EGRenderer::CreateDynamicBuffer( eg_string_crc VertexType , eg_uint VertexBufferSize, eg_uint IndexBufferSize )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->CreateDynamicBuffer( VertexType , VertexBufferSize , IndexBufferSize );
}

void EGRenderer::DestroyDynamicBuffer( EGVDynamicBuffer* DynamicBuffer )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->DestroyDynamicBuffer( DynamicBuffer );
}

void EGRenderer::UpdateDynamicBuffer( EGVDynamicBuffer* DynamicBuffer )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->UpdateDynamicBuffer( DynamicBuffer );
}

eg_real EGRenderer::GetAspectRatio( void )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->GetAspect();
}

eg_uint EGRenderer::GetDisplayWidth( void )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->GetViewWidth();
}

eg_uint EGRenderer::GetDisplayHeight( void )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->GetViewHeight();
}

void EGRenderer::GetResolutions(  EGArray<eg_ivec2>& ResOut  )
{
	EGRenderThread* Renderer = EGWorkerThreads_GetRenderThread();
	return Renderer->GetResolutions( ResOut );
}

///////////////////////////////////////////////////////

EGRendererShaderQualityConfigVar::EGRendererShaderQualityConfigVar( eg_cpstr VarName, eg_string_crc DisplayName, RENDERER_SHADER_QUALITY DefaultValue, eg_flags VarFlags )
: EGSettingsVarType<RENDERER_SHADER_QUALITY>( VarName , DisplayName , DefaultValue , VarFlags )
{

}

void EGRendererShaderQualityConfigVar::SetFromString( eg_cpstr NewValue )
{
	RENDERER_SHADER_QUALITY StringToQuality = SHADER_QUALITY_VERY_HIGH;

	static const struct
	{
		RENDERER_SHADER_QUALITY RawValue;
		eg_cpstr StringValue;
	}
	Table[] =
	{
		{ SHADER_QUALITY_VERY_HIGH , "VERY_HIGH" },
		{ SHADER_QUALITY_HIGH , "HIGH" },
		{ SHADER_QUALITY_MEDIUM , "MEDIUM" },
		{ SHADER_QUALITY_LOW , "LOW" },
		{ SHADER_QUALITY_VERY_LOW , "VERY_LOW" },
	};

	for( eg_uint i=0; i<countof(Table); i++ )
	{
		if( EGString_EqualsI( Table[i].StringValue , NewValue ) )
		{
			StringToQuality = Table[i].RawValue;
			break;
		}
	}

	SetValue( StringToQuality );
}

eg_d_string EGRendererShaderQualityConfigVar::ToString() const
{
	eg_d_string Out;
	const RENDERER_SHADER_QUALITY CurrentValue = GetValueThreadSafe();

	static const struct
	{
		RENDERER_SHADER_QUALITY RawValue;
		eg_cpstr StringValue;
	}
	Table[] =
	{
		{ SHADER_QUALITY_VERY_HIGH , "VERY_HIGH" },
		{ SHADER_QUALITY_HIGH , "HIGH" },
		{ SHADER_QUALITY_MEDIUM , "MEDIUM" },
		{ SHADER_QUALITY_LOW , "LOW" },
		{ SHADER_QUALITY_VERY_LOW , "VERY_LOW" },
	};

	for( eg_uint i = 0; i < countof( Table ); i++ )
	{
		if( Table[i].RawValue == CurrentValue )
		{
			Out = Table[i].StringValue;
			break;
		}
	}

	return Out;
}

eg_loc_text EGRendererShaderQualityConfigVar::ToLocText() const
{
	const RENDERER_SHADER_QUALITY CurrentValue = GetValueThreadSafe();

	switch( CurrentValue )
	{
		case SHADER_QUALITY_VERY_HIGH: return eg_loc_text(eg_loc("ShaderQualityVeryHigh","Very High"));
		case SHADER_QUALITY_HIGH: return eg_loc_text(eg_loc("ShaderQualityHigh","High"));
		case SHADER_QUALITY_MEDIUM: return eg_loc_text(eg_loc("ShaderQualityMedium","Medium"));
		case SHADER_QUALITY_LOW: return eg_loc_text(eg_loc("ShaderQualityLow","Low"));
		case SHADER_QUALITY_VERY_LOW: return eg_loc_text(eg_loc("ShaderQualityVeryLow","Very Low"));
		default: break;
	}

	return eg_loc_text(eg_loc("ShaderQualityUnknown","Unknown"));
}

void EGRendererShaderQualityConfigVar::Inc()
{
	const RENDERER_SHADER_QUALITY CurrentValue = GetValueThreadSafe();
	const eg_int32 CurrentValueAsInt = EG_To<eg_int32>(CurrentValue);
	const RENDERER_SHADER_QUALITY NewValue = EG_Clamp( EG_To<RENDERER_SHADER_QUALITY>(CurrentValueAsInt-1) , SHADER_QUALITY_VERY_HIGH , SHADER_QUALITY_VERY_LOW );
	SetValue( NewValue );
}

void EGRendererShaderQualityConfigVar::Dec()
{
	const RENDERER_SHADER_QUALITY CurrentValue = GetValueThreadSafe();
	const eg_int32 CurrentValueAsInt = EG_To<eg_int32>( CurrentValue );
	const RENDERER_SHADER_QUALITY NewValue = EG_Clamp( EG_To<RENDERER_SHADER_QUALITY>(CurrentValueAsInt+1), SHADER_QUALITY_VERY_HIGH , SHADER_QUALITY_VERY_LOW );
	SetValue( NewValue );
}

EGRendererScreenResConfigVar::EGRendererScreenResConfigVar( eg_cpstr VarName, eg_string_crc DisplayName, eg_ivec2 DefaultValue, eg_flags VarFlags )
: EGSettingsVarType<eg_ivec2>( VarName , DisplayName , DefaultValue , VarFlags )
{

}

void EGRendererScreenResConfigVar::SetFromString( eg_cpstr NewValue )
{
	eg_string BaseString( NewValue );
	eg_uint ToIntArray[2] = { 0 , 0 };
	BaseString.ToUIntArray( ToIntArray , countof(ToIntArray) );
	SetValue( eg_ivec2( ToIntArray[0] , ToIntArray[1] ) );
}

eg_d_string EGRendererScreenResConfigVar::ToString() const
{
	const eg_ivec2 ResXY = GetValueThreadSafe();
	return EGString_Format( "%d,%d" , ResXY.x , ResXY.y ).String();
}

eg_loc_text EGRendererScreenResConfigVar::ToLocText() const
{
	const eg_ivec2 ResXY = GetValueThreadSafe();

	return EGFormat( eg_loc("ResFormat","{0:nocomma}x{1:nocomma}") , ResXY.x , ResXY.y );
}

void EGRendererScreenResConfigVar::Inc()
{
	const eg_ivec2 ResXY = GetValueThreadSafe();

	if( m_AvailableResolutions.Len() > 0 )
	{
		eg_int CurrentIndex = 0;
		for( eg_int i=0; i<m_AvailableResolutions.Len(); i++ )
		{
			if( m_AvailableResolutions[i] == ResXY )
			{
				CurrentIndex = i;
				break;
			}
		}

		const eg_int NewIndex = CurrentIndex+1;
		if( m_AvailableResolutions.IsValidIndex( NewIndex ) )
		{
			SetValue( m_AvailableResolutions[NewIndex] );
		}
	}
}

void EGRendererScreenResConfigVar::Dec()
{
	const eg_ivec2 ResXY = GetValueThreadSafe();

	if( m_AvailableResolutions.Len() > 0 )
	{
		eg_int CurrentIndex = 0;
		for( eg_int i = 0; i < m_AvailableResolutions.Len(); i++ )
		{
			if( m_AvailableResolutions[i] == ResXY )
			{
				CurrentIndex = i;
				break;
			}
		}

		const eg_int NewIndex = CurrentIndex-1;
		if( m_AvailableResolutions.IsValidIndex( NewIndex ) )
		{
			SetValue( m_AvailableResolutions[NewIndex] );
		}
	}
}

void EGRendererScreenResConfigVar::InitAvailableResolutions( const EGArray<eg_ivec2>& InRes )
{
	m_AvailableResolutions = InRes;
}

void EGRendererScreenResConfigVar::DeinitAvailableResolutions()
{
	m_AvailableResolutions.Clear( true );
}

eg_int EGRendererScreenResConfigVar::GetSelectedToggle() const
{
	const eg_ivec2 ResXY = GetValueThreadSafe();

	for( eg_int i = 0; i < m_AvailableResolutions.Len(); i++ )
	{
		if( m_AvailableResolutions[i] == ResXY )
		{
			return i;
		}
	}

	return -1;
}

void EGRendererScreenResConfigVar::SetSelectedToggle( eg_int NewValue )
{
	if( m_AvailableResolutions.IsValidIndex( NewValue ) )
	{
		SetValue( m_AvailableResolutions[NewValue] );
	}
	else
	{
		assert( false ); // Bad value
	}
}

void EGRendererScreenResConfigVar::GetToggleValues( EGArray<eg_loc_text>& Out ) const
{
	static const eg_string_crc RES_FORMAT_NORATIO = eg_loc("EGRendererResFormatWithNoRatio","{0:nocomma}x{1:nocomma}");
	static const eg_string_crc RES_FORMAT = eg_loc("EGRendererResFormatWithRatio","{0:nocomma}x{1:nocomma} ({2:nocomma}:{3:nocomma})");

	for( eg_int i=0; i<m_AvailableResolutions.LenAs<eg_int>(); i++ )
	{
		const eg_ivec2& Res = m_AvailableResolutions[i];

		eg_ivec2 ClosestRatio( CT_Clear );

		const eg_real Ratio = EG_To<eg_real>(Res.x)/Res.y;

		if( EG_IsEqualEps( Ratio , 16.f/9.f , EG_SMALL_NUMBER ) )
		{
			ClosestRatio = eg_ivec2(16,9);
		}
		else if( EG_IsEqualEps( Ratio , 16.f/10.f , EG_SMALL_NUMBER ) )
		{
			ClosestRatio = eg_ivec2(16,10);
		}
		else if( EG_IsEqualEps( Ratio , 4.f/3.f , EG_SMALL_NUMBER ) )
		{
			ClosestRatio = eg_ivec2(4,3);
		}

		eg_loc_text Value = EGFormat( ClosestRatio.x != 0 ? RES_FORMAT : RES_FORMAT_NORATIO , Res.x , Res.y , ClosestRatio.x , ClosestRatio.y );
		Out.Append( Value );
	}
}
