/******************************************************************************
	IRawAudioFile - Represents actual audio data.

	(c) 2011 Beem Software
******************************************************************************/
#pragma once

class EGAudioFile : public EGObject
{
	EG_ABSTRACT_CLASS_BODY( EGAudioFile , EGObject )

public: 

	static EGAudioFile* OpenAF( void* Data , eg_size_t DataSize );
	static void CloseAF(EGAudioFile*& pFile);

public:

	virtual eg_uint Read(void* pBuffer, eg_uint nSize)=0;
	virtual void Reset()=0;
	virtual eg_uint GetNumChannels()=0;
	virtual eg_uint GetBitsPerSample()=0;
	virtual eg_uint GetSamplesPerSecond()=0;
	virtual eg_uint GetDataSize()=0;
	virtual eg_bool IsEOF()=0;

protected:

	virtual eg_bool Open( void* Data , eg_size_t DataSize )=0;
};
