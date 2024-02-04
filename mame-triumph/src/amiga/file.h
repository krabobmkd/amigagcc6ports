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
#include <stdint.h>
#include <memory>
#ifndef BPTR
//from dos/dos.h
typedef long  BPTR;
#endif

class sFile {
public:
    sFile();
    ~sFile();
    BPTR          File;
//    int          Type;
    std::string Name;
    unsigned char *Data;
    uint32_t      Length;
    int           Offset;
    unsigned int  CRC;
protected:
    virtual void close();
};

class sFileNormal : public sFile
{
protected:
    void close() override;
};
class sFileTmp : public sFile
{
protected:
    void close() override;
};
class sFileZip : public sFile
{
protected:
    void close() override;
};
class sFileCustom : public sFile
{
protected:
    void close() override;
};
//struct sFile
//{
//  sFile();
// ~sFile();
//  BPTR          File;
//  int          Type;
//  std::string Name;
//  unsigned char *Data;
//  uint32_t      Length;
//  int           Offset;
//  unsigned int  CRC;
//};

//#define FILETYPE_NORMAL 0
//#define FILETYPE_TMP    1
//#define FILETYPE_ZIP    2
//#define FILETYPE_CUSTOM 3

// new / delete to this
sFile *OpenFile(const char *dir_name, const char *file_name, int mode);
sFile *OpenFileType(const char *dir_name, const char *file_name, int mode, int filetype);
//done at destructor:
void CloseFile(sFile *file);

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
