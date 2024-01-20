#ifndef FILE_H
#define FILE_H
/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: file.h,v 1.1 1999/04/28 18:52:08 meh Exp $
 *
 * $Log: file.h,v $
 * Revision 1.1  1999/04/28 18:52:08  meh
 * Initial revision
 *
 *
 *************************************************************************/

#include <string>

#ifndef BPTR
//from dos/dos.h
typedef long  BPTR;
#endif

struct File
{
  BPTR          File;
  int          Type;
  std::string Name;
  unsigned char *Data;
  int           Length;
  int           Offset;
  unsigned int  CRC;
};

#define FILETYPE_NORMAL 0
#define FILETYPE_TMP    1
#define FILETYPE_ZIP    2
#define FILETYPE_CUSTOM 3

struct File *OpenFile(const char *dir_name, const char *file_name, int mode);
struct File *OpenFileType(const char *dir_name, const char *file_name, int mode, int filetype);
void        CloseFile(struct File *file);

extern "C" {
#ifdef POWERUP
#ifdef __PPC__
#include <powerup/gcclib/powerup_protos.h>
#endif

#define ReadFile(a,b,c)   PPCRead(a,b,c)
#define WriteFile(a,b,c)  PPCWrite(a,b,c)
#define SeekFile(a,b,c)   PPCSeek(a,b,c)
#else
#include <exec/types.h>
#include <inline/dos.h>

#define ReadFile(a,b,c)   Read(a,b,c)
#define WriteFile(a,b,c)  Write(a,b,c)
#define SeekFile(a,b,c)   Seek(a,b,c)
#endif

#endif
}
