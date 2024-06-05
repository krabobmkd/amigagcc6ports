#ifndef SCANDIR_H
#define SCANDIR_H

#include <dos/dos.h>

#define SDE_OK				0
#define SDE_NameBufferFull	1
#define SDE_LockFailed		2
#define SDE_ExamineFailed	3
#define SDE_LockNotDir		4
#define SDE_OutOfMemory		5
#define SDE_FuncBreak		6
#define SDE_PatternError	7
#define SDE_AllocFIBFailed	8

#define SDF_Recursive		1
#define SDF_DirPattern		2
#define SDF_FilePattern		4
#define SDF_DirCall			8
#define SDF_FileCall		16

typedef BOOL (*dir_f)(char *name, struct FileInfoBlock *fib);

int ScanDir(char *dir, dir_f func, int flags, char *pattern);

#endif
