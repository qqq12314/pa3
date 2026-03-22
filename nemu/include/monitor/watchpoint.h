#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
typedef struct watchpoint {

  int NO;

  struct watchpoint *next;

  char expr[256];

  uint32_t old_val;

} WP;
WP* new_wp(void);
void free_wp(WP *wp);
void print_watchpoints(void);
bool delete_watchpoint(int no);
#endif
