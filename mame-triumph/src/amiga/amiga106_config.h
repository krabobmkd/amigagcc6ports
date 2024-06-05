#ifndef AMIGA_MAME_CONFIG_H
#define AMIGA_MAME_CONFIG_H


#include <string>
#include <map>
#include <vector>

struct MameRomInfo
{
    std::string _id;
    std::string _name;

};

/** Main configuration.
 *  Mame106 core manage itself default and per driver configuration.
 *  We just manage here:
 *  - rom dir pref , - user file prefs
 *  - scanned rom found list.
 *  Then modeid and per game prefs should be patched in mame per driver conf.
 */
class MameConfig {
    void save();
    void load();
    void scanDrivers();
protected:

    std::vector<std::string> _orderkeys;
    std::map<std::string,MameRomInfo> _romsinfo;

};


//void ScanDrivers(void);
//#include <string.h>
class CConfig
{
public:
    virtual const char *getUserDir() = 0;
    virtual const char *getRomDir() = 0;
};



CConfig *getConfig(const char *pDriver=0L);

#endif
