#include "osdepend.h"
#ifdef MESS
#include "mess_ver.h"
#ifndef BETA
#define APPNAME  "MESS 0."REVISION
#else
#define APPNAME  "MESS 0."REVISION" BETA"
#endif
#else
#include "mame_ver.h"
#ifndef BETA
#define APPNAME  "MAME 0."REVISION
#else
#define APPNAME  "MAME 0."REVISION" BETA"
#endif
#endif
