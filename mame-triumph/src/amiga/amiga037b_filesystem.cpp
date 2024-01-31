/**************************************************************************
 *
 * Copyright (C) 2024 Vic Ferry (http://github.com/krabobmkd)
 * forked from 1999 Mats Eirik Hansen (mats.hansen at triumph.no)
 *
 * $Id: amiga.c,v 1.1 1999/04/28 18:50:15 meh Exp meh $
 *
 * $Log: amiga.c,v $
 * Revision 1.1  1999/04/28 18:50:15  meh
 * Initial revision
 *
 *
 *************************************************************************/
// from amiga
//#include <intuition/intuition.h>

#include <proto/dos.h>

// from mame:
#include "osdepend.h"
//from this project
#include "file.h"
// from project from this repos, static zlib:
#include "zlib.h"

#include <string>
// for memcpy
#include <string.h>
#include <cstdlib>



struct DosLibrary;
extern struct DosLibrary    *DOSBase;

extern std::string ROMZipName,SampleZipName;

int load_zipped_file(const char *zipfile, const char *filename, unsigned char **buf, unsigned int *length);


/* gamename holds the driver name, filename is only used for ROMs and    */
/* samples. If 'write' is not 0, the file is opened for write. Otherwise */
/* it is opened for read. */

int osd_faccess (const char *filename, int filetype)
{
    //TODO
  return(1);
}

void *osd_fopen(const char *gamename,const char *filename,int filetype,int write)
{
  struct File *file;
  std::string *zip_name=NULL;

  file = NULL;

  if(!write)
  {
    if(filetype == OSD_FILETYPE_ROM)
      zip_name = &ROMZipName;
    else if(filetype == OSD_FILETYPE_SAMPLE)
      zip_name = &SampleZipName;

    if(zip_name && !zip_name->empty() )
    {
      file = (struct File *)calloc(sizeof(struct File), 1);

      if(file)
      {
        if(!load_zipped_file(zip_name->c_str(), filename, &file->Data, &file->Length))
        {
          file->Type  = FILETYPE_CUSTOM;
          file->CRC = crc32(0, file->Data, file->Length);
        }
        else
        {
          free(file);
          file = NULL;
        }
      }
    }
  }

  if(!file)
  {
    file = OpenFileType(gamename, filename, write ? MODE_NEWFILE : MODE_OLDFILE, filetype);

    if(file && (file->Type == FILETYPE_ZIP))
    {
      if(filetype == OSD_FILETYPE_ROM)
        zip_name = &ROMZipName;
      else if(filetype == OSD_FILETYPE_SAMPLE)
        zip_name = &SampleZipName;
      else
        zip_name= NULL;

      /* Cache the zip filename. */

      if(zip_name) (*zip_name) = file->Name;

      if(load_zipped_file(zip_name->c_str(), filename, &file->Data, &file->Length))
      {
        file->Data = NULL;
        osd_fclose(file);

        return(NULL);
      }

      file->CRC = crc32(0, file->Data, file->Length);
    }
  }

  return((void *) file);
}

int osd_fread(void *file_handle, void *buffer, int length)
{
  struct File *file;
  LONG len;

  file = (struct File *) file_handle;

  switch(file->Type)
  {
    case FILETYPE_ZIP:
    case FILETYPE_CUSTOM:
      if(file->Data)
      {
        len = file->Length - file->Offset;

        if(len > length)
          len = length;

        memcpy(buffer, &file->Data[file->Offset], len);

        file->Offset += len;

        return(len);
      }

      break;

    case FILETYPE_NORMAL:
    case FILETYPE_TMP:
      len = ReadFile(file->File, buffer, length);

      return(len);
  }

  return(0);
}

void osd_fclose(void *file)
{
  if(((((struct File *) file)->Type == FILETYPE_ZIP) || (((struct File *) file)->Type == FILETYPE_CUSTOM)) && ((struct File *) file)->Data)
    free(((struct File *) file)->Data);

  if(((struct File *) file)->Type == FILETYPE_CUSTOM)
    free(file);
  else
    CloseFile((struct File *) file);
}


