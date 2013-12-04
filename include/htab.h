#ifndef __HTAB_INCLUDE__
#define __HTAB_INCLUDE__
#include <stdint.h>
#include "fnv.h"

#define HDEBUG 0

#define HTAB_DEFAULT_HSIZE 802913
/*
#define HTAB_DEFAULT_HSIZE 1605509
*/
/*
#define HTAB_DEFAULT_HSIZE 2494913
#define HTAB_DEFAULT_HSIZE 3239641
*/
#define HTAB_SECOND_INIT_BASIS "Epsum factorial non deposit quid"


typedef struct hentry_r {
  void *v;
  const char *key;
} hentry;


struct htab_stats {
  unsigned int resize;
  unsigned int entrycnt;
  unsigned int reusecnt;
  unsigned long probecnt;
};


typedef struct htab_r {
  struct htab_stats stats;
  Fnv32_t hash2_init_basis;
  char *free_slot;
  size_t size;
  hentry *tab;
} htab;


int htab_init( htab *, size_t );
int htab_find_idx( htab *, char const * );
void *htab_find( htab *, char const *, int * );
int htab_insert( htab *, char const *, void * );
int htab_update( htab *, char const *, void * );
int htab_update_index( htab *, unsigned int, void *, char const *);
void htab_iterator( htab *, void (*)( void *, void * ) );
void htab_grow( htab *, float );
void htab_free( htab * );

#endif /* __HTAB_INCLUDE__ */
