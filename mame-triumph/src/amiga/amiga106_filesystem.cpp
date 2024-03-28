// Vic Krb Ferry, Feb 2024
// from amiga

#ifndef __stdargs
#define __stdargs
#endif


#include <proto/dos.h>

// from mame:
extern "C" {
    #include "osdepend.h"
    #include "fileio.h"
    #include "unzip.h"
    #include "hash.h"
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
#include <cstdarg>
#include <vector>
#include <sstream>

#include <stdio.h>

using namespace std;

void fileio_init(void)
{
}
void fileio_exit(void)
{
}

// api tracing tools
const vector<string> filetypenames={
	"RAW",
	"ROM",
	"IMAGE",
	"IMAGE_DIFF",
	"SAMPLE",
	"ARTWORK",
	"NVRAM",
	"HIGHSCORE",
	"HIGHSCORE_DB",
	"CONFIG",
	"INPUTLOG",
	"STATE",
	"MEMCARD",
	"SCREENSHOT",
	"MOVIE",
	"HISTORY",
	"CHEAT",
	"LANGUAGE",
	"CTRLR",
	"INI",
	"COMMENT",
	"DEBUGLOG",
	"HASH",
	"end"
};



const char *osdfiletypeName(int itype) {
    if(itype<0 || itype>= (int)filetypenames.size()) return "errortype";
    return filetypenames[itype].c_str();
}


extern struct DosLibrary    *DOSBase;

stringstream &glog() {
    static stringstream _glog;
    return _glog;
}




// this is picked from mame's unzip.
// this would use a "zip file cache" (or not) and would need unzip_cache_clear() after audit
// and game.
//int load_zipped_file(const char *zipfile, const char *filename, unsigned char **buf, unsigned int *length);
extern "C" {
int load_zipped_file (int pathtype, int pathindex, const char* zipfile, const char* filename, unsigned char** buf, unsigned int* length);
}
#define FILE_IMPLEMENT_NAME
#define PRINTOSDFILESYSTEMCALLS
/** file for reading, will just read all file and
 *   use internal offset to fullfill osd_xxx api.
 *   manage DOS reading and inside zip.
*/
struct _mame_file {
public:
    _mame_file();
    ~_mame_file();
    // for roms dirs:
    int openread(const char *pFilepath);
    int openreadinzip(const char *pZipFile,const char *pFileName);
    // manage read or write of cong files, srceenshots,... in user dir.
    int openwrite(const char *pFilepath);

#ifdef FILE_IMPLEMENT_NAME
    inline const char *cname() { return _path.c_str(); }    
#endif
    inline const unsigned char *data() const { return _pData; }
    inline char *hash() { return _hash; }
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
    }
    */
    inline int write(const void *buffer,int length)
    {
        if(!_writeHdl) return 0;
        return (int)Write(_writeHdl,buffer,length);
    }
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
    // put string
    inline int fputs(const char *p) {
        if(!_writeHdl) return -1; // EOF
        int l = strlen(p);
        Write(_writeHdl,p,l); // important: term 0 is not copied
        return 0; // OK
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
    }
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
//    inline uint32_t crc() {
//        if(!_pData && _writeHdl)
//        {
//            glog() << "error: ASK CRC FOR WRITE, NOt IMPLEMENTED FILE:"<< cname() << "\n";
//        }
//        if(!_pData) return 0;
//        return crc32(0,_pData,(uint32_t)_Length );
//    }

protected:
#ifdef FILE_IMPLEMENT_NAME
    std::string _path;
#endif
    //std::vector<uint8_t> _v; // we have to keep the project's 1999 guiding lines..
    uint8_t *_pData; // malloc alloc by openXXX() or zip.
    int32_t _Length;
    int _offset;
    BPTR _writeHdl;

	char		_hash[HASH_BUF_SIZE];

    void close();
};
//typedef std::shared_ptr<_mame_file> spFile;

