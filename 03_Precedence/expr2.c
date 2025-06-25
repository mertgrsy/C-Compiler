#include "defs.h"
#include "data.h"
#include "decl.h"

// Ifadelerin tam ozyinelemeli inis ile ayristirilmasi.

// Birincil bir etkeni ayristir ve onu temsil eden bir
// AST dugumu dondur.
static struct ASTnode *primary(void) {
  struct ASTnode *n;

  // Bir INTLIT (tam sayi degeri) simgesi icin, ona ait bir yaprak AST dugumu olustur
  // ve bir sonraki simgeyi oku. Aksi durumda, diger simge turleri icin bir sentaks hatasi
  // uret.
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
static int arithop(int tok) {
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
    fprintf(stderr, "syntax error on line %d, token %d\n", Line, tok);
    exit(1);
  }
}

struct ASTnode *additive_expr(void);

// Kok dugumu '*' veya '/' ikili operator olan bir AST agaci dondur.
struct ASTnode *multiplicative_expr(void) {
  struct ASTnode *left, *right;
  int tokentype;

  // Sol taraftaki tam sayi degerini al.
  // Ayni anda bir sonraki simgeyi de oku.
  left = primary();

  // Eger hic simge kalmadiysa, sadece sol dugumu dondur.
  tokentype = Token.token;
  if (tokentype == T_EOF)
    return (left);

  // Simge '*' veya '/' oldugu surece donguye devam et.
  while ((tokentype == T_STAR) || (tokentype == T_SLASH)) {
    // Bir sonraki tam sayi degerini oku.
    scan(&Token);
    right = primary();

    // Bunu sol taraftaki tam sayi degeri ile birlestir.
    left = mkastnode(arithop(tokentype), left, right, 0);

    // Mevcut simgenin detaylarini guncelle.
    // Eger hic simge kalmadiysa, sadece sol dugumu dondur.
    tokentype = Token.token;
    if (tokentype == T_EOF)
      break;
  }

  // Olusturdugumuz agaci dondur.
  return (left);
}

// Kok dugumu '+' veya '-' ikili operator olan bir AST agaci dondur.
struct ASTnode *additive_expr(void) {
  struct ASTnode *left, *right;
  int tokentype;

  // Bizden daha yuksek oncelige sahip sol alt agaci al.
  left = multiplicative_expr();

  // Eger hic simge kalmadiysa, sadece sol dugumu dondur.
  tokentype = Token.token;
  if (tokentype == T_EOF)
    return (left);

  // '+' veya '-' simge turunu onbellege al.

  // Kendi oncelik seviyemizdeki simge uzerinde calisan dongu.
  while (1) {
    tokentype = Token.token;
    if (!((tokentype == T_PLUS) || (tokentype == T_MINUS)))
        break;
    
    // Bir sonraki tam sayi degerini oku.
    scan(&Token);

    // Bizden daha yuksek oncelige sahip sag alt agaci al.
    right = multiplicative_expr();

    // Iki alt agaci kendi dusuk oncelikli operatorumuzle birlestir.
    left = mkastnode(arithop(tokentype), left, right, 0);

    // Ve kendi oncelik seviyemizdeki bir sonraki simgeyi al.
    tokentype = Token.token;
    if (tokentype == T_EOF)
      break;
  }

  // Olusturdugumuz agaci dondur.
  return (left);
}

// binexpr fonksiyonu, ikili ifadeleri islemek icin kullanilir.
// Bu ornekte, sadece toplamsal ifadeler (additive_expr) dondurularak
// en temel ifade ayrıştırma seviyesini temsil eder. Daha karmasik
// bir derleyicide, bu fonksiyon parantezli ifadeler, karsilastirma
// operatorleri ve mantiksal operatorler gibi daha genis bir yelpazeyi
// isleyebilirdi.
struct ASTnode *binexpr(int n) {
  return (additive_expr());
}