// (c) 2019 Beem Media

// Mem File 2 is used for reading and writing to a byte array as if it
// were a file. It does not maintain the memory so whatever the source of
// the memory is (array or pointer) must survive for as long as the mem
// file is in use unless it is created with
// eg_mem_file_specia_t::HasOwnMemory. Only an EGArray<eg_byte> can be 
// written to, everything else is read only.

#pragma once

enum class eg_file_data_init_t
{
	Default,
	HasOwnMemory,
	SetableUserPointer,
};

enum class eg_file_data_seek_t
{
	Current,
	Begin,
	End,
};

class EGFileData
{
private:

	enum class eg_t
	{
		Unk,
		NonConstByteArray,
		ConstByteArray,
		ConstBytePtr,
	};

private:
	
	EGArray<eg_byte>        m_DefaultArray;
	EGArray<eg_byte>&       m_NonConstByteArray;
	const EGArray<eg_byte>& m_ConstByteArray;
	const eg_byte*          m_BytePtr;
	eg_size_t               m_BytePtrSize;
	const eg_t              m_Type;
	mutable eg_size_t       m_ReadWritePos;
	eg_bool                 m_bCanSetData;

public:

	EGFileData( eg_file_data_init_t SpecialType );
	EGFileData( EGArray<eg_byte>& InByteArray );
	EGFileData( const EGArray<eg_byte>& InConstByteArray );
	EGFileData( const eg_byte* InConstBytePtr , eg_size_t InBytePtrSize );
	EGFileData( const void* InConstVoidPtr , eg_size_t InVoidPtrSize ): EGFileData( reinterpret_cast<const eg_byte*>(InConstVoidPtr) , InVoidPtrSize ) { }

	EGFileData() = delete;
	EGFileData( const EGFileData& rhs ) = delete;
	EGFileData( EGFileData&& rhs ) = delete;
	const EGFileData& operator = ( EGFileData&& rhs ) = delete;
	const EGFileData& operator = ( const EGFileData& rhs ) = delete;

	void SetData( const void* InCosntVoidPtr , eg_size_t InVoidPtrSize );

	eg_size_t Read( void* Out , eg_size_t ReadSize ) const;
	eg_size_t Write( const void* const In , eg_size_t WriteSize );
	eg_size_t WriteStr8( eg_cpstr8 Str8 );
	void Clear();
	void Resize( eg_size_t NewSize );

	template<typename AsType> eg_size_t Write( const AsType& InValue ) { return Write( &InValue , sizeof(InValue) ); }
	template<typename AsType> AsType Read() const { AsType Out; Read( &Out , sizeof(Out) ); return Out; }

	const void* GetDataAt( eg_size_t Position ) const;
	const void* GetData() const { return GetDataAt( 0 ); }
	const void* GetDataAtReadPos() const { return GetDataAt( m_ReadWritePos ); }
	template<typename AsType> const AsType* GetDataAtReadPosAs() const{ return reinterpret_cast<const AsType*>(GetDataAtReadPos()); }
	template<typename AsType> const AsType* GetDataAs() const { return reinterpret_cast<const AsType*>(GetData()); }
	eg_size_t GetSize() const;

	eg_int Seek( eg_file_data_seek_t SeekType , eg_int Distance ) const;
	eg_size_t Tell() const;

	eg_bool Equals( const EGFileData& rhs ) const;

private:

	static eg_t SpecialTypeToType( eg_file_data_init_t SpecialType );
};