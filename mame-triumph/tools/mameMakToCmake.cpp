#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <stdlib.h>
using namespace std;

string sourcebase("../src/mame106/");

class TGameDriver {
    public:
     string _name;
     string _year;
     string _parent;
     string _bios;
     string _machine;
     string _input;
     string _init;
     string _monitor;
     string _company;
     string _fullname;
     string _flags;
};


class TMachine {
    public:
    string          _name;
    vector<string>  _sources;
    map<string,TGameDriver>  _gamedrivers;
    map<string,int>  _sound_defs;
    map<string,int>  _cpu_defs;
};

class TChip {
public:
    map<string,vector<string>> _vars;
   // vector<string> _sources;
};

inline bool isJustComment(const string &s)
{
    if(s.empty()) return false;
    for(char c : s)
    {
        if(c=='#') return true;
        if(c==' ' || c=='\t') continue;
        return false;
    }
    return false; // shnhap.
}

std::string trim(std::string& str) {
    const std::string whitespace = " \t\r";
    auto begin = str.find_first_not_of(whitespace);
    if (begin == std::string::npos) begin=0;

    auto end = str.find_last_not_of(whitespace);
    string nstr =str.substr(begin, end - begin + 1);
    str = nstr;
    return nstr;
}
inline void sepline(const string &line,const string sep,string &partA, string &partB)
{
    size_t i = line.find(sep);
    if(i == string::npos ) {
        partA = line;
        return;
    }
    partA = line.substr(0,i);
    partB = line.substr(i+sep.length());
    trim(partA);
    trim(partB);
}
vector<string> splitt(const string &s, const string sep)
{
    vector<string> v;
    size_t i=0;
    size_t in = s.find(sep);
    do {
        v.push_back(s.substr(i,in-i));
        if(in == string::npos) break;
        i = in+sep.length();
        in = s.find(sep,i+sep.length());
    } while(i != string::npos);
    return v;
}
string replace(const string &s, const string orig, const string rep)
{
    stringstream ss;
    size_t i=0;
    size_t in = s.find(orig);
    do {
        ss << s.substr(i,in-i);
        if(in == string::npos) break;
        i = in+orig.length();
        ss << rep;
        in = s.find(orig,i+orig.length());
    } while(i != string::npos);
    return ss.str();
}
// caution, sucks.
string trimquotes(std::string s)
{
    trim(s);
    replace(s,"\"","");
    return s;
}
void toUpper(string &s) {
    transform(s.begin(), s.end(), s.begin(),
              ::toupper);
}
int searchDrivers(TMachine &machine, map<string,vector<string>> &vars)
{
    const vector<string> &sounds = vars["SOUNDS"];
    const vector<string> &cpus = vars["CPUS"];

    for(const string &s : machine._sources)
    {
        string sourcepath = sourcebase+s;
        ifstream ifssrc(sourcepath);
        if(!ifssrc.good()) {
            cout << "didn't find source: " << sourcepath << endl;
            continue;
        }
        while(!ifssrc.eof())
        {
            string line;
            getline(ifssrc,line);
            trim(line);
            size_t isgamedriverline =line.find("GAME(");
            size_t isgamedriverlineb =line.find("GAMEB(");
            size_t is_soundinclude =line.find("\"sound/");
            size_t is_cpuinclude =line.find("\"cpu/");
            if(isgamedriverline != string::npos)
            {
                size_t stend = line.find(")",isgamedriverline+5);
                if(stend != string::npos) {
     // #define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)
                    vector<string> v= splitt(line.substr(isgamedriverline+5,isgamedriverline+5-stend),",");
                    if(v.size()>=10)
                    {
                        string name= trimquotes(v[1]);
                        TGameDriver &game = machine._gamedrivers[name];
                        game._name = name;
                        game._year = trimquotes(v[0]);
                        game._parent = trimquotes(v[2]);
                        game._machine = trimquotes(v[3]);
                        game._input = trimquotes(v[4]);
                        game._init = trimquotes(v[5]);
                        game._monitor = trimquotes(v[6]);
                        game._company = trimquotes(v[7]);
                        game._fullname = trimquotes(v[8]);
                        game._flags = trimquotes(v[9]);
                    }
                }
            } else
            if(isgamedriverlineb != string::npos)
            {
                size_t stend = line.find(")",isgamedriverlineb+6);
                if(stend != string::npos) {
// #define GAMEB(YEAR,NAME,PARENT,BIOS,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)
                    vector<string> v= splitt(line.substr(isgamedriverlineb+6,isgamedriverlineb+6-stend),",");
                    if(v.size()>=11)
                    {
                        string name= trimquotes(v[1]);
                        TGameDriver &game = machine._gamedrivers[name];
                        game._name = name;
                        game._year = trimquotes(v[0]);
                        game._parent = trimquotes(v[2]);
                        game._machine = trimquotes(v[4]);
                        game._input = trimquotes(v[5]);
                        game._init = trimquotes(v[6]);
                        game._monitor = trimquotes(v[7]);
                        game._company = trimquotes(v[8]);
                        game._fullname = trimquotes(v[9]);
                        game._flags = trimquotes(v[10]);
                    }
                }
            } // if GAMEB
            if(is_soundinclude != string::npos)
            {
                size_t endsi = line.find(".h\"",is_soundinclude+7);
                if(endsi !=  string::npos)
                {
                    string soundh = line.substr(is_soundinclude+7,endsi-(is_soundinclude+7));
                    // look if correspond to known sound cpu
                    toUpper(soundh);
                    bool isInDefs = false;
                    for(const string &sounditem : sounds )
                    {
                        if(sounditem == soundh) {isInDefs = true; break;}
                    }
                    cout << "got sound:"<<soundh << " isin: "<< (isInDefs?"OK":"--")<< endl;
                    if(isInDefs) {
                        machine._sound_defs[soundh]=1;
                    }
                }
            } // end if sound include
            if(is_cpuinclude != string::npos)
            {
                size_t endsi = line.find("/",is_cpuinclude+5);
                if(endsi !=  string::npos)
                {
                    string cpuh = line.substr(is_cpuinclude+5,endsi-(is_cpuinclude+5));
                    // look if correspond to known sound cpu
                    toUpper(cpuh);
                    bool isInDefs = false;
                    for(const string &cpuitem : cpus )
                    {
                        if(cpuitem == cpuh) {isInDefs = true; break;}
                    }
                    cout << "got cpu:"<<cpuh << " isin: "<< (isInDefs?"OK":"--")<< endl;
                    if(isInDefs) {
                        machine._cpu_defs[cpuh]=1;
                    }
                }
            } // end if sound include

        } // end while line
    } // end loop per each c source
    return EXIT_SUCCESS;
}

