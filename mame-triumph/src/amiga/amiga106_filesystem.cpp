// Vic Krb Ferry, Mar 2024

#ifndef __stdargs
#define __stdargs
#endif


#include <proto/dos.h>

// from mame:
extern "C" {
#include "osdepend.h"
#include "fileio.h"
#include "unzip.h"
}
//from this project
//#include "file.h"
// from project from this repos, static zlib:
#include "zlib.h"

#include <string>
// for memcpy
#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <sstream>



using namespace std;

// for api tracing tools
//const vector<string> filetypenames={
//    "none","ROM","SAMPLE","NVRAM","HIGHSCORE",
//    "HIGHSCORE_DB","CONFIG","INPUTLOG","STATE","ARTWORK",
//    "MEMCARD","SCREENSHOT","HISTORY","CHEAT","LANGUAGE"
//    "messr","messrw","end"
//};
//const char *osdfiletypeName(int itype) {
//    if(itype<0 || itype>= (int)filetypenames.size()) return "errortype";
//    return filetypenames[itype].c_str();
//}


extern struct DosLibrary    *DOSBase;

stringstream &glog() {
    static stringstream _glog;
    return _glog;
}




// this is picked from mame's unzip.
// this would use a "zip file cache" (or not) and would need unzip_cache_clear() after audit
// and game.
int load_zipped_file(const char *zipfile, const char *filename, unsigned char **buf, unsigned int *length);

#define FILE_IMPLEMENT_NAME
//#define PRINTOSDFILESYSTEMCALLS
/** file for reading, will just read all file and
 *   use internal offset to fullfill osd_xxx api.
 *   manage DOS reading and inside zip.
*/
class sFile {
public:
    sFile();
    ~sFile();
    // for roms dirs:
    int openread(const char *pFilepath);
    int openreadinzip(const char *pZipFile,const char *pFileName);
    // manage read or write of cong files, srceenshots,... in user dir.
    int openwrite(const char *pFilepath);

#ifdef FILE_IMPLEMENT_NAME
    inline const char *cname() { return _path.c_str(); }    
#endif
    inline int size() { return _Length; }
    inline int read(void *buffer, int l) {
        if(!_pData) return 0;
        if(_Length==_offset) return 0;
        if((_Length-_offset)<l) l = (_Length-_offset);
        memcpy(buffer,_pData+_offset,(size_t)l);
        _offset +=l;
        return l;
    }
    /*
    inline int readScatter(void *buffer, int l, int increment) {
        if(!_pData) return 0;
        if(_Length==_offset) return 0;
        if((_Length-_offset)<l) l = (_Length-_offset);
        const uint8_t *prd = _pData+_offset;
        uint8_t *pwr = (uint8_t *)buffer;
        _offset +=l;
        for(int i=0;i<l;i++)
        {
            *pwr = *prd++;
            pwr += increment;
        }
        return l;
    }*/

