// (c) 2011 Beem Media

#pragma once

#include "EGRendererTypes.h"
#include "EGDisplayList.h"
#include "EGSettings2Types.h"

class EGVDynamicBuffer;

extern class EGDisplayList* MainDisplayList;

class EGRenderer
{
private:

	static EGRenderer s_Renderer;

public:

	static EGRenderer& Get() { return s_Renderer; }

public:

	eg_bool Init( eg_string Driver );
	void    Deinit( void );
	void    ResetDisplay( void );
	void    BeginFrame( void );
	void    EndFrame( void );
	void    SetDrawSplash( eg_bool DrawSplash );
	egRendererSpecs GetSpecs();

	egv_material CreateMaterial( const EGMaterialDef* Def , eg_cpstr SharedId );
	egv_material CreateMaterialFromTextureFile( eg_cpstr strFile );
	void         DestroyMaterial( egv_material Mtrl );
	egv_vbuffer  CreateVB( eg_string_crc VertexType , eg_uint nNumVerts , const egv_vert_mesh* pVerts );
	void         DestroyVB( egv_vbuffer Buffer );
	egv_ibuffer  CreateIB( eg_uint NumIndexes , const egv_index* Indexes );
	void         DestroyIB( egv_ibuffer Buffer );
	egv_rtarget  CreateRenderTarget( eg_uint Width , eg_uint Height );
	void         DestroyRenderTarget( egv_rtarget RenderTarget );
	EGVDynamicBuffer* CreateDynamicBuffer( eg_string_crc VertexType , eg_uint VertexBufferSize , eg_uint IndexBufferSize );
	void DestroyDynamicBuffer( EGVDynamicBuffer* DynamicBuffer );
	void UpdateDynamicBuffer( EGVDynamicBuffer* DynamicBuffer );

	eg_real GetAspectRatio( void );
	eg_uint GetDisplayWidth( void );
	eg_uint GetDisplayHeight( void );
	void    GetResolutions( EGArray<eg_ivec2>& ResOut );
};

class EGRendererShaderQualityConfigVar : public EGSettingsVarType<RENDERER_SHADER_QUALITY>
{
public:
	EGRendererShaderQualityConfigVar( eg_cpstr VarName, eg_string_crc DisplayName, RENDERER_SHADER_QUALITY DefaultValue, eg_flags VarFlags );

	virtual void SetFromString( eg_cpstr NewValue ) override;
	virtual eg_d_string ToString() const override;
	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;
};

class EGRendererScreenResConfigVar : public EGSettingsVarType<eg_ivec2>
{
private:
	
	EGArray<eg_ivec2> m_AvailableResolutions;

public:
	
	EGRendererScreenResConfigVar( eg_cpstr VarName, eg_string_crc DisplayName, eg_ivec2 DefaultValue, eg_flags VarFlags );

	virtual void SetFromString( eg_cpstr NewValue ) override;
	virtual eg_d_string ToString() const override;
	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;

	void InitAvailableResolutions( const EGArray<eg_ivec2>& InRes );
	void DeinitAvailableResolutions();

	virtual eg_bool IsToggleType() const override { return true; }
	virtual eg_int GetSelectedToggle() const override;
	virtual void SetSelectedToggle( eg_int NewValue ) override;
	virtual void GetToggleValues( EGArray<eg_loc_text>& Out ) const override;
};

extern EGRendererShaderQualityConfigVar EGRenderer_ShaderQuality;
extern EGRendererScreenResConfigVar EGRenderer_ScreenRes;