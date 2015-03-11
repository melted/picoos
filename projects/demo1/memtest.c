/*
 *  Copyright (c) 2006, Dennis Kuschel.
 *  All rights reserved. 
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission. 
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 *  INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *  OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <picoos.h>


/* SETUP NANO ENVIRONMENT */
#define HEAPSIZE 0x20000
static char membuf_g[HEAPSIZE];
void *__heap_start  = (void*) &membuf_g[0];
void *__heap_end    = (void*) &membuf_g[HEAPSIZE-1];


/* check configuration */
#if NOSCFG_MEM_MANAGE_MODE != 1
#error  NOSCFG_MEM_MANAGE_MODE must be set to 1 for this test program !
#endif


/*=========================================================================*/


static void initMem(void *mptr, unsigned int size, UVAR_t startval)
{
  UVAR_t *b = (UVAR_t*) mptr;
  unsigned int i;
  for (i=0;i<(size/sizeof(UVAR_t));i++)
  {
    b[i] = startval;
    startval++;
  }
}

static int checkMem(void *mptr, unsigned int size, UVAR_t startval)
{
  UVAR_t *b = (UVAR_t*) mptr;
  unsigned int i;
  for (i=0;i<(size/sizeof(UVAR_t));i++)
  {
    if (b[i] != startval)
      return -1;
    startval++;
  }
  return 0;
}


/*-------------------------------------------------------------------------*/


unsigned long rand_g = 0x1234abcd;

unsigned int getRandBit()
{
  unsigned int b;
  b = rand_g & 1;
  rand_g >>= 1;
  rand_g |= (b ^ 1 ^ (rand_g >> 4) ^ (rand_g >> 11) ^ (rand_g >> 27)) << 31;
  return b;
}

unsigned int getRand(int bits)
{
  unsigned int r = 0;
  int i;
  for (i=0; i<bits; i++)
  {
    r <<= 1;
    r |= getRandBit();
  }
  return r;
}


/*-------------------------------------------------------------------------*/


typedef struct MINFO_s {
  struct MINFO_s  *next;
  void            *memptr;
  unsigned int    size;
  UVAR_t          ival;
} *MINFO_t;


MINFO_t  freelist_g;
MINFO_t  alloclist_g;
unsigned int  allocated_bytes_g = 0;
unsigned int  allocated_blocks_g = 0;
unsigned int  alloc_ok_g   = 0;
unsigned int  realloc_ok_g = 0;
unsigned int  errors_g     = 0;
unsigned int  maxMemAllocated_g = 0;

#define UVAR_ALIGN(x)  (((x)+(sizeof(UVAR_t)-1)) & ~(sizeof(UVAR_t)-1))

#define MAXLISTLEN    1000
struct MINFO_s  listmem_g[MAXLISTLEN];


void initMemList()
{
  int i;

  alloclist_g = NULL;
  freelist_g = listmem_g;

  for (i=0; i<(MAXLISTLEN-1); i++)
  {
    listmem_g[i].next = &listmem_g[i+1];
  }
  listmem_g[i].next = NULL;
}


/*-------------------------------------------------------------------------*/


int do_realloc()
{
  unsigned int r, nsize, csize;
  MINFO_t m, l;
  void *nptr;

  if (alloclist_g == NULL)
    return 0;

  r = getRand(11) % allocated_blocks_g;

  l = NULL;
  m = alloclist_g;
  while (r > 1) {
    r--;
    l = m;
    m = m->next;
    if (m == NULL)
    {
      nosPrint("error!\n");
      for(;;);
    }
  }

  if (checkMem(m->memptr, m->size, m->ival) != 0)
  {
    nosPrint("\ndo_realloc(1): check error!\n");
    errors_g++;
  }

  nsize = UVAR_ALIGN((m->size / 2) + getRand(11));

  nptr = nosMemRealloc(m->memptr, nsize);
  if (nptr == NULL)
    return -1;

  realloc_ok_g++;
  allocated_bytes_g += nsize - m->size;

  csize = m->size;
  if (csize > nsize)
    csize = nsize;

  if (checkMem(nptr, csize, m->ival) != 0)
  {
    nosPrint("\ndo_realloc(2): check error!\n");
    errors_g++;
  }

  m->size = nsize;
  m->memptr = nptr;
  m->ival = (UVAR_t) getRand(8);
  initMem(m->memptr, m->size, m->ival);

  return 1;
}