    inline int write(const void *buffer,int length)
    {
        if(!_writeHdl) return 0;
        return (int)Write(_writeHdl,buffer,length);
    }
    /*
    inline int readswap(void *buffer,int length)
    {
        if(!_pData) return 0;
        if(_Length==_offset) return 0;
        if((_Length-_offset)<length) length = (_Length-_offset);

        const uint8_t *prd = _pData+_offset+length;
        uint8_t *pwr = (uint8_t *)buffer;

        for(int i=0;i<length;i++)
        {   prd--;
            *pwr++ = *prd;
        }

        _offset +=length;
        return length;
    }
    inline int writeswap(const void *buffer,int length)
    {
        if(!_writeHdl) return 0;
        int bdone=0;
        const uint8_t *br = ((const uint8_t*)buffer)+length-1;
        for(int i=0;i<length;i++)
        {
           bdone += (int)Write(_writeHdl,br,1);
           br--;
        }
        return bdone;
    }
    inline int getc() {
        if(!_pData || _Length==_offset) return 0;
        int c=(int)*(_pData+_offset);
        _offset++;
        return c;
    }
    inline int ungetc(int c) {
        if(!_pData || _offset<=0) return 0;
        _offset--;
        *(_pData+_offset)=(uint8_t)c;
        return c;
    }
    inline char *getstring(char *s,int maxlength) {
        if(!_pData) return NULL;
        if(_Length==_offset) return NULL;
        int l=0;
        uint8_t *pw = (uint8_t *)s;
        const char *p = (const char *)_pData;
        while((_offset+l)<_Length && _pData[_offset+l] !=0 && l<maxlength-1)
        {
            *pw++ = _pData[_offset+l];
            l++;
        }
        if(l<maxlength) *pw++ = 0;
        return s;
    }*/
    inline int eof() {
        if(!_pData) return 0;
        return (int)(_offset>=_Length);
    }
    inline int seek(int offset, int whence) {
        if(whence == SEEK_SET)
        {
            if(offset>_Length) offset = _Length;
            _offset = offset;
            return 0;
        }
        if(whence == SEEK_CUR)
        {
            int ofs = _offset+offset;
            if(ofs<0) ofs = 0;
            if(ofs>_Length) ofs = _Length;
            _offset = ofs;
            return 0;
        }
        if(whence == SEEK_END)
        {
            int ofs = _Length+offset;
            if(ofs<0) ofs = 0;
            _offset = ofs;
            return 0;
        }
        return 1;
    }
    inline int tell() {
        return _offset;
    }
    inline uint32_t crc() {
        if(!_pData && _writeHdl)
        {
            glog() << "error: ASK CRC FOR WRITE, NOt IMPLEMENTED FILE:"<< cname() << "\n";
        }
        if(!_pData) return 0;
        return crc32(0,_pData,(uint32_t)_Length );
    }
protected:
#ifdef FILE_IMPLEMENT_NAME
    std::string _path;
#endif
    //std::vector<uint8_t> _v; // we have to keep the project's 1999 guiding lines..
    uint8_t *_pData; // malloc alloc by openXXX() or zip.
    int32_t _Length;
    int _offset;
    BPTR _writeHdl;

    void close();
};
//typedef std::shared_ptr<sFile> spFile;

sFile::sFile()
    : _pData(NULL),_Length(0),_offset(0),_writeHdl(0L)
{

}
sFile::~sFile()
{
    close();

}
void sFile::close()
{
    if(_pData) free(_pData);
    _pData = NULL;
    _Length = 0;
    if(_writeHdl) Close(_writeHdl);
    _writeHdl = 0L;
}
int sFile::openread(const char *pFilepath)
{    
    if(_pData || _writeHdl) close();

#ifdef FILE_IMPLEMENT_NAME
    _path = pFilepath;
#endif
    BPTR hdl = Open(pFilepath, MODE_OLDFILE);
    if(!hdl) return 0;

    struct FileInfoBlock *fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if(!fib){
        Close(hdl);
        return 0;
    }
    ExamineFH(hdl,fib);
    LONG bdone  = 0;
    if(fib->fib_DirEntryType<0) // means is file
    {
        //_v.resize(fib->fib_Size);
        _pData = (uint8_t *)malloc(fib->fib_Size);
        if(_pData)
        {
            _Length = fib->fib_Size;
            bdone = Read(hdl,_pData,fib->fib_Size);
        }
    }
    // read itself could have failed.
    if(bdone != fib->fib_Size && _pData) {
        free(_pData);
        _pData = NULL;
        _Length = 0;
    }
    FreeDosObject(DOS_FIB,fib);
    Close(hdl);
    return (int)(_Length>0);
}
int sFile::openwrite(const char *pFilepath)
{
    if(_pData || _writeHdl) close();
#ifdef FILE_IMPLEMENT_NAME
    _path = pFilepath;
#endif
    _writeHdl = Open(pFilepath, MODE_NEWFILE);
    if(!_writeHdl) return 0;
    return 1;
}
int sFile::openreadinzip(const char *pZipFile,const char *pFileName)
{
    if(_pData || _writeHdl) close();

#ifdef FILE_IMPLEMENT_NAME
    _path = pZipFile;
#endif
    if(load_zipped_file(pZipFile, pFileName,(unsigned char**)&_pData,(unsigned int *) &_Length)!=0)
    {
        return 0;
    }
    return (int)(_Length>0);
}
// - - - - - -
class ArchiveDir {
public:
    ArchiveDir();
    ~ArchiveDir();
    std::string _dirpath;
};


