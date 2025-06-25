#include "defs.h"
#include "data.h"
#include "decl.h"

// Birincil faktoru ayristir ve onu temsil eden bir AST dugumu dondur.

static struct ASTnode *primary(void) {
  struct ASTnode *n;

  // INTLIT tokeni icin bir yaprak AST dugumu yap
  // ve sonraki tokeni tara. Aksi halde,
  // diger token turleri icin sozdizimi hatasi ver.
  switch (Token.token) {
    case T_INTLIT:
      n = mkastleaf(A_INTLIT, Token.intvalue);
      scan(&Token);
      return (n);
    default:
      fprintf(stderr, "sozdizimi hatasi, satir %d\n", Line);
      exit(1);
  }
}

// Bir tokeni AST islemine donustur.
int arithop(int tok) {
  switch (tok) {
    case T_PLUS:
      return (A_ADD);
    case T_MINUS:
      return (A_SUBTRACT);
    case T_STAR:
      return (A_MULTIPLY);
    case T_SLASH:
      return (A_DIVIDE);
    default:
    
      fprintf(stderr, "bilinmeyen token arithop() fonksiyonunda, satir %d token: %c, ASCII: %d \n",
              Line, tok, tok);
      exit(1);
  }
}

// Koku bir ikili operator olan bir AST agaci dondur
struct ASTnode *binexpr(void) {
  struct ASTnode *n, *left, *right;
  int nodetype;

  // Soldaki tam sayi literali al.
  // Ayni anda sonraki tokeni de al.
  left = primary();

  // Eger token kalmadiysa, sadece sol dugumu dondur
  if (Token.token == T_EOF) return (left);

  // Tokeni bir dugum turune donustur
  nodetype = arithop(Token.token);

  // Sonraki tokeni al
  scan(&Token);

  // Sagdaki agaci ozyinelemeli olarak al
  right = binexpr();

  // Simdi iki alt agacla bir agac olustur
  n = mkastnode(nodetype, left, right, 0);
  return (n);
}