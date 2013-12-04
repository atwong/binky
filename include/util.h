#include <stdint.h>
#include <stdio.h>

#ifndef _BINKY_UTIL_INCLUDE_

unsigned int ilog2( unsigned int );

uint32_t hash32shift(uint32_t key);
uint64_t hash64shift(uint64_t key);
uint32_t hash6432shift(uint64_t key);
void wsstrip( char *p );

/*
 * variabe-byte integer array compression
 */
struct vbyte_r {
  unsigned int len;
  unsigned int size;
  unsigned char *v;
};


static inline void vbyte_init( struct vbyte_r *vr ){vr->len=vr->size=0;vr->v=NULL;};
void vbyte_add( struct vbyte_r *, const uint32_t *, unsigned int );
void vbyte_expand_cb( struct vbyte_r *, void (*)(void *, uint32_t), void *p );
int vbyte_expand( struct vbyte_r *, uint32_t [], unsigned int );
int pair_dist( int, const int [], int, const int [], int );
int vbyte_pair_dist( unsigned int, int, struct vbyte_r *, struct vbyte_r * );
void merge_distance(int, int *, int, int *, int, int *);

#define _BINKY_UTIL_INCLUDE_
#endif /* _BINKY_UTIL_INCLUDE_ */

