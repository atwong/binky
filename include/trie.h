#include "pagepool.h"
#ifndef _TRIE_INCLUDE_
#define _TRIE_INCLUDE_


#define TRIE_SENTINEL '\xfe'

typedef struct trie_backtrace_r {
  char *p;
  int idx;
} trie_btr;



/*
 *  On-disk representations
 */
struct trie_elt {
  char c;
  unsigned char nelt;
  struct {
    unsigned char eow    : 1;
    unsigned char isnode : 1;
  } flags;
};

struct trie_rec {
  unsigned int nelt;
  struct trie_elt e[1];
};




int trie_insert( char *tr, const char word[], char **parent, trie_btr btr[] );
char *trie_new( int nelt, char ***pqn );
int trie_move( char *tr, char *stk, int level );
int trie_find( char *tr, const char word[], int level );
#if DEBUG>5
int trie_find_d( char *tr, const char word[], int level );
#endif
void trie_traverse( char *tr, char *stk, int level, void (*cbf)( char *, int, void * ), void *udata );
void trie_fwrite( FILE *outf, const char *tr );
char *trie_fread( FILE *inf, unsigned int nelt, int istrie, int level, char stk[] );

#endif /* _TRIE_INCLUDE_ */
