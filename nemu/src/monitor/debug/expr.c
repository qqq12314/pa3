#include "nemu.h"
#include <stdlib.h>
#include <string.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_HEX,
  TK_REG,
  TK_NEQ,
  TK_AND

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"-", '-'},
  {"\\*", '*'},       // multiply
  {"/", '/'},         // divide
  {"\\(", '('},       // left parenthesis
  {"\\)", ')'},       // right parenthesis
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_NUM}, // decimal number
  {"\\$[a-zA-Z][a-zA-Z0-9]*", TK_REG},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
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

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        /*Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);*/
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
         case TK_NOTYPE:
           break;
         case TK_NUM:
         case TK_HEX:
         case TK_REG:
           tokens[nr_token].type = rules[i].token_type;
           strncpy(tokens[nr_token].str, substr_start, substr_len);
           tokens[nr_token].str[substr_len] = '\0';
  	   nr_token++;
  	 break;
        default:
           tokens[nr_token].type = rules[i].token_type;

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
static bool check_parentheses(int p, int q) {

  if (tokens[p].type != '(' || tokens[q].type != ')') {

    return false;

  }



  int balance = 0;

  int i;

  for (i = p; i <= q; i++) {

    if (tokens[i].type == '(') balance++;

    if (tokens[i].type == ')') balance--;



    if (balance == 0 && i < q) {

      return false;

    }

  }



  return balance == 0;

}
static int dominant_operator(int p, int q) {

  int op = -1;

  int min_pri = 100;

  int level = 0;

  int i;



  for (i = p; i <= q; i++) {

    int type = tokens[i].type;



    if (type == '(') {

      level++;

      continue;

    }

    if (type == ')') {

      level--;

      continue;

    }



    if (level > 0) continue;



    int pri = -1;

    if (type == '+' || type == '-') pri = 1;

    else if (type == '*' || type == '/') pri = 2;

    else continue;

    if (pri <= min_pri) {

      min_pri = pri;

      op = i;

    }

  }



  return op;

}
static uint32_t eval(int p, int q, bool *success) {

  if (p > q) {

    *success = false;

    return 0;

  }



  if (p == q) {

    if (tokens[p].type == TK_NUM) {

      *success = true;

      return strtoul(tokens[p].str, NULL, 10);

    }

    else if (tokens[p].type == TK_HEX) {

      *success = true;

      return strtoul(tokens[p].str, NULL, 16);

    }

    else {

      *success = false;

      return 0;

    }

  }



  if (check_parentheses(p, q)) {

    return eval(p + 1, q - 1, success);

  }



  int op = dominant_operator(p, q);

  if (op == -1) {

    *success = false;

    return 0;

  }



  uint32_t val1 = eval(p, op - 1, success);

  if (!*success) return 0;



  uint32_t val2 = eval(op + 1, q, success);

  if (!*success) return 0;



  switch (tokens[op].type) {

    case '+': return val1 + val2;

    case '-': return val1 - val2;

    case '*': return val1 * val2;

    case '/':

      if (val2 == 0) {

        *success = false;

        return 0;

      }

      return val1 / val2;
default:
      *success = false;
      return 0;
  }
}

uint32_t expr(char *e, bool *success) {

  if (!make_token(e)) {

    *success = false;

    return 0;

  }



  if (nr_token == 0) {

    *success = false;

    return 0;

  }



  return eval(0, nr_token - 1, success);

}
 
