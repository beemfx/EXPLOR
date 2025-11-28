// (c) 2018 Beem Media

#pragma once

#include "EGRendererTypes.h"

class EGVDynamicBuffer
{
private:

	static EGMutex s_DynamicBufferMutex;
	static eg_bool s_bDynamicBuffersLocked;

private:

	const egv_vbuffer m_VertexBuffer;
	const egv_ibuffer m_IndexBuffer;
	const eg_size_t m_VertexSize;
	EGArray<eg_byte> m_VertexBufferData = eg_mem_pool::RenderResource;
	EGArray<egv_index> m_IndexBufferData = eg_mem_pool::RenderResource;

public:

	static void LockDynamicBuffers() { s_DynamicBufferMutex.Lock(); assert( !s_bDynamicBuffersLocked ); s_bDynamicBuffersLocked = true; }
	static void UnlockDynamicBuffers() { assert( s_bDynamicBuffersLocked ); s_bDynamicBuffersLocked = false; s_DynamicBufferMutex.Unlock(); }

	EGVDynamicBuffer( eg_size_t InVertexSize , egv_vbuffer InVBuffer , eg_size_t InVSize , egv_ibuffer InIBuffer , eg_size_t InISize );
	EGVDynamicBuffer( const EGVDynamicBuffer& rhs ) = delete;
	~EGVDynamicBuffer();
	const EGVDynamicBuffer& operator = ( const EGVDynamicBuffer& rhs ) = delete;

	egv_vbuffer GetVertexBuffer() const { return m_VertexBuffer; }
	egv_ibuffer GetIndexBuffer() const { return m_IndexBuffer; }

	template<class RetType>
	RetType* GetVertexBufferData() { assert( s_bDynamicBuffersLocked ); assert( 1 == sizeof(RetType) || sizeof(RetType) == m_VertexSize ); return reinterpret_cast<RetType*>(m_VertexBufferData.GetArray()); }
	eg_size_t GetVertexBufferDataCount() const { return m_VertexBufferData.Len()/m_VertexSize; }
	eg_size_t GetVertexSize() const { return m_VertexSize; }
	egv_index* GetIndexBufferData() { assert( s_bDynamicBuffersLocked ); return m_IndexBufferData.GetArray(); }
	eg_size_t GetIndexBufferDataCount() const { return m_IndexBufferData.Len(); }
};

