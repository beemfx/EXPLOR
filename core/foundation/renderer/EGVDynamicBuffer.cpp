// (c) 2018 Bee Media

#include "EGVDynamicBuffer.h"

EGMutex EGVDynamicBuffer::s_DynamicBufferMutex;
eg_bool EGVDynamicBuffer::s_bDynamicBuffersLocked = false;

EGVDynamicBuffer::EGVDynamicBuffer( eg_size_t InVertexSize , egv_vbuffer InVBuffer , eg_size_t InVSize , egv_ibuffer InIBuffer , eg_size_t InISize )
: m_VertexSize( InVertexSize )
, m_VertexBuffer( InVBuffer )
, m_IndexBuffer( InIBuffer )
{
	m_VertexBufferData.Resize( InVertexSize * InVSize );
	m_IndexBufferData.Resize( InISize );
}

EGVDynamicBuffer::~EGVDynamicBuffer()
{

}
