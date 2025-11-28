// (c) 2019 Beem Media

#include "EGPath2.h"

eg_d_string16 egPathParts2::GetRoot( eg_char8 Separator /*= '/' */ ) const
{
	eg_d_string16 Out( CT_Clear );
	Out.Append( Root );
	if( Root.Len() > 0 && Root[Root.Len() - 1] != L'/' )
	{
		Out.Append( Separator );
	}
	return Out;
}

eg_d_string16 egPathParts2::ToString( eg_bool bIncludeExt /*= true */, eg_char8 Separator /*= '/' */ ) const
{
	eg_d_string16 Out( CT_Clear );
	Out.Append( GetDirectory( Separator ) );
	Out.Append( GetFilename( bIncludeExt ) );
	return Out;
}

eg_d_string16 egPathParts2::GetDirectory( eg_char8 Separator /*= '/' */ ) const
{
	eg_d_string16 Out( CT_Clear );

	Out.Append( GetRoot( Separator ) );

	for( const eg_d_string16& Folder : Folders )
	{
		Out.Append( Folder );
		Out.Append( Separator );
	}

	return Out;
}

eg_d_string16 egPathParts2::GetFilename( eg_bool bIncludeExt /*= true */ ) const
{
	eg_d_string16 Out( CT_Clear );
	Out.Append( Filename );
	if( bIncludeExt && Ext.Len() > 0 )
	{
		Out.Append( '.' );
		Out.Append( Ext );
	}
	return Out;
}

void EGPath2_BreakPath( eg_cpstr16 InPath, egPathParts2& Out )
{
	Out.Clear();

	if( nullptr == InPath )
	{
		return;
	}

	auto IsSplit = []( eg_char16 c ) -> eg_bool
	{
		return c == L'/' || c == L'\\';
	};

	eg_d_string16 CurrentRead( CT_Clear );

	eg_bool bIsNetworkFolder = false;
	if( EGString_EqualsCount( InPath , L"\\\\" , 2 ) )
	{
		// Network folder:
		bIsNetworkFolder = true;
		Out.Root = L"\\\\";
	}

	for( eg_cpstr16 Reader = bIsNetworkFolder ? &InPath[2] : InPath; *Reader != L'\0'; Reader++ )
	{
		eg_char16 c = *Reader;

		if( c == ':' )
		{
			Out.Root = CurrentRead;
			Out.Root.Append( c );
			CurrentRead.Clear();
		}
		else if( IsSplit( c ) )
		{
			if( CurrentRead.Len() == 0 )
			{
				// If nothing was read either it was a double separator or it was the root.
				if( Out.Root.Len() > 0 || Out.Folders.Len() > 0 )
				{

				}
				else
				{
					Out.Root = "/";
				}
			}
			else
			{
				Out.Folders.Append( CurrentRead );
			}

			CurrentRead.Clear();
		}
		else
		{
			CurrentRead.Append( c );
		}
	}

	// Current read will be the filename at this point
	Out.Filename = CurrentRead;
	if( Out.Filename.Len() > 0 )
	{
		// We use the last '.' as the start of the ext.
		eg_size_t PeriodIndex = Out.Filename.Len();
		for( eg_size_t i = 0; i < ( Out.Filename.Len() - 1 ); i++ ) // Not subtracting 1 from length because if the last character is a period we don't treat it as an extension.
		{
			if( Out.Filename[i] == L'.' )
			{
				PeriodIndex = i;
			}
		}
		if( PeriodIndex < Out.Filename.Len() )
		{
			Out.Ext = &Out.Filename[PeriodIndex + 1];
			Out.Filename.ClampEnd( Out.Ext.Len() + 1 );
		}
	}

	if( bIsNetworkFolder && Out.Folders.IsValidIndex(0) )
	{
		Out.Root.Append( Out.Folders[0] );
		Out.Folders.DeleteByIndex( 0 );
	}
}