std::vector<std::string>
            _rompathlist({"PROGDIR:roms"}), // default values.
            _samplepathlist({"PROGDIR:samples"});

inline const std::string trimSlach(const std::string &s) {
    if(s.length()>0 && s.back()=='/') {
        return s.substr(0,s.length()-1);
    } else return s;
}

void setRomPaths(std::vector<std::string> &extrarompaths,std::vector<std::string> &extrasamplepaths)
{
    _rompathlist.resize(extrarompaths.size()+1); // path from config are prioritary.
    for(size_t i=0;i<extrarompaths.size();i++) {
        _rompathlist[i] =trimSlach(extrarompaths[i]);
    }
    _rompathlist[extrarompaths.size()] = "PROGDIR:roms"; // then only this. Amiga-ish isn't it ?

    /* Some sets (games) of MAME need some additional files to emulate the audio perfectly,
     *  these files are contained in compressed zip package with the same name of the file
     *  containing the roms and it should be placed in the "samples" folder of your MAME .
    */
    _samplepathlist.resize(extrasamplepaths.size()+1);
    for(size_t i=0;i<extrasamplepaths.size();i++)
    {
        _samplepathlist[1+i] =trimSlach(extrasamplepaths[i]);
    }
    _samplepathlist[extrasamplepaths.size()] = "PROGDIR:samples";
    // then samples must also tests roms...
    _samplepathlist.insert(_samplepathlist.end(),_rompathlist.begin(),_rompathlist.end());
}



