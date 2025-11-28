// (c) 2018 Beem Media

#pragma once

#include "EGItemMap.h"
#include "EGHeap2.h"
#include "EGDirectXAPI.h"
#include "EGRendererTypes.h"

struct egD11R_VertexFormat;

EG_DECLARE_FLAG( F_D11RAB_DYNAMICBUFFER , 1 );

class EGD11R_AllocBuffer
{
private:

	ID3D11Device*const         m_OwnerDevice;
	ID3D11DeviceContext* const m_OwnerContext;
	ID3D11InputLayout*const    m_InputLayout;
	ID3D11Buffer*              m_Buffer;
	const eg_string_crc        m_DeclType;
	const D3D11_BIND_FLAG      m_BindType;
	const UINT                 m_ItemSize;
	const UINT                 m_MaxItems;
	const UINT                 m_BufferByteSize;
	eg_byte*                   m_MemChunk;
	eg_size_t                  m_MemChunkSize;
	eg_size_t                  m_DynamicBufferSize;
	EGHeap2                    m_MemHeap;
	const eg_flags             m_Flags;
	eg_int                     m_NumCreated;
	eg_bool                    m_bIsDirty;
	eg_bool                    m_bIsValid;

public:

	EGD11R_AllocBuffer( eg_string_crc InDeclType , ID3D11Device* InOwnerDevice , ID3D11DeviceContext* InOwnerContext , ID3D11InputLayout* InInputLayout , D3D11_BIND_FLAG InBindType , const UINT InItemSize , const UINT InMaxItems , eg_flags InFlags );
	~EGD11R_AllocBuffer();

	void Validate( eg_bool bInit );
	egv_vbuffer CreateVB( eg_uint NumVerts , const void* pVerts );
	void DestroyVB( const egv_vbuffer& Buffer );
	egv_ibuffer CreateIB( eg_uint NumInds , const egv_index* pIndices );
	void DestroyIB( const egv_ibuffer& Buffer );
	void SetBufferData( eg_uintptr_t Addr , const void* SourceV , const eg_size_t SourceVSize );
	void UpdateDynamicResource( const void* Data , eg_size_t DataSize );
	void PreFrameUpdate();
	void SetAsCurrentVertexBuffer();
	void SetAsCurrentIndexBuffer();
};

struct egD11R_VertexLayoutData
{
	const egD11R_VertexFormat* Format = nullptr;
	ID3D11InputLayout*         Layout = nullptr;	
};

class EGD11R_BufferMgr
{
public:

	static const eg_uint MAX_VERTEXES = 1000000;
	static const eg_uint MAX_INDEXES = MAX_VERTEXES*3;
	static const eg_uint MAX_RAWTRI_VERTS = 6*256; // Basically enough to fit a 256 character buffer...

private:

	ID3D11Device*const m_OwnerDevice;
	ID3D11DeviceContext*const m_OwnerContext;
	EGFixedItemMap<eg_string_crc,egD11R_VertexLayoutData,5> m_LayoutDatas;

	//Vertex and index buffers are managed as a heap. There is only actually one
	//Direct3D buffer, and the vertexes for an individual mesh are allocated
	//using the heap. Whenever a buffer changes, the new buffer will be copied
	//to vram as soon as it is safe to do so.
	EGD11R_AllocBuffer* m_MeshVB = nullptr;
	EGD11R_AllocBuffer* m_TerrainVB = nullptr;
	EGD11R_AllocBuffer* m_IB = nullptr;
	EGD11R_AllocBuffer* m_SimpleVB = nullptr;

	eg_string_crc m_LastVertexLayoutType = CT_Clear;

public:

	EGD11R_BufferMgr( ID3D11Device* InOwnerDevice , ID3D11DeviceContext* InOwnerContext );
	~EGD11R_BufferMgr();

	void Validate( eg_bool bInitialize );
	void PreFrameUpdate();

	ID3D11InputLayout* GetInputLayoutForType( eg_string_crc VertexType ) const;
	const egD11R_VertexFormat& GetVertexFormatForType( eg_string_crc VertexType ) const;

	egv_vbuffer CreateVB( eg_string_crc VertexType , eg_uint NumVerts , const void* VertData );
	void DestroyVB( const egv_vbuffer& Buffer );
	egv_ibuffer CreateIB( eg_uint NumIndexes , const egv_index* IndexData );
	void DestroyIB( const egv_ibuffer& Buffer );

	void SetBufferData( const egv_vbuffer& Buffer , const void* SourceV , const eg_size_t SourceVSize );
	void SetBufferData( const egv_ibuffer& Buffer , const void* SourceV , const eg_size_t SourceVSize );
	void UpdateSimpleVB( const void* Data , eg_size_t DataSize );

	void SetVertexLayout( eg_string_crc NewType );
	EGD11R_AllocBuffer* GetAllocForType( eg_string_crc VertexType ) const;
};
