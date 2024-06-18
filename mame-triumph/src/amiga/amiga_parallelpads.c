/*
    By Krb, most codes from examples.

    from:
     https://wiki.amigaos.net/wiki/Exec_Interrupts
    and from amiga developper CD 2.1 read34.asm


https://github.com/niklasekstrom/amiga-par-to-spi-adapter/blob/master/spi-lib/spi.c#L296
*/

#include "amiga_parallelpads.h"
//

#include <proto/exec.h>
#define ALIB_HARDWARE_CIA
#include <proto/alib.h>
#include <proto/misc.h>

#include    <exec/types.h>
#include    <libraries/dos.h>

#include <resources/misc.h>

#include <hardware/custom.h>
#include <hardware/intbits.h>


#include    <stdlib.h>
#include    <stdio.h>

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


struct ParallelPads // : public AParallelPads
{
    // extend the old way:
    struct AParallelPads _public;

    UWORD _parallelResOK;
    UWORD _parallelBitsOK;
    BYTE _signr;
    BYTE _prior_enable;
    struct Interrupt *_prior_interupt;
    // - - - -
     // must be allocated in MEMF_PUBLIC
    struct Interrupt *_rbfint;
    // must be allocated in MEMF_PUBLIC
    struct RBFData *_rbfdata;
};


void closeParallelPads(struct AParallelPads *parpads);

static UBYTE *allocname = "Mame"; // or use task name ?

struct AParallelPads *createParallelPads()
{
    struct Interrupt *rbfint;
    struct RBFData *rbfdata;
    BOOL priorenable;
    BYTE signr;

    struct ParallelPads *pparpads = AllocVec(sizeof(struct ParallelPads),MEMF_CLEAR);
    if(!pparpads) return NULL;
    pparpads->_signr = -1; // default error state for this.

    // - - - - acquire parallel port
    Disable();
        pparpads->_parallelResOK = (UWORD)(AllocMiscResource(MR_PARALLELPORT,allocname)==NULL);
        if(!pparpads->_parallelResOK) { Enable(); goto error; }

        pparpads->_parallelBitsOK = (UWORD)(AllocMiscResource(MR_PARALLELBITS,allocname)==NULL);
        if(!pparpads->_parallelBitsOK) { Enable(); goto error; }

        // use hard address using amiga.lib:
        ciaaddrb = 0; // all lines read
        ciabddra	= 0xFF; // busy, pout, and sel. to read

        // Well, we made it this far, so we've got exclusive access to
        // the parallel port, and all the lines we want to use are
        // set up.
    Enable();
    // - - - - -
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
    pparpads->_public._signalBit = (1<<(pparpads->_signr));

    return &pparpads->_public;
error:
    closeParallelPads((struct AParallelPads *)pparpads);
    return NULL;
}

void readParallelPads(struct AParallelPads *parpads)
{
    if(!parpads) return;
    parpads->_aprb = ciaaprb;
    parpads->_bpra = ciabpra;
}

void closeParallelPads(struct AParallelPads *paparpads)
{
    struct ParallelPads *pparpads = (struct ParallelPads *)paparpads;
    if(!pparpads) return;

    if(pparpads->_rbfdata) FreeVec(pparpads->_rbfdata);
    if(pparpads->_rbfint) FreeVec(pparpads->_rbfint);
    if(pparpads->_signr != -1) FreeSignal(pparpads->_signr);

    if(pparpads->_parallelBitsOK) FreeMiscResource(MR_PARALLELBITS);
    if(pparpads->_parallelResOK) FreeMiscResource(MR_PARALLELPORT);

    FreeVec(pparpads);

}
