#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "trie.h"


char **trie_off_p( const char *tr ); 

#define MIN_TRIE_NODE_LEN 8
#define MIN_TRIE_SIZE (sizeof(char)*MIN_TRIE_NODE_LEN + sizeof(char *)*(MIN_TRIE_NODE_LEN-2))


/* ======================================================
 * 
 *   Debugging cruft
 *
 * ====================================================== */

void trie_backtrace( trie_btr *pstk )
{
  int i, n;
  char *p;
  printf("  BTR ---\n");
  for (; pstk->p!=NULL; --pstk) {
    printf("%c  --  ", (pstk->p[2+pstk->idx] & 0x7f));
    n = (unsigned char)pstk->p[1];
    p = pstk->p+2;
    for (i=0; i<n; ++i) {
      printf("%c", (p[i]&0x7f));
    }
    printf("\n");
  }
  printf("\n");
}


#if DEBUG>5
int trie_validate( char *tr )
{
  unsigned char n;
  char **q, *p, c;
  int i;
  if (*tr == TRIE_SENTINEL) {
    n = *((unsigned char *)(tr+1));
    q = trie_off_p(tr);
    p = tr+2;
    for (i=0; (i<n)&&(p[i]!='\0'); ++i) {
      if (q[i] == NULL) {
	assert(((p[i] & 0x80) != 0));
      }
      if ((p[i] & 0x80) == 0) {
	assert((q[i] != NULL));
      }
      c = p[i] & 0x7f;
      if (!((c >= '0' && c <= '9') ||
	    (c >= 'A' && c <= 'Z') ||
	    (c >= 'a' && c <= 'z'))) {
	printf("About to fail:  C = %c\n", c);
      }
      assert(((c >= '0' && c <= '9') ||
	      (c >= 'A' && c <= 'Z') ||
	      (c >= 'a' && c <= 'z')));

    }
  }
  return 0;
}
#else
int trie_validate( char *tr )
{
  return 0;
}
#endif




/* ======================================================
 * 
 *   Utility
 *
 * ====================================================== */

char **trie_off_p( const char *tr ) 
{
  assert((*tr == TRIE_SENTINEL));
  unsigned char *pn = (unsigned char*)(tr+1);
  unsigned char n = *pn;
  int n2 = n+2;
  int nn = n2/sizeof(char*);
  int mm = (nn + ((n2%sizeof(char*))?1:0))*sizeof(char*);
  return (char **)(tr+mm);
}


char *trie_new( int nelt, char ***pqn )
{
  int psiz = sizeof(char*);
  int n2 = nelt + 2;
  int nn = n2/psiz;
  int mm = (nn + ((n2%psiz)?1:0))*psiz;
  size_t tsiz = sizeof(char)*mm + sizeof(char *)*nelt;
  assert(((tsiz%psiz)==0));
  char *p = malloc(tsiz);
  unsigned char *q;
  p[0] = TRIE_SENTINEL;
  q = (unsigned char *)(p+1);
  *q = nelt;
  memset(p+2, '\0', sizeof(char)*nelt);
  *pqn = (char **)(p+mm);
  return p;
}


