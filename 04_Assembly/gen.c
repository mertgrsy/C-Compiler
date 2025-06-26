#include "defs.h"
#include "data.h"
#include "decl.h"

// Genel kod uretimi

// Bir AST (Soyut Sozdizimi Agaci) verildiginde,
// ozyinelemeli olarak assembly kodu uret.

static int genAST(struct ASTnode *n) {
    int leftreg, rightreg;

    // Sol ve sag alt-agac degerlerini al.
    if(n->left)
        leftreg = genAST(n->left);
    if(n->right)
        rightreg = genAST(n->right);

    // islemi gerceklestir
    switch (n->op) {
    case A_ADD:
        return cgadd(leftreg, rightreg);
    case A_SUBTRACT:
        return cgsub(leftreg, rightreg);
    case A_MULTIPLY:
        return cgmul(leftreg, rightreg);
    case A_DIVIDE:
        return cgdiv(leftreg, rightreg);
    case A_INTLIT:
        return cgload(n->intvalue); // Tam sayi degerini yukle
    default:
        fprintf(stderr, "Unknown AST node type: %d\n", n->op);
        exit(1);
    }
}

void generatecode(struct ASTnode *n) {
    int reg;

    // Kod uretimine basla
    cgpreamble();

    // AST'yi isleyerek assembly kodu uret
    reg = genAST(n);

    // Sonucu yazdir
    cgprintint(reg);

    // Kod uretimini bitir
    cgpostamble();
}