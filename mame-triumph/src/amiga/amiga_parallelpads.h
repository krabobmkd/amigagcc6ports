#ifndef AMIGA_PARALLELPADS_H
#define AMIGA_PARALLELPADS_H

#ifdef __cplusplus
extern "C" {
#endif

struct AParallelPads;

struct AParallelPads *createParallelPads();

void closeParallelPads(struct AParallelPads *parpads);

#ifdef __cplusplus
}
#endif

#endif
