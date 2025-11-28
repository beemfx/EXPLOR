// (c) 2017 Beem Media

#include "EGLog.h"
#include "EGStringMap.h"
#include "EGLogDispatcher.h"
#include "EGStdLibAPI.h"
#include "EGWindowsAPI.h"

static egLogHandler EGLog_Handler = nullptr;

static EGMutex                          EGLog_DispatchersLock;
static EGLogDispatcher                  EGLog_BufferDispatcher;
static EGFixedArray<EGLogDispatcher*,8> EGLog_Dispatchers( CT_Clear );

static EGStringCrcMapFixedSize<eg_string_crc,50> EGLog_Supressed( CT_Clear , CT_Clear );

static void EGLog_DefaultHandler( eg_log_channel LogChannel , const eg_string_base& LogString )
{
	unused( LogChannel );
	
	eg_string_big StrOutput = EGString_Format( "[LOG] %s\n" , LogString.String() );
	printf( "%s" , StrOutput.String() );
	OutputDebugStringA( StrOutput );
}

void EGLog_SetHandler( egLogHandler Handler )
{
	EGLog_Handler = Handler;
}

void EGLog_SetChannelSuppressed( eg_log_channel Channel, eg_bool bSuppressed )
{
	if( bSuppressed && !EGLog_Supressed.Contains( Channel.GetId() ) )
	{
		EGLog_Supressed.Insert( Channel.GetId() , Channel.GetId() );
	}
	if( !bSuppressed && EGLog_Supressed.Contains( Channel.GetId() ) )
	{
		EGLog_Supressed.Delete( Channel.GetId() );
	}
}

void EGLog_SetDefaultSuppressedChannels()
{
	// EGLog_SetChannelSuppressed( eg_log_t::General , true );
	// EGLog_SetChannelSuppressed( eg_log_t::Error , true );
	// EGLog_SetChannelSuppressed( eg_log_t::Warning , true );
	EGLog_SetChannelSuppressed( eg_log_t::Verbose , true );
	EGLog_SetChannelSuppressed( eg_log_t::Audio , true );
	EGLog_SetChannelSuppressed( eg_log_t::AudioActivity , true );
	EGLog_SetChannelSuppressed( eg_log_t::Thread , true );
	EGLog_SetChannelSuppressed( eg_log_t::Client , true );
	EGLog_SetChannelSuppressed( eg_log_t::Server , true );
	EGLog_SetChannelSuppressed( eg_log_t::GameLib , true );
	EGLog_SetChannelSuppressed( eg_log_t::GameLibFromGame , true );
	EGLog_SetChannelSuppressed( eg_log_t::FileSys , true );
	EGLog_SetChannelSuppressed( eg_log_t::MemInfo , true );
	EGLog_SetChannelSuppressed( eg_log_t::Settings , true );
	EGLog_SetChannelSuppressed( eg_log_t::Web , true );
	EGLog_SetChannelSuppressed( eg_log_t::NetCommC , true );
	EGLog_SetChannelSuppressed( eg_log_t::NetCommS , true );
	EGLog_SetChannelSuppressed( eg_log_t::NetOnline , true );
	EGLog_SetChannelSuppressed( eg_log_t::NetOnlineTCP , true );
	EGLog_SetChannelSuppressed( eg_log_t::NetOnlineUDP , true );
	EGLog_SetChannelSuppressed( eg_log_t::Physics , true );
	EGLog_SetChannelSuppressed( eg_log_t::Renderer9 , true );
	EGLog_SetChannelSuppressed( eg_log_t::Renderer11 , true );
	EGLog_SetChannelSuppressed( eg_log_t::RendererActivity , true );
	EGLog_SetChannelSuppressed( eg_log_t::SettingsActivity , true );
	EGLog_SetChannelSuppressed( eg_log_t::Assets , true );
}

