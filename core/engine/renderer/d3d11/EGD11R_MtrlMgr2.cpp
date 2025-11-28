// (c) 2014 Beem Media

/******************************************************************************
Material Manager 2

Some comments on thread safetey. The most obvious thing to note is that there
is not thread locking around GetMaterial. This is only called from the render
thread, so one might argue that a GetMaterial could be called on the render
thread while CreateMaterial or DestroyMaterial is being called on the main
thread. This won't happen for the following reasons: 

When CreateMaterial is called the operation happens immediately and the 
material is available for immediate use (albeit the texutres might still be
loading in the background), and prior to being created it wouldn't exist so
there shouldn't be any calls made to GetMaterial on that particular material
until after it has been created anyway.

For DestroyMaterial the problem is a little more complex. Rather than
destroying the material immediately, since it could well be being used in the
current frame being draw, the material is moved to a destruction queue buffer.
This only modifies the ListId and Next and Prev pointers which are only ever
modified during a lock. So it is safe to modify them in DestroyMaterial. The
destruction queue is emptied out and the assets are actually destroy on the
render thread outside any draw calls. So GetMaterial cannot possibly be called
then.

So we never have a state where the actual material can be modified and used
at the same time. Whether it be because it is newly created and therefore not
used, or it is in a state of limbo waiting till it can safely be destroyed.

Of course there exists the issue of the render thread processing a display list
that references a material that has since been destroyed and may already have
been cleared by the destruction queue. This is handled by globally keeping
track of the resource state, and if the current resource state doesn't match
the state assigned to the display list when it was created. Then that display
list is skipped (the frame is essentially dropped from rendering).

And that is how the material manager is thread safe, even though technically
not everything is locked while being read.

******************************************************************************/
#include "EGD11R_MtrlMgr2.h"
#include "EGD11R_ShaderMgr.h"

EGD11R_MtrlMgr2::EGD11R_MtrlMgr2( class EGD11R_ShaderMgr* ShaderMgr )
: m_ShaderMgr( ShaderMgr )
{	
	m_TxDef.Texture = nullptr;
	//m_TxDef.Filename = "/egdata/textures/default_white.";
	//m_TxDef.Filename.Append( EXT_TEX );

	// Add a null item, that way index 0 is not used for anything real
	egItem NullItem;
	NullItem.State = eg_item_s::NullItem;
	m_MasterList.Append( NullItem );
}

EGD11R_MtrlMgr2::~EGD11R_MtrlMgr2()
{
	for( egItem& Item : m_MasterList )
	{
		assert( Item.State == eg_item_s::Unused || Item.State == eg_item_s::NullItem );

		EG_SafeDelete( Item.Material );
	}
}

void EGD11R_MtrlMgr2::Update_RenderThread( void )
{
	EGFunctionLock Lock( &m_Lock );

	PurgeDestroyQueue();
}

void EGD11R_MtrlMgr2::PurgeDestroyQueue( void )
{
	//Handle destruction queue:

	for( eg_size_t DestroyItemIndex : m_DestroyQue )
	{
		if( m_MasterList.IsValidIndex( DestroyItemIndex ) )
		{
			egItem& DestroyItem = m_MasterList[DestroyItemIndex];

			assert( DestroyItem.RefCount == 0 && DestroyItem.State == eg_item_s::BeingDestroyed );

			assert( DestroyItem.Material );
			if( DestroyItem.Material )
			{
				EGLogf( eg_log_t::RendererActivity , "Destroying material (Tx[0]=\"%s\")" , DestroyItem.Material->m_Tx[0]->Filename.String() );
		
				for( eg_uint i=0; i<EGMaterialDef::MAX_TEX; i++ )
				{
					DestroyItem.Material->m_Tx[i]->Release();
					DestroyItem.Material->m_Tx[i] = nullptr;
				}

				DestroyItem.Material->m_Vs = nullptr;
				DestroyItem.Material->m_Ps = nullptr;
			}

			DestroyItem.UniqueId.Clear();
			DestroyItem.State = eg_item_s::Unused;
		}
	}

	m_DestroyQue.Clear();
}