void *fopen_archive_or_disk(const char *gamename,const char *filename,int filetype,int read_or_write)
{
    if(!gamename || !filename) return NULL;
    if(read_or_write !=0) return NULL; // only read we manage here.
    sFile *pfile = new sFile();
    if(!pfile) return NULL;

    vector<string> &pathlistToSearch= (filetype==FILETYPE_SAMPLE)?_samplepathlist:_rompathlist;

    // some popular packages uses this:
    string subdir;
    if(filename[0]!=0) {
        if(filename[0]>='a' && filename[0]<='z')
            subdir = filename[0];
        else
            subdir = "0-9";
    }

    for(const string &rompath : pathlistToSearch)
    {
        // more common cases first
        {   // try inside zip file
            stringstream ss;
            ss<< rompath << (rompath.back()==':'?"":"/")<< gamename<< ".zip";
            string fp = ss.str();
            if(pfile->openreadinzip(fp.c_str(),filename)) break; // means OK.
        }
        {   // try inside zip file with subdir
            stringstream ss;
            ss<< rompath << (rompath.back()==':'?"":"/")<< subdir<< "/"<< gamename<< ".zip";
            string fp = ss.str();
            if(pfile->openreadinzip(fp.c_str(),filename)) break; // means OK.
        }
        {   // try disk file
            stringstream ss;
            ss<< rompath << (rompath.back()==':'?"":"/")<< gamename<< "/"<< filename;
            string fp = ss.str();
            if(pfile->openread(fp.c_str())) break; // means OK.
        }
        {   // try disk file with subdr
            stringstream ss;
            ss<< rompath << (rompath.back()==':'?"":"/")<<subdir<<"/" << gamename<< "/"<< filename;
            string fp = ss.str();
            if(pfile->openread(fp.c_str())) break; // means OK.
        }
    }
    if(pfile->size()==0)
    {
        delete pfile;
        return NULL;
    }
    return (void *)pfile;
}
void *fopen_userdir(const char *gamename,const char *filename,int filetype,int write)
{
    sFile *pfile = new sFile();
    if(!pfile) return NULL;

    string typedirname;
    switch(filetype)
    {
        case FILETYPE_HIGHSCORE:
        case FILETYPE_HIGHSCORE_DB:
            typedirname="hiscores"; break;
        case FILETYPE_SCREENSHOT:
            typedirname = "screenshots"; break;
        default: //
            typedirname="configs"; break;
    }
    string fileext;
    switch(filetype)
    {
        case FILETYPE_HIGHSCORE: fileext="hi"; break;
        case FILETYPE_HIGHSCORE_DB: fileext="hidb"; break;
        case FILETYPE_SCREENSHOT: fileext="png"; break; // verify that
        case FILETYPE_CONFIG: fileext="cfg"; break;
        case FILETYPE_INPUTLOG: fileext="inp"; break;
        case FILETYPE_STATE: fileext="state"; break; // verify
        case FILETYPE_HISTORY: fileext="hist"; break; // really dunno
        case FILETYPE_CHEAT: fileext="cheat"; break; // really dunno
        case FILETYPE_LANGUAGE: fileext="lang"; break; // really dunno
        default: //
            fileext="setmyextpls"; break;
    }

    if(write)
    {
        BPTR lock = Lock(typedirname.c_str(), ACCESS_READ);
        if(!lock) {
            lock = CreateDir(typedirname.c_str());
            if(!lock) { glog() << "error: can't create dir:"<<typedirname<<".\n";
                return 0;
            }
            UnLock(lock);
        }
        stringstream ss;
        ss << typedirname << "/" << gamename<< "."<< fileext ;
        sFile *pfile = new sFile();
        if(!pfile) return NULL;
        string fpath = ss.str();
        if(!pfile->openwrite(fpath.c_str()))  {
            glog() << "error: can't write file:"<<fpath<<".\n";
            delete pfile;
            return NULL;
        }
        return pfile;
        // end write ok
    } else {
        // read
        stringstream ss;
        ss << typedirname << "/" << gamename<< "."<< fileext ;
        sFile *pfile = new sFile();
        if(!pfile) return NULL;
        string fpath = ss.str();
        if(!pfile->openread(fpath.c_str()))  {
            // actually not an error if file not created yet.
            delete pfile;
            return NULL;
        }
        return pfile;
    }


    return (void *)pfile;
}
osd_file *osd_fopen(int pathtype, int pathindex, const char *filename, const char *mode, osd_file_error *error);
//void *osd_fopen(const char *gamename,const char *filename,int filetype,int write)
{
    TODO
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fopen:type:%s:w:%d,%s:%s\n",osdfiletypeName(filetype),write,gamename,filename);
#endif
    // - - - - -reading a file from disk or zip searching with possible paths.
    if(filetype == FILETYPE_HIGHSCORE ||
       filetype == FILETYPE_HIGHSCORE_DB ||
       filetype == FILETYPE_CONFIG ||
       filetype == FILETYPE_INPUTLOG ||
       filetype == FILETYPE_STATE ||
       filetype == FILETYPE_SCREENSHOT
            )
    {
        return fopen_userdir(gamename,filename,filetype,write);
    } else
    {
        // FILETYPE_ROM
        // FILETYPE_SAMPLE
        return fopen_archive_or_disk(gamename,filename,filetype,write);
    }
}
// actually just test file existence for reading, must be parralel to osd_fopen()
// -> because not used for zipped ROMS it seems...
int osd_faccess(const char *filename, int filetype)
{
    if(!filename || *filename ==0) return 0;
    BPTR hdl = Open(filename,MODE_OLDFILE);
    if(hdl) Close(hdl);
    return (hdl !=0);
}

// return bytes read.
UINT32 osd_fread(osd_file *file, void *buffer, UINT32 length)
//int osd_fread(void *file,void *buffer,int length)
{
    if(!file) return 0;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fread: l:%d %s\n",length,f.cname());
#endif

    return f.read(buffer,length);
}
UINT32 osd_fwrite(osd_file *file, const void *buffer, UINT32 length)
//int osd_fwrite(void *file,const void *buffer,int length)
{
    if(!file) return 0;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fwrite: l:%d %s\n",length,f.cname());
#endif

    return f.write(buffer,length);
}
//int osd_fread_swap(void *file,void *buffer,int length)
//{
//    if(!file) return 0;
//    sFile &f = *((sFile *)file);
//#ifdef PRINTOSDFILESYSTEMCALLS
//    printf("osd_fread_swap: l:%d %s\n",length,f.cname());
//#endif

