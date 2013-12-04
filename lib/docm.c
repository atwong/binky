#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mi.h"
#include "docm.h"


docM *getDocM( docMMgr *dmp, Mi mi )
{
  assert((dmp->basePageP[mi.jpgb] != NULL));
  assert((mi.ioff < dmp->page_max_docm));
  docM *d = (docM*)(dmp->basePageP[mi.jpgb] + (sizeof(unsigned char)*dmp->docm_size*mi.ioff));
}
