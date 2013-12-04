#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mi.h"
#include "docm.h"
#include "htab.h"
#include "util.h"
#include "strpool.h"
#define __CBINKY_SRC__
#include "cbinky.h"


Iiv *iiv_alloc( unsigned int vsize, unsigned int flags )
{
  size_t realsize = sizeof(Iiv) +  sizeof(uint32_t)*vsize;
  if (flags & CBIIV_TRMFRQ) {
    realsize += sizeof(uint16_t)*vsize;
  }
  Iiv *p = malloc(realsize);
  p->flgs = (flags | CBIIV_VALID);
  p->size = vsize;
  p->len  = 0;
  p->tf   = NULL;
  if (flags & CBIIV_TRMFRQ) {
    p->tf = (uint16_t *)(p->v+vsize);
  }
  iiv_zero(p);
  return p;
}


void iiv_zero( Iiv *iiv )
{
  iiv->flgs |= CBIIV_ZEROED;
  (void)memset(iiv->v, 0, sizeof(uint32_t)*iiv->size);
  if (iiv->flgs & CBIIV_TRMFRQ) {
    (void)memset(iiv->tf, 0, sizeof(uint16_t)*iiv->size);
  }
}


void iiv_copy( Iiv *srciv, Iiv *destiv ) 
{
  destiv->len  = srciv->len;
  destiv->flgs = srciv->flgs;
  memcpy(destiv->v, srciv->v, sizeof(uint32_t)*srciv->len);
  destiv->flgs &= ~CBIIV_ZEROED;
  assert(((srciv->flgs & CBIIV_TRMFRQ) == 0) || (destiv->tf!=NULL));
  if (destiv->flgs & CBIIV_TRMFRQ) {
    memcpy(destiv->tf, srciv->tf, sizeof(uint16_t)*srciv->len);
  }
  
}


Iiv *iiv_append( Iiv *iiv, const uint32_t value ) 
{
  uint32_t *v= iiv->v;
  iiv->flgs &= ~CBIIV_ZEROED;
  if ((iiv->len) && (v[iiv->len-1] == value)) {
    if (iiv->flgs & CBIIV_TRMFRQ) {
      assert((iiv->tf!=NULL));
      iiv->tf[iiv->len-1] += 1;
    }
    return NULL;
  }

  if (iiv->len >= iiv->size) {
    Iiv *newiv = iiv_alloc((iiv->size*2), iiv->flgs);
    iiv_copy(iiv, newiv);
    v = newiv->v;
    v[newiv->len] = value;
    if (iiv->flgs & CBIIV_TRMFRQ) {
      assert((newiv->tf!=NULL));
      newiv->tf[newiv->len] += 1;
    }
    ++newiv->len;
    return newiv;
  }
  else {
    v[iiv->len] = value;
    if (iiv->flgs & CBIIV_TRMFRQ) {
      assert((iiv->tf!=NULL));
      iiv->tf[iiv->len] += 1;
    }
    ++iiv->len;
  }
  return NULL;
}




cBinky *cbinky_init(unsigned int hsize, size_t dmetasize, unsigned int flags )
{
  cBinky *cb;
  cb = malloc(sizeof(cBinky));
  if (!cb) {
    return NULL;
  }
  memset(cb, '\0', sizeof(cBinky));
  /*
   *  map from II --> docid+meta
   */
  cb->vlt    = malloc(sizeof(Mi)*MAX_IIV_LEN);         // crude - return and make growable
  if (cb->vlt == NULL) {
    free(cb);
    return NULL;
  }
  memset(cb->vlt, '\0', sizeof(Mi)*MAX_IIV_LEN);
  /*
   *  map from docid --> II
   */
  if (htab_init(&(cb->di2ii), INIT_DI2II_SIZE)<0) {
    free(cb->vlt);
    free(cb);
    return NULL;
  }
  /*
   *
   */
  if (htab_init(&(cb->hTab), hsize)<0) {
    htab_free(&cb->di2ii);
    free(cb->vlt);
    free(cb);
    return NULL;
  }
  /*
   *
   */
  strpool_init(&cb->sPo,0);
  /*
   *
   */
  dmetapool_init(&cb->dmp, 0, 0);
  cb->cur_ii = 0;
  cb->dmsize = dmetasize;
  cb->flags  = flags;
  cb->flags |= (flags & CBINKY_TERM_FREQ)  ? CBIIV_TRMFRQ : 0;
  cb->flags |= (flags & CBINKY_WORD_OFFSETS) ? CBIIV_WRDOFF : 0;
  if (cb->flags & CBIIV_TRMFRQ) {
    fprintf(stderr,"Building index with term freq\n");
  }
  cb->iv_init_len = INIT_IIV_LEN;
  return cb;
}



/*
 * basic operation -- find docs that contain a token
 * returns number of DocIds in result vector
 *
 */