int read_mak_machines(
            map<string,vector<string>> &vars,
            map<string,TMachine> &machinetargets
        )
{
    ifstream ifsmak(sourcebase+"mame.mak");
    if(!ifsmak.good()) {
        cout << "need mame.mak" << endl;
        return EXIT_FAILURE;
    }

    string line, nextline;
    bool dolinknext = false;
    size_t il=0;
    while(!ifsmak.eof())
    {
        getline(ifsmak,nextline);
        il++;
        if(isJustComment(nextline) ) {
            continue;
        }
       // cout <<"l:"<<il << " : "<< nextline << endl;
        bool hasbackward = (!nextline.empty() && nextline.back()=='\\');
        if(hasbackward) nextline.pop_back();
        trim(nextline);

        if(dolinknext)
        {
            if(!line.empty()) line+=" ";
            line += nextline;
            dolinknext = false;
        }
        else
        {
            line = nextline;
        }
        if(hasbackward){
            dolinknext = true;
            continue;
        }

        // treat real line
        if(line.empty()) continue;
        cout << "line:"<< line << endl;
        if(line.find("+=") != string::npos){
            string skey,sval;
            sepline(line,"+=",skey,sval);
            if(!skey.empty())
            {
                vector<string> v = splitt(sval," ");
                vector<string> &mv = vars[skey];
                mv.insert(mv.end(),v.begin(),v.end());
            }

        } else if(line.find("=") != string::npos)
        {
            string skey,sval;
            sepline(line,"=",skey,sval);
            if(!skey.empty())
            {
                vector<string> v = splitt(sval," ");
                vector<string> &mv = vars[skey];
                mv.insert(mv.end(),v.begin(),v.end());
            }
        } else if(line.find(":") != string::npos)
        {
            string skey,sval;
            sepline(line,":",skey,sval);
            if(!skey.empty())
            {
                skey = replace(skey,"$(OBJ)/","");
                skey = replace(skey,".a","");
                sval = replace(sval,"$(OBJ)/","");
                sval = replace(sval,".o",".c");
                vector<string> v = splitt(sval," ");
                TMachine &m = machinetargets[skey];
                m._name = skey;
                m._sources.insert( m._sources.end(),v.begin(),v.end());
                searchDrivers(m,vars);
            }
        }
        line.clear();
        dolinknext = false;
    }
    return EXIT_SUCCESS;
}


