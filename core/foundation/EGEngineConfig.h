/******************************************************************************
Engine Config

Some consts that determine how the engine is built.

(c) 2014 Beem Software
******************************************************************************/
#pragma once

static const eg_char ENGINE_NAME[]=("Emergence Game Engine");
static const eg_char ENGINE_VERSION[]="(" __DATE__ " " __TIME__ ")";
#if defined( __WIN32__ )
static const eg_char ENGINE_PLATFORM[] = "WIN32";
#elif defined( __WIN64__ )
static const eg_char ENGINE_PLATFORM[] = "WIN64";
#else
static const eg_char ENGINE_PLATFORM[] = "UNKNOWN PLATFORM";
#endif

#if defined( __DEBUG__ )
static const eg_char ENGINE_BUILD[] = "DEBUG";
#elif defined( __RELEASE__ )
static const eg_char ENGINE_BUILD[] = "RELEASE";
#else
static const eg_char ENGINE_BUILD[] = "UNKNOWN BUILD";
#endif

//Note that these paths are internal only and have nothing to do with the
//windows file system. The windows file paths are passed in on the EGEngine::Init
//function.
#define GAME_DATA_PATH "/game"
#define GAME_USER_PATH "/user"
#define GAME_SYSCFG_PATH "/syscfg"

static const eg_uint MAX_CLIENTS              = 32;
static const eg_uint MAX_LOCAL_CLIENTS        = 1;
static const eg_real CLI_DISCONNECT_WARN_TIME = 10.0f;
static const eg_real CLI_DISCONNECT_TIME      = 20.0f;
static const eg_size_t CLIENT_PROFILE_DATA_SIZE = 1*1024; // 1K for now
static const eg_size_t EG_PHYSICS_SIM_MEM_SIZE = 25*1024*1024;

static const eg_uint VID_MAX_FPS           = 120;
static const eg_real VID_MAX_FRAME_TIME    = 1.0f/VID_MAX_FPS;
static const eg_uint VID_MAX_FRAME_TIME_MS = 1000/VID_MAX_FPS;
static const eg_uint EG_DISPLAY_LIST_MEGS  = 3;

static const eg_uint AUDIO_MAX_FPS           = 60;
static const eg_real AUDIO_MAX_FRAME_TIME    = 1.0f/VID_MAX_FPS;
static const eg_uint AUDIO_MAX_FRAME_TIME_MS = 1000/VID_MAX_FPS;

static const eg_real INPUT_PRESS_REPEAT_FIRST_DELAY = .3f;
static const eg_real INPUT_PRESS_REPEAT_TIME = .04f;
static const eg_real INPUT_BASE_MOUSE_AXIS_MULTIPLIER = 100.f;

static const eg_cpstr      CON_BG   = ("/egdata/console/conbg");
static const eg_string_crc CON_FONT_ID(eg_crc("genbasb"));

static const eg_cpstr EXT_TEX = "dds";
static const eg_cpstr EXT_SND = "ogg";

#define WITH_NET_COMS (0)
