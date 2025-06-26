#include "defs.h"
#include "data.h"
#include "decl.h"

// AST tree fonksiyonları

// Genel bir AST dugumu olustur ve geri dondur
struct ASTnode *mkastnode(int op, struct ASTnode *left, struct ASTnode *right, int intvalue) {
  struct ASTnode *n;

  // Yeni bir AST dugumu icin malloc yap
  n = (struct ASTnode *) malloc(sizeof(struct ASTnode));
  if (n == NULL) {
    fprintf(stderr, "Unable to malloc in mkastnode()\n");
    exit(1);
  }
  // Alan degerlerini kopyala ve geri dondur
  n->op = op;
  n->left = left;
  n->right = right;
  n->intvalue = intvalue;
  return (n);
}


// Bir AST yaprak dugumu yap
struct ASTnode *mkastleaf(int op, int intvalue) {
    return (mkastnode(op, NULL, NULL, intvalue));
  }
  
// Tekli bir AST dugumu yap: sadece bir cocugu var
struct ASTnode *mkastunary(int op, struct ASTnode *left, int intvalue) {
    return (mkastnode(op, left, NULL, intvalue));
  }