// (c) 2018 Beem Media

#if defined( __EGEDITOR__ )

#include "EGDataBuilder.h"
#include "EGToolsHelper.h"
#include "EGExtProcess.h"

void EGDataBuilder::ExecuteFirstStep( eg_cpstr GameName )
{
	m_GameName = GameName;
	m_SrcDir = EGToolsHelper_GetEnvVar( "EGSRC" );
	m_OutDir = EGToolsHelper_GetEnvVar( "EGOUT" );

	EGLogf( eg_log_t::General , "Beginning build for %s..." , GameName );
	EGLogf( eg_log_t::General , "EGSRC: \"%s\"" , m_SrcDir.String() );
	EGLogf( eg_log_t::General , "EGOUT: \"%s\"" , m_OutDir.String() );

	if( m_SrcDir.Len() > 0  && m_OutDir.Len() > 0 )
	{
		eg_int ProcessRes = 0;
		eg_bool bRanProcess = false;
		bRanProcess = EGExtProcess_Run( EGString_Format( "egmake2_x64 SET_GAME_BUILD %s" , m_GameName.String() ) , &ProcessRes );
		bRanProcess = EGExtProcess_Run( "egmake2_x64 DATA" , &ProcessRes );
		// bRanProcess = EGExtProcess_Run( "egmake2_x64 MAKEWEBSERVICE" , &ProcessRes );
		bRanProcess = EGExtProcess_Run( "egmake2_x64 CREATE_GAME_INI" , &ProcessRes );
	}
	else
	{
		EGLogf( eg_log_t::Warning , "EGSRC and/or EGOUT was not specified. Data cannot be built." );
	}
}

void EGDataBuilder::ExecuteNextStep( eg_real DeltaTime )
{
	unused( DeltaTime );

	if( !m_bIsDone )
	{
		m_bIsDone = true;
		EGLogf( eg_log_t::General , "The build completed." );
	}
}

#endif
