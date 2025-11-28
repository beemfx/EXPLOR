// (c) 2017 Beem Media

#pragma once

#include "EGSmTypes.h"
#include "EGList.h"

class EGSmProc : public EGObject , public IListable
{
	EG_CLASS_BODY( EGSmProc , EGObject )

private:
	
	egsmByteCodeHeader m_Header;
	eg_size_t          m_ByteCodeSize = 0;
	eg_byte*           m_ByteCode = nullptr;

public:

	EGSmProc(): Super(){ zero( &m_Header ); }
	eg_bool Init( const void* ByteCode , eg_size_t CodeSize );
	virtual ~EGSmProc() override;

	eg_string_crc GetId() const { return m_Header.Props.Id; }
	egsm_node_id FindEntryNode( eg_string_crc LabelName , eg_bool bDefaultEntryOkay ) const;
	const egsmNodeBc* GetNode( egsm_node_id NodeId ) const;
	const egsmBranchBc* GetBranch( const egsmNodeBc* Node , eg_size_t Index )const;
};