int createCmake(map<string,TMachine> machinetargets,
                map<string,TChip> &soundsources,
                map<string,TChip> &cpusources)
{
    ofstream ofs("gamedrivers.cmake", ios::binary|ios::out);
    if(!ofs.good()) return 1;

    for(const auto &p: machinetargets)
    {
        string upname = p.first;
        toUpper(upname);
        ofs << "option(OPT_"<< upname<< " \"\" OFF)\n";
    }

    for(const auto &p: machinetargets)
    {
        string upname = p.first;
        const TMachine &machine=p.second;
        toUpper(upname);
        ofs << "if(OPT_"<< upname<< ")\n";
        ofs << "\tadd_compile_definitions(LINK_"<< upname<< "=1)\n";
        ofs << "\tlist(APPEND MAME_DRIVERS_SRC\n";
        int nbinline=0;
        for(const string &src : machine._sources)
        {
            if(nbinline==0) ofs << "\t\t";
            ofs << src << " ";
            nbinline++;
            if(nbinline==4) {
                ofs << "\n";
            nbinline=0;
            }
        }
        ofs << "\t)\n";
        // - - - add sound sources
        if(machine._sound_defs.size()>0)
        {
            ofs << "\tlist(APPEND MAME_SOUND_SRC\n";
            nbinline=0;
            for(const auto &defs : machine._sound_defs)
            {
                string updefs = defs.first;
                toUpper(updefs);
                // search if sources known for that chip
                if(soundsources.find(updefs)!=soundsources.end())
                {
                    TChip &schip = soundsources[updefs];
                    if(schip._vars.find("SOUNDOBJS")!=schip._vars.end() )
                    {
                        for(const string &src : schip._vars["SOUNDOBJS"])
                        {
                            if(nbinline==0) ofs << "\t\t";
                            ofs << src << " ";
                            nbinline++;
                            if(nbinline==4) {
                                ofs << "\n";
                            nbinline=0;
                            }

                        }
                    }
                } else
                {
                    cout << " **** no source find for soundchip: "<<updefs << endl;
                }
            }
            ofs << "\t)\n";
        }
        // - - - add cpu sources
        if(machine._cpu_defs.size()>0)
        {
            ofs << "\tlist(APPEND MAME_CPU_SRC\n";
            nbinline=0;
            for(const auto &defs : machine._cpu_defs)
            {
                string updefs = defs.first;
                toUpper(updefs);
                // search if sources known for that chip
                if(cpusources.find(updefs)!=cpusources.end())
                {
                    TChip &schip = cpusources[updefs];
                    if(schip._vars.find("CPUOBJS")!=schip._vars.end() )
                    {
                        for(const string &src : schip._vars["CPUOBJS"])
                        {
                            if(nbinline==0) ofs << "\t\t";
                            ofs << src << " ";
                            nbinline++;
                            if(nbinline==4) {
                                ofs << "\n";
                            nbinline=0;
                            }

                        }
                    }
                } else
                {
                    cout << " **** no source find for cpuchip: "<<updefs << endl;
                }
            }
            ofs << "\t)\n";
        }
        // - - - add sound defs
        if(machine._sound_defs.size()>0)
        {
            ofs << "\tlist(APPEND CPU_DEFS\n";
            nbinline=0;
            for(const auto &defs : machine._sound_defs)
            {
                string updefs = defs.first;
                toUpper(updefs);
                if(nbinline==0) ofs << "\t\t";
                ofs << "HAS_" << updefs << "=1 ";
                nbinline++;
                if(nbinline==4) {
                    ofs << "\n";
                nbinline=0;
                }
            }
            ofs << "\t)\n";
        }
        if(machine._cpu_defs.size()>0)
        {
            ofs << "\tlist(APPEND CPU_DEFS\n";
            nbinline=0;
            for(const auto &defs : machine._cpu_defs)
            {
                string updefs = defs.first;
                toUpper(updefs);
                if(nbinline==0) ofs << "\t\t";
                ofs << "HAS_" << updefs << "=1 ";
                nbinline++;
                if(nbinline==4) {
                    ofs << "\n";
                nbinline=0;
                }
            }
            ofs << "\t)\n";
        }

        ofs << "endif()\n";
    }

    ofs <<
    "list(REMOVE_DUPLICATES MAME_DRIVERS_SRC)\n"
    "list(REMOVE_DUPLICATES MAME_SOUND_SRC)\n"
    "list(REMOVE_DUPLICATES MAME_CPU_SRC)\n"
    "list(REMOVE_DUPLICATES CPU_DEFS)\n"
    ;

    return EXIT_SUCCESS;
}

