#ifndef AMIGA_PARALLELPADS_H
#define AMIGA_PARALLELPADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <exec/types.h>

struct Task;
struct Interrupt;
//struct AParallelPads
//{
//    ULONG _signalBit; // to know when things happens...
//    // par. raw read values
////    UBYTE _aprb;
////    UBYTE _bpra;
////    UWORD _dummy;
//    // note: struct is actually extended with private members.
//};


//extern void RBFHandler();   /* proto for asm interrupt handler */
//#define BUFFERSIZE 256
struct ParPadsInteruptData {
    struct Task *_Task;
    UBYTE _ciaaprb;
    UBYTE _ciabpra;
    ULONG _counter;
    ULONG _Signal;

};


struct ParallelPads // : public AParallelPads
{
    UWORD _parallelResOK;
    UWORD _parallelBitsOK;
    BYTE _signr;
    BYTE _d;
    WORD _dd;

    // - - - -
     // must be allocated in MEMF_PUBLIC
    struct Interrupt *_rbfint;
    // must be allocated in MEMF_PUBLIC
    struct ParPadsInteruptData *_ppidata;
};




struct ParallelPads *createParallelPads();
void readParallelPads(struct ParallelPads *parpads);
void closeParallelPads(struct ParallelPads *parpads);

#ifdef __cplusplus
}
#endif

#endif
