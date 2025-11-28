// (c) 2019 Beem Media

#include "EGFileData.h"


EGFileData::EGFileData( eg_file_data_init_t SpecialType )
: m_DefaultArray( eg_mem_pool::DefaultHi )
, m_NonConstByteArray( m_DefaultArray )
, m_ConstByteArray( m_DefaultArray )
, m_BytePtr( nullptr )
, m_BytePtrSize( 0 )
, m_Type( SpecialTypeToType(SpecialType) )
, m_ReadWritePos( 0 )
, m_bCanSetData( SpecialType == eg_file_data_init_t::SetableUserPointer )
{

}

EGFileData::EGFileData( EGArray<eg_byte>& InByteArray )
: m_DefaultArray( eg_mem_pool::DefaultHi )
, m_NonConstByteArray( InByteArray )
, m_ConstByteArray( m_DefaultArray )
, m_BytePtr( nullptr )
, m_BytePtrSize( 0 )
, m_Type( eg_t::NonConstByteArray )
, m_ReadWritePos( 0 )
, m_bCanSetData( false )
{

}

EGFileData::EGFileData( const EGArray<eg_byte>& InConstByteArray )
: m_DefaultArray( eg_mem_pool::DefaultHi )
, m_NonConstByteArray( m_DefaultArray )
, m_ConstByteArray( InConstByteArray )
, m_BytePtr( nullptr )
, m_BytePtrSize( 0 )
, m_Type( eg_t::ConstByteArray )
, m_ReadWritePos( 0 )
, m_bCanSetData( false )
{

}

EGFileData::EGFileData( const eg_byte* InConstBytePtr , eg_size_t InBytePtrSize )
: m_DefaultArray( eg_mem_pool::DefaultHi )
, m_NonConstByteArray( m_DefaultArray )
, m_ConstByteArray( m_DefaultArray )
, m_BytePtr( InConstBytePtr )
, m_BytePtrSize( InBytePtrSize )
, m_Type( eg_t::ConstBytePtr )
, m_ReadWritePos( 0 )
, m_bCanSetData( false )
{

}

void EGFileData::SetData( const void* InConstVoidPtr , eg_size_t InVoidPtrSize )
{
	if( m_Type == eg_t::ConstBytePtr && m_bCanSetData )
	{
		m_BytePtr = reinterpret_cast<const eg_byte*>(InConstVoidPtr);
		m_BytePtrSize = InVoidPtrSize;
		m_ReadWritePos = 0;
	}
	else
	{
		assert( false ); // Only a pointer or unknown type can be opened as a pointer.
	}
}

eg_size_t EGFileData::Read( void* Out , eg_size_t ReadSize ) const
{
	eg_size_t ReadSizeOut = 0;

	const eg_size_t FinalReadSize = EG_Min( ReadSize , GetSize() - m_ReadWritePos );

	if( Out && FinalReadSize > 0 )
	{
		if( const eg_byte* DataAtReadPos = GetDataAtReadPosAs<eg_byte>() )
		{
			EGMem_Copy( Out , DataAtReadPos , FinalReadSize );
			m_ReadWritePos += FinalReadSize;
			ReadSizeOut = FinalReadSize;
		}
	}

	return ReadSizeOut;
}

eg_size_t EGFileData::Write( const void* const In , eg_size_t WriteSize )
{
	eg_size_t WrittenSizeOut = 0;

	if( m_Type == eg_t::NonConstByteArray )
	{
		const eg_size_t CurrentSize = m_NonConstByteArray.Len();
		const eg_size_t NewWritePos = m_ReadWritePos + WriteSize;

		if( NewWritePos > CurrentSize )
		{
			m_NonConstByteArray.Resize( NewWritePos );
		}

		const eg_size_t NewSize = m_NonConstByteArray.Len();

		if( NewWritePos <= NewSize )
		{
			const eg_size_t FinalWriteSize = EG_Min( WriteSize , NewSize - m_ReadWritePos );
			EGMem_Copy( &m_NonConstByteArray.GetArray()[m_ReadWritePos] , In , FinalWriteSize );
			m_ReadWritePos += FinalWriteSize;
			WrittenSizeOut = FinalWriteSize;
		}
		else
		{
			assert( false ); // Couldn't resize array. Out of memory?
		}
	}
	else
	{
		assert( false ); // Cannot write to anything other than NonConstByteArray.
	}

	return WrittenSizeOut;
}