int createMameDrivc( map<string,TMachine> &machinetargets)
{
    ifstream ifs(sourcebase+"mamedriv.c");
    if(!ifs.good()) return 1;

    ofstream ofs("mamedriv.c", ios::binary|ios::out);
    if(!ofs.good()) return 1;

    while(!ifs.eof())
    {
        string line;
        getline(ifs,line);
        ofs << line << "\n";
        if(line.find("#else	/* DRIVER_RECURSIVE */")==0)
        {
            break;
        }
    }
    // - - - -from there on,
    for(const auto &p : machinetargets)
    {
        const TMachine &machine = p.second;
        string upName = p.first;
        toUpper(upName);
        ofs << "#ifdef LINK_"<<upName<<"\n";
        for(const auto &pgd : machine._gamedrivers)
        {
            const TGameDriver &gd = pgd.second;
            ofs << "\tDRIVER( " <<pgd.first<< " ) /* "<< gd._year << " " << gd._company<< " "  <<gd._fullname<< " */\n";
        }

        ofs << "#endif\n";
    }

    // end is easy:
    ofs << "#endif	/* DRIVER_RECURSIVE */" << endl;

//#else	/* DRIVER_RECURSIVE */
//#ifdef OTHERDRIVERS
//	/* "Pacman hardware" games */
//	DRIVER( puckman )	/* (c) 1980 Namco */
//	DRIVER( puckmana )	/* (c) 1980 Namco */
    return 0;
}

int read_mak_sounds(map<string,TChip> &soundsources)
{
    ifstream ifsmak(sourcebase+"sound/sound.mak");
    if(!ifsmak.good()) {
        cout << "need sound.mak" << endl;
        return EXIT_FAILURE;
    }

    string currentSoundChip;


    string line, nextline;
    bool dolinknext = false;
    size_t il=0;
    while(!ifsmak.eof())
    {
        getline(ifsmak,nextline);
        il++;
        if(isJustComment(nextline) ) {
            continue;
        }
       // cout <<"l:"<<il << " : "<< nextline << endl;
        bool hasbackward = (!nextline.empty() && nextline.back()=='\\');
        if(hasbackward) nextline.pop_back();
        trim(nextline);

        if(dolinknext)
        {
            if(!line.empty()) line+=" ";
            line += nextline;
            dolinknext = false;
        }
        else
        {
            line = nextline;
        }
        if(hasbackward){
            dolinknext = true;
            continue;
        }

        // treat real line
        if(line.empty()) continue;
//        cout << "line:"<< line << endl;
        string filterpattern="ifneq ($(filter ";
        size_t iFilter = line.find(filterpattern);
        if(iFilter != string::npos)
        {
            size_t iend =line.find(",",iFilter+filterpattern.length());
            if(iend != string::npos)
            {
                currentSoundChip = line.substr(iFilter+filterpattern.length(),iend-(iFilter+filterpattern.length()));
            }
        }


        if(line.find("+=") != string::npos){
            string skey,sval;
            sepline(line,"+=",skey,sval);
            if(!skey.empty())
            {
                sval = replace(sval,"$(OBJ)/","");
                sval = replace(sval,".o",".c");
                vector<string> v = splitt(sval," ");
                vector<string> &mv = soundsources[currentSoundChip]._vars[skey];
                mv.insert(mv.end(),v.begin(),v.end());
            }

        } else if(line.find("=") != string::npos)
        {
            string skey,sval;
            sepline(line,"=",skey,sval);
            if(!skey.empty())
            {
                sval = replace(sval,"$(OBJ)/","");
                sval = replace(sval,".o",".c");
                vector<string> v = splitt(sval," ");
                vector<string> &mv = soundsources[currentSoundChip]._vars[skey];
                mv.insert(mv.end(),v.begin(),v.end());
            }
        }
        line.clear();
        dolinknext = false;
    }
    return 0;
}

