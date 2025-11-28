/******************************************************************************
EGNullR_Renderer

(c) 2015 Beem Software
******************************************************************************/
#include "EGNullR_Renderer.h"
#include "EGWindowsAPI.h"
#include "EGInputKbmDevice.h"
#include "EGEngineConfig.h"

EG_CLASS_DECL( EGNullR_Renderer )

EGNullR_Renderer::EGNullR_Renderer()
: m_DlMem( nullptr )
, m_DlMemSize( 0 )
{
	InputKbmDevice_WindowThread_Init( nullptr );

	m_DlMemSize = EG_DISPLAY_LIST_MEGS*1024*1024;
	m_DlMem = EGMem2_Alloc( m_DlMemSize , eg_mem_pool::System );
	if( nullptr == m_DlMem )
	{
		m_DlMemSize = 0;
	}
}

EGNullR_Renderer::~EGNullR_Renderer()
{
	EGMem2_Free( m_DlMem );
	m_DlMem = nullptr;
	m_DlMemSize = 0;
	InputKbmDevice_WindowThread_Deinit( nullptr );
}

EGDisplayList* EGNullR_Renderer::BeginFrame_MainThread()
{
	m_DlList.InitDisplayList( m_DlMem , m_DlMemSize , 0 );
	return &m_DlList;
}

void EGNullR_Renderer::EndFrame_MainThread( EGDisplayList* DisplayList )
{
	assert( &m_DlList == DisplayList );
	DisplayList->DeinitDisplayList();
}
