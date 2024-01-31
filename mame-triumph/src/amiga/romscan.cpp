
#include <proto/exec.h>
#include <proto/dos.h>

#include "driver.h"
#include "config_v37.h"
#include <string>
#include <map>

// when not mess
#ifdef MESS
#define DRIVER_OFFSET 0
#else
#define DRIVER_OFFSET 2
#endif
using namespace std;

static struct GameDriver ***SortedDrivers = NULL;

static char  *DriversFound = NULL;
static ULONG NumDrivers;

void ScanDrivers(void)
{
  struct FileInfoBlock *fib;
  //const char           *str;
  string str;

  BPTR locks[4];
  LONG i, j, len;
  char buf[13];    /* 8.3 filename. */
  int  bitmap_lock;
  int  vector_lock;

  const MameConfig &config = Config();

  if(DriversFound)
  {
    memset(DriversFound, 0, NumDrivers);

    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);

    if(fib)
    {
      bitmap_lock = -1;
      vector_lock = -1;

      j = 0;

      locks[j++] = DupLock(((struct Process *) FindTask(NULL))->pr_CurrentDir);

      locks[j++] = Lock("roms", ACCESS_READ);

      str = config.rom_path;  // GetRomPath(0, 0);


    if(!config.rom_path.empty())
    {
      locks[j] = Lock((STRPTR) config.rom_path.c_str(), ACCESS_READ);

      if(locks[j])
      {
        if( (SameLock(locks[0], locks[j]) == LOCK_SAME)
        ||  (SameLock(locks[1], locks[j]) == LOCK_SAME))
          UnLock(locks[j]);
        else
          bitmap_lock = j++;
      }
    }


//      str = GetRomPath(1, 0);
//      if(str)
//      {
//        if(strlen(str))
//        {
//          locks[j] = Lock((STRPTR) str, ACCESS_READ);

//          if(locks[j])
//          {
//            if( (SameLock(locks[0], locks[j]) == LOCK_SAME)
//            ||  (SameLock(locks[1], locks[j]) == LOCK_SAME))
//              UnLock(locks[j]);
//            else
//              vector_lock = j++;
//          }
//        }
//      }

      for(; --j >= 0;)
      {
        if(Examine(locks[j], fib))
        {
          if(fib->fib_DirEntryType > 0)
          {
            while(ExNext(locks[j], fib))
            {
              for(i = 0; i < NumDrivers; i++)
              {
                if( !DriversFound[i]
                &&  ((j != bitmap_lock) || !((*SortedDrivers[i+DRIVER_OFFSET])->drv->video_attributes & VIDEO_TYPE_VECTOR))
                &&  ((j != vector_lock) || ((*SortedDrivers[i+DRIVER_OFFSET])->drv->video_attributes & VIDEO_TYPE_VECTOR)))
                {
                  len = strlen((*SortedDrivers[i+DRIVER_OFFSET])->name);

                  if(!strnicmp(fib->fib_FileName, (*SortedDrivers[i+DRIVER_OFFSET])->name, len))
                  {
                    if(!fib->fib_FileName[len])
                    {
                      if(fib->fib_DirEntryType > 0)
                      {
                        DriversFound[i] = 1;
                        break;
                      }
                    }
                    else if(fib->fib_DirEntryType < 0)
                    {
                      if( !stricmp(&fib->fib_FileName[len], ".zip")
                      ||  !stricmp(&fib->fib_FileName[len], ".lha")
                      ||  !stricmp(&fib->fib_FileName[len], ".lzx"))
                      {
                        DriversFound[i] = 1;
                        break;
                      }
                    }
                  }
                }
              }
            }
          }
        }

        if(locks[j])
          UnLock(locks[j]);
      }
    }

    /* The code above searched in current dir, roms/ and any the rom path specified for
     * the bitmap and vector driver defaults. Now I'll look for any driver that has its
     * own rom path. */

    for(i = 0; i < NumDrivers; i++)
    {
      if(!DriversFound[i])
      {
        if(!GetUseDefaults(GetSortedDriverIndex(DRIVER_OFFSET+i)))
        {
          str = GetRomPath(GetSortedDriverIndex(DRIVER_OFFSET+i), 0);

          if(str && strlen(str))
          {
            locks[0] = Lock((STRPTR) str, ACCESS_READ);

            if(locks[0])
            {
              locks[1] = CurrentDir(locks[0]);

              locks[2] = Lock((char *) (*SortedDrivers[i+DRIVER_OFFSET])->name, ACCESS_READ);

              if(!locks[2])
              {
                sprintf(buf, "%s.zip", (*SortedDrivers[i+DRIVER_OFFSET])->name);
                locks[2] = Lock(buf, ACCESS_READ);

                if(!locks[2])
                {
                  sprintf(buf, "%s.lha", (*SortedDrivers[i+DRIVER_OFFSET])->name);
                  locks[2] = Lock(buf, ACCESS_READ);

                  if(!locks[2])
                  {
                    sprintf(buf, "%s.lzx", (*SortedDrivers[i+DRIVER_OFFSET])->name);
                    locks[2] = Lock(buf, ACCESS_READ);
                  }
                }
              }

              if(locks[2])
              {
                UnLock(locks[2]);

                DriversFound[i] = 1;
              }

              CurrentDir(locks[1]);
              UnLock(locks[0]);
            }
          }
        }
      }
    }

    for(i = 0; i < NumDrivers; i++)
      SetFound(DRIVER_OFFSET+GetSortedDriverIndex(DRIVER_OFFSET+i),DriversFound[i]);
  }
}
