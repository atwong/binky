#define PAGEPOOL_SRC
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pagepool.h"


void pp_init( pagepool *pp )
{
  pp->frsiz_head = NULL;
  pp->frsiz_tail = NULL;
  pp->frloc_head = NULL;
  pp->frloc_tail = NULL;
  pp->pool_head = NULL;
  pp->pool_tail = NULL;
  pp->ncoal = 0L;
}


void pp_stats( pagepool *pp )
{
  struct pagepool_page_r *pg;
  struct ppll *pll;
  size_t cum_free_chunk;
  int nfr;
  for (cum_free_chunk=0,pll=pp->frsiz_head,nfr=0; pll; pll=pll->snext, ++nfr)
    cum_free_chunk += pll->chunk.size;

  printf("Free list len: %d  Cum free size: %lu  PagesCount: %d  TotalPageSize: %lu  Coalesc:%ld\n", 
	 nfr, cum_free_chunk, (pp->pool_tail->idx+1), 
	 (size_t)(pp->pool_tail->idx+1)*PAGEPOOL_PAGE_SIZE, pp->ncoal);
}


void pp_validate( pagepool *pp )
{
  struct ppll *pll, *prev; 
  if (pp->frsiz_head) {
    for (prev=pp->frsiz_head, pll=pp->frsiz_head->snext; pll; prev=pll, pll=pll->snext) {
      assert((prev->chunk.size <= pll->chunk.size));
    }
  }
  if (pp->frloc_head) {
    for (prev=pp->frloc_head, pll=pp->frloc_head->lnext; pll; prev=pll, pll=pll->lnext) {
      assert((PAGEPOOL_LOC(prev->chunk) < PAGEPOOL_LOC(pll->chunk)));;
    }
  }
}




void pp_ll_detach( pagepool *pp, struct ppll *xpll)
{
  struct ppll *pll, *prev; 

  if (xpll == NULL)
    return;
  if (pp->frsiz_head != NULL) {
    if (xpll == pp->frsiz_head) {
      pp->frsiz_head = xpll->snext;
      if (xpll == pp->frsiz_tail) {
	assert((xpll->snext == NULL));
	pp->frsiz_tail = pp->frsiz_head = 0;
      }
    }
    else {
      for (prev=pp->frsiz_head, pll=pp->frsiz_head->snext; pll; prev=pll, pll=pll->snext) {
	if (xpll == pll) {
	  prev->snext = pll->snext;
	  if (xpll == pp->frsiz_tail) {
	    assert((pll->snext == NULL));
	    pp->frsiz_tail = prev;
	  }
	  break;
	}
      }
    }
  }


  if (pp->frloc_head != NULL) {
    if (xpll == pp->frloc_head) {
      pp->frloc_head = xpll->lnext;
      if (xpll == pp->frloc_tail) {
	assert((xpll->lnext == NULL));
	pp->frloc_tail = pp->frloc_tail = 0;
      }
    }
    else {
      for (prev=pp->frloc_head, pll=pp->frloc_head->lnext; pll; prev=pll, pll=pll->lnext) {
	if (xpll == pll) {
	  assert((prev != pll->lnext));
	  prev->lnext = pll->lnext;
	  if (xpll == pp->frloc_tail) {
	    assert((pll->lnext == NULL));
	    pp->frloc_tail = prev;
	  }
	  break;
	}
      }
    }
  }
}




