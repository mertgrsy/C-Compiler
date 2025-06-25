#include "defs.h"
#include "data.h"
#include "decl.h"

// Leksik tarama

// Karakter c'nin s dizisindeki konumunu dondurur
// veya c bulunamazsa -1 dondurur
static int chrpos(char *s, int c) {
  char *p;

  p = strchr(s, c);
  return (p ? p - s : -1);
}

// Girdi dosyasindan bir sonraki karakteri alir.
static int next(void) {
  int c;

  if (Putback) {		// Geri konmus bir karakter varsa,
    c = Putback;		// onu kullan.
    Putback = 0;
    return c;
  }

  c = fgetc(Infile);		// Girdi dosyasindan oku
  if ('\n' == c)
    Line++;			// Satir sayacini artir
  return c;
}

// Istenmeyen bir karakteri geri koyar
static void putback(int c) {
  Putback = c;
}

// Ilgilenmemiz gerekmeyen girdileri (yani bosluk, yeni satir) atlar.
// Ilgilenmemiz gereken ilk karakteri dondurur.
static int skip(void) {
  int c;

  c = next();
  while (' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c) {
    c = next();
  }
  return (c);
}

// Girdi dosyasindan bir tam sayi degismez degeri tarar ve dondurur.
static int scanint(int c) {
  int k, val = 0;

  // Her karakteri bir int degere donustur
  while ((k = chrpos("0123456789", c)) >= 0) {
    val = val * 10 + k;
    c = next();
  }

  // Tam sayi olmayan bir karakterle karsilastik, onu geri koy.
  putback(c);
  return val;
}

// Girdide bulunan bir sonraki token'i tarar ve dondurur.
// Token gecerliyse 1, baska token kalmadiysa 0 dondurur.
int scan(struct token *t) {
  int c;

  // Bosluklari atla
  c = skip();

  // Girdi karakterine gore token'i belirle
  switch (c) {
  case EOF: // Dosya sonu
    return (0);
  case '+':
    t->token = T_PLUS;
    break;
  case '-':
    t->token = T_MINUS;
    break;
  case '*':
    t->token = T_STAR;
    break;
  case '/':
    t->token = T_SLASH;
    break;
  default:

    // Eger bir rakamsa,
    // degismez tam sayi degerini tara
    if (isdigit(c)) {
      t->intvalue = scanint(c);
      t->token = T_INTLIT;
      break;
    }

    printf("Tanimlanamayan karakter %c, satir %d\n", c, Line);
    exit(1);
  }

  // Bir token bulduk
  return (1);
}