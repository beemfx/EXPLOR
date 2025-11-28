// (c) 2019 Beem Media

#include "EGCompress.h"
#include <zlib128/zlib.h>

struct egCompressDataHeader
{
	static const eg_uint32 COMPRESS_ID = 0x4C5A4745; // "EGZL"
	
	eg_uint32 CompressId;
	eg_uint32 CompressedSize32;
	eg_uint32 UncompressedSize32;
};

static voidpf EGCompress_ZLIB_Alloc( voidpf opaque, uInt items, uInt size )
{
	unused( opaque );

	return EGMem2_Alloc( items*size , eg_mem_pool::DefaultHi );
}

static void EGCompress_ZLIB_Free( voidpf opaque, voidpf address )
{
	unused( opaque );

	EGMem2_Free( address );
}

eg_bool EGCompress_CompressData( const EGArray<eg_byte>& DataIn, EGArray<eg_byte>& DataOut )
{
	const eg_size_t DataInSize = DataIn.Len();
	EGArray<eg_byte> TempBuffer;
	TempBuffer.Resize( DataIn.Len() );
	if( TempBuffer.Len() != DataIn.Len() )
	{
		// Could not allocate destination buffer.
		return false;
	}

	z_stream stream;
	stream.next_in = (Bytef*)DataIn.GetArray();
	stream.avail_in = (uInt)DataInSize;
	stream.next_out = TempBuffer.GetArray();
	stream.avail_out = TempBuffer.LenAs<uInt>();
	stream.zalloc = EGCompress_ZLIB_Alloc;
	stream.zfree = EGCompress_ZLIB_Free;
	stream.opaque = (voidpf)0;

	int err = deflateInit( &stream, Z_DEFAULT_COMPRESSION );
	if( err != Z_OK )
	{
		return false;
	}
	err = deflate( &stream, Z_FINISH );
	if( err != Z_STREAM_END )
	{
		deflateEnd( &stream );
		return false;
	}
	const eg_size_t DataOutSize = stream.total_out;
	deflateEnd( &stream );
	DataOut.Resize( DataOutSize + sizeof(egCompressDataHeader) );
	if( (DataOut.Len()-sizeof(egCompressDataHeader)) != DataOutSize )
	{
		return false;
	}
	// Note that it is okay if DataOut is the same as DataIn as at this point we will not read from DataIn again.
	const egCompressDataHeader DataHeader = { egCompressDataHeader::COMPRESS_ID , static_cast<eg_uint32>(DataOutSize) , static_cast<eg_uint32>(DataInSize) };
	EGMem_Copy( DataOut.GetArray() , &DataHeader , sizeof(egCompressDataHeader) );
	EGMem_Copy( DataOut.GetArray() + sizeof(egCompressDataHeader) , TempBuffer.GetArray() , DataOutSize );
	return true;
}

eg_bool EGCompress_DecompressData( const EGArray<eg_byte>& DataIn, EGArray<eg_byte>& DataOut )
{
	if( DataIn.Len() < sizeof(egCompressDataHeader) )
	{
		// Bad data not even enough room for header.
		return false;
	}

	egCompressDataHeader DataHeader = { 0 , 0 };
	EGMem_Copy( &DataHeader , DataIn.GetArray() , sizeof(egCompressDataHeader) );
	if( DataHeader.CompressId != egCompressDataHeader::COMPRESS_ID )
	{
		return false;
	}

	EGArray<eg_byte> TempBuffer;
	TempBuffer.Resize( DataHeader.UncompressedSize32 );

	if( TempBuffer.Len() != DataHeader.UncompressedSize32 )
	{
		return false;
	}

	z_stream in_stream;
	in_stream.zalloc = EGCompress_ZLIB_Alloc;
	in_stream.zfree = EGCompress_ZLIB_Free;
	in_stream.opaque = (voidpf)0;

	in_stream.next_out = TempBuffer.GetArray();
	in_stream.avail_out = TempBuffer.LenAs<uInt>();
	in_stream.next_in = const_cast<z_const Bytef*>(static_cast<const Bytef*>(DataIn.GetArray() + sizeof(egCompressDataHeader)));
	in_stream.avail_in = DataIn.LenAs<uInt>() - sizeof(egCompressDataHeader);

	int err = inflateInit( &in_stream );
	if( Z_OK != err )
	{
		return false;
	}

	err = inflate( &in_stream, Z_FINISH );
	const eg_size_t DataSizeRead = in_stream.total_out;
	if( DataSizeRead == DataHeader.UncompressedSize32 )
	{
		DataOut.Resize( DataSizeRead );
		if( DataOut.Len() == DataSizeRead )
		{
			EGMem_Copy( DataOut.GetArray() , TempBuffer.GetArray() , DataSizeRead );
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	inflateEnd( &in_stream );
	return true;
}

eg_bool EGCompress_IsCompressedData( const EGArray<eg_byte>& Data )
{
	if( Data.Len() < sizeof(egCompressDataHeader) )
	{
		// Bad data not even enough room for header.
		return false;
	}

	egCompressDataHeader DataHeader = { 0 , 0 };
	EGMem_Copy( &DataHeader , Data.GetArray(), sizeof(egCompressDataHeader) );
	if( DataHeader.CompressId != egCompressDataHeader::COMPRESS_ID )
	{
		return false;
	}

	return true;
}
