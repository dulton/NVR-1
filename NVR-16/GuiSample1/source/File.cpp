

#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32

#include <direct.h>
//#include <Windows.h>

#endif

#ifdef  __linux__
#include <sys/statfs.h>
#endif

#ifdef __TCS__
#include <unistd.h>
#endif

#include "System/File.h"

#ifndef WIN32
#define _O_BINARY 0
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_RDWR O_RDWR
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
#define _open open
#define _access access
#define _stat stat
#endif

#include <string>

//////////////////////////////////////////////////////////////////////////////
///////// File find wrap
static long findfirst(const char* fileName, FileInfo* data)
{
#ifdef WIN32
	return _findfirst(fileName, (_finddata_t*)data);
#else
	return -1;
#endif
}

static int findnext(long handle, FileInfo* data)
{
#ifdef WIN32
	return _findnext(handle, (_finddata_t*)data);
#else
	return -1;
#endif
}

static int findclose(long handle)
{
#ifdef WIN32
	return _findclose(handle);
#else
	return -1;
#endif
}

#ifndef WIN32
static int _mkdir( const char *dirname)
{
	return mkdir((char *)dirname, 777);
}

static int _rmdir( const char *dirname)
{
	return rmdir((char *)dirname);
}
#endif

static int _statfs(const char *path, uint64* userFreeBytes, uint64* totalBytes, uint64* totalFreeBytes)
{
#ifdef WIN32
	/*GetDiskFreeSpaceEx(path,
		(PULARGE_INTEGER)userFreeBytes,
		(PULARGE_INTEGER)totalBytes,
		(PULARGE_INTEGER)totalFreeBytes); */
#elif defined(__GNUC__)
	struct statfs info;

	statfs(path, &info);
	*userFreeBytes = (uint64)(info.f_bsize) * info.f_bavail;
	*totalBytes = (uint64)(info.f_bsize) * info.f_blocks;
	*totalFreeBytes = (uint64)(info.f_bsize) * info.f_ffree;
#endif
	return 0;
}

