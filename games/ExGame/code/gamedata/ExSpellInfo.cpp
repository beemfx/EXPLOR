// (c) 2017 Beem Media

#include "ExSpellInfo.h"
#include "ExFighter.h"
#include "ExRoster.h"
#include "ExCore.h"

const ExFighter* exSpellInfo::s_ReferenceFighter = nullptr;

ex_attr_value exSpellInfo::GetManaCost( ex_attr_value Magic , ex_attr_value Level ) const
{
	ex_attr_value Out = 0;

	switch( ManaCostFunction )
	{
	case ex_spell_func::Fixed:
		Out = EGMath_round(ManaCost);
		break;
	case ex_spell_func::MagicMul:
		Out = EG_Max<ex_attr_value>( 1 , EGMath_round(ManaCost*Magic) );
		break;
	case ex_spell_func::MagicMulLvlGrowth:
		Out = EG_Max<ex_attr_value>( 1 , ExCore_GetRampedValue_Level( ManaCost*Magic , Level ) );
		break;
	}

	return Out;
}

exDamage exSpellInfo::GetDamage( ex_attr_value Magic, ex_attr_value Level ) const
{
	assert( Type == ex_spell_t::CombatDamage || Type == ex_spell_t::Resurrect );
	exDamage Out( CT_Clear );

	exDamage DamageBase( CT_Clear );
	DamageBase.Physical = AttributesEffected.PDMG;
	DamageBase.Fire = AttributesEffected.FDMG;
	DamageBase.Water = AttributesEffected.WDMG;
	DamageBase.Air = AttributesEffected.ADMG;
	DamageBase.Earth = AttributesEffected.EDMG;

	switch( DamageFunction )
	{
	case ex_spell_func::Fixed:
		Out = DamageBase;
		break;
	case ex_spell_func::MagicMul:
		Out = DamageBase.GetMagicMultiplied( Magic );
		break;
	case ex_spell_func::MagicMulLvlGrowth:
		Out = DamageBase.GetMagicMultiplyLevelGrowth( Magic, Level );
		break;
	}

	return Out;
}

ex_attr_value exSpellInfo::GetAttrBoost( ex_attr_t AttrType, ex_attr_value Magic, ex_attr_value Level ) const
{
	ex_attr_value Out = 0;

	switch( AttrType )
	{
#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: Out = AttributesEffected._var_; break;
#include "ExAttrs.items"
	}

	switch( DamageFunction )
	{
	case ex_spell_func::Fixed:
		break;
	case ex_spell_func::MagicMul:
		Out = Magic*Out;
		break;
	case ex_spell_func::MagicMulLvlGrowth:
		Out = ExCore_GetRampedValue_Level( Magic*Out , Level );
		break;
	}

	return Out;
}

eg_bool exSpellInfo::CanTargetEnemy() const
{
	return Target == ex_spell_target::AllActive || Target == ex_spell_target::EnemyActive || Target == ex_spell_target::EnemyAny;
}

eg_bool exSpellInfo::CanTargetFriendly() const
{
	return Target == ex_spell_target::AllActive || Target == ex_spell_target::AllyActive || Target == ex_spell_target::AllyAny;
}

eg_bool exSpellInfo::CanTargetOutOfCombat() const
{
	return Target == ex_spell_target::AllAny || Target == ex_spell_target::EnemyAny || Target == ex_spell_target::AllyAny;
}

eg_bool exSpellInfo::CanTargetInWorld() const
{
	return TargetsEveryoneInWorld() || Target == ex_spell_target::AllyActive || Target == ex_spell_target::AllyAny;
}

eg_bool exSpellInfo::TargetsEveryoneInWorld() const
{
	return Target == ex_spell_target::AllActive || Target == ex_spell_target::AllAny;
}

eg_bool exSpellInfo::CanClassUse( ex_class_t ClassType ) const
{
	return AllowedPlayerClasses.IsClassSet( ClassType );
}

