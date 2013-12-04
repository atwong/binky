#ifndef __DOCM_INCLUDE__
#define __DOCM_INCLUDE__
#include <stdint.h>
#include "mi.h"


typedef struct docm_r {
  uint64_t docid;
  union {
    struct {
      unsigned int valid : 1;
    } flgs;
    uint16_t pad16;
  } u;
  uint16_t data[1];
} docM;


typedef struct docmmgr_r {
  unsigned char *(basePageP[1024]);
  unsigned int page_max_docm;                   /* max docms per page */
  unsigned int docm_size;                       /* doc meta size */
} docMMgr;




docM *getDocM( docMMgr *dmp, Mi x);

#endif /* __DOCM_INCLUDE__ */

