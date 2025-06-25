#include "defs.h"
#include "data.h"
#include "decl.h"

// AST agaci yorumlayici

// AST operatorlerinin listesi
static char *ASTop[] = { "+", "-", "*", "/" };

// Verilen bir AST'yi yorumla
// icindeki operatorleri ve geri dondur
// bir nihai deger.
int interpretAST(struct ASTnode *n) {
  int leftval, rightval;

  // Sol ve sag alt-agac degerlerini al
  if (n->left)
    leftval = interpretAST(n->left);
  if (n->right)
    rightval = interpretAST(n->right);

  // Debug: Neler yapacagimizi yazdir
  if (n->op == A_INTLIT)
    printf("int %d\n", n->intvalue);
  else
    printf("%d %s %d\n", leftval, ASTop[n->op], rightval);

  switch (n->op) {
    case A_ADD:
      return (leftval + rightval);
    case A_SUBTRACT:
      return (leftval - rightval);
    case A_MULTIPLY:
      return (leftval * rightval);
    case A_DIVIDE:
      return (leftval / rightval);
    case A_INTLIT:
      return (n->intvalue);
    default:
      fprintf(stderr, "Bilinmeyen AST operatoru %d\n", n->op);
      exit(1);
  }
}