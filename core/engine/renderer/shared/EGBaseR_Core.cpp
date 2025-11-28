// (c) 2016 Beem Media

#include "EGBaseR_Core.h"
#include "EGEngineConfig.h"

EG_CLASS_DECL( EGBaseR_Core )

const eg_char EGBaseR_Core::SPLASH_TEX_FILE[] = GAME_DATA_PATH "/splash/T_Splash_d";

EGDisplayList* EGBaseR_Core::BeginFrame_MainThread()
{
	EGFunctionLock Lock( &m_DlLock );

	assert( nullptr == m_DlMainThread );

	if( !m_DlAvailable.HasItems() )
	{
		assert( false ); //No display list available...
		return nullptr;
	}

	egDisplayListData* Data = m_DlAvailable.Top();
	m_DlAvailable.Pop();
	m_DlMainThread = Data;
	Data->List.InitDisplayList( Data->Mem, Data->MemSize, GetAssetState() );
	return &Data->List;
}

void EGBaseR_Core::EndFrame_MainThread( EGDisplayList* DisplayList )
{
	unused( DisplayList );

	EGFunctionLock Lock( &m_DlLock );

	assert( DisplayList == &m_DlMainThread->List ); //Something went seriously wrong.

	if( m_DlNext && m_DlNext->List.CannotBeDropped() )
	{
		while( m_DlNext )
		{
			m_DlLock.Unlock();
			m_DlLock.Lock();
		}
	}

	if( m_DlNext )
	{
		//EGLogf( eg_log_t::RendererActivity , ( "Display list dropped." ) );
		m_DlNext->List.DeinitDisplayList();
		m_DlAvailable.Push( m_DlNext );
	}
	m_DlNext = m_DlMainThread;
	m_DlMainThread = nullptr;
}

void EGBaseR_Core::GetResolutions( EGArray<eg_ivec2>& ResOut ) const
{
	for( eg_size_t i = 0; i < m_Resolutions.Len(); i++ )
	{
		eg_uint32 Size = m_Resolutions.GetByIndex( i );
		eg_ivec2 Res( Size & 0x0000FFFF, ( Size & 0xFFFF0000 ) >> 16 );
		ResOut.Append( Res );
	}
}

void EGBaseR_Core::HandleCommonDisplayListCmd( const EGDisplayList::egCmdListItem* Cmd )
{
	const egDisplayListCmdData* Data = Cmd->Data;

	switch( Cmd->Cmd )
	{
	case DLFUNC_SetWorldTFInternal: SetWorldTF( Data->SetWorldTFInternal.Mat ); break;
	case DLFUNC_SetViewTFInternal: SetViewTF( Data->SetViewTFInternal.Mat ); break;
	case DLFUNC_SetProjTFInternal: SetProjTF( Data->SetProjTFInternal.Mat ); break;
	case DLFUNC_SetFloat: SetFloat( Data->SetFloat.type, Data->SetFloat.fValue ); break;
	case DLFUNC_SetVec4: SetVec4( Data->SetVec4.type, &Data->SetVec4.pv ); break;
	case DLFUNC_SetIVec4: SetIVec4( Data->SetIVec4.type , Data->SetIVec4.pv ); break;
	case DLFUNC_SetBoneMats: SetBoneMats( Data->SetBoneMats.pTrans, Data->SetBoneMats.nCount ); break;
	case DLFUNC_SetMaterial: SetMaterial( Data->SetMaterial.mtrl ); break;
	case DLFUNC_SetMaterialOverride: SetMaterialOverride( Data->SetMaterialOverride.mtrl ); break;
	case DLFUNC_SetLight: SetLight( Data->SetLight.LightIndex, &Data->SetLight.Light ); break;
	case DLFUNC_EnableLight: EnableLight( Data->EnableLight.LightIndex, Data->EnableLight.bEnable ); break;
	case DLFUNC_DisableAllLights: DisableAllLights(); break;
	case DLFUNC_PushRasterizerState: m_CurRasterizerState.Push( Data->PushRasterizerState.State ); break;
	case DLFUNC_PopRasterizerState: m_CurRasterizerState.Pop(); break;
	case DLFUNC_PushSamplerState: m_CurSamplerState.Push( Data->PushSamplerState.State ); break;
	case DLFUNC_PopSamplerState: m_CurSamplerState.Pop(); break;
	case DLFUNC_PushBlendState: m_CurBlendState.Push( Data->PushBlendState.State ); break;
	case DLFUNC_PopBlendState: m_CurBlendState.Pop(); break;
	case DLFUNC_PushDepthStencilState: m_CurDepthStencilState.Push( Data->PushDepthStencilState.State ); break;
	case DLFUNC_PopDepthStencilState: m_CurDepthStencilState.Pop(); break;
	case DLFUNC_PushDefaultShader:
		m_CurDefaultShader.Push( Data->PushDefaultShader.Shader );
		SetMaterial( EGV_MATERIAL_NULL );
		break;
	case DLFUNC_PopDefaultShader:
		m_CurDefaultShader.Pop();
		SetMaterial( EGV_MATERIAL_NULL );
		break;
	default:
		assert( false ); // Something should have handled this.
		break;
	}
}

