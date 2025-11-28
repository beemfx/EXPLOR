// (c) 2019 Beem Media

#pragma once

struct egPathParts2
{
	EGArray<eg_d_string16> Folders;
	eg_d_string16 Root;
	eg_d_string16 Filename;
	eg_d_string16 Ext;

	egPathParts2() { }
	egPathParts2( eg_ctor_t Ct ) :egPathParts2() { unused( Ct ); assert( Ct != CT_Preserve ); }
	void Clear()
	{
		Root.Clear();
		Folders.Clear();
		Filename.Clear();
		Ext.Clear();
	}
	eg_d_string16 GetRoot( eg_char8 Separator = '/' ) const;
	eg_d_string16 ToString( eg_bool bIncludeExt = true, eg_char8 Separator = '/' ) const;
	eg_d_string16 GetDirectory( eg_char8 Separator = '/' ) const;
	eg_d_string16 GetFilename( eg_bool bIncludeExt = true ) const;

	eg_d_string8 GetRoot8( eg_char8 Separator = '/' ) const { return eg_d_string8(*GetRoot( Separator )); }
	eg_d_string8 ToString8( eg_bool bIncludeExt = true, eg_char8 Separator = '/' ) const { return eg_d_string8(*ToString(bIncludeExt,Separator)); }
	eg_d_string8 GetDirectory8( eg_char8 Separator = '/' ) const { return eg_d_string8(*GetDirectory(Separator)); }
	eg_d_string8 GetFilename8( eg_bool bIncludeExt = true ) const { return eg_d_string8(*GetFilename(bIncludeExt)); }
};

void EGPath2_BreakPath( eg_cpstr16 InPath, egPathParts2& Out );
static inline egPathParts2 EGPath2_BreakPath( eg_cpstr16 InPath ) { egPathParts2 Out; EGPath2_BreakPath( InPath, Out ); return Out; }
void EGPath2_ResolveSpecialFolders( egPathParts2& PathParts );
eg_d_string16 EGPath2_GetFullPathRelativeTo( eg_cpstr16 RelativePath, eg_cpstr16 FullPathReference );
eg_d_string16 EGPath2_GetRelativePathTo( eg_cpstr16 FullPath, eg_cpstr16 ReferencePath, eg_bool bCaseSensitiveFolders = false );
eg_d_string16 EGPath2_CleanPath( eg_cpstr16 InPath, eg_char8 Separator );
eg_d_string16 EGPath2_GetFilename( eg_cpstr16 InPath, eg_bool bIncludeExt = true );

static inline egPathParts2 EGPath2_BreakPath( eg_cpstr8 InPath ) { egPathParts2 Out; EGPath2_BreakPath( *eg_d_string16(InPath) , Out ); return Out; }
static inline eg_d_string8 EGPath2_GetFullPathRelativeTo( eg_cpstr8 RelativePath, eg_cpstr8 FullPathReference ) { return *EGPath2_GetFullPathRelativeTo( *eg_d_string16(RelativePath) , *eg_d_string16(FullPathReference) ); }
static inline eg_d_string8 EGPath2_GetRelativePathTo( eg_cpstr8 FullPath, eg_cpstr8 ReferencePath, eg_bool bCaseSensitiveFolders = false ) { return *EGPath2_GetRelativePathTo( *eg_d_string16(FullPath) , *eg_d_string16(ReferencePath) , bCaseSensitiveFolders ); }
static inline eg_d_string8 EGPath2_CleanPath( eg_cpstr8 InPath, eg_char8 Separator ) { return *EGPath2_CleanPath( *eg_d_string16(InPath) , Separator ); }
static inline eg_d_string8 EGPath2_GetFilename( eg_cpstr8 InPath, eg_bool bIncludeExt = true ) { return *EGPath2_GetFilename( *eg_d_string16(InPath) , bIncludeExt ); }
