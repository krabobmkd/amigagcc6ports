/*
    By Krb, most codes from examples.

    from:
     https://wiki.amigaos.net/wiki/Exec_Interrupts
    and from amiga developper CD 2.1 read34.asm

*/

#include "amiga_parallelpads.h"
//
#include    <exec/types.h>
#include    <libraries/dos.h>

#include    <stdlib.h>
#include    <stdio.h>

#include    <clib/alib_protos.h>
#include    <clib/exec_protos.h>
#include    <clib/misc_protos.h>

#include <resources/misc.h>

#include <hardware/custom.h>

// apparently from alib:
extern struct Custom custom;

// https://wiki.amigaos.net/wiki/Exec_Interrupts

extern void RBFHandler();   /* proto for asm interrupt handler */

#define BUFFERSIZE 256

struct RBFData {
    struct Task *rd_Task;
    ULONG rd_Signal;
    ULONG rd_BufferCount;
    UBYTE rd_CharBuffer[BUFFERSIZE + 2];
    UBYTE rd_FlagBuffer[BUFFERSIZE + 2];
    UBYTE rd_Name[32];
};


struct AParallelPads
{
    //APTR _miscResource;
    UWORD _parallelResOK;
    UWORD _parallelBitsOK;
    BYTE _signr;
    BYTE _prior_enable;
    struct Interrupt *_prior_interupt;
    // - - - -
     // must be allocated in MEMF_PUBLIC
    struct RBFData *_rbfint;
    // must be allocated in MEMF_PUBLIC
    struct RBFData *_rbfdata;


};



void closeParallelPads(struct AParallelPads *parpads);

static UBYTE *allocname = "Mame"; // or use task name ?

struct AParallelPads *createParallelPads()
{
    struct RBFData *rbfint;
    struct RBFData *rbfdata;
    BOOL priorenable;
    BYTE signr;

    struct AParallelPads *pparpads = AllocVec(sizeof(struct AParallelPads),MEMF_CLEAR);
    if(!pparpads) return NULL;
    pparpads->_signr = -1; // default error state for this.

    pparpads->_parallelResOK = (UWORD)(AllocMiscResource(MR_PARALLELPORT,allocname)==NULL);
    if(!pparpads->_parallelResOK) goto error;

    pparpads->_parallelBitsOK = (UWORD)(AllocMiscResource(MR_PARALLELBITS,allocname)==NULL);
    if(!pparpads->_parallelBitsOK) goto error;

    pparpads->_signr = signr = AllocSignal(-1);
    if(signr == -1) goto error;

    pparpads->_rbfint = rbfint = AllocVec(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR);
    if(!rbfint) goto error;

    pparpads->_rbfdata = rbfdata = AllocVec(sizeof(struct RBFData), MEMF_PUBLIC|MEMF_CLEAR);
    if(!rbfdata) goto error;

        rbfdata->rd_Task = FindTask(NULL);        /* Init rfbdata structure. */
        rbfdata->rd_Signal = 1L << signr;

        rbfint->is_Node.ln_Type = NT_INTERRUPT;      /* Init interrupt node. */
        strcpy(rbfdata->rd_Name, allocname);
        rbfint->is_Node.ln_Name = rbfdata->rd_Name;
        rbfint->is_Data = (APTR)rbfdata;
        rbfint->is_Code = RBFHandler;


    pparpads->_prior_enable = (BYTE)((custom.intenar & INTF_RBF)!=0) ; /* interrupt */
    custom.intena = INTF_RBF;                             /* disable it. */
    pparpads->_prior_interupt = SetIntVector(INTB_RBF, rbfint);


//    rbfdata->rd_Task = FindTask(NULL);        /* Init rfbdata structure. */
//    rbfdata->rd_Signal = 1L << pparpads->_signr;

//    rbfint->is_Node.ln_Type = NT_INTERRUPT;      /* Init interrupt node. */
//    strcpy(rbfdata->rd_Name, allocname);
//    rbfint->is_Node.ln_Name = rbfdata->rd_Name;
//    rbfint->is_Data = (APTR)rbfdata;
//    rbfint->is_Code = RBFHandler;
//                                                /* Save state of RBF and */
//    priorenable = custom.intenar & INTF_RBF ? TRUE : FALSE; /* interrupt */
//    custom.intena = INTF_RBF;                             /* disable it. */
//    priorint = SetIntVector(INTB_RBF, rbfint);

    // went OK
    return pparpads;
error:
    closeParallelPads(pparpads);
    return NULL;
}

void closeParallelPads(struct AParallelPads *parpads)
{
    if(!parpads) return;

    if(pparpads->_rbfdata) FreeVec(pparpads->_rbfdata);
    if(pparpads->_rbfint) FreeVec(pparpads->_rbfint);
    if(pparpads->_signr != -1) FreeSignal(pparpads->signr);

    if(pparpads->_parallelBitsOK) FreeMiscResource(MR_PARALLELBITS);
    if(pparpads->_parallelResOK) FreeMiscResource(MR_PARALLELPORT);

    FreeVec(pparpads);

}
