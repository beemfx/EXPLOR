// (c) 2018 Beem Media

#include "EGAssetPath.h"
#include "EGPath2.h"

void eg_asset_path::SetCurrentFile( eg_cpstr Filename )
{
	s_CurrentFilePath = *EGPath2_BreakPath( Filename ).GetDirectory();
}

egFnGetAssetPaths eg_asset_path::s_GetAssetPathsCb = nullptr;
eg_s_string_sml8 eg_asset_path::s_CurrentFilePath = "";

void eg_asset_path::GetComboChoices( EGArray<eg_d_string>& Out ) const
{
	Out.Append( "" );
	if( s_GetAssetPathsCb )
	{
		s_GetAssetPathsCb( SpecialType , Ext.String() , Out );
	}

	if( SpecialType == eg_asset_path_special_t::None && bUseRelativePath && s_CurrentFilePath.Len() )
	{
		eg_d_string RelativeFilename;
		for( eg_d_string& Filename : Out )
		{
			RelativeFilename = EGPath2_GetRelativePathTo( *Filename , *s_CurrentFilePath );

			// Don't make paths in sub directories relative.
			if( !EGString_EqualsCount( *RelativeFilename , ".." , 2 ) )
			{
				Filename = RelativeFilename;
			}
		}

		// This sort will make relative filenames show up first, then full paths.
		auto FilenameSortCb = []( const eg_d_string& Left , const eg_d_string& Right ) -> eg_bool
		{
			if( Left.Len() > 1 && Right.Len() > 1 )
			{
				if( Left[0] == '/' && Right[0] != '/' )
				{
					return false;
				}

				if( Left[0] != '/' && Right[0] == '/' )
				{
					return true;
				}
			}

			return Left < Right;
		};

		Out.Sort( FilenameSortCb );
	}
}

eg_d_string16 eg_asset_path::GetFullPathWithExt() const
{
	eg_d_string16 Out = EGSFormat16( L"{0}.{1}" , *FullPath , *Ext );

	return Out;
}
