#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/timer.h>

extern "C" {
    #include "osdepend.h"
}


extern struct Device      *TimerBase ;

/* return the current number of cycles, or some other high-resolution timer */
cycles_t osd_cycles(void)
{
   // cycles_t
    struct timeval tt;
    GetSysTime( &tt);

    return  (cycles_t)((long long)tt.tv_secs*1000000LL + (long long)tt.tv_micro);
}

/* return the number of cycles per second */
cycles_t osd_cycles_per_second(void)
{
    return 1000000LL;
}

/* return the current number of cycles, or some other high-resolution timer.
   This call must be the fastest possible because it is called by the profiler;
   it isn't necessary to know the number of ticks per seconds. */
cycles_t osd_profiling_ticks(void)
{
     struct timeval tt;
     GetSysTime( &tt);

     return  (cycles_t)((long long)tt.tv_secs*1000000LL + (long long)tt.tv_micro);
}

