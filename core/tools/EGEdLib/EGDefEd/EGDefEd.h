// (c) 2017 Beem Media

#pragma once

int EGDefEd_Run( eg_cpstr16 InitialFilename , const struct egSdkEngineInitParms& EngineParms );
void EGDefEd_SetDirty();
class EGWndPanelPropEditor* EGDefEd_GetSettingsPanel();
class EGWndPanelPropEditor* EGDefEd_GetEditorConfigPanel();
class EGDefEdTimelineEditor* EGDefEd_GetTimelineEditor(); 
eg_string_big EGDefEd_GetCurrentGameFilename();