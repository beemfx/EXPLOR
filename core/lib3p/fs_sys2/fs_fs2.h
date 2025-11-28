#ifndef __FS_FS2_H__
#define __FS_FS2_H__

#include "fs_sys2.h"
#include "fs_file.h"
#include "fs_list_stack.h"
#include "fs_internal.h"

template<class T> static void FS_SAFE_DELETE_ARRAY(T*& p, LF_ALLOC_REASON reason){ if(p){FS_Free(p, reason); p=FS_NULL;} }
template<class T> static void FS_SAFE_DELETE(T*& p, LF_ALLOC_REASON reason){ if(p){FS_Free(p, reason); p=FS_NULL;} }

template<class T> static T FS_max(T v1, T v2){return ((v1)>(v2))?(v1):(v2); }
template<class T> static T FS_min(T v1, T v2){return ((v1)<(v2))?(v1):(v2); }
template<class T> static T FS_clamp(T v1, T min, T max){ return ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) ); }

class FS_SYS2_EXPORTS CLFileSystem: public IFsMemObj
{
public:
	CLFileSystem();
	~CLFileSystem();
	fs_dword MountBase(fs_cstr szOSPath);
	fs_dword Mount(fs_cstr szOSPath, fs_cstr szMountPath, fs_dword Flags);
	fs_dword MountLPK(fs_cstr szMountedFile, fs_dword Flags);
	fs_bool UnMountAll();
	fs_bool Exists(fs_cstr szFilename);
	CLFile* OpenFile(fs_cstr szFilename, fs_dword Access, fs_dword CreateMode);
	fs_bool CloseFile(CLFile* pFile);
	void EnumerateFilesOfType( fs_cstr Ext , FS_ENUMERATE_FUNC EnumerateFunc , void* Data );
	void PrintMountInfo(FS_PRINT_MOUNT_INFO_TYPE Type);

private:
	static const fs_dword PART_TABLE_SIZE = 256;
	//Data containing information about a mounted file:
	struct MOUNT_FILE
	{
		fs_path szMountFileW;
		fs_path szOSFileW;
		fs_dword ExtOffset;
		fs_dword Flags;
		fs_dword nSize;
		fs_dword nCmpSize;
		fs_dword nOffset;
		static const fs_dword MOUNT_FILE_ARCHIVED =(1<<0); //This file is archived.
		static const fs_dword MOUNT_FILE_READONLY =(1<<1); //The file is read only.
		static const fs_dword MOUNT_FILE_ZLIBCMP  =(1<<2);  //The file is compressed with zlib compression.
		static const fs_dword MOUNT_FILE_DIRECTORY=(1<<3); //The mount file is a directory.
		static const fs_dword MOUNT_FILE_EMPTY    =(1<<4);    //This partition table entry is empty and can be mounted over.
	};

	//The partition table is used to store all the mounted file information.
	//It stores an array of MOUNT_FILEs in a hash table and uses hash values
	//to lookup information.
	class FS_SYS2_EXPORTS CPartTable: public IFsMemObj
	{
	private:
		static const fs_dword HASH_SIZE = 1024;
		struct MOUNT_FILE_EX: public MOUNT_FILE, CLfListStack::LSItem
		{
			fs_dword nHashValue;
			MOUNT_FILE_EX* pNext;
		};
		
		MOUNT_FILE_EX* m_HashList[HASH_SIZE];
		
		const fs_dword m_nMaxFiles;
		MOUNT_FILE_EX* m_pMasterList;
		CLfListStack   m_UnusedFiles;
		CPartTable*const m_Next;
		
		
	public:
		CPartTable( CPartTable* Next , fs_dword nMaxFiles );
		~CPartTable();
		
		void Clear();
		fs_bool IsFull()const;

		CPartTable* GetNext()const{ return m_Next; }
		
		//The MountFile function mounts the specified MOUNT_FILE data.
		//The only flag that is meaningful here is MOUNT_FILE_OVERWRITE
		//and MOUNT_FILE_OVERWRITELPKONLY.
		//which, when specified, will cause a duplicate mount file to
		//overwrite a previous mount file.  If not specifed the duplicate
		//will not be overwritten.  The LPK only will only overwrite a file
		//if the previous file is archived.  If the file is a disk file it
		//will not be overwritten.  This is useful so that new lpk files
		//will take priority over old ones, but disk files still have
		//the highest priority.
		fs_bool MountFile(MOUNT_FILE* pFile, fs_dword Flags);
		
		CLFileSystem::MOUNT_FILE* FindFile(fs_cstr szFile);
		
		void PrintMountInfo(FS_PRINT_MOUNT_INFO_TYPE Type);
		static void PrintFileInfo(MOUNT_FILE_EX* pFile);

		void EnumerateFilesOfType( fs_cstr Ext , FS_ENUMERATE_FUNC EnumerateFunc , void* Data );
	private:
		static fs_dword GetHashValue1024(fs_cstr szString);
	};	
	
private:
	CPartTable* m_PrtTableList;
	fs_bool m_bBaseMounted;
	fs_path m_szBasePath;
	
	fs_dword MountDir(fs_cstr szOSPath, fs_cstr szMountPath, fs_dword Flags);
	fs_bool MountFile(MOUNT_FILE* pFile, fs_dword Flags);
	
	MOUNT_FILE* GetFileInfo( fs_cstr szMountPath , CPartTable** OwnerTable );
	
	CLFile* OpenExistingFile(const MOUNT_FILE* pMountInfo, BDIO_FILE File, fs_dword Access, fs_bool bNew);
};

#endif __FS_FS2_H__