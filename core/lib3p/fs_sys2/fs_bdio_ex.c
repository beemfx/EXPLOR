/**********************************************************************
*** fs_bdio_ex.c - Includes extra functions for basic input output
***
**********************************************************************/
#include "fs_bdio.h"
#include "fs_internal.h"
#include <zlib.h>

voidpf ZLIB_Alloc(voidpf opaque, uInt items, uInt size)
{
	return FS_Malloc(items*size, LF_ALLOC_REASON_SCRATCH, "FS_ZLIB",__FILE__,__LINE__);
}

void ZLIB_Free(voidpf opaque, voidpf address)
{
	FS_Free(address, LF_ALLOC_REASON_SCRATCH);
}


fs_dword BDIO_WriteCompressed(BDIO_FILE File, fs_dword nSize, void* pInBuffer)
{
	fs_dword nWritten=0;
	z_stream stream;
	int err;
	fs_byte* pCmpBuffer=FS_Malloc(nSize, LF_ALLOC_REASON_SCRATCH, "FS", __FILE__, __LINE__);
	if(!pCmpBuffer)
		return 0;
		
	stream.next_in=(Bytef*)pInBuffer;
	stream.avail_in=(uInt)nSize;
	stream.next_out=pCmpBuffer;
	stream.avail_out=nSize;
	stream.zalloc=ZLIB_Alloc;
	stream.zfree=ZLIB_Free;
	stream.opaque=(voidpf)0;
	
	err=deflateInit(&stream, Z_DEFAULT_COMPRESSION);
	if(err!=Z_OK)
	{
		FS_Free(pCmpBuffer, LF_ALLOC_REASON_SCRATCH);
		return 0;
	}
	err=deflate(&stream, Z_FINISH);
	if(err!=Z_STREAM_END)
	{
		deflateEnd(&stream);
		FS_Free(pCmpBuffer, LF_ALLOC_REASON_SCRATCH);
		return 0;
	}
	nWritten=stream.total_out;
	deflateEnd(&stream);
	nWritten=BDIO_Write(File, nWritten, pCmpBuffer);
	
	FS_Free(pCmpBuffer, LF_ALLOC_REASON_SCRATCH);
	return nWritten;
}


fs_dword BDIO_ReadCompressed(BDIO_FILE File, fs_dword nSize, void* pInBuffer)
{
	//The BDIO_ReadCompressed function is somewhat complicated.  Unlike the
	//BDIO_WriteCompressed method. Also note that this function returns the 
	//size of the data that was decompressed, and not the actual amount of 
	//data read from the file.
	z_stream in_stream;
	int err;
	fs_dword nRead;
	fs_dword nPos;
	fs_byte* pCmpBuffer=FS_Malloc(nSize, LF_ALLOC_REASON_SCRATCH, "FS", __FILE__, __LINE__);
	
	if(!pCmpBuffer)
		return 0;
	
	nPos=BDIO_Tell(File);
	nRead=BDIO_Read(File, nSize, pCmpBuffer);
	
	in_stream.zalloc=ZLIB_Alloc;
	in_stream.zfree=ZLIB_Free;
	in_stream.opaque=(voidpf)0;
		
	in_stream.next_out=pInBuffer;
	in_stream.avail_out=nSize;
	in_stream.next_in=pCmpBuffer;
	in_stream.avail_in=nRead;
		
	err=inflateInit(&in_stream);
	if(Z_OK != err)
	{
		FS_Free(pCmpBuffer, LF_ALLOC_REASON_SCRATCH);
		BDIO_Seek(File, nPos, LF_SEEK_BEGIN);
		return 0;
	}
	
	err=inflate(&in_stream, Z_FINISH);
	nRead=in_stream.total_out;
	BDIO_Seek(File, nPos+in_stream.total_in, LF_SEEK_BEGIN);
	inflateEnd(&in_stream);
	
	FS_Free(pCmpBuffer, LF_ALLOC_REASON_SCRATCH);
	return nRead;
}

fs_dword BDIO_CopyData(BDIO_FILE DestFile, BDIO_FILE SourceFile, fs_dword nSize)
{
	fs_dword i=0;
	fs_byte byte=0;
	fs_dword nWritten=0;
	fs_byte* pData=FS_Malloc(nSize, LF_ALLOC_REASON_SCRATCH, "FS", __FILE__,__LINE__);
	if(!pData)
		return 0;
		
	nWritten=BDIO_Read(SourceFile, nSize, pData);
	nWritten=BDIO_Write(DestFile, nWritten, pData);
	FS_Free(pData, LF_ALLOC_REASON_SCRATCH);
	return nWritten;
}

