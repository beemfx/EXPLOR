/******************************************************************************
EGNullR_Renderer - A renderer that doesn't actually draw anything.

(c) 2015 Beem Software
******************************************************************************/
#pragma once

#include "EGRendererTypes.h"
#include "EGRendererInterface.h"
#include "EGDisplayList.h"
#include "EGThread.h"


class EGNullR_Renderer: public EGRendererInterface
{
	EG_CLASS_BODY( EGNullR_Renderer , EGRendererInterface )

	EGNullR_Renderer();
	~EGNullR_Renderer();

	virtual eg_bool InitRenderer( void* Context ) override { unused( Context ); return true; }

	virtual void Reset() override{ }
	virtual egRendererSpecs GetSpecs()const override
	{ 
		egRendererSpecs Out;
		Out.bDoSimpleOnePass = true;
		return Out; 
	}

	virtual EGDisplayList* BeginFrame_MainThread() override;
	virtual void EndFrame_MainThread( EGDisplayList* DisplayList ) override;

	virtual void Update_RenderThread( eg_real DeltaTime ) override{ unused( DeltaTime ); EGThread_Sleep( 1.f ); }

	void SetDrawSplash( eg_bool DrawSplash ){ unused( DrawSplash ); }

	/////////////////
	// Asset Creation
	/////////////////
	virtual egv_material CreateMaterial( const EGMaterialDef* pMtrlDef , eg_cpstr SharedId ) override{ unused( pMtrlDef , SharedId ); return static_cast<egv_material>(1); }
	virtual void         ReleaseMaterial(egv_material mtrl) override{ unused( mtrl ); }
	virtual egv_vbuffer  CreateVB( eg_string_crc VertexType , eg_uint nNumVerts , const void* pVerts ) override{ unused( VertexType , nNumVerts , pVerts ); egv_vbuffer Out; Out.IntV1 = 1; return Out; }
	virtual void         ReleaseVB(egv_vbuffer buffer) override{ unused( buffer ); }
	virtual egv_ibuffer  CreateIB(eg_uint nNumInds, const egv_index* pIndices) override{ unused( nNumInds , pIndices ); return static_cast<egv_ibuffer>(1); }
	virtual void         ReleaseIB(egv_ibuffer buffer) override{ unused( buffer ); }
	virtual egv_rtarget  CreateRenderTarget( eg_uint Width , eg_uint Height ) override { unused( Width , Height ); return egv_rtarget::Null; }
	virtual void         DestroyRenderTarget( egv_rtarget RenderTarget ) override { unused( RenderTarget ); }
	virtual EGVDynamicBuffer* CreateDynamicBuffer( eg_string_crc VertexType , eg_uint VertexBufferSize , eg_uint IndexBufferSize ) override final { unused( VertexType , VertexBufferSize , IndexBufferSize ); return nullptr; }
	virtual void DestroyDynamicBuffer( EGVDynamicBuffer* DynamicBuffer ) override final { unused( DynamicBuffer ); }
	virtual void UpdateDynamicBuffer( EGVDynamicBuffer* DynamicBuffer ) override final { unused( DynamicBuffer ); }

	//The ViewWidth and height can be obtained, this is important for some
	//drawing functions, and projection matrices.
	virtual eg_uint GetViewWidth()const override{ return 320; }
	virtual eg_uint GetViewHeight()const override{ return 240; }
	virtual eg_real GetAspectRatio()const override{ return static_cast<eg_real>(GetViewWidth())/static_cast<eg_real>(GetViewHeight()); }
	virtual eg_bool IsWindowed()const override{ return true; }

	virtual void    GetResolutions( EGArray<eg_ivec2>& ResOut )const override{ ResOut.Append( eg_ivec2( GetViewWidth() , GetViewHeight() ) ); }

private:
	EGDisplayList m_DlList;
	void*         m_DlMem;
	eg_size_t     m_DlMemSize;
};