void pp_ll_add( pagepool *pp, struct ppll *npll ) 
{
  struct ppll *pll, *prev; 

  for (prev=NULL, pll=pp->frsiz_head; pll; prev=pll, pll=pll->snext) {
    if (npll->chunk.size < pll->chunk.size) {
      npll->snext = pll;
      if (prev) {
	prev->snext = npll;
      }
      else {
	pp->frsiz_head = npll;
      }
      break;
    }
  }
  if (pll == NULL) {
    if (pp->frsiz_tail)
      pp->frsiz_tail->snext = npll;
    pp->frsiz_tail = npll;
    if (!pp->frsiz_head)
      pp->frsiz_head = npll;
  }
  /*
   *
   */
  unsigned long loc = PAGEPOOL_LOC(npll->chunk);

  for (prev=NULL, pll=pp->frloc_head; pll; prev=pll, pll=pll->lnext) {
    if (loc <= PAGEPOOL_LOC(pll->chunk)) {
      assert((npll != pll));
      if (prev) {
	assert((prev != npll));
	if ((PAGEPOOL_LOC(prev->chunk)+prev->chunk.size) == loc) {
	  ++pp->ncoal;
	  //	  prev->chunk.size += npll->chunk.size;
	  //  coalesce  ==>  reinject prev element into free size list
	  prev->lnext = npll;
	  npll->lnext = pll;
	}
	else {
	  prev->lnext = npll;
	  npll->lnext = pll;
	}
      }
      else {
	assert((pp->frloc_head == pll));
	pp->frloc_head = npll;
	npll->lnext = pll;
      }
      break;
    }
  }
  if (pll == NULL) {
    assert((pp->frloc_tail != npll));
    if (pp->frloc_tail) 
      pp->frloc_tail->lnext = npll;
    pp->frloc_tail = npll;
    if (!pp->frloc_head)
      pp->frloc_head = npll;
  }
  return;
}



pgpp pp_alloc( pagepool *pp, size_t size )
{
  pgpp rval;
  struct ppll *pll, *prev, *nppll;
  /*
   *  search free list
   */
  for (prev=NULL, pll=pp->frsiz_head; pll; prev=pll, pll=pll->snext) {
    if (size > pll->chunk.size) continue;
    if ((pll->chunk.size - size) < PAGEPOOL_MIN_ALLOC) {
      pp_ll_detach(pp, pll);
      rval = pll->chunk;
      free(pll);
      pp_validate(pp);
      return rval;
    }
    else if (pll->chunk.size >= size) {
      rval = pll->chunk;
      rval.size = size;
      pp_ll_detach(pp, pll);
      pll->chunk.size -= size;
      pll->chunk.poff += size;
      pp_ll_add(pp, pll);
      pp_validate(pp);
      return rval;
    }
  }
  /*
   *  grab new page
   */
  struct pagepool_page_r *pg = malloc(sizeof(struct pagepool_page_r));
  pg->page = malloc(sizeof(char)*PAGEPOOL_PAGE_SIZE);
  pg->next = NULL;
  pg->idx  = (pp->pool_tail) ? (pp->pool_tail->idx+1) : 0;
  if (pp->pool_tail)
    pp->pool_tail->next = pg;
  if (!pp->pool_head)
    pp->pool_head = pg;
  pp->pool_tail = pg;
  /*
   *
   */
  rval.pidx = pg->idx;
  rval.poff = 0;
  rval.size = (size >= PAGEPOOL_MIN_ALLOC) ? size : PAGEPOOL_MIN_ALLOC;
  /* 
   *  add remainder of page to free list
   */
  nppll = malloc(sizeof(struct ppll));
  nppll->chunk.pidx = pg->idx;
  nppll->chunk.poff = rval.size;
  nppll->chunk.size = PAGEPOOL_PAGE_SIZE - rval.size;
  nppll->snext       = NULL;
  nppll->lnext       = NULL;
  pp_ll_add(pp, nppll);
  pp_validate(pp);
  return rval;
}



void pp_free( pagepool *pp, pgpp chunk )
{
  struct ppll *pll, *prev, *npll;
  /*
   *  search free list
   */
  npll = malloc(sizeof(struct ppll));
  npll->chunk = chunk;
  npll->snext = npll->lnext = NULL;
  pp_ll_add(pp, npll);
  /*
  for (prev=NULL, pll=pp->frsiz_head; pll; prev=pll, pll=pll->snext) {
    if (pll->chunk.size > npll->chunk.size) {
      if (prev)
	prev->snext = npll;
       npll->snext = pll;
       return;
    }
  }
  assert((pp->frsiz_tail->chunk.size <= npll->chunk.size));
  pp->frsiz_tail->snext = npll;
  pp->frsiz_tail = npll;
  */
  return;
}
