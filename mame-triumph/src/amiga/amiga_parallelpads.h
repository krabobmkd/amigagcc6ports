#ifndef AMIGA_PARALLELPADS_H
#define AMIGA_PARALLELPADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <exec/types.h>

struct AParallelPads
{
    ULONG _signalBit; // to know when things happens...
    // par. raw read values
    UBYTE _aprb;
    UBYTE _bpra;
    UWORD _dummy;
    // note: struct is actually extended with private members.
};

struct AParallelPads *createParallelPads();
void readParallelPads(struct AParallelPads *parpads);
void closeParallelPads(struct AParallelPads *parpads);

#ifdef __cplusplus
}
#endif

#endif
