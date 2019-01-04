#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

static int WP_NO = 1;

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

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  if(free_ == NULL) assert(0);
  WP * new_free = free_->next;
  free_->next = head;
  head = free_;
  free_ = new_free;
  head->NO = WP_NO ++;
  return head;
}

bool free_wp(int WP_NO){
  if(head == NULL) assert(0);
  if(head->NO == WP_NO){
    WP * new_head = head->next;
    head->next = free_;
    free_ = head;
    head = new_head;
    return true;
  }

  WP * p = head;
  while(p->next != NULL){
    if(p->next->NO == WP_NO){
      WP * to_be_free = p->next;
      p->next = p->next->next;
      to_be_free->next = free_;
      free_ =  to_be_free;
      return true;
    }
    
    p = p->next;
  }

  // we can not find the specific WP_NO in this line
  return false;
}


uint32_t expr(char *e, bool *success);
void info_w(){
  WP * p = head;
  // be careful about the NULL
  while(p != NULL){
    bool suc = true;
    int res = expr(p->expr, &suc);
    if(!suc) assert(0);
    printf("NO : %d EXPR : %s NEW : %d\n",p->NO, p->expr, res);
    p = p->next;
  }
}


bool watchpoint_changed(){
  WP * p = head;
  // be careful about the NULL
  bool changed = false;
  while(p != NULL){
    bool suc = true;
    // printf("DDD --> NO : %d EXPR : [%s]\n",p->NO, p->expr);
    int res = expr(p->expr, &suc);
    if(!suc) assert(0);
    if(p->init_valid != res){
      changed = true;
      printf("NO : %d EXPR : %s NEW : %d OLD : %d\n",p->NO, p->expr, res, p->init_valid);
      p->init_valid = res;
    }
    p = p->next;
  }
  return changed;
}
