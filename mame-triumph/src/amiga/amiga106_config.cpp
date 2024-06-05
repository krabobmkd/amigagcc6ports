#include "amiga106_config.h"

#include <proto/exec.h>
#include <proto/dos.h>


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// from mame
extern "C" {
    #include "driver.h"
    //#include "mamecore.h"
}

using namespace std;

inline const std::string trimSlach( std::string s) {
    if(s.length()>0 && s.back()=='/') {
        return s.substr(0,s.length()-1);
    } else return s;
}


MameConfig &getMainConfig()
{
    static MameConfig config;
    return config;
}

MameConfig::MameConfig()
    : _startWindowed(0); // else fullscreen.
    ,_lastActiveDriver(-1)
{
    initDriverIndex();
}
MameConfig::~MameConfig()
{}

void MameConfig::setActiveDriver(int driverIndex)
{
    //TODO
}

void MameConfig::save()
{
    // note: got to save rom short name id, not driver index ! index evolve with compilation.

}
void MameConfig::load()
{

    // resolve short name to index after load, like scan does.
}
void MameConfig::init(int argc,char **argv)
{

}
void MameConfig::setRomPath(const char *rompath)
{
    if(!rompath || *rompath==0)_rompath = "PROGDIR:roms";
    else { _rompath = rompath; _rompath = trimSlach(_rompath); }

    // todo send update

}
void MameConfig::setUserPath(const char *userpath)
{
    if(!userpath || *userpath==0) _rompath = "PROGDIR:roms";
     else { _userDir = userpath;  _userDir = trimSlach(_userDir); }
    // todo send update
}

// extern const game_driver * const drivers[];

int MameConfig::initDriverIndex()
{
    // to be done once.
  for(int NumDrivers = 0; drivers[NumDrivers]; NumDrivers++)
  {
        _driverIndex.insert(drivers[NumDrivers]->name,NumDrivers);
  }
}
int MameConfig::scanDrivers()
{
  printf(" *** ScanDrivers\n");
  _romsFound.clear();
  if(_rompath.empty()) return 0;

    struct FileInfoBlock *fib;
    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if(!fib) return 0;

    BPTR lock = Lock( _rompath.c_str(), ACCESS_READ);
    if(lock)
    {
        scanDriversRecurse(lock,fib);
        UnLock(lock);
    }

    FreeDosObject(DOS_FIB,fib);

    sortDrivers();
    printf(" *** ScanDrivers end\n");

    return n;
}
int MameConfig::scanDriversRecurse(BPTR lock, FileInfoBlock*fib)
{
    if(!Examine(lock, fib)) return 0;

    if(fib->fib_DirEntryType <= 0) return 0; // if >0, a directory

    while(ExNext(lock, fib))
    {
        // trick: force lowercase at this level
        int i=0,char c;
        while((c=fib->fib_FileName[i])!=0) {
            if(c>='A' && c<='Z') c=fib->fib_FileName[i]+= 32;
            i++;
        }
        if(fib->fib_DirEntryType > 0) // if >0, a directory
        { // sub is a dir.
            // could be unzip roms or a subdir
           int idriver = _driverIndex.index(fib->fib_FileName);
           if(idriver >= 0)
           {
                _romsFound.push(&driver[idriver]);
           }
           else
           {    // subdir ?
                //TODO or not.
           }
        } else
        {   // is a file.
            // if end with zip
            // fast, no alloc version
            char *p = fib->fib_FileName;
            int l =strlen(p);
            if(l>4 && p[l-4]=='.' && p[l-3]=='z'&& p[l-2]=='i' && && p[l-1]=='p')
            {
                p[l-4] = 0;
                int idriver = _driverIndex.index(p);
                if(idriver >= 0)
                {
                    _romsFound.push(&driver[idriver]);
                }
            }
        } // end if is file.

    } // end loop per dir file

}
static int DriverCompare(struct _game_driver ***drv1, struct _game_driver ***drv2)
{
  return(stricmp((**drv1)->description, (**drv2)->description));
}

void MameConfig::sortDrivers()
{
    if(_romsFound.size()==0) return;

    qsort(_romsFound.data(), //&SortedDrivers[DRIVER_OFFSET],
        (int)_romsFound.size() ,//NumDrivers,
         sizeof(struct _game_driver **),
          (int (*)(const void *, const void *)) DriverCompare);

}

