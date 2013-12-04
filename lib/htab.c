#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "fnv.h"
#include "htab.h"


static const int hsize_primes[] = {
				 53,
				 97,
				 193,
				 389,
				 769,
				 1543,
				 3079,
				 6151,
				 12289,
				 24593,
				 49157,
				 98317,
				 196613,
				 393241,
				 786433,
				 1572869,
				 3145739,
				 6291469,
				 12582917,
				 25165843,
				 50331653,
				 100663319,
				 201326611,
				 402653189,
				 805306457,
				 1610612741,
				 -1
};



char htab_sentinel[2] = "\0\0";


int htab_init( htab *ht, size_t size ) {
  int i;
  size_t sizreq = (size) ? size : HTAB_DEFAULT_HSIZE;
  for (i=0; hsize_primes[i] > 0; ++i) {
    if (hsize_primes[i] > sizreq) {
      ht->size = hsize_primes[i];
      fprintf(stderr,"Requested size: %zu  Actual size: %zu\n", sizreq, ht->size);
      break;
    }
  }
  memset(&ht->stats, '\0', sizeof(struct htab_stats));
  ht->hash2_init_basis = fnv_32_str(HTAB_SECOND_INIT_BASIS, FNV1_32_INIT);
  if ((ht->tab = malloc(sizeof(hentry)*ht->size))) {
    memset(ht->tab, '\0', sizeof(hentry)*ht->size);
    return 0;
  }
  return -1;
}


void htab_free( htab *ht ) {
  if (ht->tab) {
    free(ht->tab);
    ht->tab = NULL;
  }
}



inline int htab_find_idx( htab *ht, char const *key ) {
  int probeinc = 0, ih, j, dj, ih0 = -1;
  Fnv32_t h = fnv_32_str((char*)key, FNV1_32_INIT);
  for (ih=h%ht->size, dj=0; (ht->tab[ih].key)&&(ih0!=ih); dj+=probeinc, ih=(h+dj)%ht->size) {
    if (!strcmp((char*)ht->tab[ih].key, (char*)key)) {
      return ih;
    }
    else if (!probeinc) {
      probeinc = fnv_32_str((char *)key, ht->hash2_init_basis);
    }
    if (ih0 < 0)
      ih0 = ih;
  }
  if (ih0 == ih) {
    fprintf(stderr,"Circled entire htab!\n");
  }
  return -1;
}



void *htab_find( htab *ht, char const *key, int *hidx ) {
  int ih = htab_find_idx(ht, key);
  if (hidx)
    *hidx = ih;
  return (ih >= 0) ? ht->tab[ih].v : NULL;
}



int htab_insert( htab *ht, char const *key, void *p ) {
  Fnv32_t h = fnv_32_str((char*)key, FNV1_32_INIT);
  Fnv32_t probeinc;
  int ih, jempty = -1, j;
  unsigned int idx = h % ht->size;

  int nprobe = 0;
  probeinc = fnv_32_str((char *)key, ht->hash2_init_basis);
  for (j=0, ih=(h%ht->size); ht->tab[ih].key; ++j, ih=((h+j*probeinc)%ht->size)) {
    assert((ih >= 0) && (ih<ht->size));
    //    fprintf(stderr,"PROBE: %p %d %d (%d)\n", ht->tab[ih].key, probeinc, ht->size, strlen(ht->tab[ih].key));
    if (!strcmp((char*)ht->tab[ih].key, (char*)key)) {
      //      fprintf(stderr,"PEND I %s %s\n", ht->tab[ih].key, key);
      ht->stats.probecnt += nprobe;
      return ih;
    }
    //    fprintf(stderr,"PEND II\n");
    if ((jempty < 0) && (ht->tab[ih].key == htab_sentinel)) {
#if HDEBUG>8
      printf("key: %s reuse slot@%d  table fill:[%u/%zu]\n", key, ih, ht->stats.entrycnt, ht->size);
#endif
      printf("key: %s reuse slot@%d  table fill:[%u/%zu]\n", key, ih, ht->stats.entrycnt, ht->size);
      ht->stats.reusecnt++;
      jempty = ih;
    }
    nprobe++;
  }
  if (jempty < 0) {
    jempty = ih;
  }
  ht->stats.entrycnt++;
  ht->stats.probecnt += nprobe;
  assert((jempty >= 0) && (jempty < ht->size));
  ht->tab[jempty].key = key;
  //  ht->tab[jempty].key = 0;
  ht->tab[jempty].v   = p;
  return jempty;
}


int htab_update( htab *ht, char const *key, void *p ) {
  Fnv32_t h = fnv_32_str((char*)key, FNV1_32_INIT);
  int probeinc = 1, ih, jempty = -1, j;
  unsigned int idx = h % ht->size;

  int nprobe = 0;
  probeinc = fnv_32_str((char *)key, ht->hash2_init_basis);
  for (j=0, ih=(h%ht->size); ht->tab[ih].key; ++j, ih=((h+j*probeinc)%ht->size)) {
    if (!strcmp((char*)ht->tab[ih].key, (char*)key)) {
      ht->tab[ih].v = p;
      ht->stats.probecnt += nprobe;
      return ih;
    }
    nprobe++;
  }
  return -1;
}


int htab_update_index( htab *ht, unsigned int ih, void *p, char const *key ) {
  if (key != NULL) {
    assert((strcmp((char*)ht->tab[ih].key, key)==0));
  }
  ht->tab[ih].v = p;
  return ih;
}


void htab_iterator( htab *ht, void (*cb)( void *, void * ) )
{
  int ih;
  for (ih=0; ih<ht->size; ++ih) {
    if ((ht->tab[ih].key) && (ht->tab[ih].key != htab_sentinel)) {
      cb((void*)ht->tab[ih].key, (void*)ht->tab[ih].v);
    }
  }
}


void htab_grow( htab *ht, float scale )
{
  Fnv32_t h; 
  int i, j, probeinc, ih;
  size_t newsize = (int)(ht->size * scale);
  hentry *tabnew = malloc(sizeof(hentry)*newsize);
  if (!tabnew) {
    return;
  }
  memset(tabnew, '\0', sizeof(hentry)*newsize);
  for (i=0; i<ht->size; ++i) {
    if ((ht->tab[i].key != NULL) && (ht->tab[i].key != htab_sentinel)) {
      h = fnv_32_str((char*)ht->tab[i].key, FNV1_32_INIT);
      probeinc = fnv_32_str((char *)ht->tab[i].key, ht->hash2_init_basis);
      for (j=0, ih=(h%newsize); tabnew[ih].key; ++j, ih=((h+j*probeinc)%newsize)) {
	;
      }
      tabnew[ih].key = ht->tab[i].key;
      tabnew[ih].v   = ht->tab[i].v;
    }
  }
  free(ht->tab);
  ht->size = newsize;
  ht->tab  = tabnew;
  ht->stats.probecnt = 0;
  ++ht->stats.resize;
  return;
}