void EGPath2_ResolveSpecialFolders( egPathParts2& PathParts )
{
	EGArray<eg_d_string16> NewFolders;

	for( const eg_d_string16& Folder : PathParts.Folders )
	{
		if( Folder == L".." )
		{
			// Delete the topmost folder
			if( NewFolders.Len() > 0 )
			{
				NewFolders.DeleteByIndex( NewFolders.Len() - 1 );
			}
			else
			{
				// assert( false ); // Got to root..
			}
		}
		else if( Folder == L"." )
		{
			// Ignore
		}
		else
		{
			NewFolders.Append( Folder );
		}
	}

	PathParts.Folders = NewFolders;
}

eg_d_string16 EGPath2_GetFullPathRelativeTo( eg_cpstr16 RelativePath, eg_cpstr16 FullPathReference )
{
	egPathParts2 RelParts( CT_Clear );
	egPathParts2 FullRefParts( CT_Clear );

	if( RelativePath && RelativePath[0] == L'\0' )
	{
		return CT_Clear;
	}

	EGPath2_BreakPath( RelativePath, RelParts );

	if( RelParts.Root.Len() > 0 )
	{
		// If the relative path had a root, then it is good.
		EGPath2_ResolveSpecialFolders( RelParts );

		return RelParts.ToString();
	}

	EGPath2_BreakPath( FullPathReference, FullRefParts );

	RelParts.Root = FullRefParts.Root;

	EGArray<eg_d_string16> NewFolders = FullRefParts.Folders;
	NewFolders.Append( RelParts.Folders );
	RelParts.Folders = NewFolders;

	EGPath2_ResolveSpecialFolders( RelParts );

	return RelParts.ToString();
}

eg_d_string16 EGPath2_GetRelativePathTo( eg_cpstr16 FullPath, eg_cpstr16 ReferencePath, eg_bool bCaseSensitiveFolders /*= false */ )
{
	egPathParts2 FullParts( CT_Clear );
	egPathParts2 RefParts( CT_Clear );

	auto IsStrEqual = [&bCaseSensitiveFolders]( eg_cpstr16 A, eg_cpstr16 B ) -> eg_bool
	{
		return bCaseSensitiveFolders ? EGString_Equals( A, B ) : EGString_EqualsI( A, B );
	};

	EGPath2_BreakPath( FullPath, FullParts );
	EGPath2_BreakPath( ReferencePath, RefParts );

	if( !IsStrEqual( *FullParts.Root, *RefParts.Root ) )
	{
		// If the roots were not the same there is nothing to make relative
		return FullParts.ToString();
	}

	EGArray<eg_d_string16>& A = FullParts.Folders;
	EGArray<eg_d_string16>& R = RefParts.Folders;

	auto GetGreatestCommonRoot = [&A, &R, &IsStrEqual]() -> eg_size_t
	{
		const eg_size_t FolderSize = EG_Max( A.Len(), R.Len() );

		for( eg_size_t i = 0; i < FolderSize; i++ )
		{
			eg_bool bCommon = A.IsValidIndex( i ) && R.IsValidIndex( i ) && IsStrEqual( *A[i], *R[i] );

			if( !bCommon )
			{
				return i;
			}
		}

		return FolderSize;
	};

	const eg_size_t Gcr = GetGreatestCommonRoot();

	EGArray<eg_d_string16> NewPath;

	// For everything left in the reference add ".."
	for( eg_size_t i = Gcr; i < R.Len(); i++ )
	{
		NewPath.Append( L".." );
	}

	// And for everything else add the path
	for( eg_size_t i = Gcr; i < A.Len(); i++ )
	{
		NewPath.Append( A[i] );
	}

	FullParts.Root = L"";
	FullParts.Folders = NewPath;

	return FullParts.ToString();
}

eg_d_string16 EGPath2_CleanPath( eg_cpstr16 InPath, eg_char8 Separator )
{
	egPathParts2 Parts;
	EGPath2_BreakPath( InPath, Parts );
	return Parts.ToString( true, Separator );
}

eg_d_string16 EGPath2_GetFilename( eg_cpstr16 InPath, eg_bool bIncludedExt /* = true */ )
{
	egPathParts2 Parts;
	EGPath2_BreakPath( InPath, Parts );
	return Parts.GetFilename( bIncludedExt );
}
