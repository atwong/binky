#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include <assert.h>


unsigned int ilog2( unsigned int x ) {
  int i;
  for (i=0;x>1;++i)
    x = x >> 1;
  return i;
}



uint32_t hash32shift(uint32_t key) {
  key = ~key + (key << 15); // key = (key << 15) - key - 1;
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key * 2057; // key = (key + (key << 3)) + (key << 11);
  key = key ^ (key >> 16);
  return key;
}

uint64_t hash64shift(uint64_t key) {
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ (key >> 28);
  key = key + (key << 31);
  return key;
}


uint32_t hash6432shift(uint64_t key) {
  key = (~key) + (key << 18); // key = (key << 18) - key - 1;
  key = key ^ (key >> 31);
  key = key * 21; // key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return (uint32_t) key;
}




void vbyte_add( struct vbyte_r *vr, const uint32_t *iv, unsigned int vlen )
{
  unsigned int char7bit = 0x80, d, m;
  int i, j;
  uint32_t u;
  unsigned char *q;

  for (i=0; i<vlen; ++i) {
    u = iv[i];
    //    printf("V  %u\n", u);
    do {
      if (vr->len == vr->size) {
	vr->size = (vr->size) ? (2*vr->size) : 16;
	q = malloc(sizeof(unsigned char)*vr->size);
	if (vr->v) {
	  memcpy(q, vr->v, sizeof(unsigned char)*vr->len);
	  free(vr->v);
	}
	vr->v = q;
	//	printf("Alloc: %09p   [%d]\n", q, vr->size);
      }
      m = u%char7bit;
      vr->v[vr->len] = (m << 1);
      if ((u = u/char7bit)) {
	vr->v[vr->len] |= 0x1;
      }
      //      printf("   M:  %04x   Z: %02x\n", m, vr->v[vr->len]);
      ++vr->len;
    } while (u!=0);
  }
}



void vbyte_expand_cb( struct vbyte_r *vr, void (*cb)(void *, uint32_t), void *p )
{
  int i, rshft;
  uint32_t u;
  for (i=0; i<vr->len; ++i) {
    for (u=0, rshft=0; i<vr->len;++i, rshft+=7) {
      u |= ((vr->v[i] >> 1) << rshft);
      if ((vr->v[i] & 0x1) == 0)
	break;
    }
    if (cb)
      cb(p, u);
  }
}




int vbyte_expand( struct vbyte_r *vr, uint32_t desc[], unsigned int dlen)
{
  int i, rshft;
  uint32_t u;
  for (dlen=i=0; i<vr->len; ++i) {
    for (u=0, rshft=0; i<vr->len;++i, rshft+=7) {
      u |= ((vr->v[i] >> 1) << rshft);
      if ((vr->v[i] & 0x1) == 0)
	break;
    }
    desc[dlen++] = u;
  }
  return dlen;
}


/*
 *
 *   Test if two integer arrays have an pair of elements that differ by
 *   less-than-equal to a specified distance.
 *
 *   dist  : max distance
 *   pos   : left-right order 
 *   vrl   : variable-byte ordered array
 *   vrr   : variable-byte ordered array
 *
 */
int vbyte_pair_dist( unsigned int dist, int pos, struct vbyte_r *vrl, struct vbyte_r *vrr )
{
  int i, j, i0, j0, rshft;
  uint32_t l, r;

  for (i0=j0=-1, i=j=0; (i<vrl->len)&&(j<vrr->len); i+=(l<=r), j+=(r<=l)) {
    if (i!=i0) {
      for (l=0, rshft=0; (i<vrl->len)&&(vrl->v[i]&0x1); ++i, rshft+=7)
	l |= ((vrl->v[i] >> 1) << rshft);
      l |= ((vrl->v[i] >> 1) << rshft);
      i0 = i;
    }
    if (j!=j0) {
      for (r=0, rshft=0; (j<vrr->len)&&(vrr->v[j]&0x1); ++j, rshft+=7)
	r |= ((vrr->v[j] >> 1) << rshft); 
      r |= ((vrr->v[j] >> 1) << rshft);
      j0 = j;
    }
    //    printf("%d [%d]   %d [%d]\n", l, i, r, j);
    if (pos) {
      if ((r>l) && ((r-l)<=dist))
	return 1;
    }
    else {
      if (abs((r-l))<=dist)
	return 1;
    }
  }
  return 0;
}




/*
 *
 * basic algorithms (uncompressed)
 *
 */

int pair_dist( int dist, const int p[], int plen, const int q[], int qlen )
{
  int i, j, l, r;

  for (i=j=0; (i<plen)&&(j<qlen); i+=(int)(l<=r), j+=(int)(r<=l)) {
    l = p[i];
    r = q[j];
    if (abs((l-r)) <= dist)
      return 1;
  }
  return 0;
}


void merge_eq( int *p, int plen, int *q, int qlen, int *z)
{
  int i, j, k;
  for (k=i=j=0; (i<plen)&&(j<qlen); ) {
    if (p[i] < q[j]) {
      ++i;
    }
    else if (q[j] < p[i]) {
      ++j;
    }
    else {
      z[k++] = p[i++];
      ++j;
    }
  }
}
  

/*
void merge_distance(int dist, int *p, int plen, int *q, int qlen, int *z)
{
  int l, r;
  int i, j, k;
  int e =0, d = 0, ed;
  i = j = 0;

  for (i=j=0; (i<plen)&&(j<qlen); ) {
    l = p[i];
    r = q[j];
    e = d;
    d = l - r;
    ed = e*d;
    if (ed <= 0)
	printf(" >>>>  %d %d  : %d %d\n", p[i], q[j], e, d);
    assert((i < plen));
    assert((j < qlen));
    if (l <= r)
      ++i;
    if (r <= l)
      ++j;
  }
}
*/


/*
 * strip prefix & suffix whitespace from string -- inplace
 */
void wsstrip( char *p )
{
  char *q, *r, *s, *eos;
  for (q=p; (*p == ' ') || (*p == '\t') || (*p == '\n') || (*p == '\r'); ++p)
    ;
  for (s=r=p; *r; ++r)
    ;
  eos = r;
  for ( ;(r>p) && ((*r == '\0') || (*r == ' ') || (*r == '\t') || (*r == '\n') || (*r == '\r')); --r)
    ;
  for ( ;s<=r; ++s, ++q) 
    if (s!=q)
      *q = *s;
  *q = '\0';
  return;
}
