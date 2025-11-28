/******************************************************************************
File: ETContainer.cpp
Class: EGEntDict
Purpose: See header.

(c) 2011 Beem Software
******************************************************************************/

#include "EGEntDict.h"
#include "EGEnt.h"
#include "EGEngineTemplates.h"
#include "EGAlwaysLoaded.h"
#include "EGTimer.h"

static class EGEntDict2
{
private:
	typedef eg_byte egEntMemChunk[sizeof(EGEntDef)];
private:
	EGSysMemItemMap<EGEntDef*> m_SearchTree;
	//Memory chunk for the definitions:
	egEntMemChunk* m_Chunks;
	eg_size_t      m_ChunksSize;
	eg_size_t      m_NextChunk;

	static const eg_string_crc  ALWAYS_LOADED_DEFS[3];

public:
	EGEntDict2()
	: m_SearchTree()
	, m_NextChunk(0)
	, m_Chunks(nullptr)
	, m_ChunksSize(0)
	{

	}

	~EGEntDict2()
	{

	}

	void Init( void )
	{
		Deinit();

		eg_uint64 StartTime = Timer_GetRawTime();

		EGAlwaysLoadedFilenameList EnumList;
		AlwaysLoaded_GetFileList( &EnumList , "edef" );

		m_Chunks = EGMem2_NewArray<egEntMemChunk>( EnumList.Len() , eg_mem_pool::System );
		if( m_Chunks )
		{
			m_ChunksSize = EnumList.Len();
		}
		m_SearchTree.Init( EnumList.Len() );
		for( egAlwaysLoadedFilename* FnItem : EnumList )
		{
			LoadDef( FnItem->Filename );
		}

		for( eg_size_t i =0; i<countof(ALWAYS_LOADED_DEFS); i++ )
		{
			const EGEntDef* MaskDef = GetDef( ALWAYS_LOADED_DEFS[i] );
			if( MaskDef )
			{
				MaskDef->CreateAssets();
			}
		}

		eg_uint64 EndTime = Timer_GetRawTime();
		eg_real LoadingSeconds = Timer_GetRawTimeElapsedSec( StartTime , EndTime );
		EGLogf( eg_log_t::Performance , "Entity definitions loaded in %g seconds." , LoadingSeconds );

		// ShowInfo();
	}

	void Deinit( void )
	{
		for( eg_size_t i =0; i<countof(ALWAYS_LOADED_DEFS); i++ )
		{
			const EGEntDef* MaskDef = GetDef( ALWAYS_LOADED_DEFS[i] );
			if( MaskDef )
			{
				MaskDef->DestroyAssets();
			}
		}
		
		for(eg_uint i=0; i<m_SearchTree.Len(); i++)
		{
			EGEntDef* pDef = m_SearchTree.GetByIndex(i);
			if(pDef)
			{
				pDef->~EGEntDef();
				pDef=nullptr;
			}
		}
		m_NextChunk = 0;
		if( m_Chunks )
		{
			EGMem2_Free( m_Chunks );
			m_Chunks = nullptr;
		}
		m_ChunksSize = 0;
		m_SearchTree.Clear();
		m_SearchTree.Deinit();
	}

	void Update( void )
	{
		
	}

	const EGEntDef* GetDef( eg_string_crc TemplateCrcId )
	{
		return m_SearchTree.Get( TemplateCrcId );
	}

	void ShowInfo( void )
	{
		EGLogf( eg_log_t::General , "Entity Definitions:" );
		for(eg_uint i=0; i<m_SearchTree.Len(); i++)
		{
			eg_string Name = *m_SearchTree.GetByIndex(i)->m_Id;
			eg_string_crc CrcId = eg_string_crc(Name);
			eg_uint32 Value = CrcId.ToUint32();
			EGLogf( eg_log_t::General , "\t\"%s\" (0x%08X)", Name.String() , Value );
		}
	}

	void GetDefStrings( EGArray<eg_string>& ArrayOut , eg_bool bIncludeGameClasses , eg_bool bIncludeUiClasses )
	{
		ArrayOut.Clear();
		for(eg_uint i=0; i<m_SearchTree.Len(); i++)
		{
			EGEntDef* Def = m_SearchTree.GetByIndex(i);
			eg_bool bWants = false;
			switch( Def->m_ClassType )
			{
				case eg_ent_class_t::None:
					bWants = false;
					break;
				case eg_ent_class_t::Game:
					bWants = bIncludeGameClasses;
					break;
				case eg_ent_class_t::UI:
					bWants = bIncludeUiClasses;
					break;
				case eg_ent_class_t::GameAndUI:
					bWants = bIncludeGameClasses || bIncludeUiClasses;
					break;
			}
			if( bWants )
			{
				eg_string Name = *Def->m_Id;
				ArrayOut.Append( Name );
			}
		}
	}
private:
	void LoadDef( eg_cpstr Filename )
	{
		if( m_NextChunk < m_ChunksSize )
		{
			EGEntDef* pDef = nullptr;
			eg_byte* pChunk = m_Chunks[m_NextChunk];
			m_NextChunk++;
			pDef = new(pChunk) EGEntDef(Filename);
			if( !m_SearchTree.Contains( eg_string_crc(*pDef->m_Id) ) )
			{
				m_SearchTree.Insert(eg_string_crc(*pDef->m_Id), pDef);
			}
			else
			{
				assert( false ); //Two edefs have the same name? Or crc collision.
				m_NextChunk--;
				pDef->~EGEntDef();
			}
		}
		else
		{
			assert( false ); //No more room for entity definitions, need to increase the size of MAX_ENT_DEFS?
		}
	}

} EntDict;

const eg_string_crc EGEntDict2::ALWAYS_LOADED_DEFS[] = { eg_crc("Mask1x1") , eg_crc("EGUIObjectImage1x1") , eg_crc("TextureWithPalette1x1") };


void EntDict_Init( void ){ EntDict.Init(); }
void EntDict_Deinit( void ){ EntDict.Deinit(); }
void EntDict_Update( void ){ EntDict.Update(); }
const EGEntDef* EntDict_GetDef( eg_string_crc TemplateCrcId ){ return EntDict.GetDef( TemplateCrcId ); }
void EntDict_ShowInfo( void ) { EntDict.ShowInfo(); }

void EntDict_GetDefStrings( EGArray<eg_string>& ArrayOut , eg_bool bIncludeGameClasses , eg_bool bIncludeUiClasses )
{
	EntDict.GetDefStrings( ArrayOut , bIncludeGameClasses , bIncludeUiClasses );
}
