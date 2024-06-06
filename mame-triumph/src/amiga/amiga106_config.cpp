#include "amiga106_config.h"

#include <sstream>

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

MameConfig::MameConfig()
    : _userDir("PROGDIR:user")
    , _romsDir("PROGDIR:roms")
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
static const char *pMainConfig="Mame";
int MameConfig::save()
{
    // note: got to save rom short name id, not driver index ! index evolve with compilation.
    printf("MameConfig::save\n");

    xml_data_node *root = xml_file_create();
    xml_data_node *confignode;
    mame_file *file=NULL;
    xml_data_node *display;
//    config_type *type;

    /* if we don't have a root, bail */
    if (!root)
        return 0;

    file = mame_fopen("main", 0, FILETYPE_CONFIG, 1);
    if(!file)  goto error;

    /* create a config node */
    confignode = xml_add_child(root,pMainConfig, NULL);
    if (!confignode)
        goto error;
    xml_set_attribute_int(confignode, "version", 1);

    /* create a system node */
   // systemnode = xml_add_child(confignode, "system", NULL);
//    if (!systemnode)
//        goto error;
    xml_set_attribute(confignode, "name","main" /*(which_type == CONFIG_TYPE_DEFAULT) ? "default" : Machine->gamedrv->name*/);

    // save known rom list
    if(_romsFound.size()>0)
    {
        stringstream ssroms;
        int i=0;
        for(const _game_driver *const*d : _romsFound)
        {
            char sep=' ';
            if(i==8) {i=0; sep='\n';}
            ssroms << string((*d)->name) << sep;
            i++;
        }
        string romslist = ssroms.str();
        xml_add_child(confignode,"Roms", romslist.c_str());
    }

    if(!_romsDir.empty()) xml_add_child(confignode,"RomsDir", _romsDir.c_str());
    if(!_userDir.empty()) xml_add_child(confignode,"UserDir", _userDir.c_str());

    if(_activeDriver !=-1)
    {
        xml_add_child(confignode,"Last", drivers[_activeDriver]->name );
    }

    display = xml_add_child(confignode,"Display", NULL );
    if(display)
    {
        if(_startWindowed) xml_add_child(confignode,"StartWindowed",NULL );
        if(_doubleWindow)  xml_add_child(confignode,"DoubleWindow",NULL );
    }

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
    xml_data_node *root=NULL,*confignode,*node; //, *confignode, *systemnode;
	const char *srcfile;
	int version, count;
	mame_file *file=NULL;
    printf("MameConfig::load\n");
    // resolve short name to index after load, like scan does.

    _userDir="PROGDIR:user";
    _romsDir="PROGDIR:roms";
    // had to reset first ?
    _romsFound.clear();
    _activeDriver =-1;

    file = mame_fopen("main", 0, FILETYPE_CONFIG, 0);
    if(!file)  goto error;

    printf("MameConfig::load 2\n");
    /* read the file */
	root = xml_file_read(file, NULL);
	if (!root)
		goto error;
    printf("MameConfig::load 3\n");
    /* find the config node */
	confignode = xml_get_sibling(root->child, pMainConfig);
	if (!confignode)
		goto error;

    {
        xml_data_node*node = xml_get_sibling(confignode->child, "Roms");
         printf(" rom node::%08x:\n",(int)node);
        if(node && node->value)
        {
            string roms( node->value );
            size_t i=0;
            while(i != string::npos)
            {
               size_t in = roms.find_first_of(" \t\n",i+1);
                string s = roms.substr(i,in);
                if(s.size()>0) {
                    printf("read rom:%s:\n",s.c_str());
                    int idriver = _driverIndex.index(s.c_str());
                    if(idriver>=0) _romsFound.push_back(&drivers[idriver]);
                }
               i = in;
            }



        }
    }
    node = xml_get_sibling(confignode->child, "RomsDir");
    if(node && node->value) _romsDir = node->value;

    node = xml_get_sibling(confignode->child, "UserDir");
    if(node && node->value) _userDir = node->value;

    node = xml_get_sibling(confignode->child, "Last");
    if(node && node->value) _activeDriver = _driverIndex.index(node->value);


//    version = xml_get_attribute_int(confignode, "version", 0);
//	if (version != CONFIG_VERSION)
//		goto error;
    /* loop over all system nodes in the file */
	//count = 0;
    // get all system
/*	for (systemnode = xml_get_sibling(confignode->child, "system"); systemnode; systemnode = xml_get_sibling(systemnode->next, "system"))
	{

    }*/
    xml_file_free(root);
	mame_fclose(file);
	return 1;
error:
    if(root) xml_file_free(root);
    if(file) mame_fclose(file);
    return 0;
}
void MameConfig::init(int argc,char **argv)
{

}
void MameConfig::setRomPath(const char *rompath)
{
    if(!rompath || *rompath==0)_romsDir = "PROGDIR:roms";
    else { _romsDir = rompath; _romsDir = trimSlach(_romsDir); }
    // todo send update

}
void MameConfig::setUserPath(const char *userpath)
{
    if(!userpath || *userpath==0) _userDir = "PROGDIR:user";
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
  printf(" *** ScanDrivers: _romsDir:%s\n", _romsDir.c_str());
  _romsFound.clear();
  if(_romsDir.empty()) return 0;
  printf(" *** ScanDrivers 1\n");

    struct FileInfoBlock *fib;
    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if(!fib) return 0;

    BPTR lock = Lock( _romsDir.c_str(), ACCESS_READ);
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

