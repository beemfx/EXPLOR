// (c) 2016 Beem Media

#include "EGMake.h"
#include "EGXMLBase.h"
#include "EGParse.h"
#include "EGWindowsAPI.h"
#include "EGExtProcess.h"

class EGEBuild: private IXmlBase
{
private:
	enum e_cmd_t
	{
		CMD_COMMAND,
		CMD_CLEAN,
	};

	struct egCmd
	{
		e_cmd_t Type;
		eg_string_big Action;
	};

private:
	EGArray<egCmd> m_CommandList;

public:
	EGEBuild( eg_cpstr Filename )
	{
		XMLLoad( Filename );
	}

	eg_bool ExeCmds()
	{
		if( 0 == m_CommandList.Len() )
		{
			return false;
		}

		eg_bool WasCmdExecuted = true;

		for( eg_uint i=0; WasCmdExecuted && i<m_CommandList.Len(); i++ )
		{
			eg_string_big Cmd = m_CommandList[i].Action;
			Cmd.Replace( "$(IN_DIR)" , EGMake_GetInputPath( FINFO_DIR ) );
			Cmd.Replace( "$(OUT_DIR)" , EGMake_GetOutputPath( FINFO_DIR ) );
			Cmd.Replace( "$(OUT_NOEXT_FILE)" , EGMake_GetOutputPath( FINFO_NOEXT_FILE ) );
			Cmd.Replace( "$(OUT_SHORT_NOEXT)" , EGMake_GetOutputPath( FINFO_SHORT_NOEXT ) );

			switch( m_CommandList[i].Type )
			{
				case CMD_COMMAND:
					EGLogf( eg_log_t::General , "EBuild: %s" , Cmd.String() );
					WasCmdExecuted = EGExtProcess_Run( Cmd , nullptr );
					break;
				case CMD_CLEAN:
					EGLogf( eg_log_t::General , "EBuild Clean: %s" , Cmd.String() );
					WasCmdExecuted = 0 != ::DeleteFileA( Cmd );
					break;
			}
		}

		return WasCmdExecuted;
	}

private:
	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& atts) override
	{
		unused( Tag , atts );
	}

	virtual void OnData(eg_cpstr strData, eg_uint nLen) override
	{
		unused( nLen );

		if( GetXmlTagUp(0).Equals( "command" ) )
		{
			egCmd NewCommand;
			NewCommand.Type = CMD_COMMAND;
			NewCommand.Action = eg_string_big(strData);
			m_CommandList.Append( NewCommand );
		}
		else if( GetXmlTagUp(0).Equals( "clean" ) )
		{
			egCmd NewCommand;
			NewCommand.Type = CMD_CLEAN;
			NewCommand.Action = eg_string_big(strData);
			m_CommandList.Append( NewCommand );
		}
	}

	virtual eg_cpstr XMLObjName()const override{ return __FUNCTION__ ; }
};

eg_bool EGMakeRes_ebuild()
{
	eg_string_big In = EGMake_GetInputPath();

	EGEBuild Builder( In );

	return Builder.ExeCmds();
}