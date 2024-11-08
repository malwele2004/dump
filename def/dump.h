#ifndef D_DUMP_H
#define D_DUMP_H

typedef struct
{
  int in_use;
  long b_off;
  long unit_size;
  long min_bound;
  long max_bound;
} Alloc_t ;

typedef struct
{
  void *origin;
  long b_span;
  
  Alloc_t *alloc_info;
  long min_bound;
  long max_bound;
} Dump_t ;

#endif
