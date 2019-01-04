#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char * expr;
  int init_valid;
  /* TODO: Add more members if necessary */
} WP;

WP* new_wp();
// void free_wp(WP * wp); // can not understand why should have this
bool free_wp(int WP_NO);

void info_w();
bool watchpoint_changed();

#endif
