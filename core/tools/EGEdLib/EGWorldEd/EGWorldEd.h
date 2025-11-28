// (c) 2018 Beem Media

#pragma once

class EGWorldFile;
class EGWorldEdObjLibPanel;
class EGWorldEdWorldObjsPanel;
class EGWorldEdPreviewPanel;
class EGWorldObject;
struct egRflEditor;

int EGWorldEd_Run( eg_cpstr16 InitialFilename , const struct egSdkEngineInitParms& EngineParms );
void EGWorldEd_InsertNewWorldObject( EGClass* ObjectClass );
void EGWorldEd_SetWorldObjectToEdit( EGWorldObject* ObjectToEdit );
void EGWorldEd_DeleteWorldObject( EGWorldObject* Object );
void EGWorldEd_MoveObject( EGWorldObject* ObjToMove , EGWorldObject* ObjToMoveBefore , EGWorldObject* ObjParent );
void EGWorldEd_OnPropChanged( egRflEditor& BaseEditor , const egRflEditor& ChangedProperty );
void EGWorldEd_DrawPreview();
void EGWorldEd_SetCameraPose( const eg_transform& CameraPose );
void EGWorldEd_SetDirty();

EGWorldEdObjLibPanel& EGWorldEd_GetObjLibPanel();
EGWorldEdWorldObjsPanel& EGWorldEd_GetWorldObjsPanel();
EGWorldEdPreviewPanel& EGWorldEd_GetPreviewPanel();
EGWorldFile& EGWorldEd_GetWorldFile();
