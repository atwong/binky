#ifndef __VPOS_INCLUDE__
#define __VPOS_INCLUDE__
#include <stdio.h>
#include <stdint.h>

#define VPOS_F_ACTIVE  0x02
#define VPOS_F_ORDERED 0x04
#define VPOS_F_UNIQUE  0x08
#define VPOS_F_BT      0x10
#define VPOS_MAGIC     0xcafe
#define VPOS_MIN_ALLOCLEN  8             /* 256 */


typedef union {
  struct packedIndexHit_rec {
    unsigned long long docId  : 48;
    unsigned int hitCnt : 16;
  } r;
  uint64_t i;
} vIndex;



typedef struct vIndexNode_r {
  vIndex vi;
  int32_t le;
  int32_t re;
} vIndexNode;


#define VI_NULLEDGE     ((int32_t)(~0x0))




typedef union u_vHdr {
  struct {
    unsigned int len    : 24;
    unsigned int btlen  : 24;
    unsigned int magic  : 16;
    unsigned int flags  : 16;
    unsigned int btoff  : 16;
    unsigned int alen   :  8;
    unsigned int unused :  8;
  } s;
  vIndex   vdum[2];
} vHdr;




typedef struct vecPos_r {
  vHdr   h;
  vIndex v[1];
} vecPos;


typedef struct vecPosBT_r {
  vHdr   h;
  vIndexNode v[1];
} vecPosBT;



typedef struct vecPosPool_r {
  void *block;
  unsigned int bsize;
} vecPosPool;


#define vecpos_pool_size  (4*1024*1024)
#define vecpos_pool_count  4
#define MAX_POOLS          16*1024


typedef struct vecPosPoolMgr_r {
  unsigned int pcnt;
  vecPosPool   pools[MAX_POOLS];
} vecPosPoolMgr;



void vppmgr_init( vecPosPoolMgr * );
void vppmgr_release( vecPosPoolMgr *, vecPos * );
vecPos *vppmgr_new( vecPosPoolMgr *, size_t );
vecPosBT *vppmgr_newbt( vecPosPoolMgr *, size_t );

inline void vpos_clear( vecPos * );
inline void vposbt_clear( vecPosBT * );
int vpos_append( vecPos *, vIndex );
int vpos_and( const vecPos *, const vecPos *, vecPos * );
int vpos_or( const vecPos *, const vecPos *, vecPos * );
void vpos_print( FILE *, const vecPos * );

void vpos_sort_docid( vecPos * );

#ifdef VPOS_SOURCE
int ilog2( const int );
#endif

int vindex_doc_cmp( const void *, const void * );
int vposbt__append( vIndexNode [], const int, vIndex );
int vposbt__unwind( vIndexNode [], const int, vecPos * );
int vposbt_append( vecPosBT *, vIndex );
int vposbt_unwind( vecPosBT *, vecPos * );
int vposbt_traverse( vecPosBT *, void (*)( vIndex ) );
int vposbt__traverse( vIndexNode [], int, void (*)( vIndex ) );
#endif
