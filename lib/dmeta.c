#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dmeta.h"




int dmetapool_init( struct dmeta_pool_r *dmp,  size_t pagesize, unsigned int mxpagecnt )
{
  dmp->pagesize     = (pagesize) ? pagesize : DMETAPOOL_PAGE_SIZE;
  dmp->lpagelen     = (mxpagecnt) ? mxpagecnt : DMETAPOOL_DEFAULT_MAX_PAGE_COUNT;
  dmp->page_list    = malloc(sizeof(struct dmeta_page_r *)*dmp->lpagelen);
  dmp->page_list[0].freep = dmp->page_list[0].base = malloc(sizeof(char)*dmp->pagesize);
  dmp->page_cnt     = 1;
  return 0;
}



int dmetapool_insert_dmeta( struct dmeta_pool_r *dmp, void *dm, unsigned int dsize )
{
  struct dmeta_page_r *pcur = &(dmp->page_list[dmp->page_cnt-1]);
  if ((pcur->freep - pcur->base) + dsize > dmp->pagesize) {
    ++dmp->page_cnt;
    ++pcur;
    pcur->freep = pcur->base = malloc(sizeof(char)*dmp->pagesize);
    
  }
  //  printf("Free: %u\n", dmp->pagesize - (pcur->freep - pcur->base));
  memcpy(pcur->freep, dm, (dsize*sizeof(char)));
  pcur->freep += dsize;
  return 0;
}
