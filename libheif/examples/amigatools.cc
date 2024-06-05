#include <exec/execbase.h>

#include <proto/dos.h>

#include <stdio.h>
#include <stdlib.h>

#include "libheif/heif_version.h"
#include "libde265/de265-version.h"

extern struct ExecBase *SysBase;

/*
const char *getcpu(void)
{
  UWORD attnflags = SysBase->AttnFlags;

  if (attnflags & 0x80) return "68060";
  if (attnflags & AFF_68040) return "68040";
  if (attnflags & AFF_68030) return "68030";
  if (attnflags & AFF_68020) return "68020";
  if (attnflags & AFF_68010) return "68010";
  return "68000";
}
*/
const char *pVersion="$VER: heif:" LIBHEIF_VERSION " de265:"LIBDE265_VERSION;

int amiga_hasFPU()
{
    UWORD attnflags = SysBase->AttnFlags;

    return (int)((attnflags &
        (AFF_68881|AFF_68882|AFF_FPU40))!=0);

}


void amiga_cpucheck()
{
    int hasFPU = amiga_hasFPU();
    if(!hasFPU) {
        printf("This tool was compiled for machines with a FPU.\n");
        exit(1);
    }
}

// make it happen before main() with a global constructor, less intrusive to original code.
struct beforeMainInit
{
    beforeMainInit() {
      amiga_cpucheck();
    }
};
beforeMainInit _ginit;
