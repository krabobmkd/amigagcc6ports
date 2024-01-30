#include "config_v37.h"


class BinWriter : public Serializer {
public:
    BinWriter();
    void open(const char *pFilePath)
	int isReading() override {
        return 0;
	}
	void operator()(const char *pName, uint32_t &) override {

	}
	void operator()(const char* pName, uint16_t&) override {

	}
	void operator()(const char* pName, uint8_t&) override {

	}
	void operator()(const char* pName, std::string &)override {
	// all members starts with: jump+type ID+name
	unsigned long long jumpsize = getMemberHeaderLength(sMemberName);
	// then...
	jumpsize += str.length() +1;
#ifndef NDEBUG
	// for assert
	streampos p1 = m_stream.tellp();
#endif
		writeMemberHeader(m_stream, jumpsize, ebt_String, sMemberName);
		m_stream.write(str.c_str(), str.length() + 1);
#ifndef NDEBUG
	streampos p2 = m_stream.tellp();
	unsigned long long dif = (unsigned long long)(p2 - p1);
	assert(dif == jumpsize);
#endif
	}
	void operator()(const char* pName, Serializable&) override {

//        streampos startpos = m_stream.tellp();
//        writeMemberHeader(m_stream, 0, ebt_Class, sMemberName); // first, wrong jump size

//        //m_ClassMap.find(sClassName); // do not use class map at write

//        string strClassName = Obj.className(); // may be "" in that case because type is implicit to Obj.

//        m_stream.write(strClassName.c_str(), strClassName.length() + 1);
//        Obj.serialize(*this);

//        // write back size
//        streampos endpos = m_stream.tellp();
//        m_stream.seekp(startpos);
//        unsigned long long bsize = static_cast<unsigned long long>(endpos - startpos);
//        m_stream.write((const char *)&bsize, sizeof(unsigned long long));
//        m_stream.seekp(endpos);
	}
protected:
    inline unsigned long long getMemberHeaderLength(const std::string &sMemberName)
    {

        return (unsigned long long)(sizeof(unsigned long long) + 1 + sMemberName.length()+1 );
    }
};



MameConfig_Screen::MameConfig_Screen() : Serializable()
 , type(eType::Best) // default values
 , modeid(0)
 , directMode(eDirect::Off)
 , buffering(eBuffering::Triple)
 , rotation(eRotation::No)
// , uflags(0)
 , depth(8)
 , frameskip(0)
{}

void MameConfig_Screen::serialize(Serializer& ser)
{
    ser("t",(uint8_t&)type);
    ser("id",modeid);
    ser("dm",(uint8_t&)directMode);
    ser("bf",(uint8_t&)buffering);
    ser("rl",(uint8_t&)rotation);
    ser("f",(uint8_t&)flags);
    ser("d",depth);
    ser("sk",frameskip);
}

MameConfig_Audio::MameConfig_Audio() : Serializable()
 ,sound(eSound::Paula)
{}

void MameConfig_Audio::serialize(Serializer& ser)
{
    ser("s",(uint8_t&)sound);
}

MameConfig::MameConfig() : Serializable()
    ,rom_path("PROGDIR:roms")
//	std::string conf_id;
//	std::string rom_path;
//	std::string sample_path;
{}

void MameConfig::serialize(Serializer& ser)
{
    if(ser.isReading())
    {
        ser("id",conf_id);
        if(conf_id!="mamecfg") return;
    } else
    {   // is writing
        conf_id="mamecfg";
        ser("id",conf_id);
    }

    ser("rp",rom_path);
    ser("sp",sample_path);
    ser("f",(uint8_t&)flags);
    ser("ld",lastDriverName);

    ser("sc",screen);
    ser("au",audio);
}
