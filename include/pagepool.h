#ifndef _PAGEPOOL_INCLUDE_
#define _PAGEPOOL_INCLUDE_
#include <stdlib.h>
#include <stdint.h>

#define PAGEPOOL_PAGE_SIZE (1<<16)
#define PAGEPOOL_MIN_ALLOC 16
#define PAGEPOOL_LOC(x) (((x).pidx*2*PAGEPOOL_PAGE_SIZE)+(x).poff)      /* order chunks by page idx/offset (x2 to prevent coalescing disjoint pages */


typedef struct pagepool_ptr_r {
  uint16_t pidx;
  uint16_t poff;
  uint16_t size;
} pgpp;

struct ppll {
  pgpp  chunk;
  struct ppll *snext;  /* next in size */
  struct ppll *lnext;  /* next in location */
};


struct pagepool_page_r {
  struct pagepool_page_r *next;
  char *page;
  int  idx;
};


typedef struct pagepool_r {
  struct pagepool_page_r *pool_head;
  struct pagepool_page_r *pool_tail;
  struct ppll *frsiz_head;
  struct ppll *frsiz_tail;
  struct ppll *frloc_head;
  struct ppll *frloc_tail;
  unsigned long ncoal;
} pagepool;


void pp_init( pagepool *pp );
pgpp pp_alloc( pagepool *pp, size_t size );
void pp_free( pagepool *pp, pgpp );
void pp_stats( pagepool *pp );

#endif /* PAGEPOOL_INCLUDE */
