#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Yapi ve enum tanimlari

// Token yapisi
struct token {
    int token;
    int intvalue;
  };

// Tokenlar
enum {
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT
  };
