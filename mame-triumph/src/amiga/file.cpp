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
#include <string>
#include <stdio.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "main.h"
#include "file.h"
#include "osdepend.h"

using namespace std;

sFile::sFile()
    : File(0),Data(NULL),Length(0),Offset(0),CRC(0)
{}
sFile::~sFile()
{
    close();
    if(Type != FILETYPE_CUSTOM)
    {
        if(Data) free(Data);
        if(File !=0 && Type != FILETYPE_ZIP)
            Close(File);

        if(Type == FILETYPE_TMP)
          DeleteFile(Name.c_str());
    }
}
void sFile::close()
{

}
void sFileNormal::close()
{

}
void sFileTmp::close()
{

}
void sFileZip::close()
{

}
void sFileCustom::close()
{

}


sFile *OpenFile(const char *dir_name, const char *file_name, int mode)
{
  BPTR    lock;
  LONG    i;


  printf("OpenFile:mode:%d d:%s: f:%s:\n",mode,dir_name,file_name);
 //olde file = (struct File *)calloc(sizeof(struct File),1);
  sFile *file = new sFile;
  
  if(!file) return file;


    i = strlen(dir_name);

    if(i)
    {

       { stringstream ss;
        ss << dir_name << "/" <<file_name;
        file->Name = ss.str(); }
        printf("try normal open:\n");
      file->File  = (BPTR)Open(file->Name.c_str(), mode);
      file->Type  = FILETYPE_NORMAL;

      if(file->File)
      {
         // normal open


      } else
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
      printf("no dir name open\n");
      file->File  = Open((STRPTR) file_name, mode);
      file->Type  = FILETYPE_NORMAL;
    }

    if(!file->File)
    {
      delete file;
      return(NULL);
    }

  return(file);
}

sFile *OpenFileType(const char *dir_name, const char *file_name, int mode, int type)
{
  struct spFile *file;
  const char  *path;
  std::string sname;
  int  path_len;
  int  path_num;
  BPTR lock;

  printf("OpenFileType: mode:%d type:%d dir_name:%s file_name:%s\n",mode,type,dir_name,file_name);
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
            stringstream ss;
            ss << path;
            if((path[path_len - 1] != ':') && (path[path_len - 1] != '/')) ss << "/";
            ss << dir_name;
            string romgamepath = ss.str();
          file = OpenFile(romgamepath.c_str(), file_name, mode);
        }
        if(file) break;
      }

      if(!file)
      {
        stringstream ss;
        ss << "roms/" << dir_name;
        file = OpenFile(ss.str().c_str(), file_name, mode);
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
            stringstream ss;
            ss << path;
            if((path[path_len - 1] != ':') && (path[path_len - 1] != '/')) ss << "/";
            ss << dir_name;
            string rompath = ss.str();

              file = OpenFile(rompath.c_str(), file_name, mode);
        }
        if(file) break;
      }

      if(!file)
      {
        stringstream ss;
        ss << "samples/" << dir_name;
        file = OpenFile(ss.str().c_str(), file_name, mode);
      }
      // also look in rom
    if(!file)
      for(path_num = 0;
          (path = GetRomPath(Config[CFG_DRIVER], path_num)) != NULL;
          path_num++)
      {
        path_len = strlen(path);

        if(path_len)
        {
            stringstream ss;
            ss << path;
            if((path[path_len - 1] != ':') && (path[path_len - 1] != '/')) ss << "/";
            ss << dir_name;
            string romgamepath = ss.str();
          file = OpenFile(romgamepath.c_str(), file_name, mode);
        }
        if(file) break;
      }
    if(!file)
    {
      stringstream ss;
      ss << "roms/" << dir_name;
      file = OpenFile(ss.str().c_str(), file_name, mode);
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
    {
      stringstream ss;
      ss << dir_name << ".hi";
      return(OpenFile("hi", ss.str().c_str(), mode));
    }
    case OSD_FILETYPE_CONFIG:
      if(mode == MODE_NEWFILE)
      {
        lock = Lock("cfg", ACCESS_READ);
        
        if(!lock)
          lock = CreateDir("cfg");

        if(lock)
          UnLock(lock);
      }
      {
        stringstream ss;
        ss << dir_name << ".cfg";
        return(OpenFile("cfg", ss.str().c_str(), mode));
      }

    case OSD_FILETYPE_INPUTLOG:
      {
        stringstream ss;
        ss << file_name << ".inp";
        return(OpenFile("", ss.str().c_str(), mode));
      }
    default:
      return(NULL);
  }
}

void CloseFile(sFile *file)
{
    delete file;

}
