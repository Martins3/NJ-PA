#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <sys/types.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NEQ,
  TK_AND,
  DECIMAL,
  HEX,
  REG,
  INDIRECTION, // (dereference)
  NEG,

  /* TODO: Add more token types */

};

/**
算术表达式, 即待求值表达式中只允许出现以下的token类型:
十进制整数
+ , - , * , /
( , )
空格串
*/
static struct rule {
  char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\+", '+'},      // plus
    {"\\-", '-'},
    {"\\*", '*'},
    {"\\/", '/'},
    {"==", TK_EQ},  // equal
    {"!=", TK_NEQ}, // not equal
    {"&&", TK_AND}, // not equal
    {"=", '='},
    {"\\(", '('},
    {"\\)", ')'},
    {"0[xX][0-9a-fA-f]+", HEX},
    {"[1-9][0-9]*|0", DECIMAL},
    {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip)", REG},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

// put tokens here
// TODO what if three are more tokens than expected

static const int MAX_TOK_NUM = 100;
Token tokens[MAX_TOK_NUM];
int nr_token;

// extract the string into lines
// WARN p r
// No symbol "r" in current context
static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            // rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case HEX:
        case REG:
        case DECIMAL:
          tokens[nr_token].type = rules[i].token_type;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;
        case TK_NOTYPE:
          // doing nothing
          break;
        default:
          tokens[nr_token].type = rules[i].token_type;
          // printf("get type is ---> [%c]\n", rules[i].token_type);
          nr_token++;
          break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int expr_transform();
uint32_t expr_evaluation();

// check the validation of expr
bool valid_expr(char *e) {
  if (!make_token(e)) {
    return false;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // calculate the value

  if (expr_transform()) {
    return false;
  }
  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!valid_expr(e)) {
    *success = false;
    return 0;
  }

  return expr_evaluation();
}

// get value from int, hex and reg
uint32_t get_value(Token *t) {
  uint32_t s;
  if (t->type == DECIMAL) {
    sscanf(t->str, "%d", &s);
    return s;
  } else if (t->type == HEX) {
    sscanf(t->str, "%x", &s);
    return s;
  } else {
    if (strcmp(t->str, "$eax") == 0) {
      return cpu.eax;
    }
    if (strcmp(t->str, "$ebx") == 0) {
      return cpu.ebx;
    }
    if (strcmp(t->str, "$ecx") == 0) {
      return cpu.ecx;
    }
    if (strcmp(t->str, "$edx") == 0) {
      return cpu.edx;
    }
    if (strcmp(t->str, "$esp") == 0) {
      return cpu.esp;
    }
    if (strcmp(t->str, "$ebp") == 0) {
      return cpu.ebp;
    }
    if (strcmp(t->str, "$esi") == 0) {
      return cpu.esi;
    }
    if (strcmp(t->str, "$edi") == 0) {
      return cpu.edi;
    }
    if (strcmp(t->str, "$eip") == 0) {
      return cpu.eip;
    }
    assert(0);
  }
}

typedef struct {
  Token *arr[MAX_TOK_NUM];
  int size;
} Stack;

Stack s1, s2;

struct {
  uint32_t arr[MAX_TOK_NUM];
  int size;
} s3;

void init_stacks() {
  s1.size = 0;
  s2.size = 0;
  s3.size = 0;
}

#define push(e, b)                                                             \
  assert(e.size < MAX_TOK_NUM && e.size >= 0);                                 \
  e.arr[e.size] = b;                                                           \
  e.size++;

// TODO maybe we need some assertion
#define pop(e) e.size--

#define top(e) e.arr[e.size - 1]

static inline bool is_operator(int t) {
  return t == '+' || t == '-' || t == '*' || t == '/' || t == TK_EQ ||
         t == TK_AND || t == TK_NEQ || t == NEG || t == INDIRECTION;
}

static inline bool is_num(int t) {
  return t == DECIMAL || t == REG || t == HEX;
}

static int get_op_pri(int t) {
  switch (t) {
  case '+':
    return 1;
  case '-':
    return 1;
  case '*':
    return 2;
  case '/':
    return 2;
  case INDIRECTION:
    return 3;
  case NEG:
    return 3;
  case TK_EQ:
    return 0;
  case TK_NEQ:
    return 0;
  case TK_AND:
    return 0;
  default:
    printf("[%c]\n", t);
    return 0;
    assert(0);
  }
}

#define DEBUG_EXPR

#ifdef DEBUG_EXPR

static void print_token_type(int token_type) {

  if (is_num(token_type)) {
    printf(" num ");
  }

  else if (token_type == TK_EQ) {
    printf(" %s ", "==");
  }

  else {
    putchar(' ');
    if (token_type == NEG) {
      printf("NEG");
    }

    else if (token_type == INDIRECTION) {
      printf("IND");
    }

    else {
      putchar(token_type);
    }

    putchar(' ');
  }
}

static void print_token(Token *token) {

  if (is_num(token->type)) {
    printf(" %s ", token->str);
  }

  else if (token->type == TK_EQ) {
    printf(" %s ", "==");
  }

  else {
    putchar(' ');
    if (token->type == NEG) {
      printf("NEG");
    }

    else if (token->type == INDIRECTION) {
      printf("IND");
    }

    else {
      putchar(token->type);
    }

    putchar(' ');
  }
}
#endif

static inline bool op_not_less(int op_a, int op_b) {
  // calculate the expression from right to left
  printf("compare --- ");
  print_token_type(op_a);
  print_token_type(op_b);
  printf(" --- end\n");
  return get_op_pri(op_a) < get_op_pri(op_b);
}

// change if the expression is valid
// neg and * to indirection
bool check_tag() {
  int left_p = 0;
  int last_t = '+'; // init last token type

  for (int i = 0; i < nr_token; ++i) {
    Token *token = &(tokens[i]);
    int t = token->type;

    if (is_num(t)) {
      if (is_num(last_t) || last_t == ')') {
        return false;
      }
    }

    else if (is_operator(t)) {
      if (is_operator(last_t) || last_t == '(') {
        if (t == '*') {
          token->type = INDIRECTION;
        } else if (t == '-') {
          token->type = NEG;
        } else {
          return false;
        }
      }
    }

    else if (t == '(') {
      if (last_t == ')' || is_num(last_t)) {
        return false;
      }
      left_p++;
    }

    else if (t == ')') {
      if (last_t == '(' || is_operator(last_t)) {
        return false;
      }

      left_p--;
    }

    else {
      assert(0);
    }
    last_t = t;
  }

  return left_p == 0;
}

// https://en.wikipedia.org/wiki/Reverse_Polish_notation
// https://en.wikipedia.org/wiki/Shunting-yard_algorithm
int expr_transform() {
  init_stacks();
  if (!check_tag()) {
    printf("Invalid Expression\n");
    return 1;
  }
  for (int i = 0; i < nr_token; ++i) {
    int t = tokens[i].type;
    if (is_num(t)) {
      push(s1, &(tokens[i]))
    }

    else if (t == '(') {
      push(s2, &(tokens[i]))
    }

    else if (t == ')') {
      while (s2.size && top(s2)->type != '(') {
        Token *token = top(s2);
        pop(s2);
        if (is_operator(token->type)) {
          push(s1, token);
        } else {
          assert(0);
        }
      }

      // parenthesis shoulde match
      if (!s2.size) {
        assert(0);
      } else {
        pop(s2);
      }
    }

    else if (is_operator(t)) {
      while (s2.size != 0 && top(s2)->type != '(' &&
             (op_not_less(tokens[i].type, top(s2)->type))) {

        Token *t = top(s2);
        pop(s2);
        push(s1, t)
      }

      push(s2, &(tokens[i]))
    }
  }

  while (s2.size) {
    Token *t = top(s2);
    pop(s2);
    assert(is_operator(t->type));
    push(s1, t);
  }

  // for debug
#ifdef DEBUG_EXPR
  for (int i = 0; i < s1.size; ++i) {
    print_token(s1.arr[i]);
  }
  printf("\n");
#endif
  return 0;
}

#define OPERATION(op)                                                          \
  s3.arr[s3.size - 2] = s3.arr[s3.size - 2] op s3.arr[s3.size - 1];
uint32_t expr_evaluation() {
  for (int i = 0; i < s1.size; i++) {
    int t = s1.arr[i]->type;
    if (is_num(t)) {
      push(s3, get_value(s1.arr[i]))
    }

    else if (is_operator(t)) {
      switch (t) {
      case '+':
        OPERATION(+)
        pop(s3);
        break;
      case '-':
        OPERATION(-)
        pop(s3);
        break;
      case '*':
        OPERATION(*)
        pop(s3);
        break;
      case '/':
        // TODO shoulde we check the divide zero error
        OPERATION(/)
        pop(s3);
        break;
      case NEG:
        s3.arr[s3.size - 1] = -s3.arr[s3.size - 1];
        break;
      case INDIRECTION:
        // uint32_t vaddr_read(vaddr_t, int);
        s3.arr[s3.size - 1] = vaddr_read(s3.arr[s3.size - 1], 4);
        break;
      case TK_EQ:
        OPERATION(==)
        pop(s3);
        break;
      case TK_NEQ:
        OPERATION(!=)
        pop(s3);
        break;
      case TK_AND:
        OPERATION(&&)
        pop(s3);
        break;
      default:
        assert(0);
      }
    }

    else {
      assert(0);
    }
  }
  return s3.arr[0];
}
// (1 + (3 * 2) + (($eax - $eax) + (*$eip - 1 **$eip) + (0x5 -- 5 + *0x1000005 -
// *0x1000005)) * 4)
