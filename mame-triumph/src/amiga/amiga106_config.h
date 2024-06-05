#ifndef AMIGA_MAME_CONFIG_H
#define AMIGA_MAME_CONFIG_H


#include <string>
#include <unordered_map>
#include <vector>

typedef long BPTR;
struct FileInfoBlock;
struct _game_driver;

// driver name list could actually get big, avoid looping it.
class nameDriverMap {
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
    void save();
    void load();
    // - -  update detected rom list - - -
    int scanDrivers();
    // - - path to main dirs --
    const char *getUserDir() {return _userDir.c_str(); }
    const char *getRomDir() {return _rompath.c_str(); }

    const std::vector<_game_driver **> &romsFound() const { return _romsFound; };
protected:
    std::string _userDir;
    std::string _rompath; // finally just use one, but a tested one.

    // name to current mame index to fasten dir search
    nameDriverMap _driverIndex;

    // - - - - - scanned roms zip or dir for UI.
    //std::vector<MameRomFound> _romsFound; // what to save
    // mui like a ptr to ptr list, to insert in one blow.
    // this is meant to be sorted a way or another
    std::vector<_game_driver **> _romsFound;
    int initDriverIndex();
    int scanDriversRecurse(BPTR lock, FileInfoBlock*fib);

    void sortDrivers();
};

// access to singleton
MameConfig &getMainConfig();


#endif