static const struct
{
	const eg_log_channel& FilterType;
	eg_cpstr              Prefix;
	eg_color32            Color;
}
EGLog_InfoTable[] =
{
	{ eg_log_t::General           , "EG"                , eg_color32(170,171,89) } ,
	{ eg_log_t::Error             , "ERROR"             , eg_color32( 0xAA , 0x00, 0x00 ) } ,
	{ eg_log_t::Warning           , "WARNING"           , eg_color32( 0xFF , 0xFF, 0x00 ) } ,
	{ eg_log_t::Verbose           , "EG:VERBOSE"        , eg_color32( 255 , 255 , 255 ) } ,
	{ eg_log_t::Audio             , "AUDIO"             , eg_color32( 0x55 , 0xCC, 0x55 ) } ,
	{ eg_log_t::AudioActivity     , "AUDIO ACTIVITY"    , eg_color32( 0x55 , 0xCC, 0x55 ) } ,
	{ eg_log_t::Thread            , "THREAD"            , eg_color32( 255 , 255, 255 ) } ,
	{ eg_log_t::Client            , "CLIENT"            , eg_color32( 0x00 , 0xAA, 0x00 ) } ,
	{ eg_log_t::Server            , "SERVER"            , eg_color32( 0x59 , 0x59, 0xEE ) } ,
	{ eg_log_t::GameLib           , "GAMELIB"           , eg_color32( 255 , 255 , 255 ) } ,
	{ eg_log_t::GameLibFromGame   , "GAME"              , eg_color32( 0xAA , 0xAB, 0x59 ) } ,
	{ eg_log_t::FileSys           , "FILESYS"           , eg_color32( 255 , 255, 255 ) } ,
	{ eg_log_t::MemInfo           , "MEMINFO"           , eg_color32( 255 , 255, 255 ) } ,
	{ eg_log_t::Settings          , "SETTINGS"          , eg_color32( 255 , 255, 255 ) } ,
	{ eg_log_t::NetCommC          , "NETCOMM CLIENT"    , eg_color32( 0xAA , 0xAA, 0x00 ) } ,
	{ eg_log_t::NetCommS          , "NETCOMM SERVER"    , eg_color32( 0xAA , 0xAA, 0x00 ) } ,
	{ eg_log_t::NetOnline         , "NETONLINE"         , eg_color32( 0xAA , 0xAA, 0x00 ) } ,
	{ eg_log_t::NetOnlineTCP      , "NETONLINE TCP"     , eg_color32( 0xAA , 0xAA, 0x00 ) } ,
	{ eg_log_t::NetOnlineUDP      , "NETONLINE UDP"     , eg_color32( 0xAA , 0xAA, 0x00 ) } ,
	{ eg_log_t::Physics           , "PHYSICS"           , eg_color32( 255 , 255 , 255 ) } ,
	{ eg_log_t::Renderer9         , "RENDERER9"         , eg_color32( 0xAA , 0x00, 0xAA ) } ,
	{ eg_log_t::Renderer11        , "RENDERER11"        , eg_color32( 0xAA , 0x00, 0xAA ) } ,
	{ eg_log_t::RendererActivity  , "RENDERER ACTIVITY" , eg_color32( 0xAA , 0x00, 0xAA ) } ,
	{ eg_log_t::SettingsActivity  , "SETTINGS ACTIVITY" , eg_color32( 255 , 255 , 255 ) } ,
	{ eg_log_t::Assets            , "ASSETS"            , eg_color32( 0x3B , 0x02, 0x2D ) } ,
	{ eg_log_t::Web               , "WEB"               , eg_color32( 0xAA , 0xAA, 0x00 ) } ,
	{ eg_log_t::SourceControl     , "SC"                , eg_color32( 120 , 0 , 120 ) },
	{ eg_log_t::Performance       , "PERF"              , eg_color32( 255 , 255 , 255 ) },
};

void EGLog_AddDispatcher( class EGLogDispatcher* Dispatcher )
{
	EGFunctionLock Lock( &EGLog_DispatchersLock );
	if( Dispatcher )
	{
		EGLog_BufferDispatcher.CopyTo( *Dispatcher );
		EGLog_Dispatchers.AppendUnique( Dispatcher );
	}
}

void EGLog_RemoveDispatcher( class EGLogDispatcher* Dispatcher )
{
	EGFunctionLock Lock( &EGLog_DispatchersLock );
	EGLog_Dispatchers.DeleteByItem( Dispatcher );
}

