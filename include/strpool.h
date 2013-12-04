#include <string.h>
#include <stdlib.h>

#ifndef _STRPOOL_INCLUDE_

struct strpool_blob_r {
  struct strpool_blob_r *next;
  struct strpool_blob_r *prev;
  char *blob;
  size_t rem;
};


typedef struct strpool_r {
  struct strpool_blob_r *head;
  size_t bsize;
  unsigned int bcnt;
  char *nextfree;
} Strpool;


#define DEFAULT_BLOB_SIZE   (5*1024*1024)

void strpool_init( Strpool *sp, size_t bsize );
void strpool_release( Strpool *sp );
char *strpool_add( Strpool *sp, const char *p );

#define _STRPOOL_INCLUDE_
#endif