_mame_file::_mame_file()
    : _pData(NULL),_Length(0),_offset(0),_writeHdl(0L)
{

}
_mame_file::~_mame_file()
{
    close();

}
void _mame_file::close()
{
    if(_pData) free(_pData);
    _pData = NULL;
    _Length = 0;
    if(_writeHdl) Close(_writeHdl);
    _writeHdl = 0L;
}
int _mame_file::openread(const char *pFilepath)
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
int _mame_file::openwrite(const char *pFilepath)
{
    if(_pData || _writeHdl) close();
#ifdef FILE_IMPLEMENT_NAME
    _path = pFilepath;
#endif
    _writeHdl = Open(pFilepath, MODE_NEWFILE);
    if(!_writeHdl) return 0;
    return 1;
}
int _mame_file::openreadinzip(const char *pZipFile,const char *pFileName)
{
    if(_pData || _writeHdl) close();

#ifdef FILE_IMPLEMENT_NAME
    _path = pZipFile;
#endif
    //int load_zipped_file (int pathtype, int pathindex, const char* zipfile, const char* filename, unsigned char** buf, unsigned int* length);
 printf("_mame_file::openreadinzip\n");
    if(load_zipped_file(0,0,pZipFile, pFileName,(unsigned char**)&_pData,(unsigned int *) &_Length)!=0)
    {
        return 0;
    }
    return (int)(_Length>0);
}
// - - - - - -
//class ArchiveDir {
//public:
//    ArchiveDir();
//    ~ArchiveDir();
//    std::string _dirpath;
//};
// - - - - - - - -
//class _mame_file_fromZip : public _mame_file {
//public:
//    std::shared_ptr<ArchiveDir> _dir;
//};
//std::shared_ptr<ArchiveDir> amiga_openArchive(const char *gamename)
//{

//}


/* gamename holds the driver name, filename is only used for ROMs and    */
/* samples. If 'write' is not 0, the file is opened for write. Otherwise */
/* it is opened for read. */


/*
enum
{
	FILETYPE_ROM = 1,
	FILETYPE_SAMPLE,
	FILETYPE_NVRAM,
	FILETYPE_HIGHSCORE,
	FILETYPE_HIGHSCORE_DB,  LBO 040400
	FILETYPE_CONFIG,
	FILETYPE_INPUTLOG,
	FILETYPE_STATE,
	FILETYPE_ARTWORK,
	FILETYPE_MEMCARD,
	FILETYPE_SCREENSHOT,
	FILETYPE_HISTORY,   LBO 040400
	FILETYPE_CHEAT,   LBO 040400
	FILETYPE_LANGUAGE,  LBO 042400
#ifdef MESS
	FILETYPE_IMAGE_R,
	FILETYPE_IMAGE_RW,
#endif
	FILETYPE_end // dummy last entry
};
*/
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


// note: only read

mame_file *fopen_archive_or_disk(const char *gamename,const char *filename,int filetype, osd_file_error *error)
{
    if(error) *error = FILEERR_FAILURE;
    if(!gamename || !filename) return NULL;
    _mame_file *pfile = new _mame_file();
    if(!pfile) return NULL;

 printf("fopen_archive_or_disk:%s %s\n",gamename,filename);

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
        if(error) *error = FILEERR_FAILURE;
        return NULL;
    }

//	unsigned int functions = hash_data_used_functions(hash);
//	hash_compute(f->hash, f->data(), f->size(), functions);



    if(error) *error = FILEERR_SUCCESS;
    return pfile;
}

mame_file *fopen_simple(const char *filename,int filetype,int write, osd_file_error *error)
{
    if(error) *error = FILEERR_FAILURE;
    if( !filename) return NULL;
    _mame_file *pfile = new _mame_file();
    if(!pfile) return NULL;

    pfile->openread(filename);

    if(pfile->size()==0)
    {
        delete pfile;
        if(error) *error = FILEERR_FAILURE;
        return NULL;
    }

    if(error) *error = FILEERR_SUCCESS;
    return pfile;
}
mame_file *fopen_userdir(const char *gamename,const char *filename,int filetype,int write, osd_file_error *error)
{
    _mame_file *pfile = new _mame_file();
    if(!pfile)
    {
        if(error) *error = FILEERR_OUT_OF_MEMORY;
        return NULL;
    }

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
                if(error) *error = FILEERR_ACCESS_DENIED;
                return 0;
            }
            UnLock(lock);
        }
        stringstream ss;
        ss << typedirname << "/" << gamename<< "."<< fileext ;
        _mame_file *pfile = new _mame_file();
        if(!pfile) return NULL;
        string fpath = ss.str();
        if(!pfile->openwrite(fpath.c_str()))  {
            glog() << "error: can't write file:"<<fpath<<".\n";
            delete pfile;
            if(error) *error = FILEERR_FAILURE;
            return NULL;
        }
        return pfile;
        // end write ok
    } else {
        // read
        stringstream ss;
        ss << typedirname << "/" << gamename<< "."<< fileext ;
        _mame_file *pfile = new _mame_file();
        if(!pfile) return NULL;
        string fpath = ss.str();
        if(!pfile->openread(fpath.c_str()))  {
            // actually not an error if file not created yet.
            delete pfile;
            if(error) *error = FILEERR_FAILURE;
            return NULL;
        }
        return pfile;
    }

    if(error) *error = FILEERR_SUCCESS;
    return pfile;
}
/*
enum _osd_file_error
{
	FILEERR_SUCCESS,
	FILEERR_FAILURE,
	FILEERR_OUT_OF_MEMORY,
	FILEERR_NOT_FOUND,
	FILEERR_ACCESS_DENIED,
	FILEERR_ALREADY_OPEN,
	FILEERR_TOO_MANY_FILES
};

*/
//mame_file *mame_fopen(const char *gamename, const char *filename, int filetype, int openforwrite);
mame_file *mame_fopen(const char *gamename, const char *filename, int filetype, int openforwrite)
{
    return mame_fopen_error(gamename,filename,filetype,openforwrite,NULL);
}
osd_file *osd_fopen(int pathtype, int pathindex, const char *filename, const char *mode, osd_file_error *error)
{

return (osd_file *) fopen_simple(filename,0,0, error);
    // only used for unzip read...
    if(strchr(mode,'b' )!=NULL)
    {
        printf("mame_fopen for read\n");
        // note: old API only used from unzip.
        return (osd_file *)fopen_simple(filename,FILETYPE_ROM,0,error);
    } else
    {
        return (osd_file *)fopen_simple(filename,0,1,error);
    }
}