int trie_insert( char *tr, const char word[], char **parent, trie_btr pstk[] )
{
  int i;
  unsigned char n;
  char **q;
  char *p;
  char w7[64];
  int dbg;

  assert((*word != '\0'));
  if (*tr == TRIE_SENTINEL) {
    n = *((unsigned char *)(tr+1));
    q = trie_off_p(tr);
    p = tr+2;
    /*
     * search for leading char
     */
    for (i=0; (i<n)&&(p[i]!='\0'); ++i) {
      if ((p[i]&0x7f) == word[0]) {
	if ((word[1] == '\0') && (p[i] & 0x80)) {
	  ;
	}
	else {
	  assert((((p[i] & 0x80) != 0) || (q[i]!=NULL)));
	  p[i] |= (word[1] == '\0') ? 0x80 : 0x0;
	  if (word[1] != '\0') {
	    if (q[i] != NULL) {
	      trie_validate(tr);
#if DEBUG>5
	      pstk[0].p   = tr;
	      pstk[0].idx = i;
	      return trie_insert(*(q+i), word+1, q+i, ++pstk);
#else
	      return trie_insert(*(q+i), word+1, q+i, 0);
#endif
	    }
	    else {
	      q[i] = strdup(word+1);
	    }
	  }
	}
	trie_validate(tr);
	return 0;
      }
    }
    if (i==n) {
      /*
       * resize trie
       */
      {
	assert(((*parent) == tr));
	char **nq, *pn = *parent = trie_new(n*2, &nq);
	char *l = pn+2;
	int j;
	for (j=0; j<n; ++j) {
	  l[j]  = p[j];
	  nq[j] = q[j];
	}
	free(tr);
	p = l;
	q = nq;
	tr = pn;
      }
    }
    p[i] =  word[0] | (unsigned char)((word[1] == '\0') ? 0x80 : 0x0);
    q[i] = (word[1] != '\0') ? strdup(word+1) : NULL;
    trie_validate(tr);
    return 1;
  }
  /*
   *  Word-frag
   */
  if (!strcmp(word,tr)) {
    trie_validate(tr);
    return 1;
  }
  {
    int j;
    char *p, *tn, *trx;
    char **nq, **par;
      
    par = parent;
    for (i=0; (word[i]) && (word[i]==tr[i]); ++i) {
      trx = trie_new(MIN_TRIE_NODE_LEN, &nq);
      trx[2] = word[i];
      *par   = trx;
      par = nq;
    }

    if ((word[i] == '\0') || (tr[i] == '\0')) {
      trx[2] |= 0x80;
    }

    trx = trie_new(MIN_TRIE_NODE_LEN, &nq);
    *par = trx;
    if (word[i]) {
      trx[2] = word[i];
      if (word[i+1]) {
	nq[0] = strdup(word+i+1);
      }
      else {
	trx[2] |= 0x80;
	nq[0] = NULL;
      }
    }
    if (tr[i]) {
      j = (word[i])?1:0;
      trx[2+j] = tr[i];
      if (tr[i+1]) {
	nq[j] = strdup(tr+i+1);
      }
      else {
	trx[2+j] |= 0x80;
	nq[j] = NULL;
      }
    }
    free(tr);
  }
  return 0;
}






int trie_move( char *tr, char *stk, int level )
{
  int i, n;
  char *p;
  char **q;
  if (*tr == TRIE_SENTINEL) {
    n = (unsigned char)tr[1];
    p = tr+2;
    q = trie_off_p(tr);
    for (i=0; (i<n)&&(p[i]); ++i) {
      *stk = (p[i] & 0x7f);
      if (p[i] & 0x80) {
	*(stk+1) = '\0';
	//	printf("W %s\n", stk-level);
      }
      if (q[i] != NULL) {
	(void)trie_move(q[i], stk+1, (level+1));
      }
    }
  }
  else {
    strcpy(stk, tr);
    //    printf("Q %s\n", stk-level);
  }
  return 0;
}



int trie_find( char *tr, const char word[], int level )
{
  int i, n;
  char *p;
  char **q;
  if (*tr != TRIE_SENTINEL)
    return strcmp(tr, word);
  
  n = (unsigned char)tr[1];
  p = tr+2;
  q = trie_off_p(tr);
  for (i=0; (i<n)&&(p[i]); ++i) {
    if ((p[i] & 0x7f) == word[0]) {
      if ((word[1] == '\0') && (p[i] & 0x80))
	return 0;
      else if (q[i] != NULL) {
	return trie_find(q[i], word+1, level+1);
      }
      else {
	return 1;
      }
    }
  }
  return 1;
}



void trie_traverse( char *tr, char *stk, int level, void (*cbf)( char *, int, void * ), void *udata )
{
  int i, n;
  char *p;
  char **q;
  
  if (*tr == TRIE_SENTINEL) {
    n = (unsigned char)tr[1];
    p = tr+2;
    q = trie_off_p(tr);
    for (i=0; (i<n) && (p[i]!='\0'); ++i) {
      stk[level] = p[i] & 0x7f;
      if ((p[i] & 0x80) != 0x0) {
	stk[level+1] = '\0';
	cbf(stk, level, udata);
      }
      if (q[i] != NULL) {
	trie_traverse(q[i], stk, (level+1), cbf, udata);
      }
    }
  }
  else {
    strcpy(stk+level, tr);
    cbf(stk, level, udata);
  }
  return;
}



