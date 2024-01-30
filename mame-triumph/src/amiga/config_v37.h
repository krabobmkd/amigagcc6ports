#ifndef MAMECONFIGV37_H
#define MAMECONFIGV37_H
/**************************************************************************
 *
 * Copyright (C) 2024 vic "krb" ferry, License LPGL.
 *
 *************************************************************************/
#include <string>
#include <stdint.h>

//? #include "amiga_inputs.h"


struct Serializable;

struct Serializer {
	virtual int isReading() = 0;
	virtual void operator()(const char *pName, uint32_t &) = 0;
	virtual void operator()(const char* pName, uint16_t&) = 0;
	virtual void operator()(const char* pName, uint8_t&) = 0;
	virtual void operator()(const char* pName, std::string &) = 0;
	virtual void operator()(const char* pName, Serializable&) = 0;

};

struct Serializable {
	virtual void serialize(Serializer &ser)=0;

};

// - - - -- -- --
class MameConfig_Screen : public Serializable {
public:
    MameConfig_Screen();
	void serialize(Serializer& ser) override;

	enum class eType : uint8_t {
        Best,
        WB,
        Custom,
        UserSelect
	};
	eType	 type;
	uint32_t modeid;


	enum class eDirect : uint8_t {
        Off,
        Draw,
        Copy
	};
    eDirect directMode;

	enum class eBuffering : uint8_t {
        Single,
        Double,
        Triple
	};
    eBuffering buffering;

	enum class eRotation : uint8_t {
        No,
        Left,
        Right
	};
    eRotation rotation;

    // this one will not evolve.
	struct Flags
	{
		uint8_t dirtyLines : 1;
		uint8_t allow16Bit : 1;
		uint8_t flipx : 1;
		uint8_t flipy : 1;
		uint8_t autoframeskip : 1;
		//uint8_t : 2; // next 2 bits (in 1st byte) are blocked out as unused
	};
	Flags flags;

    uint8_t depth; // 1->8,16,24

    uint8_t frameskip;

};
class MameConfig_Audio : public Serializable {
public:
    MameConfig_Audio();
	void serialize(Serializer& ser) override;

	enum class eSound : uint8_t {
        No,
        Paula,
        Ahi
	};
    eSound sound;

};

class MameConfig : public Serializable  {
public:
    MameConfig();
	void serialize(Serializer& ser) override;

	// - - - -
	std::string conf_id;
	std::string rom_path;
	std::string sample_path;

	struct Flags
	{
		uint8_t b1 : 1; 
		//uint8_t : 2; // next 2 bits (in 1st byte) are blocked out as unused
	};
	Flags flags;

    std::string lastDriverName;

	MameConfig_Screen screen;
	MameConfig_Audio audio;

};

MameConfig &Config();

#endif