mame_file *mame_fopen_rom(const char *gamename, const char *filename, const char *exphash)
{
    mame_file *f = fopen_archive_or_disk(gamename,filename,FILETYPE_ROM, NULL);
    if(f && exphash)
    {
        unsigned int functions = hash_data_used_functions(exphash);
        hash_compute(f->hash(), f->data(), f->size(), functions);
    }
    return f;
}
mame_file *mame_fopen_error(const char *gamename, const char *filename, int filetype, int openforwrite, osd_file_error *error)
{
    if(error) *error = FILEERR_NOT_FOUND;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fopen:type:%s:w:%d,%s:%s\n",osdfiletypeName(filetype),openforwrite,gamename,filename);
#endif
    // - - - - -reading a file from disk or zip searching with possible paths.
    if( filetype == FILETYPE_ROM ||
        filetype == FILETYPE_SAMPLE ||
        filetype == FILETYPE_ARTWORK ||
          filetype == FILETYPE_MOVIE // add dunnowhat
        )
    {
    printf("use archive or disk\n");
        if(openforwrite) {
            if(error) *error = FILEERR_FAILURE;
            return NULL;
        }
    printf("use archive or disk2\n");
        return fopen_archive_or_disk(gamename,filename,filetype, error);
    } else
    {
    printf("use user dir\n");
        return fopen_userdir(gamename,filename,filetype,openforwrite, error);
    }
}
// actually just test file existence for reading, must be parralel to osd_fopen()
// -> because not used for zipped ROMS it seems...
int mame_faccess(const char *filename, int filetype)
//int osd_faccess(const char *filename, int filetype)
{
    if(!filename || *filename ==0) return 0;
    BPTR hdl = Open(filename,MODE_OLDFILE);
    if(hdl) Close(hdl);
    return (hdl !=0);
}

// return bytes read.


UINT32 mame_fread(mame_file *file, void *buffer, UINT32 length)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fread: l:%d %s\n",length,file->cname());
#endif
    return file->read(buffer,length);
}
UINT32 osd_fread(osd_file *file, void *buffer, UINT32 length)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    //printf("osd_fread: l:%d %s\n",length,file->cname());
#endif
    return ((mame_file *)file)->read(buffer,length);
}

UINT32 mame_fwrite(mame_file *file, const void *buffer, UINT32 length)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fwrite: l:%d %s\n",length,file->cname());
#endif
    return file->write(buffer,length);
}
UINT32 mame_fread_swap(mame_file *file, void *buffer, UINT32 length)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fread_swap: l:%d %s\n",length,file->cname());
#endif

    return file->readswap(buffer,length);
}

UINT32 mame_fwrite_swap(mame_file *file, const void *buffer, UINT32 length)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fwrite_swap: l:%d %s\n",length,file->cname());
#endif
    return file->writeswap(buffer,length);
}
/*
int osd_fread_scatter(void *file,void *buffer,int length,int increment)
{
    if(!file) return 0;
    _mame_file &f = *((_mame_file *)file);
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fread_scatter: l:%d i:%d %s\n",length,increment,file->cname());
#endif

    return f.readScatter(buffer,length,increment);
}
*/