unsigned int trie_fsize( const char *tr )
{
  unsigned int siz;
  
  if (*tr == TRIE_SENTINEL) {
    unsigned char nelt = *((unsigned char*)(tr+1));
    siz = sizeof(struct trie_elt)*nelt + sizeof(unsigned int);
  }
  else {
    siz  = strlen(tr)+1;
  }
  return siz;
}


void trie_fwrite( FILE *outf, const char *tr )
{
  int i;
  size_t trsiz;
  unsigned char nelt;
  const char *p;
  char **q, *frec;
  struct trie_elt *pv;
  struct trie_rec *rec;

  if (*tr == TRIE_SENTINEL) {
    nelt = *((unsigned char*)(tr+1));
    p = tr + 2;
    q = trie_off_p(tr);
    trsiz = trie_fsize(tr);
    rec = malloc(trsiz);
    memset(rec, '\0', trsiz);
    rec->nelt = nelt;
    for (i=0; (i<nelt) && (p[i]!='\0'); ++i) {
      rec->e[i].c         = (p[i] & 0x7f);
      rec->e[i].flags.eow = ((p[i] & 0x80) != 0);
      if (q[i] != NULL) {
	if ((rec->e[i].flags.isnode = (*(q[i]) == TRIE_SENTINEL))) {
	  rec->e[i].nelt = *((unsigned char*)(q[i]+1));
	}
	else {
	  rec->e[i].nelt = strlen(q[i])+1;
	}
      }
      else {
	assert(((p[i] & 0x80) != 0));
	rec->e[i].nelt = 0;
      }
    }
    fwrite(rec, sizeof(char), trsiz, outf);
    free(rec);
    for (i=0; (i<nelt) && (p[i]!='\0'); ++i) {
      if (q[i] != NULL)
	trie_fwrite(outf, q[i]);
    }
  }
  else {
    fwrite(tr, sizeof(char), strlen(tr)+1, outf);
  }
  return;
}



char *trie_fread( FILE *inf, unsigned int nelt, int istrie, int level, char stk[] )
{
  unsigned int rsiz;
  struct trie_rec *rec;
  struct trie_elt *pe;
  char *tr, *p;
  char **q;
  int i;

  if (istrie) {
    rsiz = sizeof(unsigned int)+sizeof(struct trie_elt)*nelt;
    rec = malloc(rsiz);
    tr = trie_new(nelt, &q);
    p = tr + 2;
    fread(rec, sizeof(char), rsiz, inf);
    pe = rec->e;
    for (pe=rec->e,i=0; i<rec->nelt && (pe->c!='\0'); ++i, ++pe) {
      stk[level] = pe->c;
      p[i] = pe->c;
      if (pe->flags.eow) {
	stk[level+1] = '\0';
	p[i] |= 0x80;
      }
      q[i] = NULL;
      if (pe->nelt > 0)
	q[i] = trie_fread(inf, pe->nelt, pe->flags.isnode, (level+1), stk);
    }
    free(rec);
  }
  else {
    tr = malloc(sizeof(char)*nelt);
    fread(tr, sizeof(char), nelt, inf);
    assert((tr[nelt-1] == '\0'));
  }
  return tr;
}




#if DEBUG>5
int trie_find_d( char *tr, const char word[], int level )
{
  int i, n;
  char *p;
  char **q;
  if (*tr != TRIE_SENTINEL) {
    //    printf("Wzz %p %s %s %d\n", tr, tr, word, strcmp(tr,word));
    return strcmp(tr, word);
  }
  
  n = (unsigned char)tr[1];
  p = tr+2;
  q = trie_off_p(tr);
  printf("[");
  for (i=0; (i<n)&&(p[i]); ++i) {
    printf("%c", (char)(p[i] & 0x7f));
  }
  printf("] ");
  for (i=0; (i<n)&&(p[i]); ++i) {
    printf("%c", (char)(p[i] & 0x7f));
    if ((p[i] & 0x7f) == word[0]) {
      if ((word[1] == '\0') && (p[i] & 0x80)) {
	printf(" Y\n");
	return 0;
      }
      else if (q[i] != NULL) {
	printf("->\n");
	return trie_find_d(q[i], word+1, level+1);
      }
      else {
	printf(" x\n");
	return 1;
      }
    }
  }
  printf(" z\n");
  return 1;
}
#endif