int read_mak_cpus(map<string,TChip> &sources)
{
    ifstream ifsmak(sourcebase+"cpu/cpu.mak");
    if(!ifsmak.good()) {
        cout << "need cpu.mak" << endl;
        return EXIT_FAILURE;
    }

    string currentChip;


    string line, nextline;
    bool dolinknext = false;
    size_t il=0;
    while(!ifsmak.eof())
    {
        getline(ifsmak,nextline);
        il++;
        if(isJustComment(nextline) ) {
            continue;
        }
       // cout <<"l:"<<il << " : "<< nextline << endl;
        bool hasbackward = (!nextline.empty() && nextline.back()=='\\');
        if(hasbackward) nextline.pop_back();
        trim(nextline);

        if(dolinknext)
        {
            if(!line.empty()) line+=" ";
            line += nextline;
            dolinknext = false;
        }
        else
        {
            line = nextline;
        }
        if(hasbackward){
            dolinknext = true;
            continue;
        }

        // treat real line
        if(line.empty()) continue;
//        cout << "line:"<< line << endl;
        string filterpattern="ifneq ($(filter ";
        size_t iFilter = line.find(filterpattern);
        if(iFilter != string::npos)
        {
            size_t iend =line.find(",",iFilter+filterpattern.length());
            if(iend != string::npos)
            {
                currentChip = line.substr(iFilter+filterpattern.length(),iend-(iFilter+filterpattern.length()));
            }
        }

        if(line.find("+=") != string::npos){
            string skey,sval;
            sepline(line,"+=",skey,sval);
            if(!skey.empty())
            {
                sval = replace(sval,"$(OBJ)/","");
                sval = replace(sval,".o",".c");
                vector<string> v = splitt(sval," ");
                vector<string> &mv = sources[currentChip]._vars[skey];
                mv.insert(mv.end(),v.begin(),v.end());
            }

        } else if(line.find("=") != string::npos)
        {
            string skey,sval;
            sepline(line,"=",skey,sval);
            if(!skey.empty())
            {
                sval = replace(sval,"$(OBJ)/","");
                sval = replace(sval,".o",".c");
                vector<string> v = splitt(sval," ");
                vector<string> &mv = sources[currentChip]._vars[skey];
                mv.insert(mv.end(),v.begin(),v.end());
            }
        }
        line.clear();
        dolinknext = false;
    }
    return 0;
}
int main(int argc, char **argv)
{

    map<string,vector<string>> vars;
    map<string,TMachine> machinetargets;

    int r = read_mak_machines(vars,machinetargets);
    if(r) return r;

    map<string,TChip> soundsources;
    r = read_mak_sounds(soundsources);
    if(r) return r;

    map<string,TChip> cpusources;
    r = read_mak_cpus(cpusources);
    if(r) return r;

//    TMachine &tm = machinetargets["sega"];

    createCmake(machinetargets,soundsources,cpusources);

    createMameDrivc(machinetargets);


    return EXIT_SUCCESS;
}
