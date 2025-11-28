// (c) 2019 Beem Media

#pragma once

// Compression is implemented in such a way than DataIn and DataOut can be the same.
eg_bool EGCompress_CompressData( const EGArray<eg_byte>& DataIn , EGArray<eg_byte>& DataOut );
eg_bool EGCompress_DecompressData( const EGArray<eg_byte>& DataIn , EGArray<eg_byte>& DataOut );
eg_bool EGCompress_IsCompressedData( const EGArray<eg_byte>& Data );
