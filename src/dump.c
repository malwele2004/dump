#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../def/dump.h"

/***
 * dump
 * ----
 * is an allocater for applications improving dynamic allocations
 * by making them immutable sections that cannot dynamically increase
 * and reusing unused spots. 
 * spots can be reused and new ones can be created
 **/

#define INIT_SIZE 64

int
Dump_Ok(Dump_t *dump)
{
  return dump->alloc_info != NULL;
}

Dump_t
Dump_New()
{
  return (Dump_t){
	.origin = NULL,
	.b_span = 0,
	.alloc_info = calloc(INIT_SIZE, sizeof(Alloc_t)),
	.min_bound = 0,
	.max_bound = INIT_SIZE};
}

int
Dump_Delete(Dump_t *dump)
{
  if (Dump_Ok(dump))
	{
	  if (dump->origin)
		{
		  free(dump->origin);
		  dump->origin = NULL;
		}
	  free(dump->alloc_info);
	  return 1;
	}
  return 0;
}

int
Dump_IsIDAllocated(Dump_t *dump, long id)
{
  return Dump_Ok(dump) && id >= 0 && id < dump->min_bound;
}

int
Dump_IsIDInUse(Dump_t *dump, long id)
{
  return Dump_IsIDAllocated(dump, id) && dump->alloc_info[id].in_use;
}

int
_Dump_Allocate(Dump_t *dump, long word_size, long n, long *retID)
{
  if (Dump_Ok(dump) && word_size > 0 && n > 0)
	{
	  // Info
	  if (dump->min_bound == dump->max_bound)
		{
		  long new_max_bound = dump->max_bound * 2.0;
		  void *new_alloc_info = reallocarray(dump->alloc_info, new_max_bound, sizeof(Alloc_t));
		  if (!new_alloc_info) return 0;
		  dump->alloc_info = new_alloc_info;
		  dump->max_bound = new_max_bound;
		}

	  *retID = dump->min_bound;
	  
	  // Allocation
	  long b_alloc = word_size * n;
	  long new_b_span = dump->b_span + b_alloc;
	  void *new_origin = realloc(dump->origin, new_b_span);
	  Alloc_t *alloc = &dump->alloc_info[*retID];
	  
	  if (new_origin)
		{
		  alloc->b_off = dump->b_span;
		  alloc->min_bound = 0;
		  alloc->max_bound = n;
		  alloc->unit_size = word_size;
		  alloc->in_use = 1;
			
		  dump->origin = new_origin;
		  dump->b_span = new_b_span;

		  dump->min_bound++; // move to next info
		  return 1;
		}
	}
  return 0;
}

int
Dump_ReleaseAllocation(Dump_t *dump, long id)
{
  if (Dump_IsIDInUse(dump, id))
	{
	  dump->alloc_info[id].in_use= 0;
	  return 1;
	}
  return 0;
}

int
_Dump_SpawnAllocation(Dump_t *dump, long word_size, long n, long *retID)
{
  if (word_size <= 0 && n <= 0) return 0;
  if (Dump_Ok(dump))
	{
	  for (long i = 0; i < dump->min_bound ; i++)
		{
		  Alloc_t *alloc = &dump->alloc_info[i];
		  long b_goal = alloc->unit_size * alloc->max_bound;
		  long b_req = word_size * n;
		  if (b_goal >= b_req/***&& (b_goal % b_req) == 0 [ALLOW INTERNAL FRAGMENTATION]***/)
			{
			  *retID = i;
			  alloc->in_use = 1;
			  alloc->min_bound = 0;
			  alloc->unit_size = word_size;
			  alloc->max_bound = b_goal / word_size;
			  return 1;
			}
		}
	}
  return _Dump_Allocate(dump, word_size, n, retID);
}

const Alloc_t*
Dump_StatAllocation(Dump_t *dump, long id)
{
  if (Dump_IsIDAllocated(dump, id)) return &dump->alloc_info[id];
  return NULL;
}

void*
Dump_PeekAllocation(Dump_t *dump, long id, long start, long n, long *retBytes)
{
  Alloc_t *alloc = (Alloc_t*)Dump_StatAllocation(dump, id); 
  if (alloc && start >= 0 && n >=1 && (start + n) <= alloc->min_bound)
	{
	  if (retBytes) *retBytes = alloc->unit_size * n;
	  return dump->origin + alloc->b_off + (alloc->unit_size * start);
	}
  return NULL;
}

int
Dump_IOAllocation(Dump_t *dump, long id, long start, long n, void *res, int isread)
{
  long n_bytes = 0;
  void *dump_origin_res = Dump_PeekAllocation(dump, id, start, n, &n_bytes);
  if (dump_origin_res)
	{
	  void *dest;
	  void *src;
	  if (isread)
		{
		  dest = res;
		  src = dump_origin_res;
		}
	  else
		{
		  dest = dump_origin_res;
		  src = res;
		}
	  memcpy(dest, src, n_bytes);
	  return 1;
	}
  return 0;
}

int
Dump_ReadAllocation(Dump_t *dump, long id, long start, long n, void *dest)
{
  return Dump_IOAllocation(dump, id, start, n, dest, 1);
}

int
Dump_WriteAllocation(Dump_t *dump, long id, long start, long n, void *src)
{
  return Dump_IOAllocation(dump, id, start, n, src, 0);
}

int
Dump_PopAllocation(Dump_t *dump, long id, void *dest)
{
  Alloc_t *p = (Alloc_t*)Dump_StatAllocation(dump, id);
  if (p && p->min_bound >= 1)
	{
	  Dump_ReadAllocation(dump, id, p->min_bound - 1, 1, dest);
	  p->min_bound--;
	  return 1;
	}
  return 0;
}

int
Dump_PushAllocation(Dump_t *dump, long *id, void *src)
{
  Alloc_t *p = (Alloc_t*)Dump_StatAllocation(dump, *id);
  if (p)
	{
	  long old_id = *id;
	  if (p->min_bound == p->max_bound)
		{
		  if (_Dump_SpawnAllocation(dump, p->unit_size, p->max_bound * 2.0, id))
			{
			  Alloc_t *new_p = (Alloc_t*)Dump_StatAllocation(dump, *id);
			  new_p->min_bound = p->min_bound;
			  Dump_WriteAllocation(dump,
								   *id,
								   0,
								   p->min_bound,
								   Dump_PeekAllocation(dump,
													   old_id,
													   0,
													   p->min_bound,
													   NULL));
			  p->in_use = 0; // not in use
			}
		  else return 0;
		}
	  p->min_bound++; // add
	  Dump_WriteAllocation(dump, *id, p->min_bound - 1, 1, src);
	  return 1;
	}
  return 0;
}



