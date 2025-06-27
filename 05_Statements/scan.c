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

// Girdi dosyasindan bir tanimlayiciyi tara ve
// onu buf[] icinde depola. Tanimlayicinin uzunlugunu dondur.
static int scanident(int c, char *buf, int lim) {
  int i = 0;

  // Rakamlara, harflere ve alt cizgilere izin ver.
  while (isalpha(c) || isdigit(c) || '_' == c) {
    // Eger tanimlayici uzunluk limitine ulasirsak hata ver,
    // aksi halde buf[]'e ekle ve bir sonraki karakteri al.
    if (lim - 1 == i) {
      printf("identifier too long on line %d\n", Line);
      exit(1);
    } else if (i < lim - 1) {
      buf[i++] = c;
    }
    c = next(); // Bir sonraki karakteri oku.
  }
  // Gecersiz bir karaktere rastladik, onu geri koy.
  // buf[]'u NUL ile sonlandir ve uzunlugu dondur.
  putback(c);
  buf[i] = '\0';
  return (i);
}

// Girdiden bir kelime verildiginde, eslesen
// anahtar kelime simge numarasini veya bir anahtar kelime degilse 0 dondur.
// Tum anahtar kelimelere karsi strcmp() yaparak zaman kaybetmemek icin
// ilk harfe gore kontrol yap.
static int keyword(char *s) {
  // Anahtar kelimenin ilk harfini al
  switch (*s) {
    case 'p': // Kelime 'p' ile basliyorsa kontrol et.
      if (!strcmp(s, "print")) // Kelime "print" ise.
        return (T_PRINT); // T_PRINT simgesini dondur.
      break;
  }
  return (0); // Anahtar kelime degilse 0 dondur.
}

// Girdide bulunan bir sonraki token'i tarar ve dondurur.
// Token gecerliyse 1, baska token kalmadiysa 0 dondurur.
int scan(struct token *t) {
  t->token = 0;
  t->intvalue = 0;
  int c, tokentype;

  // Bosluklari atla
  c = skip();

  // Girdi karakterine gore token'i belirle
  switch (c) {
    case EOF: // Dosya sonu
      t->token = T_EOF; // EOF tokenini ayarla
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
    case ';':
      t->token = T_SEMI;
      break;
    default:
      // Eger bir rakamsa,
      // degismez tam sayi degerini tara
      if (isdigit(c)) {
        t->intvalue = scanint(c);
        t->token = T_INTLIT;
        break;
      }
      else if (isalpha(c) || '_' == c) {
	      // Bir anahtar kelime veya tanimlayici oku.
	      scanident(c, Text, TEXTLEN); // Tanimlayiciyi tara ve Text bufferina kaydet.
	
	        // Eger taninan bir anahtar kelimeyse, o simgeyi dondur.
	      if ((tokentype = keyword(Text))) { // Atama yap ve sonucu kontrol et.
              t->token = tokentype; // Simgeyi tanınan anahtar kelime türüne ayarla.
              break;
        }
	        // Taninan bir anahtar kelime degil, simdilik hata ver.
	        printf("Unrecognised symbol %s on line %d\n", Text, Line);
	        exit(1);
	      }
	      // Karakter, taninan hicbir simgeye ait degilse hata ver.
	      printf("Unrecognised character %c on line %d\n", c, Line);
	      exit(1);
      }

  // token bulduk
  return (1);
}
