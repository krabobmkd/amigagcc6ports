#include <stdio.h>
#include <stdlib.h>

#include <dos/dos.h>

#include <inline/dos.h>

#include "scandir.h"

extern struct DosLibrary	*DOSBase;

struct dir
{
	struct dir				*Next;
	struct dir				*Prev;
	struct FileInfoBlock	*FIB;
	BPTR					Lock;
	int						NameLen;
};

typedef struct dir dir_t;

int ScanDir(char *dir, dir_f func, int flags, char *pattern)
{
	STRPTR	parsed_pattern;
	dir_t	root_dir;
	dir_t	*current_dir;
	char	name[256];
	int		name_len;
	int		len;
	int		rc;
	
	root_dir.Next	= NULL;
	root_dir.Prev	= NULL;
	root_dir.FIB	= NULL;
	current_dir		= &root_dir;

	if(pattern)
	{
		len = strlen(pattern);
	
		if(!(parsed_pattern = malloc(2*len+2)))
			return(SDE_OutOfMemory);
		
		if(ParsePatternNoCase(pattern, parsed_pattern, 2*len+2) < 0)
			return(SDE_PatternError);
	}
	else
		parsed_pattern = NULL;

	if(dir)
	{
		name_len = strlen(dir);

		if(name_len >= sizeof(name))
			return(SDE_NameBufferFull);

		strcpy(name, dir);
		
		if((name[name_len-1] != ':') || (name[name_len-1] != '/'))
		{
			if(name_len >= sizeof(name) - 1)
				return(SDE_NameBufferFull);
	
			name[name_len] = '/';
			name_len++;
			name[name_len] = 0;
		}
	}
	else
	{
		name_len	= 0;
		name[0]		= 0;
	}

	rc = SDE_LockFailed;
		
	while((current_dir->Lock = Lock(name, ACCESS_READ)))
	{
		if(!current_dir->FIB)
		{
			if(!(current_dir->FIB = AllocDosObject(DOS_FIB, NULL)))
			{
				rc = SDE_AllocFIBFailed;
				break;
			}
		}
		
		if(!Examine(current_dir->Lock, current_dir->FIB))
		{
			rc = SDE_ExamineFailed;
			break;
		}
		
		if(current_dir->FIB->fib_DirEntryType <= 0)
		{
			rc = SDE_LockNotDir;
			break;
		}
		
		while(1)
		{
			if(ExNext(current_dir->Lock, current_dir->FIB))
			{
				if(current_dir->FIB->fib_DirEntryType > 0)
				{
					if(!(flags & SDF_DirPattern)
					|| MatchPatternNoCase(parsed_pattern, current_dir->FIB->fib_FileName))
					{
						len = strlen(current_dir->FIB->fib_FileName);
						
						if(name_len + len >= sizeof(name))
						{
							rc = SDE_NameBufferFull;
							break;
						}
						
						strcpy(&name[name_len], current_dir->FIB->fib_FileName);

						if(flags & SDF_DirCall)
						{
							if(!func(name, current_dir->FIB))
							{
								rc = SDE_FuncBreak;
								break;
							}
						}						
						
						if(flags & SDF_Recursive)
						{
							if(name_len + len >= sizeof(name) - 1)
							{
								rc = SDE_NameBufferFull;
								break;
							}

							current_dir->NameLen = name_len;

							if(current_dir->Next)
								current_dir = current_dir->Next;
							else
							{
								if(!(current_dir->Next = calloc(sizeof(dir_t),1)))
								{
									rc = SDE_OutOfMemory;
									break;
								}
								
								current_dir->Next->Prev	= current_dir;
								current_dir				= current_dir->Next;
							}
							
							name_len		+= len;
							name[name_len]	= '/';
							name_len++;
							name[name_len]	= 0;

							break;
						}
					}
				}
				else if(current_dir->FIB->fib_DirEntryType < 0)
				{
					if((flags & SDF_FileCall) && (!(flags & SDF_FilePattern)
					|| MatchPatternNoCase(parsed_pattern, current_dir->FIB->fib_FileName)))
					{
						len = strlen(current_dir->FIB->fib_FileName);
						
						if(name_len + len >= sizeof(name))
						{
							rc = SDE_NameBufferFull;
							break;
						}
						
						strcpy(&name[name_len], current_dir->FIB->fib_FileName);

						if(!func(name, current_dir->FIB))
						{
							rc = SDE_FuncBreak;
							break;
						}
					}
				}
			}
			else
			{
				if(current_dir->Lock)
				{
					UnLock(current_dir->Lock);
					current_dir->Lock = NULL;
				}
				
				if(current_dir->Prev)
				{
					current_dir	= current_dir->Prev;
					name_len	= current_dir->NameLen;
				}
				else
				{
					rc = SDE_OK;
					break;
				}
			}
		}
		
		if(rc != SDE_LockFailed)
			break;
	}

	current_dir = &root_dir;
	
	while(1)
	{
		if(current_dir->FIB)
			FreeDosObject(DOS_FIB, current_dir->FIB);
		
		if(current_dir->Lock)
			UnLock(current_dir->Lock);
		
		if(current_dir->Next)
			current_dir = current_dir->Next;
		else
			break;
	}
	
	while(current_dir->Prev)
	{
		current_dir = current_dir->Prev;
		
		free(current_dir->Next);
	}

	if(parsed_pattern)
		free(parsed_pattern);

	return(rc);
}
