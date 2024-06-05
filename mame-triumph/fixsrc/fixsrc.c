#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dos.h>

#include <inline/exec.h>
#include <inline/dos.h>

#include "scandir.h"

extern struct ExecBase	*SysBase;

struct DosLibrary		*DOSBase	= NULL;

BOOL FileFunc(char *name, struct FileInfoBlock *fib)
{
	static char include[]="include";
	BPTR		file;
	LONG		size;
	char		*buffer;
	int			rename;
	int			i, j, k;

	if(SetSignal(0, 0) & SIGBREAKF_CTRL_C)
		return(FALSE);
	
	rename = FALSE;
	
	for(i = 0; name[i]; i++)
	{
		if(isupper(name[i]))
		{
			name[i]	= tolower(name[i]);
			rename	= TRUE;
		}
	}

	if(fib->fib_DirEntryType < 0)
	{
		size = fib->fib_Size;
	
		buffer = malloc(size);
		
		if(buffer)
		{
			file = Open(name, MODE_OLDFILE);
			
			if(file)
			{
				if(Read(file, buffer, size) == size)
				{
					SetFileSize(file, 0, OFFSET_BEGINNING);
					Seek(file, 0, OFFSET_BEGINNING);
				
					for(i = 0, j = 0; j < size; j++)
					{
						if(buffer[j] == 13)
						{
							if(i < j)
								Write(file, &buffer[i], j - i);

							i = j + 1;
						}
						else if(buffer[j] == '#')
						{
							j++;
							
							for(k = 0; (k < 7) && (j < size) && (include[k] == tolower(buffer[j])); j++, k++);

							if(k == 7)
							{
								for(; (j < size) && ((buffer[j] == ' ') || (buffer[j] == '\t')); j++);
	
								if(j < size)
								{
									if((buffer[j] == '<') || (buffer[j] == '"'))
									{
										j++;
										
										for(; (j < size) && (buffer[j] != '>') && (buffer[j] != '"'); j++)
										{
											if(buffer[j] == '\\')
												buffer[j] = '/';
											else
												buffer[j] = tolower(buffer[j]);
										}
									}
									else
										j--;
								}
							}
							else
								j--;
						}
					}
					
					if(i < j)
						Write(file, &buffer[i], j - i);
					
					Close(file);
				}
				else
					Close(file);
			}
			
			free(buffer);
		}
	}

	if(rename)
		Rename(name,name);
	
	return(TRUE);
}

int main(int argc, char **argv)
{
	char *dir;
	int	rc;

	if((DOSBase = (struct DosLibrary *) OpenLibrary("dos.library", 37)))
	{
		if(argc > 1)
			dir = argv[1];
		else
			dir = NULL;
		
		rc = ScanDir(dir, FileFunc, SDF_DirCall|SDF_FilePattern|SDF_FileCall|SDF_Recursive, "(#?.c|#?.h|#?.txt)");
			
		if(rc)
			printf("Error: %d\n", rc);
	}

	return(0);
}
