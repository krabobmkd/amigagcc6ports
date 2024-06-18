
extern "C"
{
    #include <proto/exec.h>
    #include <dos/dos.h>
}

#include "amiga_parallelpads.h"
#include <stdio.h>


AParallelPads *g_pParPads=NULL;

int main(int argc, char **argv)
{
    g_pParPads = createParallelPads();

    if(!g_pParPads)
    {
        printf("init failed\n");
        return 1;
    }
       printf("init ok\n");
    int signal;
    while( signal = Wait(g_pParPads->_signalBit | SIGBREAKF_CTRL_C))
    {
        if(signal &SIGBREAKF_CTRL_C) break;
        readParallelPads(g_pParPads);
        printf("aprb: %02x bpra:%02x\n",(int)g_pParPads->_aprb,(int)g_pParPads->_bpra);
    }

    printf("closing\n");
    closeParallelPads(g_pParPads);
    printf("out ok\n");
    return 0;
}
