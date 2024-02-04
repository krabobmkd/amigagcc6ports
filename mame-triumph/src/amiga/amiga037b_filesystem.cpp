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
#include <stdio.h>


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
  printf("osd_fopen:type:%d:w:%d,%s:%s\n",(int)filetype,write,gamename,filename);
  printf("ROMZipName:%s\n",ROMZipName.c_str());

  sFile *file=NULL;
  std::string *zip_name=NULL;

  if(!write)
  {
    if(filetype == OSD_FILETYPE_ROM)
      zip_name = &ROMZipName;
    else if(filetype == OSD_FILETYPE_SAMPLE)
      zip_name = &SampleZipName;

    if(zip_name && !zip_name->empty() )
    {
      file = new sFile;

      if(file)
      {
        if(!load_zipped_file(zip_name->c_str(), filename, &file->Data, &file->Length))
        {
          file->Type  = FILETYPE_CUSTOM;
          file->CRC = crc32(0, file->Data,(uint32_t) file->Length);
        }
        else
        {
          delete file;
          file = NULL;
        }
      }
    }
  }

  if(!file)
  {
    file = OpenFileType(gamename, filename, write ? MODE_NEWFILE : MODE_OLDFILE, filetype);
    // test
    printf("OpenFileType:%08x\n",(int)file);
    if(file)
    {
        printf("l:%d  crc:%08x\n",(int)file->Length,file->CRC );
    }

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
        return(NULL);
      }

      file->CRC = crc32(0, file->Data, file->Length);
    }
  }

  return((void *) file);
}

int osd_fread(void *file_handle, void *buffer, int length)
{
  struct sFile *file;
  LONG len;

  file = (struct sFile *) file_handle;

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
  if(((((struct sFile *) file)->Type == FILETYPE_ZIP) || (((struct sFile *) file)->Type == FILETYPE_CUSTOM)) && ((struct sFile *) file)->Data)
    free(((struct sFile *) file)->Data);

  if(((struct sFile *) file)->Type == FILETYPE_CUSTOM)
    free(file);
  else
    CloseFile((struct sFile *) file);
}


int osd_fwrite(void *void_file_p, const void *buffer_p, int length)
{
  struct sFile *file_p;
  LONG rc;

  file_p = (struct sFile *) void_file_p;

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

  if((((struct sFile *) file)->Type == FILETYPE_ZIP) || (((struct sFile *) file)->Type == FILETYPE_CUSTOM))
  {
    switch(mode)
    {
      case SEEK_SET:
        ((struct sFile *) file)->Offset = position;
        break;
      case SEEK_CUR:
        ((struct sFile *) file)->Offset += position;
        break;
      case SEEK_END:
        ((struct sFile *) file)->Offset = ((struct sFile *) file)->Length + position;
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
        rc = SeekFile(((struct sFile *) file)->File, position, OFFSET_BEGINNING);
        break;
      case SEEK_CUR:
        rc = SeekFile(((struct sFile *) file)->File, position, OFFSET_CURRENT);
        break;
      case SEEK_END:
        rc = SeekFile(((struct sFile *) file)->File, position, OFFSET_END);
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
  struct sFile *file_p = (struct sFile *) file;
  if(!file_p) return 1;
  // cast bool to int.
  return (int)( (uint32_t)file_p->Offset >= (uint32_t)file_p->Length );
}
// only used by mame datafile.cpp
int osd_ftell(void *file)
{
    struct sFile *file_p = (struct sFile *) file;
    if(!file_p) return -1;
    return (file_p->Offset);
}



int osd_fread_scatter(void *void_file_p, void *buffer_p, int length, int increment)
{
  struct sFile *file_p;
  uint8_t buf[4096];
  uint8_t *dst_p;
  uint8_t *src_p;
  int   remaining_len;
  int   len;

  file_p = (struct sFile *) void_file_p;
  dst_p  = (uint8_t*)buffer_p;

  switch(file_p->Type)
  {
    case FILETYPE_ZIP:
    case FILETYPE_CUSTOM:
      if(file_p->Data)
      {
        len = file_p->Length - file_p->Offset;

        if(len > length)
          len = length;

        length = len;

        src_p = &file_p->Data[file_p->Offset];

        while(len--)
        {
          *dst_p = *src_p++;

          dst_p += increment;
        }

        file_p->Offset += length;

        return(length);
      }

      break;

    case FILETYPE_NORMAL:
    case FILETYPE_TMP:
      remaining_len = length;

      while(remaining_len)
      {
        if(remaining_len < sizeof(buf))
          len = remaining_len;
        else
          len = sizeof(buf);

        len = ReadFile(file_p->File, buf, len);

        if(len == 0)
          break;

        remaining_len -= len;

        src_p = buf;

        while(len--)
        {
          *dst_p = *src_p++;

          dst_p += increment;
        }
      }

      length = length - remaining_len;

      return(length);

      break;
  }

  return(0);
}
int osd_fsize(void *file)
{
  if((((struct sFile *) file)->Type == FILETYPE_ZIP) || (((struct sFile *) file)->Type == FILETYPE_CUSTOM))
    return(((struct sFile *) file)->Length);

  return(0);
}

unsigned int osd_fcrc(void *file)
{
  if((((struct sFile *) file)->Type == FILETYPE_ZIP) || (((struct sFile *) file)->Type == FILETYPE_CUSTOM))
    return(((struct sFile *) file)->CRC);

  return(0);
}