static int __stat( const char *path, FileInfo * info)
{
	int ret;
	struct _stat s;

	ret = _stat(path, &s);
	if(ret != 0)
	{
		return ret;
	}

	strcpy(info->name, path);
	info->attrib = s.st_mode;
	info->time = s.st_mtime;
	info->size = s.st_size;

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////   various io entries
//csp modify 20121125
//static const int fsMaxOptNum = 32;
static const int fsMaxOptNum = 1;
static char fsPaths[fsMaxOptNum][32] = {};
static FSOperations fsOpts[fsMaxOptNum] = 
{
	{
		fopen,
		fclose,
		fread,
		fwrite,
		fflush,
		fseek,
		ftell,
		fgets,
		fputs,
		rename,
		remove,
		findfirst,
		findnext,
		findclose,
		_mkdir,
		_rmdir,
		_statfs,
		_access,
		__stat,
	},
};

void hookFS(const char* path, const FSOperations* opts)
{
	int i;

	for(i = 1; i < fsMaxOptNum; i++)
	{
		if(opts)
		{
			if(strlen(fsPaths[i]) == 0)
			{
				strcpy(fsPaths[i], path);
				fsOpts[i] = *opts;
				break;
			}
		}
		else
		{
			if(strcmp(path, fsPaths[i]) == 0)
			{
				strcpy(fsPaths[i], "");
				break;
			}
		}
	}
}

static FSOperations* findOpts(const char* pFileName)
{
	int i;

	for(i = 1; i < fsMaxOptNum; i++)
	{
		int len = strlen(fsPaths[i]);
		if(len != 0 && strncmp(pFileName, fsPaths[i], len) == 0)
		{
			break;
		}
	}

	//csp modify 20121125
	//if(i == fsMaxOptNum)
	if(i >= fsMaxOptNum)
	{
		i = 0;
	}

	return &fsOpts[i];
}

///////////////////////////////////////////////////////////////////////////////
///////   CFile implement
CFile::CFile()
{
	m_pFile = NULL;
	m_pBuffer = NULL;
	m_dwLength = 0;
	m_opt = &fsOpts[0];
}

CFile::~CFile()
{
	if(m_pFile)
	{
		Close();
	}
	if(m_pBuffer)
	{
		UnLoad();
	}
}

bool CFile::Open(const char* pFileName, unsigned int dwFlags /* = modeReadWrite */)
{
	int oflag = _O_BINARY;
	const char* mode = "";

	m_opt = findOpts(pFileName);
	if(m_opt != &fsOpts[0])
	{
		switch (dwFlags & 0xf) 
		{
		case modeRead:
			mode = "rb";
			break;
		case modeWrite:
			mode = "wb";
			break;
		case modeReadWrite:
			if(dwFlags & modeCreate)
			{
				mode = "wb+";
			}
			else
			{
				mode = "rb+";
			}
		}
		m_pFile = m_opt->fopen(pFileName, mode);

		if(!m_pFile)
		{
			return false;
		}

		return true;
	}

	switch (dwFlags & 0xf) 
	{
	case modeRead:
		oflag |= _O_RDONLY;
		break;
	case modeWrite:
		oflag |= _O_WRONLY;
		break;
	case modeReadWrite:
		oflag |= _O_RDWR;
		break;
	default:
		break;
	}

	if(dwFlags & modeCreate)
	{
		oflag |= _O_CREAT;
		if(!(dwFlags & modeNoTruncate))
		{
			oflag |= _O_TRUNC;
		}
	}

	if(dwFlags & osNoBuffer)
	{
		
	}

	int fd = _open(pFileName, oflag, _S_IREAD | _S_IWRITE);

	if(fd == -1)
	{
		return false;
	}

	switch (dwFlags & 0xf) 
	{
	case modeRead:
		mode = "rb";
		break;
	case modeWrite:
		mode = "wb";
		break;
	case modeReadWrite:
		if(dwFlags & modeCreate)
		{
			mode = "wb+";
		}
		else
		{
			mode = "rb+";
		}
	}

	m_pFile = fdopen(fd, mode);

	if(!m_pFile)
	{
		return false;
	}

	return true;
}

bool CFile::IsOpened()
{
	return m_pFile != 0;
}

void CFile::Close()
{
	if(!m_pFile)
	{
		return;
	}

	m_opt->fclose(m_pFile);

	m_pFile = NULL;
}

unsigned char * CFile::Load(const char* pFileName)
{
	unsigned int ret;

	assert(!m_pBuffer);
	if(Open(pFileName, modeRead))
	{
		m_dwLength = GetLength();
		if(m_dwLength)
		{
			m_pBuffer = new unsigned char[m_dwLength];
			if(m_pBuffer)
			{
				Seek(0, begin);
				ret = Read(m_pBuffer, m_dwLength);
				assert(ret == m_dwLength);
			}
		}
		Close();
	}
	return m_pBuffer;
}

void CFile::UnLoad()
{
	if(m_pBuffer)
	{
		delete []m_pBuffer;
		m_pBuffer = NULL;
	}
}

unsigned int CFile::Read(void *pBuffer, unsigned int dwCount)
{
	if(!m_pFile)
	{
		return 0;
	}

	return m_opt->fread(pBuffer, 1, dwCount, m_pFile);
}

unsigned int CFile::Write(void *pBuffer, unsigned int dwCount)
{
	if(!m_pFile)
	{
		return 0;
	}

	return m_opt->fwrite(pBuffer, 1, dwCount, m_pFile);
}

void CFile::Flush()
{
	if(!m_pFile)
	{
		return;
	}

	m_opt->fflush(m_pFile);
}

unsigned int CFile::Seek(long lOff, unsigned int nFrom)
{
	if(!m_pFile)
	{
		return 0;
	}
	int origin = 0;
	switch(nFrom){
	case begin:
		origin = SEEK_SET;
		break;
	case current:
		origin = SEEK_CUR;
		break;
	case end:
		origin = SEEK_END;
		break;
	}

	if(!m_opt->fseek(m_pFile, lOff, origin)){
		return GetPosition();
	};
	return 0;
}

unsigned int CFile::GetPosition()
{
	if(!m_pFile)
	{
		return 0;
	}

	int pos = m_opt->ftell(m_pFile);
	if(pos >= 0)
	{
		return pos;
	}
	return 0;
}

unsigned int CFile::GetLength()
{
	if(!m_pBuffer)//非Load方式装载
	{
		unsigned int oldpos = GetPosition();
		if(Seek(0, end))
		{
			m_dwLength = GetPosition();
		};
		Seek(oldpos, begin);
	}

	return m_dwLength;
}

char * CFile::Gets(char *s, int size)
{
	if(!m_pFile)
	{
		return 0;
	}

	return m_opt->fgets(s, size, m_pFile);
}

int CFile::Puts(char *s)
{
	if(!m_pFile)
	{
		return 0;
	}

	return m_opt->fputs(s, m_pFile);
}

bool CFile::Rename(const char* oldName, const char* newName)
{
	FSOperations* opt = findOpts(oldName);

	return (opt->rename(oldName, newName) == 0);
}

bool CFile::Remove(const char* fileName)
{
	FSOperations* opt = findOpts(fileName);

	return (opt->remove(fileName) == 0);
}

bool CFile::MakeDirectory(const char* dirName)
{
	FSOperations* opt = findOpts(dirName);

	return(opt->mkdir(dirName) == 0);
}

bool CFile::RemoveDirectory(const char* dirName)
{
	FSOperations* opt = findOpts(dirName);

	return (opt->rmdir(dirName) == 0);
}

bool CFile::StatFS(const char* path, uint64* userFreeBytes, uint64* totalBytes, uint64* totalFreeBytes)
{
	FSOperations* opt = findOpts(path);

	return (opt->statfs(path, userFreeBytes, totalBytes, totalFreeBytes) == 0);
}

bool CFile::Access(const char* path, int mode)
{
	FSOperations* opt = findOpts(path);

	return (opt->access(path, mode) == 0);
}

bool CFile::Stat(const char* path, FileInfo* info)
{
	FSOperations* opt = findOpts(path);

	return (opt->stat(path, info) == 0);
}


///////////////////////////////////////////////////////////////////////////////
///////   CFileFind implement
CFileFind::CFileFind()
{
	m_opt = &fsOpts[0];
}

CFileFind::~CFileFind()
{

}

bool CFileFind::findFile(const char* fileName)
{
	const char* p = fileName + strlen(fileName);
	while(*p != '/' && p != fileName)
	{
		p--;
	}
	m_path.assign(fileName, p - fileName + 1);

	m_opt = findOpts(fileName);

	return ((m_handle = m_opt->findfirst(fileName, &m_fileInfo)) != -1);
}

bool CFileFind::findNextFile()
{
	return (m_opt->findnext(m_handle, &m_fileInfo) != -1);
}

void CFileFind::close()
{
	m_opt->findclose(m_handle);
}

unsigned int CFileFind::getLength()
{
	return (unsigned int)m_fileInfo.size;
}

std::string CFileFind::getFileName()
{
	return m_fileInfo.name;
}

std::string CFileFind::getFilePath()
{
	return m_path + m_fileInfo.name;
}

bool CFileFind::isReadOnly()
{
	return ((m_fileInfo.attrib & CFile::readOnly) != 0);
}

bool CFileFind::isDirectory()
{
	return ((m_fileInfo.attrib & CFile::directory) != 0);
}

bool CFileFind::isHidden()
{
	return ((m_fileInfo.attrib & CFile::hidden) != 0);
}

bool CFileFind::isNormal()
{
	return (m_fileInfo.attrib == CFile::normal);
}