eg_color32 EGLog_GetColorForChannel( eg_log_channel LogChannel )
{
	for( eg_uint i=0; i<countof(EGLog_InfoTable); i++ )
	{
		if( EGLog_InfoTable[i].FilterType == LogChannel )
		{
			return EGLog_InfoTable[i].Color;
		}
	}

	return eg_color32(170,171,89);
}

eg_string_small EGLog_GetNameForChannel( eg_log_channel LogChannel )
{
	for( eg_uint i=0; i<countof(EGLog_InfoTable); i++ )
	{
		if( EGLog_InfoTable[i].FilterType == LogChannel )
		{
			return EGLog_InfoTable[i].Prefix;
		}
	}

	return "LOG";
}

void EGLogf( eg_log_channel LogChannel , eg_cpstr StrFormat , ... )
{
	if( EGLog_Supressed.Contains( LogChannel.GetId() ) )
	{
		return;
	}

	eg_string_big StrFormatted;
	va_list arglist = nullptr;
	va_start( arglist , StrFormat );
	eg_char Temp[eg_string_big::STR_SIZE];
	_vsnprintf_s( Temp , countof( Temp ) , _TRUNCATE , StrFormat , arglist );
	Temp[countof( Temp ) - 1] = 0;
	StrFormatted = Temp;
	va_end( arglist );

	if( EGLog_Handler )
	{
		EGLog_Handler( LogChannel , StrFormatted );
	}
	else
	{
		EGLog_DefaultHandler( LogChannel , StrFormatted );
	}

	EGLog_BufferDispatcher.QueueLog( LogChannel , StrFormatted );

	{
		EGFunctionLock Lock( &EGLog_DispatchersLock );
		for( EGLogDispatcher* Dispatcher : EGLog_Dispatchers )
		{
			Dispatcher->QueueLog( LogChannel , StrFormatted );
		}
	}
}

namespace eg_log_t {

const eg_log_channel General( eg_crc("GENERAL") );
const eg_log_channel Error( eg_crc("ERROR") );
const eg_log_channel Warning( eg_crc("WARNING") );
const eg_log_channel Verbose( eg_crc("VERBOSE") ); //Meant only to be seen if the tool is printing verbose.
const eg_log_channel Audio( eg_crc("AUDIO") );
const eg_log_channel AudioActivity( eg_crc("AUDIO_ACTIVITY") );
const eg_log_channel Thread( eg_crc("THREAD") );
const eg_log_channel Client( eg_crc("CLIENT") );
const eg_log_channel Server( eg_crc("SERVER") );
const eg_log_channel GameLib( eg_crc("GAMELIB") );
const eg_log_channel GameLibFromGame( eg_crc("GAMELIB_FROMGAME") );
const eg_log_channel FileSys( eg_crc("FILESYS") );
const eg_log_channel MemInfo( eg_crc("MEMINFO") );
const eg_log_channel Settings( eg_crc("SETTINGS") );
const eg_log_channel Web( eg_crc("WEB") );
const eg_log_channel NetCommC( eg_crc("NETCOMM_C") );
const eg_log_channel NetCommS( eg_crc("NETCOMM_S") );
const eg_log_channel NetOnline( eg_crc("NETONLINE") );
const eg_log_channel NetOnlineTCP( eg_crc("NETONLINE_TCP") );
const eg_log_channel NetOnlineUDP( eg_crc("NETONLINE_UDP") );
const eg_log_channel Physics( eg_crc("PHYSICS") );
const eg_log_channel Renderer9( eg_crc("RENDERER9") );
const eg_log_channel Renderer11( eg_crc("RENDERER11") );
const eg_log_channel RendererActivity( eg_crc("RENDERER_ACTIVITY") );
const eg_log_channel SettingsActivity( eg_crc("SETTINGS_ACTIVITY") );
const eg_log_channel Assets( eg_crc("ASSETS") );
const eg_log_channel SourceControl( eg_crc("SC") );
const eg_log_channel Performance( eg_crc("PERF") );

}