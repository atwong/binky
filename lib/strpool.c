#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "strpool.h"

void x_strpool_new_blob( Strpool *sp ) {
  struct strpool_blob_r *nsb = malloc(sizeof(struct strpool_blob_r));
  memset(nsb, '\0', sizeof(struct strpool_blob_r));
  sp->nextfree = nsb->blob = malloc(sp->bsize);
  if (sp->head != NULL) {
    sp->head->prev = nsb;
  }
  nsb->next = sp->head;
  nsb->rem  = sp->bsize;
  sp->head  = nsb;
  ++sp->bcnt;
}



void strpool_init( Strpool *sp, size_t bsize ) {
  sp->bsize     = (bsize) ? bsize : (DEFAULT_BLOB_SIZE*sizeof(char));
  sp->head      = NULL;
  sp->bcnt      = 0;
  x_strpool_new_blob(sp);
}




char *strpool_add( Strpool *sp, const char *p ) {
  int slen = strlen(p)+2;
  if ((slen*sizeof(char)) > sp->head->rem) {
    x_strpool_new_blob(sp);
  }
  char *q = sp->nextfree;
  strcpy((q+1), p);
  sp->nextfree += slen;
  sp->head->rem -= slen*sizeof(char);
  return (q+1);
}



void strpool_release( Strpool *sp ) {
  struct strpool_blob_r *sb, *sbx;
  for (sb=sp->head; sb; sb=sb->next) {
    free(sb->blob);
  }
  for (sbx=0, sb=sp->head; sb; sbx=sb,sb=sb->next)
    free(sbx);
  free(sbx);
}
