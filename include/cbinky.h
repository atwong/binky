#ifndef __CBINKY_INCLUDE__
#define __CBINKY_INCLUDE__
#include <stdlib.h>
#include <stdint.h>
#include "mi.h"
#include "docm.h"
#include "htab.h"
#include "strpool.h"
#include "util.h"
#include "dmeta.h"
/*
 *   Internal Index Vector
 * 
 * List of documents that contain the token 
 * by with one-level of indirection.
 *
 * What the hash table entries point to
 *
 * NB. This vector is guaranteed to be in ascending 
 * order by construction. Can only append and value
 * must be >= than final value.
 *
 */
typedef struct iiv_r {
  uint32_t len;
  uint32_t size;
  uint32_t flgs;
  uint16_t *tf;
  struct vbyte_r woff;
  uint32_t v[1];
} Iiv;


#define CBIIV_VALID         0x10000   //
#define CBIIV_ZEROED        0x20000   // 
#define CBIIV_TRMFRQ        0x40000   // term weights enabled --> maintain term freq per doc
#define CBIIV_WRDOFF        0x80000   // word offsets enabled 
#define CBIIV_MASK          0xf0000











/*
 *
 *  Core Binky context
 *
 */

#define MAX_IIV_LOGLEN         22                        
#define INIT_IIV_LOGLEN        4
#define INIT_IIV_LEN           ((01<<INIT_IIV_LOGLEN))   // initial iv length -- critical for memory consumption
#ifdef __CBINKY_SRC__
#define HTAB_MAX_DENSITY       0.25                      // max hash table density before resizing htab
#define MAX_IIV_LEN            ((01<<MAX_IIV_LOGLEN))    // max internal index value, upperbound on docs, also max IIV len allowed
#define INIT_DI2II_SIZE        (64*1024)
#endif

/*
 * option flags
 */

#define CBINKY_WORD_OFFSETS 0x2       // flag
#define CBINKY_TERM_FREQ    0x4


struct cbinky_stats_r {
  unsigned int  longest_iiv;
  unsigned int  iiv_len_histo[MAX_IIV_LOGLEN+1];
};

typedef struct cbinky_r {
  docMMgr                     *docmmgr;
  htab                        hTab;
  Strpool                     sPo;
  Mi                          *vlt;            // maps ii    --> doc Mi record
  htab                        di2ii;           // maps docId --> doc Mi record
  struct dmeta_pool_r         dmp;
  uint32_t                    cur_ii;
  size_t                      docmsize;
  unsigned int                dmsize;
  unsigned int                flags;
  size_t                      iv_init_len;
  struct cbinky_stats_r       stats;
} cBinky;

#ifdef __CBINKY_SRC__
Iiv *iiv_alloc( unsigned int len, unsigned int flags );
void iiv_zero( Iiv *iiv );
Iiv *iiv_append( Iiv *iiv, const uint32_t value );
void cbinky_iiv_stats_incr( struct cbinky_stats_r *, int );
#endif
cBinky *cbinky_init(unsigned int hsize, size_t dmetasize, unsigned int flags );
int  cbinky_find( cBinky *cb, char const *token, docid_t *resv, uint16_t *tfv, int reslen );
static inline int cbinky_find_idx( cBinky *cb, char const *token ){ return htab_find_idx(&(cb->hTab),token);};
static inline void *cbinky_find_internal( cBinky *cb, char const *token) {return htab_find(&(cb->hTab),token,0);};
int cbinky_find_insert( cBinky *cb, const char *token, const uint32_t woff );
int cbinky_add_docid( cBinky *cb, uint64_t docid );
int cbinky_insert_dmeta( cBinky *cb, uint64_t docid, void *dmeta );
int cbinky_doc_count( cBinky *cb, int invalid );
int cbinky_get_docids(cBinky *cB, docid_t vdid[], int vdidlen);


#endif /* __CBINKY_INCLUDE__ */
