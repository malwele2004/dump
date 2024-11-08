#include "../def/dump.h"

#ifndef I_DUMP_H
#define I_DUMP_H

int
Dump_Ok(Dump_t*); 

Dump_t
Dump_New();

int
Dump_Delete(Dump_t*);

int
Dump_IsIDAllocated(Dump_t*);

int
Dump_IsIDUse(Dump_t*, long id);

int
_Dump_Allocate(Dump_t*, long word_size, long n, long *retID);
#define Dump_Allocate(Type, dump, n, retID) _Dump_Allocate(dump, sizeof(Type), n, retID)

int
Dump_ReleaseAllocation(Dump_t*, long id);

int
_Dump_SpawnAllocation(Dump_t*, long word_size, long n, long *retID);
#define Dump_SpawnAllocation(Type, dump, n, retID) _Dump_SpawnAllocation(dump, sizeof(Type), n, retID)

const Alloc_t*
Dump_StatAllocation(Dump_t*, long id);

void*
Dump_PeekAllocation(Dump_t*, long id, long start, long n, long *retBytes);

int
Dump_ReadAllocation(Dump_t*, long id ,long start, long n, void *dest); 

int
Dump_WriteAllocation(Dump_t*, long id, long start, long n, void *src);

int
Dump_PopAllocation(Dump_t*, long id, void *dest);

int
Dump_PushAllocation(Dump_t*, long *id, void *src);
#endif 
