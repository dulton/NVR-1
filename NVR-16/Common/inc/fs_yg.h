#ifndef	__FS_YG_H__
#define	__FS_YG_H__

//FILE_VECTOR_SIZE 文件容器大小
//NFILE_PER_SECT 容器中可容纳的文件数目

#define FILE_VECTOR_SIZE	(256*1024*1024)
#define SIZE_OF_FILE_VECTOR	((long long)(256*1024*1024))
#define NFILE_PER_SECT		(32)

//#define FILE_VECTOR_SIZE	(128*1024*1024)
//#define SIZE_OF_FILE_VECTOR	((long long)(128*1024*1024))
//#define NFILE_PER_SECT		(256)

#endif

