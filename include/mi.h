#ifndef __MI_INCLUDE__
#define __MI_INCLUDE__
#include <stdint.h>
#include <time.h>


typedef uint64_t docid_t;


typedef struct mi_r {
  union {
    struct xbit {
      unsigned int valid : 1;
      unsigned int dmeta : 1;       /* doc meta information stored */
      unsigned int obsel : 1;
    } bflgs;
    uint16_t pad16;
  } u;
  uint16_t jpgb;
  uint32_t ioff;
  docid_t  docid;
  time_t   lastmod;
  uint32_t hifreqcnt;              // count of most frequent token in doc
} Mi;




#endif /* __MI_INCLUDE__ */
