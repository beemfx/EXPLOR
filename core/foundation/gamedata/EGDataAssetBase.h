// (c) 2018 Beem Media

#pragma once

class EGFileData;
struct egRflEditor;

egreflect class EGDataAsset : public EGObject
{
	EG_CLASS_BODY( EGDataAsset , EGObject )

public:

	virtual void OnPropChanged( const egRflEditor& ChangedProperty , egRflEditor& RootEditor , eg_bool& bNeedsRebuildOut ) { unused( ChangedProperty , RootEditor ); bNeedsRebuildOut = false; }

protected:

	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor );

public:

	void SaveGameDataTo( EGFileData& DataOut );

	static EGDataAsset* CreateDataAsset( EGClass* Class , eg_mem_pool MemPool , egRflEditor& RflEditor );
	static EGDataAsset* LoadDataAsset( eg_cpstr16 Filename , eg_mem_pool MemPool , eg_bool bForEditor , egRflEditor& RflEditor );
	static EGDataAsset* LoadDataAsset( const EGFileData& MemFile , eg_cpstr16 RefFilename , eg_mem_pool MemPool , eg_bool bForEditor , egRflEditor& RflEditor );
	static eg_bool SaveDataAsset( EGFileData& FileDataOut , EGDataAsset* DataAsset , const egRflEditor& RflEditor );
	static eg_bool SaveDataAsset( eg_cpstr16 Filename , EGDataAsset* DataAsset , const egRflEditor& RflEditor );

	template<class RetType>
	static RetType* LoadDataAsset( eg_cpstr16 Filename , eg_bool bCreateNewIsOkay = false )
	{
		EGDataAsset* LoadedAsset = LoadDataAsset( Filename , eg_mem_pool::System , false , egRflEditor() );

		if( LoadedAsset && !LoadedAsset->IsA( &RetType::GetStaticClass() ) )
		{
			EGLogf( eg_log_t::Error , "The %s data was not the right class!" , eg_cpstr(EGString_ToMultibyte( Filename )) );
			assert( false );
			EGDeleteObject( LoadedAsset );
			LoadedAsset = nullptr;
		}

		if( nullptr == LoadedAsset )
		{
			if( !bCreateNewIsOkay )
			{
				EGLogf( eg_log_t::Error , "The %s data was not loaded. Creating default data." , eg_cpstr(EGString_ToMultibyte( Filename )) );
				assert( false );
			}
			LoadedAsset = EGNewObject<RetType>( eg_mem_pool::System );
		}

		if( LoadedAsset && LoadedAsset->IsA( &RetType::GetStaticClass() ) )
		{
			return EGCast<RetType>( LoadedAsset );
		}

		assert( false ); // Wrong type...
		if( LoadedAsset )
		{
			EGDeleteObject( LoadedAsset );
		}
		return nullptr;
	}

	template<class RetType>
	static RetType* LoadDataAsset( const EGFileData& MemFile , eg_cpstr16 RefFilename , eg_bool bCreateNewIsOkay = false )
	{
		EGDataAsset* LoadedAsset = LoadDataAsset( MemFile , RefFilename , eg_mem_pool::System , false , egRflEditor() );

		if( LoadedAsset && !LoadedAsset->IsA( &RetType::GetStaticClass() ) )
		{
			EGLogf( eg_log_t::Error , "The %s data was not the right class!" , eg_cpstr(EGString_ToMultibyte( RefFilename )) );
			assert( false );
			EGDeleteObject( LoadedAsset );
			LoadedAsset = nullptr;
		}

		if( nullptr == LoadedAsset )
		{
			if( !bCreateNewIsOkay )
			{
				EGLogf( eg_log_t::Error , "The %s data was not loaded. Creating default data." , eg_cpstr(EGString_ToMultibyte( RefFilename )) );
				assert( false );
			}
			LoadedAsset = EGNewObject<RetType>( eg_mem_pool::System );
		}

		if( LoadedAsset && LoadedAsset->IsA( &RetType::GetStaticClass() ) )
		{
			return EGCast<RetType>( LoadedAsset );
		}

		assert( false ); // Wrong type...
		if( LoadedAsset )
		{
			EGDeleteObject( LoadedAsset );
		}
		return nullptr;
	}
};
