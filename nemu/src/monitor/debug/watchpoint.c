#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}
bool delete_watchpoint(int no) {
  WP *p = head;
  WP *prev = NULL;
  while (p != NULL) {
    if (p->NO == no) {
      if (prev == NULL) {
        head = p->next;
      } else {
        prev->next = p->next;
      }
      p->next = free_;
      free_ = p;
      return true;
    }
    prev = p;
    p = p->next;
  }
  return false;
}
void print_watchpoints() {
  WP *p = head;
  if (p == NULL) {
    printf("No watchpoints.\n");
    return;
  }
  while (p != NULL) {
    printf("Watchpoint %d: %s = %u (0x%x)\n",
           p->NO, p->expr, p->old_val, p->old_val);
    p = p->next;
  }
}
/* TODO: Implement the functionality of watchpoint */


