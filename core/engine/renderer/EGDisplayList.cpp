#include "EGDisplayList.h"

eg_size_t EGDisplayList::MaxBytesUsed = 0;

void EGDisplayList::InitDisplayList( void* Mem , eg_size_t MemSize , eg_uint AssetState )
{
	m_AssetState = AssetState;
	m_OutOfMem = false;
	m_bCannotbeDropped = false;
	m_Heap.Init( Mem , MemSize , EG_ALIGNMENT );
	List.Clear();
}

void EGDisplayList::DeinitDisplayList( void )
{
	//const eg_size_t BytesUsed = Heap.GetInfo( IMemAlloc::INFO_ALLOC_MEM );
	//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( ( "Frame buffer: %u bytes" ) , BytesUsed ) );
	
	//Technically the display list is just a chunk of memory with no important
	//data and no destructors need to be run or anything, but freeing up the
	//data makes the allocator happy. It'll be more efficient to be able to
	//deinit the allocator without it asserting. Or have some kind of FreeAll
	//functionality.

	MaxBytesUsed = EG_Max(m_Heap.GetInfo( IMemAlloc::INFO_ALLOC_MEM ) , MaxBytesUsed );

	//MemMgr_DisplayMemUsage( DEBUG_TEXT_OVERLAY_RENDERER , "Display List" , &m_Heap , m_Heap.GetInfo(IMemAlloc::INFO_ALLOC_BLOCKS) );

	#if defined( __DEBUG__ )
	//In debug mode we free everything
	while( List.Len() > 0 )
	{
		egCmdListItem* Item = List.GetFirst();
		List.Remove( Item );
		egDisplayListCmdData* Data = Item->Data;

		switch( Item->Cmd )
		{
			case DLFUNC_SetBoneMats: Free( Data->SetBoneMats.pTrans ); break;
			case DLFUNC_DrawRawTris: Free( Data->DrawRawTris.pVerts ); break;
			default:
				break;
		}

		if( Data )Free( Data );
		Free( Item );
	}
	assert( 0 == List.Len() );
	List.Clear();
	#else
	List.Clear();
	m_Heap.FreeAll();
	#endif
	
	m_Heap.Deinit();
	m_OutOfMem = false;
}

void EGDisplayList::DrawRawTris(const egv_vert_simple* pVerts, eg_uint nNumTris)
{
	if( 0 == nNumTris )return; //No reason to insert a command if it doesn't do anything.
	egCmd Cmd;
	Cmd.Cmd = DLFUNC_DrawRawTris;
	Cmd.Data.DrawRawTris.nNumTris = nNumTris;
	Cmd.Data.DrawRawTris.pVerts = Alloc<egv_vert_simple>( nNumTris*3*sizeof(egv_vert_simple) );
	if( nullptr == Cmd.Data.DrawRawTris.pVerts )return;
	EGMem_Copy( Cmd.Data.DrawRawTris.pVerts , pVerts , nNumTris*3*sizeof(egv_vert_simple) );
	InsertSimpleCommand( &Cmd , sizeof(Cmd.Data.DrawRawTris) );
}

void EGDisplayList::SetBoneMats(const eg_transform* pTrans, const eg_uint nCount)
{
	assert( nCount > 0 );
	assert( pTrans[0].IsIdentity() );
	egCmd Cmd;
	Cmd.Cmd = DLFUNC_SetBoneMats;
	Cmd.Data.SetBoneMats.nCount = nCount;
	Cmd.Data.SetBoneMats.pTrans = Alloc<eg_transform>( sizeof(eg_transform)*nCount );
	if( nullptr == Cmd.Data.SetBoneMats.pTrans )return;
	EGMem_Copy( Cmd.Data.SetBoneMats.pTrans , pTrans , nCount*sizeof(eg_transform) );
	InsertSimpleCommand( &Cmd , sizeof(Cmd.Data.SetBoneMats) );
}

void EGDisplayList::InsertSimpleCommand( const egCmd* Cmd , eg_uint DataSize )
{
	void* ItemMem = Alloc<void>( EG_AlignUp( sizeof(egCmdListItem) ) );
	if( nullptr == ItemMem )return;

	egCmdListItem* NewItem = new( ItemMem ) egCmdListItem;
	NewItem->Cmd = Cmd->Cmd;
	NewItem->DataSize = DataSize;
	if( DataSize > 0 )
	{
		egDisplayListCmdData* ItemDataMem = Alloc<egDisplayListCmdData>( EG_AlignUp( DataSize ) );
		if( nullptr == ItemDataMem )return;
		EGMem_Copy( ItemDataMem , &Cmd->Data , DataSize );
		NewItem->Data = ItemDataMem;
	}
	else
	{
		NewItem->Data = nullptr;
	}

	List.InsertLast( NewItem );
}