int osd_fwrite(void *void_file_p, const void *buffer_p, int length)
{
  struct File *file_p;
  LONG rc;

  file_p = (struct File *) void_file_p;

  switch(file_p->Type)
  {
    case FILETYPE_ZIP:
    case FILETYPE_CUSTOM:
      return(-1);

    case FILETYPE_NORMAL:
    case FILETYPE_TMP:
      rc = WriteFile(file_p->File, (void *) buffer_p, length);

      if(rc > 0)
        return(rc);
  }

  return(0);
}

int osd_fwrite_swap(void *file,const void *buffer,int length)
{
	int i;
	unsigned char *buf;
	unsigned char temp;
	int res;


	buf = (unsigned char *)buffer;
	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	res = osd_fwrite(file,buffer,length);

	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	return res;
}

int osd_fseek(void *file, int position, int mode)
{
  LONG  rc = 0;

  if((((struct File *) file)->Type == FILETYPE_ZIP) || (((struct File *) file)->Type == FILETYPE_CUSTOM))
  {
    switch(mode)
    {
      case SEEK_SET:
        ((struct File *) file)->Offset = position;
        break;
      case SEEK_CUR:
        ((struct File *) file)->Offset += position;
        break;
      case SEEK_END:
        ((struct File *) file)->Offset = ((struct File *) file)->Length + position;
        break;
      default:
        return(-1);
    }
  }
  else
  {
    switch(mode)
    {
      case SEEK_SET:
        rc = SeekFile(((struct File *) file)->File, position, OFFSET_BEGINNING);
        break;
      case SEEK_CUR:
        rc = SeekFile(((struct File *) file)->File, position, OFFSET_CURRENT);
        break;
      case SEEK_END:
        rc = SeekFile(((struct File *) file)->File, position, OFFSET_END);
        break;
      default:
        return(-1);
    }

#ifdef POWERUP
    return(rc);
#else
    if(DOSBase->dl_lib.lib_Version > 37)
    {
      if(rc == -1)
        return(-1);
      else
        return(0);
    }
    else
    {
      if(IoErr())
        return(-1);
      else
        return(0);
    }
#endif
  }

  return(rc);
}


int osd_fread_swap(void *file_handle, void *buffer, int length)
{
	int i;
	uint8_t *buf;
	uint8_t temp;
	int res;

	res = osd_fread(file_handle,buffer,length);

	buf = (uint8_t*)buffer;
	for (i = 0;i < length;i+=2)
	{
		temp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = temp;
	}

	return res;
}

// - - - - - -krb v0.37
int osd_fgetc(void *file)
{
    unsigned char c;
    if( osd_fread(file, &c, 1)==0)
    {
        return -1; // EOF
    }
    return (int)c;
}
/* stdio definition:
pushes the character ch (reinterpreted as unsigned char) into the input buffer
associated with the stream stream in such a manner than subsequent read operation
from stream will retrieve that character.
The external device associated with the stream is not modified.
*/
int osd_ungetc(int c, void *file)
{
    if(c==-1) return -1;
    // actually mame use is just to rewind a byte, wich means:
    osd_fseek(file, -1, SEEK_CUR);
    return c;
}
/*
reads a line from the specified stream and stores it into the string pointed to by str. It stops when either (n-1) characters are read, the newline character is read, or the end-of-file is reached, whichever comes first.
*/
char *osd_fgets(char *s, int n, void *file)
{
    char *sr = s;
    while(n>1)
    {
        unsigned char c;
        if( osd_fread(file, &c, 1)==0) break;
        if(c==0) break;
        *s++ = c;
        n--;
    }
    *s++ = 0;

    return sr;
}
int osd_feof(void *file)
{
  struct File *file_p = (struct File *) file;
  if(!file_p) return 1;
  // cast bool to int.
  return (int)( (uint32_t)file_p->Offset >= (uint32_t)file_p->Length );
}
// only used by mame datafile.cpp
int osd_ftell(void *file)
{
    struct File *file_p = (struct File *) file;
    if(!file_p) return -1;
    return (file_p->Offset);
}



