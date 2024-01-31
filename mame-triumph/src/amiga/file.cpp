/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id: file.c,v 1.1 1999/04/28 18:52:08 meh Exp meh $
 *
 * $Log: file.c,v $
 * Revision 1.1  1999/04/28 18:52:08  meh
 * Initial revision
 *
 *
 *************************************************************************/
#include <sstream>
#include <stdio.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "main.h"
#include "file.h"
#include "osdepend.h"

using namespace std;

struct File *OpenFile(const char *dir_name, const char *file_name, int mode)
{
  struct File *file;
  BPTR    lock;
  LONG    i;

  file = (struct File *)calloc(sizeof(struct File),1);
  
  if(file)
  {
    i = strlen(dir_name);

    if(i)
    {
       { stringstream ss;
        ss << dir_name << "/" <<file_name;
        file->Name = ss.str(); }

      file->File  = (BPTR)Open(file->Name.c_str(), mode);
      file->Type  = FILETYPE_NORMAL;
      
      if(!file->File && i && (mode == MODE_OLDFILE))
      {
         { stringstream ss;
          ss << dir_name << ".zip";
          file->Name = ss.str();}

        lock = Lock(file->Name.c_str(), ACCESS_READ);
  
        if(lock)
        {
          file->File  = ~0;
          file->Type  = FILETYPE_ZIP;
        }
        else
        {
            {stringstream ss;
            ss << dir_name << ".lha";
            file->Name = ss.str();}

          lock = Lock(file->Name.c_str(), ACCESS_READ);
  
          if(lock)
          {
              stringstream ss;
             ss <<  "lha <>NIL: e "<< dir_name<<".lha "<< file_name<<" T:";
              file->Name =ss.str();
           // sprintf(file->Name, "lha <>NIL: e %s.lha %s T:", dir_name, file_name);
          }
          else
          {
            { stringstream ss;
            ss << dir_name << ".lzx";
            file->Name = ss.str();}

            lock = Lock(file->Name.c_str(), ACCESS_READ);
  
            if(lock) {
                stringstream ss;
               ss <<  "lzx <>NIL: x "<< dir_name<<".lha "<< file_name<<" T:";
                file->Name =ss.str();
            }

          }

          if(lock)
          {
            System(file->Name.c_str(), NULL);
          
            { stringstream ss;
            ss << "T:"<<file_name ;
            file->Name = ss.str();}
          //  sprintf(file->Name,"T:%s", file_name);
          
            file->File  = Open(file->Name.c_str(), mode);
            file->Type  = FILETYPE_TMP;
          }
        }

        if(lock)
          UnLock(lock);
      }
    }
    else
    {
      file->File  = Open((STRPTR) file_name, mode);
      file->Type  = FILETYPE_NORMAL;
    }
    
    if(!file->File)
    {
      free(file);
      return(NULL);
    }
  }

  return(file);
}

struct File *OpenFileType(const char *dir_name, const char *file_name, int mode, int type)
{
  struct File *file;
  const char  *path;

  char name[256];
  int  path_len;
  int  path_num;
  BPTR lock;

  file = NULL;

  switch(type)
  {
    case OSD_FILETYPE_ROM:
#ifdef MESS
    case OSD_FILETYPE_ROM_CART:
    case OSD_FILETYPE_IMAGE:
#endif
      for(path_num = 0;
          (path = GetRomPath(Config[CFG_DRIVER], path_num)) != NULL;
          path_num++)
      {
        path_len = strlen(path);
        
        if(path_len)
        {
          strcpy(name, path);

          if((name[path_len - 1] != ':') && (name[path_len - 1] != '/'))
            name[path_len++] = '/';

          sprintf(&name[path_len], "%s", dir_name);

          file = OpenFile(name, file_name, mode);
        
          if(!file)
          {
            name[path_len] = 0;
            file = OpenFile(name, file_name, mode);
          }
        }
      }

      if(!file)
      {
        sprintf(name, "roms/%s", dir_name);
        file = OpenFile(name, file_name, mode);
      }

      if(!file)
        file = OpenFile(dir_name, file_name, mode);

      return(file);

    case OSD_FILETYPE_SAMPLE:
      for(path_num = 0;
          (path = GetSamplePath(Config[CFG_DRIVER], path_num)) != NULL;
          path_num++)
      {
        path_len = strlen(path);
        
        if(path_len)
        {
          strcpy(name, path);

          if((name[path_len - 1] != ':') && (name[path_len - 1] != '/'))
            name[path_len++] = '/';

          sprintf(&name[path_len], "%s", dir_name);

          file = OpenFile(name, file_name, mode);
        
          if(!file)
          {
            name[path_len] = 0;
            file = OpenFile(name, file_name, mode);
          }
        }
      }

      if(!file)
      {
        sprintf(name, "samples/%s", dir_name);
        file = OpenFile(name, file_name, mode);
      }

      for(path_num = 0;
          (path = GetRomPath(Config[CFG_DRIVER], path_num));
          path_num++)
      {
        path_len = strlen(path);
        
        if(path_len)
        {
          strcpy(name, path);

          if((name[path_len - 1] != ':') && (name[path_len - 1] != '/'))
            name[path_len++] = '/';

          sprintf(&name[path_len], "%s", dir_name);

          file = OpenFile(name, file_name, mode);
        
          if(!file)
          {
            name[path_len] = 0;
            file = OpenFile(name, file_name, mode);
          }
        }
      }

      if(!file)
      {
        sprintf(name, "roms/%s", dir_name);
        file = OpenFile(name, file_name, mode);
      }

      if(!file)
        file = OpenFile(dir_name, file_name, mode);

      if(!file)
        file = OpenFile("roms", file_name, mode);

      if(!file)
        file = OpenFile("", file_name, mode);

      return(file);

    case OSD_FILETYPE_HIGHSCORE:
      if(mode == MODE_NEWFILE)
      {
        lock = Lock("hi", ACCESS_READ);

        if(!lock)
          lock = CreateDir("hi");

        if(lock)
          UnLock(lock);       
      }
      sprintf(name, "%s.hi", dir_name);
      return(OpenFile("hi", name, mode));

    case OSD_FILETYPE_CONFIG:
      if(mode == MODE_NEWFILE)
      {
        lock = Lock("cfg", ACCESS_READ);
        
        if(!lock)
          lock = CreateDir("cfg");

        if(lock)
          UnLock(lock);
      }
      sprintf(name, "%s.cfg", dir_name);
      return(OpenFile("cfg", name, mode));

    case OSD_FILETYPE_INPUTLOG:
      sprintf(name, "%s.inp", file_name);
      return(OpenFile("", name, mode));

    default:
      return(NULL);
  }
}

void CloseFile(struct File *file)
{
    if(!file) return;
  if(file->Type != FILETYPE_CUSTOM)
  {
    if(file->Type != FILETYPE_ZIP)
      Close(file->File);
  
    if(file->Type == FILETYPE_TMP)
      DeleteFile(file->Name.c_str());
  
    free(file);
  }
}
