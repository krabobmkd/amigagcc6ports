#ifndef AMIGA_MAME_CONFIG_H
#define AMIGA_MAME_CONFIG_H
extern "C"
{
    #include <exec/types.h>
}
#include <string>
#include <unordered_map>
#include <vector>

typedef long BPTR;
struct FileInfoBlock;
struct _game_driver;

// driver name list could actually get big, avoid looping it.
class nameDriverMap {
public:
    void insert(const char *n, int mamedriverindex) {
        using namespace std;
        if(!n || *n==0) return;
        _m[*n][string(n)]=mamedriverindex;
    }
    // tell if driver known, and return index on mame driver list.
    int index(const char *n) const {
        using namespace std;
        if(!n || *n==0) return -1;
         unordered_map<char,unordered_map<string,int>>::const_iterator cit = _m.find(*n);
        if(cit==_m.end()) return -1;
        const std::unordered_map<std::string,int> &mm = cit->second;
        unordered_map<string,int>::const_iterator cit2 = mm.find(string(n));
        if(cit2==mm.end()) return -1;
        return cit2->second;
    }

    std::unordered_map<char,std::unordered_map<std::string,int>> _m;
};

struct ScreenConf {
     ULONG _modeID;
};

/** Main configuration.
 *  Mame106 core manage itself default and per driver configuration.
 *  We just manage here:
 *  - rom dir pref , - user file prefs
 *  - scanned rom found list.
 *  Then modeid and per game prefs should be patched in mame per driver conf.
 */
class MameConfig {
public:
    MameConfig();
    ~MameConfig();

    void init(int argc,char **argv);
    void setRomPath(const char *rompath);
    void setUserPath(const char *userpath);
    // set when driver selected,
    void setActiveDriver(int driverIndexInRomFoundList);
    int save();
    int load();

    int activeDriver() const { return _activeDriver; }
    ULONG audio() const { return _audio; }
    ULONG sampleRate() const { return _sampleRate; }
    // - -  update detected rom list - - -
    int scanDrivers();
    // - - path to main dirs --
    const char *getUserDir() const {return _userDir.c_str(); }
    const char *getRomsDir() const {return _romsDir.c_str(); }

    const std::vector<const _game_driver *const*> &romsFound() const { return _romsFound; };


protected:
    // - - - prefs unique for app
    std::string _userDir;
    std::string _romsDir; // finally just use one, but a tested one.

    int   _startWindowed; // else fullscreen.
    //std::string _lastActiveDriver;
    int         _activeDriver;
    int         _audio;
    ULONG       _sampleRate;

    int     _doubleWindow;

    ScreenConf _defaultscreenconf;
    // video prefs, per video config
    // keys are like: "320x224x16"
    std::unordered_map<std::string,ScreenConf> _screenconf;

    // prefs per game
    // force player1 joystick as port2 CD32 pad


    // name to current mame index to fasten dir search
    nameDriverMap _driverIndex;

    // - - - - - scanned roms zip or dir for UI.
    //std::vector<MameRomFound> _romsFound; // what to save
    // mui like a ptr to ptr list, to insert in one blow.
    // this is meant to be sorted a way or another
    std::vector<const _game_driver *const*> _romsFound;
    int initDriverIndex();
    int scanDriversRecurse(BPTR lock, FileInfoBlock*fib);

    void sortDrivers();
};

// access to singleton
MameConfig &getMainConfig();


#endif
