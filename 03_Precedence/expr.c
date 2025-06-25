#include "defs.h"
#include "data.h"
#include "decl.h"

// Ifadelerin ayristirilmasi.

// Birincil bir etkeni ayristir ve onu temsil eden bir
// AST dugumu dondur.
static struct ASTnode *primary(void) {
  struct ASTnode *n;

  // Bir INTLIT (tam sayi degeri) simgesi icin, ona ait bir yaprak AST dugumu olustur
  // ve bir sonraki simgeyi oku. Aksi durumda, diger simge turleri icin bir sentaks hatasi uret.
  switch (Token.token) {
  case T_INTLIT:
    n = mkastleaf(A_INTLIT, Token.intvalue);
    scan(&Token);
    return (n);
  default:
    fprintf(stderr, "syntax error on line %d, token %d\n", Line, Token.token);
    exit(1);
  }
}


// Ikili bir operator simgesini bir AST islemi haline donustur.
int arithop(int tokentype) {
  switch (tokentype) {
  case T_PLUS:
    return (A_ADD); // Toplama islemi
  case T_MINUS:
    return (A_SUBTRACT); // Cikarma islemi
  case T_STAR:
    return (A_MULTIPLY); // Carpma islemi
  case T_SLASH:
    return (A_DIVIDE); // Bolme islemi
  default:
    fprintf(stderr, "syntax error on line %d, token %d\n", Line, tokentype);
    exit(1);
  }
}

// Her simge icin operator onceligi tablosu.
// 0: Gecersiz/Tanimsiz (Invalid/Undefined)
// 10: Toplama/Cikarma (Addition/Subtraction)
// 20: Carpma/Bolme (Multiplication/Division)
static int OpPrec[] = { 0, 10, 10, 20, 20, 0 };

// Bir ikili operatorumuz olup olmadigini kontrol et
// ve onceligini dondur.
static int op_precedence(int tokentype) {
  int prec = OpPrec[tokentype];
  if (prec == 0) {
    fprintf(stderr, "syntax error on line %d, token %d\n", Line, tokentype);
    exit(1);
  }
  return (prec);
}

// Koku ikili bir operator olan bir AST agaci dondur.
// ptp parametresi, onceki simgenin onceligidir.
struct ASTnode *binexpr(int ptp) { // ptp = previous token precedence
  struct ASTnode *left, *right;
  int tokentype;

  // Soldaki tam sayi degerini al.
  // Ayni anda bir sonraki simgeyi de oku.
  left = primary(); // Ilk olarak, en soldaki birincil ifadeyi ayristir.

  // Eger hic simge kalmadiysa, sadece sol dugumu dondur.
  tokentype = Token.token;
  if (tokentype == T_EOF)
    return (left); // Ifade bittiyse sol dugumu dondur.

  // Bu simgenin onceligi, onceki simge onceliginden
  // yuksek oldugu surece.
  while (op_precedence(tokentype) > ptp) {
    // Bir sonraki tam sayi degerini oku.
    scan(&Token); // Mevcut operatoru oku.

    // Alt agaci olusturmak icin simgemizin onceligi ile
    // ozyinelemeli olarak binexpr() cagir.
    right = binexpr(OpPrec[tokentype]); // Mevcut operatorun sagindaki ifadeyi,
                                        // operatorun onceligini dikkate alarak ayristir.

    // Bu alt agaci bizimkiyle birlestir. Simgeyi
    // ayni anda bir AST islemine donustur.
    left = mkastnode(arithop(tokentype), left, right, 0); // Yeni bir dugum olustur.
                                                          // Mevcut operator sol ve sag agaclari birlestirir.

    // Mevcut simgenin detaylarini guncelle.
    // Eger hic simge kalmadiysa, sadece sol dugumu dondur.
    tokentype = Token.token; // Bir sonraki simgeye bak.
    if (tokentype == T_EOF)
      return (left); // Dosya sonuna ulasildiysa bitir.
  }

  // Oncelik ayni veya daha dusuk oldugunda olusturdugumuz agaci dondur.
  return (left); // Dongu sona erdiginde olusan agaci dondur.
}