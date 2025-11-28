// ExSpellBook
// (c) 2016 Beem Media

#include "ExSpellBook.h"
#include "EGCrcDb.h"
#include "ExGame.h"

EG_CLASS_DECL( ExSpellBook )

ExSpellBook* ExSpellBook::s_Inst = nullptr;

void ExSpellBook::Init( eg_cpstr Filename )
{
	assert( s_Inst == nullptr );
	s_Inst = EGDataAsset::LoadDataAsset<ExSpellBook>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Will crash.
	Get();
}

void ExSpellBook::Deinit()
{
	assert( nullptr != s_Inst );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
		s_Inst = nullptr;
	}
}

void ExSpellBook::GetAll( ExSpellList& AllOut ) const
{
	AllOut = m_AllSpells;	
}

const exSpellInfo& ExSpellBook::FindInfo( const eg_string_crc& SpellId ) const
{
	for( const exSpellInfo* SpellInfo : m_AllSpells )
	{
		if( SpellInfo->Id == SpellId )
		{
			return *SpellInfo;
		}
	}
	return m_DefaultSpell;
}

void ExSpellBook::PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor  , RflEditor );

	if( bForEditor )
	{
		RefreshVisibleProperties( RflEditor );
	}

	if( !bForEditor )
	{
		m_AllSpells.Clear();
		for( const exSpellBookCategory& Cat : m_Categories )
		{
			for( const exSpellInfo& SpellInfo : Cat.Spells )
			{
				m_AllSpells.Append( &SpellInfo );
			}
		}
	}
}

void ExSpellBook::OnPropChanged( const egRflEditor& ChangedProperty , egRflEditor& RootEditor, eg_bool& bNeedsRebuildOut )
{
	Super::OnPropChanged( ChangedProperty , RootEditor , bNeedsRebuildOut );

	if( EGString_Equals( ChangedProperty.GetVarName() , "m_Spells" ) || EGString_Equals( ChangedProperty.GetVarName() , "Spells" ) || EGString_Equals( ChangedProperty.GetVarName() , "m_Categories" ) )
	{
		bNeedsRebuildOut = true;
		RefreshVisibleProperties( RootEditor );
	}
}

void ExSpellBook::RefreshVisibleProperties( egRflEditor& RootEditor )
{
	{
		egRflEditor* SpellsEd = RootEditor.GetChildPtr( "m_Spells" );
		if( SpellsEd )
		{
			const eg_size_t NumChildren = SpellsEd->GetNumChildren();
			for( eg_size_t i=0; i<NumChildren; i++ )
			{
				egRflEditor* Spell = SpellsEd->GetArrayChildPtr( i );
				if( Spell )
				{
					egRflEditor* AttrEd = Spell->GetChildPtr( "AttributesEffected" );
					if( AttrEd )
					{
						exAttrSet* Attr = reinterpret_cast<exAttrSet*>(AttrEd->GetData());
						if( Attr )
						{
							Attr->RefreshVisibleAttributes( AttrEd );
						}
					}
				}
			}
		}
	}

	egRflEditor* CategoriesEd = RootEditor.GetChildPtr( "m_Categories" );
	if( CategoriesEd )
	{
		const eg_size_t NumCatgories = CategoriesEd->GetNumChildren();
		for( eg_size_t Cat=0; Cat<NumCatgories; Cat++ )
		{
			egRflEditor* CatEd = CategoriesEd->GetArrayChildPtr( Cat );
			egRflEditor* SpellsEd = CatEd ? CatEd->GetChildPtr( "Spells" ) : nullptr;
			if( SpellsEd )
			{
				const eg_size_t NumChildren = SpellsEd->GetNumChildren();
				for( eg_size_t i=0; i<NumChildren; i++ )
				{
					egRflEditor* Spell = SpellsEd->GetArrayChildPtr( i );
					if( Spell )
					{
						egRflEditor* AttrEd = Spell->GetChildPtr( "AttributesEffected" );
						if( AttrEd )
						{
							exAttrSet* Attr = reinterpret_cast<exAttrSet*>(AttrEd->GetData());
							if( Attr )
							{
								Attr->RefreshVisibleAttributes( AttrEd );
							}
						}
					}
				}
			}
		}
	}
}