void exSpellInfo::FormatText( eg_cpstr Flags, class EGTextParmFormatter* Formatter ) const
{
	eg_string_crc BaseFlag = Formatter->GetNextFlag( &Flags );

	switch_crc( BaseFlag )
	{
		case_crc("NAME") :
		{
			Formatter->SetText( eg_loc_text( Name ).GetString() );
		} break;

		case_crc("DESC"):
		{
			const eg_string_crc DescFlag = Formatter->GetNextFlag( &Flags );

			if( DescFlag == eg_crc("ITEM_CARD") )
			{
				eg_loc_text FullDesc = EGFormat( L"{0:REQS}\n{0:TARGET}\n{0:EFFECT}" , this , eg_loc_text( Description ) );
				Formatter->SetText( FullDesc.GetString() );
			}
			else
			{
				Formatter->SetText( eg_loc_text( Description ).GetString() );
			}

		} break;

		case_crc("TARGET"):
		{
			const eg_loc_text TargetText( eg_loc("SpellTargetText","|SC(EX_ATTR)|Target|RC()| {0}") );
			eg_loc_text TargetsText;

			switch( Target )
			{
				case ex_spell_target::None:
					TargetsText = eg_loc_text( eg_loc("SpellTargetNAText","N/A") );
					break;
				case ex_spell_target::EnemyActive:
				case ex_spell_target::EnemyAny:
					if( TargetCount > 1 )
					{
						TargetsText = EGFormat( eg_loc("SpellTargetMultEnemiesFormat","{0} Enemies") , TargetCount );
					}
					else
					{
						TargetsText = EGFormat( eg_loc("SpellTargetSingleEnemiesFormat","{0} Enemy") , TargetCount );
					}
					break;
				case ex_spell_target::AllyActive:
				case ex_spell_target::AllyAny:
					if( TargetCount >= ExRoster::PARTY_SIZE )
					{
						TargetsText = EGFormat( eg_loc("SpellTargetFullPartyAllyFormat","Entire Party") , TargetCount );
					}
					else if( TargetCount > 1 )
					{
						TargetsText = EGFormat( eg_loc("SpellTargetMultAllyFormat","{0} Party Members") , TargetCount );
					}
					else
					{
						TargetsText = EGFormat( eg_loc("SpellTargetSingleAllyFormat","{0} Party Member") , TargetCount );
					}
					break;
				case ex_spell_target::AllActive:
				case ex_spell_target::AllAny:
					TargetsText = eg_loc_text( eg_loc("SpellTargetEveryone","Everyone") );
					break;
				case ex_spell_target::Self:
					if( s_ReferenceFighter )
					{
						TargetsText = EGFormat( eg_loc("SpellTargetSelfWithName","Self ({0:NAME})") , *s_ReferenceFighter );
					}
					else
					{
						TargetsText = eg_loc_text( eg_loc("SpellTargetSelfNoName","Self") );
					}
					break;
			}

			Formatter->SetText( EGFormat( TargetText.GetString() , TargetsText ).GetString() );
		} break;

		case_crc("EFFECT"):
		{
			if( s_ReferenceFighter )
			{
				switch( Type )
				{
					case ex_spell_t::Unknown:
						break;
					case ex_spell_t::CombatDamage:
					{
						const exDamage Dmg = GetDamage( s_ReferenceFighter->GetAttrValue( ex_attr_t::MAG ) , s_ReferenceFighter->GetAttrValue( ex_attr_t::LVL ) );
						const eg_bool bIsHeal = Dmg.Physical < 0;
						if( bIsHeal )
						{
							eg_d_string16 HealText;
							if( TargetCount <= 1 )
							{
								HealText = EGFormat( eg_loc("SpellInfoHealText","Restore |SC(EX_HEAL)|{0} HP|RC()|") , -Dmg.Physical ).GetString();
							}
							else
							{
								HealText = EGFormat( eg_loc("SpellInfoHealTextMultiple","Restore |SC(EX_HEAL)|{0} HP|RC()| ({1}X)") , -Dmg.Physical , TargetCount ).GetString();
							}
							HealText.Append( EGFormat( L"\n\n|SC(EX_SPELLDESC)|{0:DESC}|RC()|" , this ).GetString() );
							Formatter->SetText( *HealText );
						}
						else
						{
							const eg_loc_text DmgHeaderText(eg_loc("SpellInfoDamagerPerTarget","|SC(EX_H2)|Damage Per Target|RC()|"));
							eg_d_string16 DmgString;
							DmgString.Append( DmgHeaderText.GetString() );
							eg_int DmgCount = 0;
							auto HandleDmgType = [&DmgString,this,&DmgCount]( ex_attr_t AttrType , ex_attr_value Value ) -> void
							{
								if( 0 != Value )
								{
									DmgCount++;
									if( DmgCount <= 1 ) // Short circuit if we have more than one type of damage.
									{
										DmgString.Append( L"\n" );

										if( TargetCount <= 1 )
										{
											DmgString.Append( EGFormat( L"|SC(EX_ATTR)|{0}|RC()| |SC({2})|{1}|RC()|" , ExCore_GetAttributeName( AttrType ) , Value , ExCore_GetAttributeValueFormatColor( AttrType ) ).GetString() );
										}
										else
										{
											DmgString.Append( EGFormat( L"|SC(EX_ATTR)|{0}|RC()| |SC({2})|{1}|RC()| ({3}X)" , ExCore_GetAttributeName( AttrType ) , Value , ExCore_GetAttributeValueFormatColor( AttrType ) , TargetCount ).GetString() );
										}
									}
								}
							};

							const ex_attr_value TotalDmg = Dmg.GetRawTotal();
							HandleDmgType( ex_attr_t::PDMG , Dmg.Physical );
							HandleDmgType( ex_attr_t::FDMG , Dmg.Fire );
							HandleDmgType( ex_attr_t::WDMG , Dmg.Water );
							HandleDmgType( ex_attr_t::EDMG , Dmg.Earth );
							HandleDmgType( ex_attr_t::ADMG , Dmg.Air );

							eg_bool bCompactDmg = DmgCount > 1;

							if( bCompactDmg )
							{
								eg_cpstr16 DamageFormatStr = L"(|SC(EX_PDMG)|{0}|RC()|/|SC(EX_FDMG)|{1}|RC()|/|SC(EX_WDMG)|{2}|RC()|/|SC(EX_EDMG)|{3}|RC()|/|SC(EX_ADMG)|{4}|RC()|)";
								eg_loc_text FullDammageText = EGFormat( DamageFormatStr , Dmg.Physical , Dmg.Fire , Dmg.Water , Dmg.Earth , Dmg.Air );
								eg_d_string16 FinalDmgText;
								if( TargetCount <= 1 )
								{
									FinalDmgText = EGFormat( L"{0}\n|SC(EX_ATTR)|{3}|RC()| {1} {2}" , DmgHeaderText , TotalDmg , FullDammageText , ExCore_GetAttributeName( ex_attr_t::DMG_ ) ).GetString();
								}
								else
								{
									FinalDmgText = EGFormat( L"{0}\n|SC(EX_ATTR)|{3}|RC()| {1} ({4}X) {2}" , DmgHeaderText , TotalDmg , FullDammageText , ExCore_GetAttributeName( ex_attr_t::DMG_ ) , TargetCount ).GetString();
								}
								FinalDmgText.Append( EGFormat( L"\n\n|SC(EX_SPELLDESC)|{0:DESC}|RC()|" , this ).GetString() );
								Formatter->SetText( *FinalDmgText );
							}
							else
							{
								DmgString.Append( EGFormat( L"\n\n|SC(EX_SPELLDESC)|{0:DESC}|RC()|" , this ).GetString() );
								Formatter->SetText( *DmgString );
							}
						}
					} break;
					case ex_spell_t::AttributeBoost:
					{
						const ex_attr_value Mag = s_ReferenceFighter->GetAttrValue( ex_attr_t::MAG );
						const ex_attr_value Lvl = s_ReferenceFighter->GetAttrValue( ex_attr_t::LVL );

						eg_d_string16 BoostText;
						BoostText.Append( eg_loc_text(eg_loc("SpellInfoAttributeBoostHeader","|SC(EX_H2)|Attribute Boosts|RC()|")).GetString() );
						auto AppendBoostLog = [this,&Mag,&Lvl,&BoostText]( ex_attr_t AttrType , ex_attr_value Value ) -> void
						{
							BoostText.Append( L"\n" );
							BoostText.Append( EGFormat(L"|SC(EX_ATTR)|{1}|RC()| +{0}" , Value , ExCore_GetAttributeName( AttrType ) ).GetString() );
						};

						#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) if( AttributesEffected._var_ != 0 ){ AppendBoostLog( ex_attr_t::_var_ , GetAttrBoost( ex_attr_t::_var_ , Mag , Lvl  ) ); }
						#include "ExAttrs.items"

						BoostText.Append( EGFormat( L"\n\n|SC(EX_SPELLDESC)|{0:DESC}|RC()|" , this ).GetString() );
						Formatter->SetText( *BoostText );
					} break;
					case ex_spell_t::Resurrect:
					case ex_spell_t::TownPortalMarkAndGo:
					case ex_spell_t::TownPortalRecall:
					case ex_spell_t::FortifyRanks:
					case ex_spell_t::ExpandRanks:
						Formatter->SetText( EGFormat( L"\n{0:DESC}" , this ).GetString() );
						break;
				}
			}
			else
			{
				Formatter->SetText( eg_loc_text( eg_loc("SpellInfoNAText","|SC(EX_HURT)|N/A|RC()|") ).GetString() );
			}
		} break;

		case_crc("REQS"):
		{
			if( s_ReferenceFighter )
			{
				const ex_attr_value Cost = GetManaCost( s_ReferenceFighter->GetAttrValue( ex_attr_t::MAG ) , s_ReferenceFighter->GetAttrValue( ex_attr_t::LVL ) );
				const ex_attr_value CurrentMana = s_ReferenceFighter->GetMP();
				const eg_bool bHasEnoughXP = Cost <= CurrentMana;
				if( bHasEnoughXP )
				{
					Formatter->SetText( EGFormat( eg_loc("SpellInfoManaCost","|SC(EX_ATTR)|MP Cost|RC()| {0}") , Cost ).GetString() );
				}
				else
				{
					Formatter->SetText( EGFormat( eg_loc("SpellInfoNotEnoughManaCost","|SC(EX_ATTR)|MP Cost|RC()| |SC(EX_HURT)|{0}|RC()|") , Cost ).GetString() );
				}
			}
			else
			{
				Formatter->SetText( eg_loc_text( eg_loc("SpellInfoNAText","|SC(EX_HURT)|N/A|RC()|") ).GetString() );
			}
		} break;
	}
}

void exSpellInfo::SetReferenceFighter( ExFighter* FighterIn )
{
	s_ReferenceFighter = FighterIn;
}