//    return f.readswap(buffer,length);
//}
//int osd_fwrite_swap(void *file,const void *buffer,int length)
//{
//    if(!file) return 0;
//    sFile &f = *((sFile *)file);
//#ifdef PRINTOSDFILESYSTEMCALLS
//    printf("osd_fwrite_swap: l:%d %s\n",length,f.cname());
//#endif

//    return f.writeswap(buffer,length);
//}

//int osd_fread_scatter(void *file,void *buffer,int length,int increment)
//{
//    if(!file) return 0;
//    sFile &f = *((sFile *)file);
//#ifdef PRINTOSDFILESYSTEMCALLS
//    printf("osd_fread_scatter: l:%d i:%d %s\n",length,increment,f.cname());
//#endif

//    return f.readScatter(buffer,length,increment);
//}
int osd_fseek(osd_file *file, INT64 offset, int whence)
//int osd_fseek(void *file,int offset,int whence)
{
    if(!file) return -1;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fseek: ofs:%d t:%d %s\n",offset,whence,f.cname());
#endif

    return f.seek(offset,whence);
}
void osd_fclose(osd_file *file)
//void osd_fclose(void *file)
{
    sFile *f = (sFile *)file;
    if(!f) return;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fclose: %s\n",f->cname());
#endif

    delete f;
}
//int osd_fchecksum(const char *gamename, const char *filename, unsigned int *length, unsigned int *sum)
//{
//    // note: other implementations (mame4all odx) just use CRC for checksum...
//    // let's just do that. This is a re-read thing and can reach roms apparently...
//    // really need to be optimized, that is a full re-read at init it seems...

//    // in mame code, this is only used to detect "rom clones"
//     // default values
//    if(length) *length=0;
//    if(sum) *sum=0;
//#ifdef PRINTOSDFILESYSTEMCALLS
//    printf("osd_fchecksum: gamename:%s filename:%s\n",gamename,filename);
//#endif

//    sFile *f = (sFile *)osd_fopen(gamename,filename,FILETYPE_ROM,0);
//    if(!f) return -1;
//    if(length) *length=f->size();
//    if(sum) *sum=f->crc();
//    delete f;

//    if(length) *length=0;
//    if(sum) *sum=0;
//    return 0; // ok for that one.

//}
int osd_fsize(void *file)
{
    if(!file) return 0;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fsize:%s\n",f.cname());
#endif

    return (int)f.size();
}
//unsigned int osd_fcrc(void *file)
//{
//    if(!file) return 0;
//    sFile &f = *((sFile *)file);
//#ifdef PRINTOSDFILESYSTEMCALLS
//    printf("osd_fcrc:%s\n",f.cname());
//#endif

//    return f.crc();
//}

int osd_fgetc(void *file)
{
    if(!file) return 0;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fgetc:%s\n",f.cname());
#endif

    return f.getc();
}
int osd_ungetc(int c, void *file)
{
    // this is just used to rewind one byte.
    if(!file) return 0;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_ungetc:%s\n",f.cname());
#endif

    return f.ungetc(c);
}
// must return s if succeed
char *osd_fgets(char *s, int n, void *file)
{
    if(!file) return NULL;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fgets:%s\n",f.cname());
#endif

    return f.getstring(s,n);
}
int osd_feof(osd_file *file)
//int osd_feof(void *file)
{
    if(!file) return 0;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_feof:%s\n",f.cname());
#endif

    return f.eof();
}
UINT64 osd_ftell(osd_file *file)
//int osd_ftell(void *file)
{
    if(!file) return 0;
    sFile &f = *((sFile *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
     printf("osd_ftell:%s\n",f.cname());
#endif

    return f.tell();
}


/* Return the number of paths for a given type */
int osd_get_path_count(int pathtype)
{
    //TODO
}

/* Get information on the existence of a file */
int osd_get_path_info(int pathtype, int pathindex, const char *filename)
{
    //TODO
    /*
    enum
{
	PATH_NOT_FOUND,
	PATH_IS_FILE,
	PATH_IS_DIRECTORY
};
    */

}

/* Create a directory if it doesn't already exist */
int osd_create_directory(int pathtype, int pathindex, const char *dirname)
{
    //TODO
}
