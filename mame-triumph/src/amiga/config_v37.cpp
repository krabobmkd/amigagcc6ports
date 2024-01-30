#include "config_v37.h"


MameConfig_Screen::MameConfig_Screen() : Serializable()
 , type(eType::Best) // default values
 , modeid(0)
 , directMode(eDirect::Off)
 , buffering(eBuffering::Triple)
 , rotation(eRotation:No)
 , flags(0)
 , depth(8)
 , frameskip(0)
{}

void MameConfig_Screen::serialize(Serializer& ser)
{
    ser("t",type);
    ser("id",modeid);
    ser("dm",directMode);
    ser("bf",buffering);
    ser("rl",rotation);
    ser("f",flags);
    ser("d",depth);
    ser("sk",frameskip);
}

MameConfig_Audio::MameConfig_Audio() : Serializable()
 ,sound(eSound::Paula)
{}

void MameConfig_Audio::serialize(Serializer& ser)
{
    ser("s",sound);
}

MameConfig::MameConfig() : Serializable()
    ,rom_path("PROGDIR:roms")
//	std::string conf_id;
//	std::string rom_path;
//	std::string sample_path;
{}

void MameConfig::serialize(Serializer& ser)
{
    ser("id",conf_id);
    ser("rp",rom_path);
    ser("sp",sample_path);
    ser("f",flags);
    ser("ld",lastDriverName);

    ser("sc",screen);
    ser("au",audio);
}
