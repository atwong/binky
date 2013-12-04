#ifndef __DMETA_INCLUDE__
struct dmeta_page_r {
  char *base;
  char *freep;
};


struct dmeta_pool_r {
  size_t pagesize;
  unsigned int lpagelen;
  struct dmeta_page_r *page_list;
  unsigned int page_cnt;
};



#define DMETAPOOL_PAGE_SIZE (8*1024)
#define DMETAPOOL_DEFAULT_MAX_PAGE_COUNT 256


int dmetapool_init( struct dmeta_pool_r *, size_t page_size, unsigned int max_page_count );
int dmetapool_insert_dmeta( struct dmeta_pool_r *dmp, void *dm, unsigned int dsize );

#define __DMETA_INCLUDE__
#endif