egv_material EGD11R_MtrlMgr2::CreateMaterial( const EGMaterialDef* Def , eg_cpstr SharedId )
{
	EGFunctionLock Lock( &m_Lock );

	// First thing if this is a shared material, increase the reference count
	if( SharedId && SharedId[0] != '\0' )
	{
		for( egItem& Item : m_MasterList )
		{
			if( Item.State == eg_item_s::Used && EGString_EqualsI( *Item.UniqueId , SharedId ) )
			{
				Item.RefCount++;
				return static_cast<egv_material>(Item.Index);
			}
		}
	}

	// Now find an empty material
	auto GetUnusedMaterial = [this]() -> egItem&
	{
		for( eg_size_t i=0; i<m_MasterList.Len(); i++ )
		{
			egItem& Item = m_MasterList[i];

			if( Item.State == eg_item_s::Unused )
			{
				assert( Item.Index == i );
				return Item;
			}
		}

		m_MasterList.Append( egItem() );
		egItem& NewItem = m_MasterList[m_MasterList.Len()-1];
		NewItem.Index = m_MasterList.LenAs<eg_uint>()-1;
		NewItem.State = eg_item_s::Unused;
		NewItem.Material = new ( eg_mem_pool::System ) EGD11R_Mtrl;
		return NewItem;
	};

	egItem& NewMaterial = GetUnusedMaterial();

	assert( NewMaterial.Material );

	NewMaterial.State = eg_item_s::Used;
	NewMaterial.RefCount = 1;
	NewMaterial.Material->m_D3DMtrl = Def->m_Mtr;

	NewMaterial.Material->m_Vs = m_ShaderMgr->GetVertexShader( Def->m_strVS );
	NewMaterial.Material->m_Ps = m_ShaderMgr->GetPixelShader( Def->m_strPS );
	NewMaterial.UniqueId = SharedId;
	// NewMaterial.DebugRef = Def->m_strTex[0];

	for( eg_uint i=0; i<EGMaterialDef::MAX_TEX; i++ )
	{
		NewMaterial.Material->m_Tx[i] = m_ShaderMgr->GetTexture( Def->m_strTex[i] );
		if( nullptr == NewMaterial.Material->m_Tx[i] )
		{
			if( Def->m_strTex[i][0] != '\0' )
			{
				EGLogf( eg_log_t::Warning , __FUNCTION__" %s did not exist." , Def->m_strTex[i] );
			}
			NewMaterial.Material->m_Tx[i] = &m_TxDef;
		}
		NewMaterial.Material->m_Tx[i]->AddRef();
	}

	return static_cast<egv_material>(NewMaterial.Index);
}

void EGD11R_MtrlMgr2::DestroyMaterial( egv_material Material )
{
	EGFunctionLock Lock( &m_Lock );

	eg_size_t AsIndex = static_cast<eg_size_t>(Material);
	assert( m_MasterList.IsValidIndex( AsIndex ) ); // Never should have gotten a number out of this range, about to crash.
	egItem& Item = m_MasterList[AsIndex];
	assert( Item.Index == AsIndex );

	// Nothing to do if we try to destory null item.
	if( 0 == AsIndex )
	{
		assert( Item.Index == 0 && Item.State == eg_item_s::NullItem );
		return;
	}

	assert( Item.RefCount > 0 );
	Item.RefCount--;
	if( Item.RefCount == 0 )
	{
		Item.State = eg_item_s::BeingDestroyed;
		m_DestroyQue.AppendUnique( AsIndex );
	}
}

const EGD11R_Mtrl* EGD11R_MtrlMgr2::GetMaterial( egv_material Material )
{
	eg_size_t AsIndex = static_cast<eg_size_t>(Material);
	assert( m_MasterList.IsValidIndex( AsIndex ) ); // Never should have gotten a number out of this range, about to crash.
	egItem& Out = m_MasterList[AsIndex];
	assert( Out.State == eg_item_s::Used || Out.State == eg_item_s::NullItem /* || Out.State == eg_item_s::BeingDestroyed */ ); //Could be in LIST_DESTROY_QUEUE but that shouldn't happen due to resource state.
	assert( Out.Index == AsIndex );
	return Out.Material;
}
