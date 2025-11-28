// (c) 2017 Beem Media

#include "EGSmProc.h"

EG_CLASS_DECL( EGSmProc )

eg_bool EGSmProc::Init( const void* ByteCode , eg_size_t CodeSize )
{
	assert( m_ByteCode == nullptr && m_ByteCodeSize == 0 );

	if( CodeSize < sizeof(m_Header) )
	{
		assert( false ); // No way this byte-code was valid.
		return false;
	}

	m_ByteCodeSize = CodeSize;
	m_ByteCode = new ( eg_mem_pool::System ) eg_byte[m_ByteCodeSize];
	if( nullptr == m_ByteCode )
	{
		// Out of memory or bad code?
		assert( false );
		m_ByteCodeSize = 0;
		return false;
	}

	EGMem_Copy( m_ByteCode , ByteCode , m_ByteCodeSize );
	EGMem_Copy( &m_Header , m_ByteCode , sizeof(m_Header) );

	if( m_Header.NodeOffset > m_ByteCodeSize 
	|| m_Header.BranchOffset > m_ByteCodeSize 
	|| m_Header.LabelOffset > m_ByteCodeSize
	|| m_Header.DebugSymbolsOffset > m_ByteCodeSize )
	{
		assert( false ); // Not possibly valid since the headers or nodes start to late.
		EG_SafeDelete( m_ByteCode );
		m_ByteCodeSize = 0;
		return false;
	}

	m_Header.Nodes = reinterpret_cast<egsmNodeBc*>( &m_ByteCode[m_Header.NodeOffset] );
	m_Header.Branches = reinterpret_cast<egsmBranchBc*>( &m_ByteCode[m_Header.BranchOffset] );
	m_Header.Labels = reinterpret_cast<egsmLabelBc*>( &m_ByteCode[m_Header.LabelOffset] );
	m_Header.DebugSymbols = reinterpret_cast<eg_char*>( &m_ByteCode[m_Header.DebugSymbolsOffset] );

	return true;
}

EGSmProc::~EGSmProc()
{
	EG_SafeDelete( m_ByteCode );
}

egsm_node_id EGSmProc::FindEntryNode( eg_string_crc LabelName , eg_bool bDefaultEntryOkay ) const
{
	// We'll search for the desired entry (if specified), failing that the
	// default entry. In theory we could also search for any entry point
	// node and use that or use the first entry, but it's better that the 
	// designer knows right away that the script was called incorrectly
	// instead of it running something at random.

	egsm_node_id FoundDefaultEntry( CT_Clear );

	for( eg_size_t i=0; i<m_Header.LabelCount; i++ )
	{
		if( m_Header.Labels[i].LabelName == LabelName )
		{
			return egsm_node_id(m_Header.Labels[i].NodeToIndexPlusOne);
		}

		if( m_Header.Labels[i].bIsDefaultEntryPoint )
		{
			FoundDefaultEntry = egsm_node_id(m_Header.Labels[i].NodeToIndexPlusOne);
		}
	}

	return bDefaultEntryOkay ? FoundDefaultEntry : CT_Clear;
}

const egsmNodeBc* EGSmProc::GetNode( egsm_node_id NodeId ) const
{
	eg_size_t IndexPlusOne = NodeId.NodeIndexPlusOne;
	if( 0 < IndexPlusOne && IndexPlusOne <= m_Header.NodeCount )
	{
		return &m_Header.Nodes[IndexPlusOne-1];
	}
	return nullptr;
}

const egsmBranchBc* EGSmProc::GetBranch( const egsmNodeBc* Node , eg_size_t Index ) const
{
	if( nullptr == Node )
	{
		return nullptr;
	}

	if( !EG_IsBetween<eg_size_t>( Index , 0 , Node->NumBranches-1 ) )
	{
		return nullptr;
	}

	if( EG_IsBetween<eg_size_t>( Node->FirstBranch + Index ,  0 , m_Header.BranchCount-1  ) )
	{
		return &m_Header.Branches[Node->FirstBranch + Index];
	}
	return nullptr;
}
