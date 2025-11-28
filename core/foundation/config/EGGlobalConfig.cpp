// (c) 2018 Beem Media

#include "EGGlobalConfig.h"

EGSettingsString GlobalConfig_PlatformAppId( "Global.PlatformAppId" , eg_loc("GlobalPlatformAppIdSettingText","Platform App ID") , "" , 0 );

EGSettingsString GlobalConfig_ScreenDriverClass( "ScreenDriverClass" , eg_loc("ScreenDriverClass","Screen Driver Class") , "EGD11R_Renderer" , EGS_F_SYS_SAVED );
EGSettingsBool GlobalConfig_ServerHasThread( "ServerHasThread" , eg_loc("ServerHasThread","Server Has Thread") , true , EGS_F_SYS_SAVED );
EGSettingsInt GlobalConfig_ServerPort( "ServerPort" , eg_loc("ServerPort","Host Port") , 5001 , EGS_F_SYS_SAVED );
EGSettingsBool GlobalConfig_IsUiLayoutTool( "IsUiLayoutTool", eg_loc("IsUiLayoutTool","Internal: Is UI Layout Tool"), false, 0 );
EGSettingsString NetConfig_DefaultServer( "DefaultServer" , eg_loc("DefaultServer","Default Server"), "", 0 );

EGSettingsVolume AudioConfig_EffectVolume( "Volume_Effect" , eg_loc("EffectVolume","Effects Volume") , 80 , EGS_F_USER_SAVED|EGS_F_EDITABLE );
EGSettingsVolume AudioConfig_SpeechVolume( "Volume_Speech" , eg_loc("SpeechVolume","Speech Volume") , 80 , EGS_F_USER_SAVED|EGS_F_EDITABLE );
EGSettingsVolume AudioConfig_MusicVolume( "Volume_Music" , eg_loc("MusicVolume","Music Volume") , 60 , EGS_F_USER_SAVED|EGS_F_EDITABLE );

EGSettingsBool VideoConfig_TextureUseMipMaps( "TextureUseMipMaps" , eg_loc("TextureUseMipMaps","Mip Mapped Textures") , true , EGS_F_SYS_SAVED|EGS_F_EDITABLE );
EGSettingsBool VideoConfig_IsWindowed( "IsWindowed" , eg_loc("IsWindowed","Windowed") , IS_DEBUG , EGS_F_SYS_SAVED|EGS_F_EDITABLE|EGS_F_NEEDSVRESTART );
EGSettingsBool VideoConfig_IsExclusiveFullScreen( "IsExclusiveFullScreen" , eg_loc("IsExclusiveFullScreen","Exclusive Full Screen") , false , EGS_F_SYS_SAVED|EGS_F_EDITABLE|EGS_F_NEEDSVRESTART );
EGSettingsBool VideoConfig_VSyncEnabled( "VSyncEnabled" , eg_loc("VSyncEnabled","V-Sync") , true , EGS_F_SYS_SAVED|EGS_F_EDITABLE|EGS_F_NEEDSVRESTART );
EGSettingsBool VideoConfig_PostFXAA( "PostFXAA" , eg_loc("PostFXAA","FXAA") , true , EGS_F_SYS_SAVED|EGS_F_EDITABLE );
EGSettingsBool VideoConfig_PostMotionBlur( "PostMotionBlur" , eg_loc("PostMotionBlur","Motion Blur") , true , EGS_F_SYS_SAVED|EGS_F_EDITABLE );
EGSettingsInt VideoConfig_MaxAnisotropy( "VideoConfig_MaxAnisotropy" , eg_loc("MaxAnisotropy","Max Anisotropy") , 0 , EGS_F_SYS_SAVED );

EGSettingsBool DebugConfig_DrawWireframe( "DebugDrawWireframe" , eg_loc("DebugDrawWireframe","Debug: Draw Wireframe") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawEntBBs( "DrawEntBBs" , eg_loc("DrawEntBBs","Debug: Draw Entity Bounding Boxes") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawMapBBs( "DrawMapBBs" , eg_loc("DrawMapBBs","Debug: Draw Map Bounding Boxes") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawMapGraphs( "DrawMapGraphs" , eg_loc("DrawMapGraphs","Bug: Draw Map Graphs") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawCloseLights( "DrawCloseLights" , eg_loc("DrawCloseLights","Debug: Draw Close Lights") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawPortals( "DrawPortals" , eg_loc("DrawPortals","Debug: Draw Portals") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawZOverlay( "DrawZOverlay " , eg_loc("DrawZOverlay ","Debug: Draw Z-Overlay") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawTextNodes( "DrawTextNodes" , eg_loc("DrawTextNodes","Debug: Draw Text Nodes") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );

EGSettingsBool DebugConfig_ShowPlayerPosition( "ShowPlayerPosition", eg_loc("ShowPlayerPosition","Debug: Show Player Position" ), false, EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawGameState( "DebugDrawGameState", eg_loc("DebugDrawGameState","Debug: Draw Game State"), false, EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawMenuButtons( "DebugDrawMenuButtons", eg_loc("DebugDrawMenuButtons","Debug: Draw Menu Buttons"), false, EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawGridMasks( "DebugDrawGridMasks", eg_loc("DebugDrawGridMasks","Debug: Draw Grid Masks"), false, EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawVolShadows( "DebugDrawVolShadows", eg_loc("DebugDrawVolShadows", "Debug: Draw Volume Shadows"), false, EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_DrawFPS( "DebugDrawFPS", eg_loc("DebugDrawFPS","Debug: Draw FPS"), IS_DEBUG, EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_ShowPhysMem( "DebugShowPhysMem", eg_loc("DebugShowPhysMem","Debug: Show Physics Memory"), false, EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
EGSettingsBool DebugConfig_ReportSlowFrames( "DebugReportSlowFrames", eg_loc("DebugReportSlowFrames","Debug: Report Slow Frames"), false, EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );
