// (c) 2018 Beem Media

#pragma once

enum class eg_asset_path_special_t
{
	None,
	Sound,
	Font,
	EntityDefinition,
	GameEntities,
	UiEntities,
};

typedef void ( * egFnGetAssetPaths )( eg_asset_path_special_t SpecialType , eg_cpstr Ext , EGArray<eg_d_string>& Out );

struct eg_asset_path
{
	eg_d_string              Path;
	eg_d_string              FullPath;
	eg_string_fixed_size<16> Ext;
	eg_bool                  bUseRelativePath = true;
	eg_asset_path_special_t  SpecialType = eg_asset_path_special_t::None;

	eg_asset_path() = default;
	eg_asset_path( eg_cpstr InExt ): Ext( InExt ){ }
	eg_asset_path( eg_cpstr InExt , eg_cpstr InPath ): Ext( InExt ) , Path(InPath) , FullPath(InPath) { }
	eg_asset_path( eg_cpstr InExt , eg_cpstr InPath , eg_bool bInUseRelativePath ): Ext( InExt ) , Path(InPath) , FullPath(InPath) , bUseRelativePath( bInUseRelativePath ) { }
	eg_asset_path( eg_asset_path_special_t InSpecialType ): SpecialType( InSpecialType ) { }
	eg_asset_path( eg_asset_path_special_t InSpecialType , eg_cpstr InPath ): SpecialType( InSpecialType ) , Path(InPath) , FullPath(InPath) { }

	void SetUseRelativePath( eg_bool bNewValue ) { bUseRelativePath = bNewValue; }
	void GetComboChoices( EGArray<eg_d_string>& Out ) const;

	eg_d_string16 GetFullPathWithExt() const;

public:

	static void SetGetAssetPathsFn( egFnGetAssetPaths InCallback ) { s_GetAssetPathsCb = InCallback; }
	static void SetCurrentFile( eg_cpstr Filename );

private:

	static egFnGetAssetPaths s_GetAssetPathsCb;
	static eg_s_string_sml8  s_CurrentFilePath;
};
