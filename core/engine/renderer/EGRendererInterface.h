/******************************************************************************
IRendererInterface - An interface for the renderer. With rendering done
primarily through display lists, we can have a pure virtual interface for the
renderer, with almost no performance hit.

(c) 2015 Beem Software
******************************************************************************/

#pragma once

#include "EGRendererTypes.h"

class EGVDynamicBuffer;

class EGRendererInterface : public EGObject
{

	EG_ABSTRACT_CLASS_BODY( EGRendererInterface , EGObject )

public:

	virtual eg_bool InitRenderer( void* Context ) = 0;

	//////////////////////////////
	// Render Thread Functionality
	//////////////////////////////
	virtual void Update_RenderThread( eg_real DeltaTime ) = 0;

	////////////////////////////////
	// Reinitialization (Any Thread)
	////////////////////////////////
	virtual void Reset() = 0;
	virtual void SetDrawSplash( eg_bool DrawSplash ) = 0;
	virtual egRendererSpecs GetSpecs()const = 0;
	virtual void OnToolsWindowResized( eg_uint Width , eg_uint Height ){ unused( Width, Height ); }

	/////////////////////////////////////
	// Device State Querying (Any Thread)
	/////////////////////////////////////
	virtual eg_uint GetViewWidth()const = 0;
	virtual eg_uint GetViewHeight()const = 0;
	virtual eg_real GetAspectRatio()const = 0; //Width as a percentage of height, height is 1.0f.
	virtual eg_bool IsWindowed()const = 0;
	virtual void    GetResolutions( EGArray<eg_ivec2>& ResOut )const = 0;

	////////////////////////
	// Drawing (Main Thread)
	////////////////////////
	virtual class EGDisplayList* BeginFrame_MainThread() = 0;
	virtual void EndFrame_MainThread( class EGDisplayList* DisplayList ) = 0;

	///////////////////////////////
	// Asset Creation (Main Thread)
	///////////////////////////////
	virtual egv_material CreateMaterial( const EGMaterialDef* pMtrlDef , eg_cpstr SharedId ) = 0;
	virtual void         ReleaseMaterial(egv_material mtrl) = 0;
	virtual egv_vbuffer  CreateVB( eg_string_crc VertexType , eg_uint nNumVerts , const void* pVerts ) = 0;
	virtual void         ReleaseVB(egv_vbuffer buffer) = 0;
	virtual egv_ibuffer  CreateIB(eg_uint nNumInds, const egv_index* pIndices) = 0;
	virtual void         ReleaseIB(egv_ibuffer buffer) = 0;
	virtual egv_rtarget  CreateRenderTarget( eg_uint Width , eg_uint Height ) = 0;
	virtual void         DestroyRenderTarget( egv_rtarget RenderTarget ) = 0;
	virtual EGVDynamicBuffer* CreateDynamicBuffer( eg_string_crc VertexType , eg_uint VertexBufferSize , eg_uint IndexBufferSize ) = 0;
	virtual void DestroyDynamicBuffer( EGVDynamicBuffer* DynamicBuffer ) = 0;
	virtual void UpdateDynamicBuffer( EGVDynamicBuffer* DynamicBuffer ) = 0;
};
