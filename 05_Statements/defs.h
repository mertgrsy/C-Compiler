#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Yapi ve enum tanimlari

#define TEXTLEN 512  // Tanımlayıcılar için tampon boyutu

// Token tipleri
enum {
  T_EOF, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT, T_SEMI, T_PRINT
};

// Token yapisi
struct token {
    int token;
    int intvalue;
};

// AST dugum turleri
enum {
  A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_INTLIT
};

// Soyut Sozdizimi Agaci yapisi
struct ASTnode {
  int op;                               // Bu agac uzerinde yapilacak "islem"
  struct ASTnode *left;                 // Sol ve sag cocuk agaclar
  struct ASTnode *right;
  int intvalue;                         // A_INTLIT icin tam sayi degeri
};