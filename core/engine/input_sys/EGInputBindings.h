// EGInputBindings (c) 2016 Beem Media

#pragma once
#include "EGInputTypes.h"
#include "EGInputButtons.h"

class EGInputBindings
{
public:
	static const eg_uint eg_cmd_t_BTICOUNT=8;
	static const eg_uint g_gp_btn_BITCOUNT=8;
	static const eg_uint eg_kbm_btn_BITCOUNT=16;

	struct egBinding
	{
		eg_cmd_t   Action   : eg_cmd_t_BTICOUNT;
		eg_gp_btn  GpButton : g_gp_btn_BITCOUNT;
		eg_kbm_btn KbButton : eg_kbm_btn_BITCOUNT;
	};

	static_assert( CMD_MAX < (1<<eg_cmd_t_BTICOUNT) , "Above struct should be resized." );
	static_assert( GP_BTN_COUNT < (1<<g_gp_btn_BITCOUNT), "Above struct should be resized." );
	static_assert( KBM_BTN_COUNT < (1<<eg_kbm_btn_BITCOUNT), "Above struct should be resized.");

	EGInputBindings()
	{
		zero( &m_Map );
		for( eg_uint i=0; i<countof(m_Map); i++ ){ m_Map[i].Action = static_cast<eg_cmd_t>(i); }
	}

	void Bind( eg_cmd_t Action , eg_gp_btn GpButton , eg_kbm_btn KbButton )
	{
		assert( m_Map[Action].Action == Action );
		m_Map[Action].GpButton = GpButton;
		m_Map[Action].KbButton = KbButton;
	}

	void Bind( eg_cmd_t Action , eg_gp_btn GpButton )
	{
		assert( m_Map[Action].Action == Action );
		m_Map[Action].GpButton = GpButton;
	}

	void Bind( eg_cmd_t Action , eg_kbm_btn KbButton )
	{
		assert( m_Map[Action].Action == Action );
		m_Map[Action].KbButton = KbButton;
	}

	eg_kbm_btn ActionToKb( eg_cmd_t Action )const
	{ 
		assert( m_Map[Action].Action == Action ); 
		return m_Map[Action].KbButton; 
	}
	eg_gp_btn ActionToGp( eg_cmd_t Action )const
	{ 
		assert( m_Map[Action].Action == Action ); 
		return m_Map[Action].GpButton; 
	}

	eg_string_crc InputActionToKey( eg_cmd_t Cmd , eg_bool bGP ) const
	{
		eg_string_crc NameCrcOut( CT_Clear );

		if( bGP )
		{
			eg_gp_btn GpButton = ActionToGp( Cmd );
			NameCrcOut = eg_string_crc(InputButtons_GpButtonToString( GpButton ));
		}
		else
		{
			eg_kbm_btn KbmButton = ActionToKb( Cmd );
			NameCrcOut = eg_string_crc(InputButtons_KbmButtonToString( KbmButton ));
		}

		return NameCrcOut;
	}

	eg_cpstr InputActionToKeyStr( eg_cmd_t Cmd , eg_bool bGP ) const
	{

		if( bGP )
		{
			eg_gp_btn GpButton = ActionToGp( Cmd );
			return InputButtons_GpButtonToString( GpButton );
		}
		else
		{
			eg_kbm_btn KbmButton = ActionToKb( Cmd );
			return InputButtons_KbmButtonToString( KbmButton );
		}

		return "";
	}

	eg_bool DoBindingConflictsExist( eg_cmd_t SourceCmd , const EGArray<eg_cmd_t>& CheckAgainst ) const
	{
		if( 0 <= SourceCmd && SourceCmd < countof(m_Map) )
		{
			const egBinding& SourceBinding = m_Map[SourceCmd];

			for( eg_cmd_t CheckCmd : CheckAgainst )
			{
				if( 0 <= CheckCmd && CheckCmd < countof(m_Map) )
				{
					const egBinding& CheckBinding = m_Map[CheckCmd];
					if( SourceBinding.KbButton == CheckBinding.KbButton )
					{
						return true;
					}
					if( SourceBinding.GpButton == CheckBinding.GpButton )
					{
						return true;
					}
				}
			}

		}
		return false;
	}

private:
	egBinding m_Map[CMD_MAX];

};
