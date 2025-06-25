#include "defs.h"    // Temel tanimlamalar (struct token, enum token tipleri vb.)
#define extern_      // data.h'deki extern_ macro'sunu tanimlariz.
                     // Bu, buradaki global degiskenlerin tanimlanmasini saglar.
#include "data.h"    // Global degiskenler icin bildirimleri (Infile, Line, Putback) icerir.
#undef extern_       // extern_ macro'sunu kaldiririz, boylece diger dosyalarda
                     // extern olarak kullanilmasini saglariz.
#include "decl.h"    // Fonksiyon prototiplerini (orn. scan) icerir.
#include <errno.h>   // Hata numaralari icin errno ve strerror fonksiyonu icin

// Derleyici kurulumu ve ust duzey yurutme

// Global degiskenleri baslatir
static void init() {
  Line = 1;         // Baslangic satir numarasini 1 olarak ayarla
  Putback = '\n';   // Putback karakterini yeni satir olarak ayarla (tarayicinin ilk karakteri okumasi icin)
}

// Yanlis baslatilirsa kullanim kilavuzunu yazdirir
static void usage(char *prog) {
  fprintf(stderr, "Kullanim: %s girdi_dosyasi\n", prog);  // Hata ciktisina kullanim formatini yazdir
  exit(1);                                                // Hata koduyla programdan cik
}


// Ana program: argumanlari kontrol et ve kullanim bilgisini yazdir
// eger arguman yoksa. Girdi dosyasini ac
// ve ilk tokeni tara.
void main(int argc, char *argv[]) {
  struct ASTnode *n;

  if (argc != 2)
    usage(argv[0]);

  init();

  if ((Infile = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
    exit(1);
  }

  scan(&Token);                 // Girdiden ilk tokeni al
  n = binexpr();                // Dosyadaki ifadeyi ayristir
  printf("%d\n", interpretAST(n));      // Sonuc degeri hesapla
  exit(0);
}