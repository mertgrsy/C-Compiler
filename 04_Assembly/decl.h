// Tum derleyici dosyalari icin fonksiyon prototipleri
int scan(struct token *t);

struct ASTnode *mkastnode(int op, struct ASTnode *left,
			  struct ASTnode *right, int intvalue);
struct ASTnode *mkastleaf(int op, int intvalue);
struct ASTnode *mkastunary(int op, struct ASTnode *left, int intvalue);
struct ASTnode *binexpr(int rbp);

int interpretAST(struct ASTnode *n);
void generatecode(struct ASTnode *n);

void freeall_registers(void);
void cgpreamble();
void cgpostamble();
int cgload(int value);
int cgadd(int reg1, int reg2);
int cgsub(int reg1, int reg2);
int cgmul(int reg1, int reg2);
int cgdiv(int reg1, int reg2);
void cgprintint(int reg);