int mame_fseek(mame_file *file, INT64 offset, int whence)
{
    if(!file) return -1;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fseek: ofs:%d t:%d %s\n",offset,whence,file->cname());
#endif
    return file->seek(offset,whence);
}
/* Seek within a file */
int osd_fseek(osd_file *file, INT64 offset, int whence)
{
    if(!file) return -1;
    #ifdef PRINTOSDFILESYSTEMCALLS
        //printf("osd_fseek: ofs:%d t:%d %s\n",offset,whence,file->cname());
    #endif
    return ((mame_file*)file)->seek(offset,whence);
}


void mame_fclose(mame_file *file)
{
    if(!file) return;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fclose: %s\n",file->cname());
#endif
    delete file; // destructor does the job.
}
/* Close an open file */
void osd_fclose(osd_file *file)
{
    if(!file) return;
#ifdef PRINTOSDFILESYSTEMCALLS
    //printf("osd_fclose: %s\n",file->cname());
#endif
    delete (mame_file*)file; // destructor does the job.
}


int mame_fchecksum(const char *gamename, const char *filename, unsigned int *length, char *hash)
{
    // note: other implementations (mame4all odx) just use CRC for checksum...
    // let's just do that. This is a re-read thing and can reach roms apparently...
    // really need to be optimized, that is a full re-read at init it seems...

    // in mame code, this is only used to detect "rom clones"
     // default values
    if(length) *length=0;
//    if(sum) *sum=0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fchecksum: gamename:%s filename:%s\n",gamename,filename);
#endif

    _mame_file *f = mame_fopen_rom(gamename,filename,hash);

  	hash_data_copy(hash, f->hash());
	if(length) *length = (int) f->size();

    if(!f) return -1;

	/* compute the checksums (only the functions for which we have an expected
       checksum). Take also care of crconly: if the user asked, we will calculate
       only the CRC, but only if there is an expected CRC for this file. */
//	unsigned int functions = hash_data_used_functions(hash);
//	hash_compute(f->hash, f->data(), f->size(), functions);




//    if(!f) return -1;
//    if(length) *length=f->size();
//    if(sum) *sum=f->crc();
    delete f;

//    if(length) *length=0;
//    if(sum) *sum=0;
    return 0; // ok for that one.

}
const char *mame_fhash(mame_file *file)
{
    file->hash();
}

UINT64 mame_fsize(mame_file *file)
//int osd_fsize(void *file)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fsize:%s\n",file->cname());
#endif
    return (UINT64)file->size();
}
//unsigned int osd_fcrc(void *file)
//{
//    if(!file) return 0;
//    _mame_file &f = *((_mame_file *)file);
//#ifdef PRINTOSDFILESYSTEMCALLS
//    printf("osd_fcrc:%s\n",file->cname());
//#endif

//    return f.crc();
//}
int mame_fgetc(mame_file *file)
//int osd_fgetc(void *file)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fgetc:%s\n",file->cname());
#endif
    return file->getc();
}
int mame_ungetc(int c, mame_file *file)
//int osd_ungetc(int c, void *file)
{
    // this is just used to rewind one byte.
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_ungetc:%s\n",file->cname());
#endif

    return file->ungetc(c);
}
// must return s if succeed
char *mame_fgets(char *s, int n, mame_file *file)
//char *osd_fgets(char *s, int n, void *file)
{
    if(!file) return NULL;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_fgets:%s\n",file->cname());
#endif

    return file->getstring(s,n);
}

int mame_fputs(mame_file *f, const char *s)
{
    if(!f) return NULL;
#ifdef PRINTOSDFILESYSTEMCALLS
    //printf("mame_fputs:%s\n",file->cname());
#endif
    return f->fputs(s);
}


int mame_feof(mame_file *file)
//int osd_feof(void *file)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
    printf("osd_feof:%s\n",file->cname());
#endif
    return file->eof();
}
UINT64 mame_ftell(mame_file *file)
//int osd_ftell(void *file)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
     printf("osd_ftell:%s\n",file->cname());
#endif

    return file->tell();
}
/* Return current file position */
UINT64 osd_ftell(osd_file *file)
{
    if(!file) return 0;
#ifdef PRINTOSDFILESYSTEMCALLS
     //printf("osd_ftell:%s\n",file->cname());
#endif

    return ((mame_file *)file)->tell();
}



int mame_fprintf(mame_file *file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if(!file) return -1;

    char temp[256];
    int l =vsnprintf(temp,255,fmt,args);
    temp[255]=0;
    mame_fputs(file,temp);
    return l;
}




/* Return 1 if we're at the end of file */
//int osd_feof(osd_file *file);

/* Read bytes from a file */


/* Write bytes to a file */
//UINT32 osd_fwrite(osd_file *file, const void *buffer, UINT32 length);