void EGBaseR_Core::ReleaseMaterial( egv_material mtrl )
{
	EGFunctionLock Lock( &m_AssetCreationLock );

	egDestroyItem* DestroyItem = new egDestroyItem;
	if( DestroyItem )
	{
		DestroyItem->Type = eg_asset_t::MATERIAL;
		DestroyItem->Material = mtrl;
		DestroyItem->LastAssetStateUsed = GetAssetState();
		m_DestroyQue.InsertLast( DestroyItem );
		UpdateAssetState();
	}
	else
	{
		assert( false ); // Couldn't allocate memory to destroy a render asset!
	}
}

void EGBaseR_Core::ReleaseVB( egv_vbuffer buffer )
{
	EGFunctionLock Lock( &m_AssetCreationLock );

	egDestroyItem* DestroyItem = new egDestroyItem;
	if( DestroyItem )
	{
		DestroyItem->Type = eg_asset_t::VB;
		DestroyItem->VBuffer = buffer;
		DestroyItem->LastAssetStateUsed = GetAssetState();
		m_DestroyQue.InsertLast( DestroyItem );
		UpdateAssetState();
	}
	else
	{
		assert( false ); // Couldn't allocate memory to destroy a render asset!
	}
}

void EGBaseR_Core::ReleaseIB( egv_ibuffer buffer )
{
	EGFunctionLock Lock( &m_AssetCreationLock );

	egDestroyItem* DestroyItem = new egDestroyItem;
	if( DestroyItem )
	{
		DestroyItem->Type = eg_asset_t::IB;
		DestroyItem->IBuffer = buffer;
		DestroyItem->LastAssetStateUsed = GetAssetState();
		m_DestroyQue.InsertLast( DestroyItem );
		UpdateAssetState();
	}
	else
	{
		assert( false ); // Couldn't allocate memory to destroy a render asset!
	}
}

void EGBaseR_Core::DestroyRenderTarget( egv_rtarget RenderTarget )
{
	EGFunctionLock Lock( &m_AssetCreationLock );

	egDestroyItem* DestroyItem = new egDestroyItem;
	if( DestroyItem )
	{
		DestroyItem->Type = eg_asset_t::RT;
		DestroyItem->RTarget = RenderTarget;
		DestroyItem->LastAssetStateUsed = GetAssetState();
		m_DestroyQue.InsertLast( DestroyItem );
		UpdateAssetState();
	}
	else
	{
		assert( false ); // Couldn't allocate memory to destroy a render asset!
	}
}

void EGBaseR_Core::ProcessDeleteQue( eg_bool bForce, eg_uint LastDlAssetState )
{
	EGFunctionLock Lock( &m_AssetCreationLock );

	EGDestroyQue ItemsToKeep( 2 ); // Id of 2 since the main list has id of 1
	EGDestroyQue ItemsToDestroy( 3 ); // Id of 3 since the main list has id of 1

	// Put all items in either the keep or destroy queues.
	while( m_DestroyQue.HasItems() )
	{
		egDestroyItem* Item = m_DestroyQue.GetOne();
		m_DestroyQue.Remove( Item );
		if( Item->LastAssetStateUsed < LastDlAssetState || bForce )
		{
			ItemsToDestroy.Insert( Item );
		}
		else
		{
			ItemsToKeep.Insert( Item );
		}
	}

	// Preserve the keep items...
	while( ItemsToKeep.HasItems() )
	{
		egDestroyItem* Item = ItemsToKeep.GetOne();
		ItemsToKeep.Remove( Item );
		m_DestroyQue.Insert( Item );
	}

	// Destroy the rest...
	while( ItemsToDestroy.HasItems() )
	{
		egDestroyItem* Item = ItemsToDestroy.GetOne();
		ItemsToDestroy.Remove( Item );

		HandleDeleteAsset( *Item );

		delete Item;
	}
}
