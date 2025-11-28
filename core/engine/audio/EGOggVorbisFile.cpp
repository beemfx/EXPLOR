/******************************************************************************
File: OggVorbisFile.cpp
Class: EGOggVorbisFile
Purpose: See header.

(c) 2011 Beem Software
******************************************************************************/

#include "EGOggVorbisFile.h"
#include "EGLoader.h"
#include "EGFileData.h"

EG_CLASS_DECL( EGOggVorbisFile )

static const eg_bool OGG_VORBIS_KEEP_DATA_COPY = false;

static const eg_uint OGG_VORBIS_BPS=16;

EGOggVorbisFile::EGOggVorbisFile()
: m_MemFile(eg_file_data_init_t::SetableUserPointer)
, m_FileData(nullptr)
, m_FileDataSize(0)
, m_bEof(true)
, m_bOpen(false)
{
	zero( &m_vinfo );
}

eg_bool EGOggVorbisFile::Open( void* Data , eg_size_t DataSize )
{
	assert( !m_bOpen );

	m_bOpen = false;
	m_bEof = true;

	ov_callbacks cb = { 0 };


	cb.read_func  = VFMem_Read;
	cb.seek_func  = VFMem_Seek;
	cb.close_func = VFMem_Close;
	cb.tell_func  = VFMem_Tell;

	{
		if( OGG_VORBIS_KEEP_DATA_COPY )
		{
			EGFileData MemFile( Data , DataSize );
			m_FileDataSize = MemFile.GetSize();
			assert( m_FileDataSize > 0 );
			m_FileData = EGMem2_Alloc( m_FileDataSize , eg_mem_pool::Audio );
			EGMem_Copy( m_FileData ,MemFile.GetData() , m_FileDataSize );
		}
		else
		{
			m_FileDataSize = DataSize;
			m_FileData     = Data;
		}
	}

	m_MemFile.SetData( m_FileData , m_FileDataSize );


	/* We don't ever call the File_Close function here,
		because the callbacks functions will call it
		on VF_Close callback. */
	int nErr=ov_open_callbacks(this,&m_VorbisFile,nullptr,OGG_VORBIS_BPS/8,cb);
		
	if(nErr<0)
	{
		m_MemFile.SetData( nullptr , 0 );
		EGMem2_Free( m_FileData );
		m_FileData = nullptr;
		m_FileDataSize = 0;
		return false;
	}

	//Get info about the stream;
	m_vinfo = *ov_info(&m_VorbisFile, -1);
	m_nNumSamples=(long)ov_pcm_total(&m_VorbisFile, -1);
	
	m_Format.wFormatTag=WAVE_FORMAT_PCM;
	m_Format.nChannels=m_vinfo.channels;
	m_Format.nSamplesPerSec=m_vinfo.rate;
	m_Format.wBitsPerSample=OGG_VORBIS_BPS;
	m_Format.nBlockAlign=m_Format.nChannels*m_Format.wBitsPerSample/8;
	m_Format.nAvgBytesPerSec=m_Format.nSamplesPerSec*m_Format.nBlockAlign;
	m_Format.cbSize=0;

	m_bEof = false;
	m_bOpen = true;
	return true;
}


void EGOggVorbisFile::OnDestruct()
{
	//Close the actual ogg file.
	if(m_bOpen)
	{
		ov_clear(&m_VorbisFile);
	}
	//Always set m_vinfo to zero so that, the functions
	//that get information about the vile will return 0
	//if the file is closed.
	zero(&m_vinfo);
	//Set the flag to eof, that way both open will be false
	//and reads will show that the file is at the end.
	m_bEof = true;
}

eg_uint EGOggVorbisFile::Read(void* pBuffer, eg_uint nSize)
{
	if(!m_bOpen || !pBuffer)
		return 0;
		
	char* pCurBuffer=(char*)pBuffer;
	eg_uint nBytesRead=0;
	int iSection=0;

	while((nBytesRead<nSize) && !m_bEof)
	{
		eg_int iRet=ov_read(&m_VorbisFile,pCurBuffer,nSize-nBytesRead,0,OGG_VORBIS_BPS/8,1,&iSection);
		
		if(iRet==0 || iSection !=0)
		{
			m_bEof = true;
		}

		nBytesRead+=iRet;
		pCurBuffer+=iRet;
	}
	return nBytesRead;
}

void EGOggVorbisFile::Reset()
{
	if(!m_bOpen)
		return;

	int Res = ov_pcm_seek(&m_VorbisFile, 0);
	assert( 0 == Res );
	m_bEof = false;
}

eg_uint EGOggVorbisFile::GetNumChannels()
{
	return m_vinfo.channels;
}

eg_uint EGOggVorbisFile::GetBitsPerSample()
{
	//Vorbis is always 16.
	return OGG_VORBIS_BPS;
}

eg_uint EGOggVorbisFile::GetSamplesPerSecond()
{
	return m_vinfo.rate;
}

eg_uint EGOggVorbisFile::GetDataSize()
{
	return m_nNumSamples*m_vinfo.channels*GetBitsPerSample()/8;
}

eg_bool  EGOggVorbisFile::IsEOF()
{
	return m_bEof;
}

/*************************
*** Reading functions. ***
*************************/

size_t EGOggVorbisFile::VFMem_Read(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	EGOggVorbisFile* _this = reinterpret_cast<EGOggVorbisFile*>(datasource);
	return _this->m_MemFile.Read( ptr , size*nmemb );
}

int EGOggVorbisFile::VFMem_Seek(void* datasource, ogg_int64_t offset, int whence)
{
	EGOggVorbisFile* _this = reinterpret_cast<EGOggVorbisFile*>(datasource);

	eg_file_data_seek_t nMode = eg_file_data_seek_t::Begin;

	switch(whence)
	{
		case SEEK_SET: nMode = eg_file_data_seek_t::Begin; break;
		case SEEK_CUR: nMode = eg_file_data_seek_t::Current; break;
		case SEEK_END: nMode = eg_file_data_seek_t::End; break;
		default: assert(false); return -1;
	}
		
	_this->m_MemFile.Seek( nMode , static_cast<eg_int>(offset) );
	return 0;
}

int EGOggVorbisFile::VFMem_Close(void* datasource)
{
	EGOggVorbisFile* _this = reinterpret_cast<EGOggVorbisFile*>(datasource);

	_this->m_MemFile.SetData( nullptr , 0 );
	if( OGG_VORBIS_KEEP_DATA_COPY )
	{
		EGMem2_Free( _this->m_FileData );
	}
	_this->m_FileData = nullptr;
	_this->m_FileDataSize = 0;

	return 0;
}

long EGOggVorbisFile::VFMem_Tell(void* datasource)
{
	EGOggVorbisFile* _this = reinterpret_cast<EGOggVorbisFile*>(datasource);
	return static_cast<long>(_this->m_MemFile.Tell());
}