eg_size_t EGFileData::WriteStr8( eg_cpstr8 Str8 )
{
	const eg_size_t StrLen = EGString_StrLen( Str8 );
	return Write( Str8 , StrLen );
}

void EGFileData::Clear()
{
	if( m_Type == eg_t::NonConstByteArray )
	{
		m_NonConstByteArray.Clear();
		m_ReadWritePos = 0;
	}
	else
	{
		assert( false ); // Cannot write to anything other than NonConstByteArray.
	}
}

void EGFileData::Resize( eg_size_t NewSize )
{
	if( m_Type == eg_t::NonConstByteArray )
	{
		m_NonConstByteArray.Resize( NewSize );
		m_ReadWritePos = EG_Clamp<eg_size_t>( m_ReadWritePos , 0 , NewSize );
	}
	else
	{
		assert( false ); // Cannot write to anything other than NonConstByteArray.
	}
}

const void* EGFileData::GetDataAt( eg_size_t Position ) const
{
	const void* MemOut = nullptr;

	if( Position < GetSize() )
	{
		switch( m_Type )
		{
		case eg_t::Unk:
			assert( false );
			break;
		case eg_t::NonConstByteArray:
			MemOut = &m_NonConstByteArray[Position];
			break;
		case eg_t::ConstByteArray:
			MemOut = &m_ConstByteArray[Position];
			break;
		case eg_t::ConstBytePtr:
		{
			MemOut = &m_BytePtr[Position];
		} break;
		}
	}

	return MemOut;
}

eg_size_t EGFileData::GetSize() const
{
	eg_size_t SizeOut = 0;

	switch( m_Type )
	{
	case eg_t::Unk:
		assert( false );
		break;
	case eg_t::NonConstByteArray:
		SizeOut = m_NonConstByteArray.Len();
		break;
	case eg_t::ConstByteArray:
		SizeOut = m_ConstByteArray.Len();
		break;
	case eg_t::ConstBytePtr:
	{
		SizeOut = m_BytePtrSize;
	} break;
	}

	return SizeOut;
}

eg_int EGFileData::Seek( eg_file_data_seek_t SeekType , eg_int Distance ) const
{
	eg_size_t NewPos = 0;

	const eg_size_t OldPos = m_ReadWritePos;

	switch( SeekType )
	{
	case eg_file_data_seek_t::Current:
		NewPos = m_ReadWritePos + Distance;
		break;
	case eg_file_data_seek_t::Begin:
		NewPos = Distance;
		break;
	case eg_file_data_seek_t::End:
		NewPos = GetSize() + Distance;
		break;
	}

	m_ReadWritePos = EG_Clamp<eg_size_t>( NewPos , 0, GetSize() );

	return EG_To<eg_int>(EG_To<eg_int64>(m_ReadWritePos) - EG_To<eg_int64>(OldPos));
}

eg_size_t EGFileData::Tell() const
{
	return m_ReadWritePos;
}

eg_bool EGFileData::Equals( const EGFileData& rhs ) const
{
	if( GetSize() != rhs.GetSize() )
	{
		return false;
	}

	const eg_size_t SizeToCompare = GetSize();

	if( 0 == SizeToCompare )
	{
		return true;
	}

	return EGMem_Equals( GetData() , rhs.GetData() , SizeToCompare );
}

EGFileData::eg_t EGFileData::SpecialTypeToType( eg_file_data_init_t SpecialType )
{
	eg_t TypeOut = eg_t::Unk;

	switch( SpecialType )
	{
	case eg_file_data_init_t::Default:
		break;
	case eg_file_data_init_t::HasOwnMemory:
		TypeOut = eg_t::NonConstByteArray;
		break;
	case eg_file_data_init_t::SetableUserPointer:
		TypeOut = eg_t::ConstBytePtr;
		break;
	}

	return TypeOut;
}