int cbinky_find( cBinky *cb, char const *token, docid_t *resv, uint16_t *tfv, int reslen )
{
  int i, j;
  Iiv *xiiv = (Iiv *)htab_find(&(cb->hTab), token, 0);
  if (xiiv!=NULL) {
    assert((xiiv->flgs & CBIIV_VALID));
    uint32_t *v   = xiiv->v;
    for (j=i=0; i<xiiv->len && (j<reslen); ++i) {
      assert((v[i] > 0));
      assert((v[i] <= cb->cur_ii));
      Mi *mi = &(cb->vlt[v[i]]);
      if (mi->u.bflgs.valid) {
	resv[j] = mi->docid;
	if ((tfv) && (xiiv->tf!=NULL) && (xiiv->flgs&CBIIV_TRMFRQ)) {
	  tfv[j] = xiiv->tf[j];
	}
	++j;
      }
    }
    return j;
  }
  return 0;
}



int cbinky_add_docid( cBinky *cb, docid_t docid )
{
  char sdid[17];
  sprintf(sdid, "%llx", docid);
  fprintf(stderr,"CL htab_find\n");
  Mi *oldmi = htab_find(&cb->di2ii, sdid, NULL);
  fprintf(stderr,"RT htab_find\n");
  if (oldmi) {
    assert((oldmi->docid == docid));
    oldmi->u.bflgs.obsel = 1;
    oldmi->u.bflgs.valid = 0;
  }
  int ii = ++cb->cur_ii;
#if DEBUG>5  
  {
    int i;
    for (i=0; i<cb->cur_ii; ++i) {
      assert(((cb->vlt[i].docid != docid) || (cb->vlt[i].u.bflgs.obsel != 0)));
    }
  }
#endif
  assert((cb->vlt[ii].u.bflgs.valid == 0));
  cb->vlt[ii].docid         = docid;
  cb->vlt[ii].u.bflgs.valid = 1;
  cb->vlt[ii].lastmod       = time(NULL);
  cb->vlt[ii].hifreqcnt     = 0;

  char *ntoken = strpool_add(&cb->sPo, sdid);
  //  char *ntoken = strdup(sdid);
  htab_insert(&cb->di2ii, ntoken, &(cb->vlt[ii]));
  return ii;
}


int cbinky_find_insert( cBinky *cb, const char *token, const uint32_t woff)
{
  int idx;
  char *ntoken;
  Iiv *miv = htab_find(&(cb->hTab), token, &idx);
  Iiv *newiv;
  if (miv == NULL) {
    miv = iiv_alloc(cb->iv_init_len , (cb->flags&CBIIV_MASK));
    ntoken = strpool_add(&cb->sPo, token);
    idx = htab_insert(&(cb->hTab), ntoken, miv);
    cbinky_iiv_stats_incr(&(cb->stats), -miv->size);
  }
  newiv = iiv_append(miv, cb->cur_ii);
  if (newiv != NULL) {
    free(miv);
    htab_update_index(&(cb->hTab), idx, newiv, token);
    miv = newiv;
    cbinky_iiv_stats_incr(&(cb->stats), miv->size);
  }
  cb->stats.longest_iiv = (miv->len > cb->stats.longest_iiv) ? miv->len : cb->stats.longest_iiv;
  float hdensity = (float)cb->hTab.stats.entrycnt/((float)cb->hTab.size);
  if (hdensity > HTAB_MAX_DENSITY) {
    htab_grow(&(cb->hTab), 4.0);
  }
  return idx;
}



int cbinky_insert_dmeta( cBinky *cb, docid_t docid, void *dmeta )
{
  int imi;
  
  for (imi=0; imi<=cb->cur_ii; ++imi) {
    if (cb->vlt[imi].docid == docid) {
      if (cb->vlt[imi].u.bflgs.dmeta == 0) {
	dmetapool_insert_dmeta(&cb->dmp, dmeta, cb->dmsize);
      }
    }
  }
  return -1;
}


int cbinky_doc_count( cBinky *cb, int invalid )
{
  int i, cnt;
  if (invalid) {
    cnt = cb->cur_ii;
  }
  else {
    for (cnt=i=0; i<=cb->cur_ii; ++i)
      cnt += ((cb->vlt[i].u.bflgs.valid) && (cb->vlt[i].u.bflgs.obsel==0))?1:0;
#if DEBUG>2
    for (i=cb->cur_ii+1; i<MAX_IIV_LEN; ++i)
      assert((cb->vlt[i].u.bflgs.valid == 0));
#endif
  }
  return cnt;
}


int cbinky_get_docids(cBinky *cB, docid_t vdid[], int vdidlen) 
{
  int i, cnt;
  for (cnt=i=0; (i<=cB->cur_ii) && (cnt<vdidlen); ++i) {
    if ((cB->vlt[i].u.bflgs.valid) && (cB->vlt[i].u.bflgs.obsel==0)) {
      vdid[cnt++] = cB->vlt[i].docid;
    }
  }
  return cnt;
}


void cbinky_iiv_stats_incr( struct cbinky_stats_r *cbs, int ilen )
{
  int initlen = (ilen<0);
  unsigned int l2 = ilog2((initlen)?-ilen:ilen);
  if (!initlen) {
    assert((l2-1)>0);
    cbs->iiv_len_histo[(l2-1)] -= 1;
  }
  cbs->iiv_len_histo[l2] += 1;
  return;
}




