#include "amiga106_config.h"

#include <proto/exec.h>
#include <proto/dos.h>

// from mame
extern "C" {
    #include "driver.h"
    // use xml from mame
    #include "xmlfile.h"

    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
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

std::string _userDir;
std::string _rompath; // finally just use one, but a tested one.


MameConfig::MameConfig()
    : _userDir("PROGDIR:user")
    , _rompath("PROGDIR:roms")
    , _startWindowed(0) // else fullscreen.
    , _activeDriver(-1)
    , _audio(1)
    , _sampleRate(16000)
{
    initDriverIndex();
}
MameConfig::~MameConfig()
{}

void MameConfig::setActiveDriver(int driverIndexInRomFoundList)
{
    if(driverIndexInRomFoundList<0 || driverIndexInRomFoundList>=(int)_romsFound.size())
    {
        _activeDriver = -1;
        return;
    }
    const _game_driver *const*drv = _romsFound[driverIndexInRomFoundList];
    int idriver = ((int)drv-(int)&drivers[0])/sizeof(const _game_driver *);
    _activeDriver = idriver;
    printf("driverfound:%d\n",_activeDriver);

   // printf("driverfound:%%s\n",drivers[_activeDriver]->description);
}

int MameConfig::save()
{
    // note: got to save rom short name id, not driver index ! index evolve with compilation.
    printf("MameConfig::save\n");

    xml_data_node *root = xml_file_create();
    xml_data_node *confignode, *systemnode;
    mame_file *file=NULL;
    xml_data_node *romsnode,*romnode;
//    config_type *type;

    /* if we don't have a root, bail */
    if (!root)
        return 0;

    file = mame_fopen("main", 0, FILETYPE_CONFIG, 1);
    if(!file)  goto error;

    /* create a config node */
    confignode = xml_add_child(root, "amigamameconfig", NULL);
    if (!confignode)
        goto error;
    xml_set_attribute_int(confignode, "version", 1);

    /* create a system node */
    systemnode = xml_add_child(confignode, "system", NULL);
    if (!systemnode)
        goto error;
    xml_set_attribute(systemnode, "name","main" /*(which_type == CONFIG_TYPE_DEFAULT) ? "default" : Machine->gamedrv->name*/);

    // save known rom list
    romsnode = xml_add_child(systemnode,"roms", NULL);
    for(const _game_driver *const*d : _romsFound)
    {
         romnode = xml_add_child(romsnode,"r", (*d)->name);
    }

    /* create the input node and write it out */
    /* loop over all registrants and call their save function */
//    for (type = typelist; type; type = type->next)
//    {
//        xml_data_node *curnode = xml_add_child(systemnode, type->name, NULL);
//        if (!curnode)
//            goto error;
//        (*type->save)(which_type, curnode);

//        /* if nothing was added, just nuke the node */
//        if (!curnode->value && !curnode->child)
//            xml_delete_node(curnode);
//    }

    /* flush the file */
    xml_file_write(root, file);

    /* free and get out of here */
    xml_file_free(root);

    if(file) mame_fclose(file);

    return 1;

error:
    xml_file_free(root);
    if(file) mame_fclose(file);
    return 0;


}
int MameConfig::load()
{
    printf("MameConfig::load\n");
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
    if(!userpath || *userpath==0) _rompath = "PROGDIR:user";
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

    return (int)_romsFound.size();
}
int MameConfig::scanDriversRecurse(BPTR lock, FileInfoBlock*fib)
{
    if(!Examine(lock, fib)) return 0;

    if(fib->fib_DirEntryType <= 0) return 0; // if >0, a directory

    while(ExNext(lock, fib))
    {
        // trick: force lowercase at this level
        int i=0;
        char c;
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
                _romsFound.push_back(&drivers[idriver]);
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
            if(l>4 && p[l-4]=='.' && p[l-3]=='z' && p[l-2]=='i' && p[l-1]=='p')
            {
                p[l-4] = 0;
                int idriver = _driverIndex.index(p);
                if(idriver >= 0)
                {
                    _romsFound.push_back(&drivers[idriver]);
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

