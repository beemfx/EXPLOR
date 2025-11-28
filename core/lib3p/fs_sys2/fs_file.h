#include "fs_sys2.h"
#include "fs_bdio.h"

class FS_SYS2_EXPORTS CLFile
{
public:
	fs_dword Read(void* pOutBuffer, fs_dword nSize);
	fs_dword Write(const void* pInBuffer, fs_dword nSize);
	fs_dword Tell();
	fs_dword Seek(LF_SEEK_TYPE nMode, fs_long nOffset);
	fs_dword GetSize();
	const void* GetMemPointer();
	fs_bool IsEOF();

private:
	fs_dword m_nAccess;
	fs_dword m_nSize;
	fs_dword m_nFilePointer;
	fs_path m_szPathW;
	fs_byte* m_pData;
	BDIO_FILE m_BaseFile;
	fs_dword m_nBaseFileBegin; //Where the CLFile starts in the base file (0 if the file is being accessed directly from the disk).
	fs_bool m_bEOF;
	
//Public methods.
private:
	friend class CLFileSystem;
	void* operator new(fs_size_t Size);
	void operator delete(void*);

	CLFile();
	~CLFile();
};