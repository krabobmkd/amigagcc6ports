#ifndef odx_WRAPPER
#define odx_WRAPPER

extern void odx_clear_video(); // for mame.cpp, to clear screen before running game

#define printf odx_printf
extern void odx_printf(char* fmt, ...);

//#define memcpy fast_memcpy
extern void* fast_memcpy(void *OUT, const void *IN, size_t N);
//krbModified added __cplusplus tests
#ifdef __cplusplus
extern "C" {
#endif
void* memcpy32(void *OUT, const void *IN, size_t N);
#ifdef __cplusplus
}
#endif


//#define memset fast_memset
extern void* fast_memset(const void *DST, int C, size_t LENGTH);
#ifdef __cplusplus
extern "C" {
#endif
void* memset32(const void *DST, int C, size_t LENGTH);
#ifdef __cplusplus
}
#endif
#endif
