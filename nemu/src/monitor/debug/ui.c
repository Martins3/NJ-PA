#include "monitor/expr.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>

void cpu_exec(uint64_t);

void info_r() {
  for (int i = 0; i < 8; ++i) {
    printf("%*s%*x%*d\n", 10, reg_name(i, 4), 20, reg_l(i), 20, reg_l(i));
  }

  // show flags
  printf("%*s%*x\n", 10, "CF", 20, cpu.CF);
  printf("%*s%*x\n", 10, "OF", 20, cpu.OF);
  printf("%*s%*x\n", 10, "SF", 20, cpu.SF);
  printf("%*s%*x\n", 10, "ZF", 20, cpu.ZF);
  printf("%*s%*x\n", 10, "AF", 20, cpu.AF);
}

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

#define CMD_NOT_FOUND                                                          \
  printf("%s\n", "can't be resloved, check help command !");                   \
  return 0;
#define NEED_CORRECT_PARA                                                      \
  printf("%s\n", "bad parameters, check help command !");                      \
  return 0;

static int cmd_q(char *args) { return -1; }

static int cmd_help(char *args);

static int cmd_si(char *args) {
  int n = 1;
  if (args == NULL || (sscanf(args, "%d", &n) == 1 && n >= 1)) {
    // printf("%s", str(EMPTY));
    cpu_exec(n);
  } else {
    CMD_NOT_FOUND
  }
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    NEED_CORRECT_PARA
  }

  if (strcmp(args, "r") == 0) {
    // show registers
    info_r();
  }

  else if (strcmp(args, "w") == 0) {
    // show watch points
    info_w();
  }

  else {
    NEED_CORRECT_PARA
  }

  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    NEED_CORRECT_PARA
  } else {
    bool success = true;
    int res = expr(args, &success);
    if (!success) {
      CMD_NOT_FOUND
    } else {
      printf("%u\n", res);
    }
  }
  return 0;
}

/**
rax            0xae101d2942323d00  -5904187048441266944
rbx            0x0                 0
rcx            0x280               640
rdx            0x7fffffffdca8      140737488346280
rsi            0x7fffffffdc98      140737488346264
rdi            0x1                 1
rbp            0x7fffffffdb90      0x7fffffffdb90
rsp            0x7fffffffdab0      0x7fffffffdab0
r8             0x7ffff7c5ebe0      140737350331360
r9             0x0                 0
r10            0x5555555b1010      93824992612368
r11            0x0                 0
r12            0x55555555a0c0      93824992256192
r13            0x7fffffffdc90      140737488346256
r14            0x0                 0
r15            0x0                 0
rip            0x55555558cf4f      0x55555558cf4f <parse_options(int, char
const**)+31> eflags         0x202               [ IF ] cs             0x33 51 ss
0x2b                43 ds             0x0                 0 es             0x0 0
fs             0x0                 0
gs             0x0                 0
*/

// 现在可以先实现一个简单的版本: 规定表达式 EXPR 中只能是一个十六进制数
static int cmd_x(char *args) {
  char expr_str[200];
  bool success = true;
  int num;

  if (args == NULL) {
    NEED_CORRECT_PARA
  } else if (sscanf(args, "%d%s", &num, expr_str) != 2) {
    CMD_NOT_FOUND
  }

  int res = expr(expr_str, &success);
  if (!success || res <= 0) {
    CMD_NOT_FOUND
  }

  for (int i = 0; i < num; ++i) {
    int data = vaddr_read(res + 4 * i, 4);
    printf("%x : %x %x %x %x\n", res + 4 * i, ((data & (0xff000000)) >> 24),
           ((data & (0x00ff0000)) >> 16), ((data & (0x0000ff00)) >> 8),
           ((data & (0x000000ff)) >> 0));
  }

  return 0;
}

/**
 * (gdb) info watchpoints
 * Num     Type           Disp Enb Address    What
 * 2       hw watchpoint  keep y              learn_gdb
 */

bool valid_expr(char *e);
static int cmd_w(char *args) {
  bool success = true;
  int res = expr(args, &success);

  if (args == NULL) {
    NEED_CORRECT_PARA
  }

  else if (!success) {
    CMD_NOT_FOUND
  }

  else {
    // init a new watch point
    WP *wp = new_wp();
    wp->init_valid = res;
    wp->expr = (char *)malloc(sizeof(char) * (strlen(args) + 1));
    strcpy(wp->expr, args);
    printf("Added a new watchpoint NO : %d Expr : %s\n", wp->NO, wp->expr);
  }

  return 0;
}

static int cmd_d(char *args) {
  int WP_NO;
  if (args == NULL) {
    NEED_CORRECT_PARA
  }

  else if (sscanf(args, "%d", &WP_NO) != 1) {
    CMD_NOT_FOUND
  }

  else {
    if (!free_wp(WP_NO)) {
      printf("WP NO doesn't exist !\n");
    } else {
      printf("WP NO %d delete !\n", WP_NO);
    }
  }

  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "   Continue the execution of the program", cmd_c},
    {"q", "   Exit NEMU", cmd_q},
    {"si", "  si [N], execute next N Instruction, N is 1 by deafault", cmd_si},
    {"info",
     "info r, print all the register. info w, print all the watch point",
     cmd_info},
    {"p", "   p EXPR, print the value of expression", cmd_p},
    {"x", "   x N EXPR, print memory", cmd_x},
    {"w", "   w EXPR, when the value of EXPR changed, stop the program", cmd_w},
    {"d", "   d N, delete the Nth watchpoint", cmd_d},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}
