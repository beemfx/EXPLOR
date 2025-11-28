#include "EGTGAWriter.h"

EGTgaWriter::EGTgaWriter(eg_uint Width, eg_uint Height)
: m_Width(Width)
, m_Height(Height)
, m_pChunk(nullptr)
, m_pHeader(nullptr)
, m_pData(nullptr)
, m_FileSize(Width*Height*sizeof(egPixel) + HEADER_SIZE)
{
	m_pChunk = new eg_byte[m_FileSize];
	m_pHeader = m_pChunk;
	m_pData = reinterpret_cast<egPixel*>(&m_pChunk[HEADER_SIZE]);

	//Prepare the header:
	//Setup the header:
	EGMem_Set(m_pHeader,0,HEADER_SIZE);
	//Setup the tga header...
	m_pHeader[2]=2;
	*(eg_uint16*)(&m_pHeader[12])=static_cast<eg_uint16>(m_Width);
	*(eg_uint16*)(&m_pHeader[14])=static_cast<eg_uint16>(m_Height);
	m_pHeader[16]=8*sizeof(egPixel);
	m_pHeader[17]=0x28; //I know 0x20 flips the image rows, I think the 0x08 may be necessary for propper alpha, but the DX Texture Tool doesn't seem to care.

	//Reset the image to all black.
	for(eg_uint i=0; i<m_Width*m_Height; i++)
	{
		m_pData[i].a = 0xFF;
		m_pData[i].r = 0x00;
		m_pData[i].g = 0x00;
		m_pData[i].b = 0x00;
	}
}

EGTgaWriter::~EGTgaWriter()
{
	delete [] m_pChunk;
}

void EGTgaWriter::WritePixel(eg_uint x, eg_uint y, const egPixel& Color)
{
	if( m_Width > 0 && m_Height > 0 && EG_IsBetween<eg_uint>( x , 0 , m_Width-1 ) && EG_IsBetween<eg_uint>( y , 0 , m_Height - 1 ) )
	{
		//We are going to assume the pixel was in range, and only throw it out
		//if it is out of bounds.
		const eg_uint nPos = y*m_Width + x;
		if(nPos >= m_Width*m_Height)return;

		m_pData[nPos] = Color;
	}
}
