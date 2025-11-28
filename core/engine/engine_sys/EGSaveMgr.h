// SaveMgr - Interface for dealin with save games (and levels)
// (c) 2015 Beem Media
#pragma once

struct egSaveInfo
{
	eg_path File;
	eg_bool IsLevel:1;
	eg_bool IsUserSave:1;
};

void SaveMgr_Init();
void SaveMgr_Deinit();
eg_uint SaveMgr_GetNumUserSaves();
eg_uint SaveMgr_GetNumLevels();
void SaveMgr_GetUserSaveInfo( eg_uint Index , egSaveInfo* SaveInfo );
void SaveMgr_GetLevelInfo( eg_uint Index , egSaveInfo* SaveInfo );
eg_bool SaveMgr_DoesSaveExist( eg_cpstr Filename );

