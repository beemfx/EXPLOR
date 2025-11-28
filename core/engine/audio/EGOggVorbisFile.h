/******************************************************************************
	EGOggVorbisFile - A type of IRawAudioFile.

	(c) 2011 Beem Software
******************************************************************************/
#pragma once

#include "EGAudioFile.h"
#include "EGFileData.h"

#include <ogg/ogg.h>
#pragma warning(disable: 4244)
#include <vorbis/vorbisfile.h>
#pragma warning(default: 4244)
#include "EGWindowsAPI.h"

class EGOggVorbisFile : public EGAudioFile
{
	EG_CLASS_BODY( EGOggVorbisFile , EGAudioFile )

public:

	EGOggVorbisFile();
	
private:

	// BEGIN EGRawAudioFile
	virtual eg_uint Read(void* pBuffer, eg_uint nSize) override final;
	virtual void    Reset() override final;
	virtual eg_uint GetNumChannels() override final;
	virtual eg_uint GetBitsPerSample() override final;
	virtual eg_uint GetSamplesPerSecond() override final;
	virtual eg_uint GetDataSize() override final;
	virtual eg_bool IsEOF() override final;
	virtual eg_bool Open( void* Data , eg_size_t DataSize );
	virtual void OnDestruct() override final;
	// END EGRawAudioFile


private:

	WAVEFORMATEX   m_Format;
	OggVorbis_File m_VorbisFile;
	vorbis_info    m_vinfo;
	eg_uint        m_nNumSamples;
	EGFileData     m_MemFile;
	void*          m_FileData;
	eg_size_t      m_FileDataSize;
	eg_bool        m_bOpen:1;
	eg_bool        m_bEof:1;

private:
	
	static size_t VFMem_Read(void* ptr, size_t size, size_t nmemb, void* datasource);
	static int    VFMem_Seek(void* datasource, ogg_int64_t offset, int whence);
	static int    VFMem_Close(void* datasource);
	static long   VFMem_Tell(void* datasource);
};