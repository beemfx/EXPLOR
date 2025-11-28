// (c) 2017 Beem Media

#pragma once

struct eg_log_channel
{
private:

	const eg_string_crc LogCrcId;

public:

	explicit eg_log_channel( eg_string_crc InLogCrcId ): LogCrcId( InLogCrcId ){ }
	eg_bool operator == ( const eg_log_channel& rhs ) const { return LogCrcId == rhs.LogCrcId; }
	eg_bool operator != ( const eg_log_channel& rhs ) const { return !(*this == rhs); }
	eg_string_crc GetId() const { return LogCrcId; }
};

namespace eg_log_t {

extern const eg_log_channel General;
extern const eg_log_channel Error;
extern const eg_log_channel Warning;
extern const eg_log_channel Verbose; //Meant only to be seen if the tool is printing verbose.
extern const eg_log_channel Audio;
extern const eg_log_channel AudioActivity;
extern const eg_log_channel Thread;
extern const eg_log_channel Client;
extern const eg_log_channel Server;
extern const eg_log_channel GameLib;
extern const eg_log_channel GameLibFromGame;
extern const eg_log_channel FileSys;
extern const eg_log_channel MemInfo;
extern const eg_log_channel Settings;
extern const eg_log_channel Web;
extern const eg_log_channel NetCommC;
extern const eg_log_channel NetCommS;
extern const eg_log_channel NetOnline;
extern const eg_log_channel NetOnlineTCP;
extern const eg_log_channel NetOnlineUDP;
extern const eg_log_channel Physics;
extern const eg_log_channel Renderer9;
extern const eg_log_channel Renderer11;
extern const eg_log_channel RendererActivity;
extern const eg_log_channel SettingsActivity;
extern const eg_log_channel Assets;
extern const eg_log_channel SourceControl;
extern const eg_log_channel Performance;

}

typedef void ( * egLogHandler )( eg_log_channel LogChannel , const eg_string_base& LogString );

void EGLog_SetHandler( egLogHandler Handler );
void EGLog_SetChannelSuppressed( eg_log_channel Channel , eg_bool bSuppressed );
void EGLog_SetDefaultSuppressedChannels();
void EGLog_AddDispatcher( class EGLogDispatcher* Dispatcher );
void EGLog_RemoveDispatcher( class EGLogDispatcher* Dispatcher );
eg_color32 EGLog_GetColorForChannel( eg_log_channel LogChannel );
eg_string_small EGLog_GetNameForChannel( eg_log_channel LogChannel );
void EGLogf( eg_log_channel LogChannel , eg_cpstr StrFormat , ... );