int do_malloc()
{
  unsigned int size;
  MINFO_t m;
  void *p;

  if (freelist_g == NULL)
    return -2;

  size = UVAR_ALIGN(getRand(11) + 5);

  p = nosMemAlloc(size);
  if (p == NULL)
    return -1;

  alloc_ok_g++;

  m = freelist_g;
  freelist_g = m->next;

  m->memptr = p;
  m->size = size;
  m->ival = (UVAR_t) getRand(8);

  initMem(p, m->size, m->ival);

  m->next = alloclist_g;
  alloclist_g = m;

  allocated_bytes_g += size;
  allocated_blocks_g += 1;
  return 0;
}


int do_free()
{
  unsigned int r;
  MINFO_t m, l;

  if (alloclist_g == NULL)
    return -1;

  r = getRand(11) % allocated_blocks_g;

  l = NULL;
  m = alloclist_g;
  while (r > 1) {
    r--;
    l = m;
    m = m->next;
    if (m == NULL)
    {
      nosPrint("do_free: list error!\n");
      errors_g++;
      return -1;
    }
  }

  if (l == NULL)
  {
    alloclist_g = m->next;
  }
  else
  {
    l->next = m->next;
  }

  if (checkMem(m->memptr, m->size, m->ival) != 0)
  {
    nosPrint("\ndo_free: check error!\n");
    errors_g++;
  }

  nosMemFree(m->memptr);

  allocated_bytes_g -= m->size;
  allocated_blocks_g -= 1;

  m->next = freelist_g;
  freelist_g = m;

  return 0;
}


/*-------------------------------------------------------------------------*/


unsigned int eventcntr_g = 0;

void run_test()
{
  unsigned int thres, r, al;
  unsigned int maxblocks, maxbytes, mby, mbl;
  unsigned int reallocOk, reallocErr;
  void *m;
  int rc;

  initMemList();

  thres = 2;
  maxblocks = 0;
  maxbytes = 0;
  mbl = 0;
  mby = 0;
  reallocOk = 0;
  reallocErr = 0;

  for(;;)
  {
    r = getRand(3);

    if (r > thres)
    {
      if (do_malloc() != 0)
      {
        thres = 4;
      }
      if (allocated_bytes_g > maxbytes)
        maxbytes = allocated_bytes_g;
      if (allocated_blocks_g > maxblocks)
        maxblocks = allocated_blocks_g;

      al = allocated_bytes_g + (allocated_blocks_g * 2*sizeof(int));
      if (al > maxMemAllocated_g)
        maxMemAllocated_g = al;
    }
    else
    {
      if (getRand(2) == 3)
      {
        rc = do_realloc();
        if (rc < 0) reallocErr++;
        if (rc > 0) reallocOk++;
      }

      if (do_free() != 0)
      {
        thres = 2;
        if (maxblocks > (mbl/4))
        {
          mby = (mby*3 + maxbytes) / 4;
          mbl = (mbl*3 + maxblocks) / 4;
        }
        maxbytes = 0;
        maxblocks = 0;

        m = nosMemAlloc(maxMemAllocated_g);
        if (m != NULL)
        {
          nosMemFree(m);
        }
        else
        {
          errors_g++;
        }
      }
    }

    nosPrintf5("bytes:%06i  max:%06i  alloc.ok:%i  realloc.ok:%i  errors:%i\r",
               allocated_bytes_g, maxMemAllocated_g,
               alloc_ok_g, realloc_ok_g, errors_g);

/*
    nosPrintf6("by: %06i  bl: %04i - max.by: %06i  max.bl: %04i  "
               "rok/rer: %04i/%04i \r", 
      allocated_bytes_g, allocated_blocks_g, mby, mbl, reallocOk, reallocErr);
*/

    eventcntr_g++;
  }
}


/*=========================================================================*/


static void task1(void *arg)
{
  nosPrint("Memory Manager Test Program:  Tests nosMemAlloc/nosMemRealloc/nosMemFree\n");

  run_test();
}


int main(void)
{
  nosInit(task1, NULL, 1, 0, 0);
  return 0;
}
