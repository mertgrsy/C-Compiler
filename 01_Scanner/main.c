#include "defs.h"    // Temel tanimlamalar (struct token, enum token tipleri vb.)
#define extern_      // data.h'deki extern_ macro'sunu tanimlariz.
                     // Bu, buradaki global degiskenlerin tanimlanmasini saglar.
#include "data.h"    // Global degiskenler icin bildirimleri (Infile, Line, Putback) icerir.
#undef extern_       // extern_ macro'sunu kaldiririz, boylece diger dosyalarda
                     // extern olarak kullanilmasini saglariz.
#include "decl.h"    // Fonksiyon prototiplerini (orn. scan) icerir.
#include <errno.h>   // Hata numaralari icin errno ve strerror fonksiyonu icin

// Derleyici kurulumu ve ust duzey yurutme
// Telif Hakki (c) 2019 Warren Toomey, GPL3

// Global degiskenleri baslatir
static void init() {
  Line = 1;         // Baslangic satir numarasini 1 olarak ayarla
  Putback = '\n';   // Putback karakterini yeni satir olarak ayarla (tarayicinin ilk karakteri okumasi icin)
}

// Yanlis baslatilirsa kullanim kilavuzunu yazdirir
static void usage(char *prog) {
  fprintf(stderr, "Kullanim: %s girdi_dosyasi\n", prog); // Hata ciktisina kullanim formatini yazdir
  exit(1);                                            // Hata koduyla programdan cik
}

// Yazdirilabilir token'larin listesi
char *tokstr[] = { "+", "-", "*", "/", "intlit" };

// Girdi dosyasindaki tum token'lari tarayan dongu.
// Bulunan her token'in ayrintilarini yazdirir.
static void scanfile() {
  struct token T;   // Bir 'token' yapisi tanimla

  // 'scan' fonksiyonu bir token buldugu surece (yani dosya sonuna ulasana kadar) donguyu surdur
  while (scan(&T)) {
    printf("Token %s", tokstr[T.token]); // Token tipini yazdir (orn. "+", "intlit")
    if (T.token == T_INTLIT)             // Eger token bir tam sayi degismezi ise
      printf(", deger %d", T.intvalue);  // Tam sayinin degerini de yazdir
    printf("\n");                        // Yeni satira gec
  }
}

// Ana program: Argumanlari kontrol eder ve
// arguman yoksa kullanim kilavuzunu yazdirir.
// Girdi dosyasini acar ve icindeki token'lari taramak icin
// scanfile() fonksiyonunu cagirir.
void main(int argc, char *argv[]) {
  if (argc != 2) // Komut satiri arguman sayisi 2 degilse (program adi + dosya adi)
    usage(argv[0]); // Kullanim kilavuzunu yazdir ve cik

  init(); // Global degiskenleri baslat

  // Girdi dosyasini okuma modunda ac
  if ((Infile = fopen(argv[1], "r")) == NULL) {
    // Dosya acilamazsa hata mesaji yazdir ve cik
    fprintf(stderr, "Dosya %s acilamadi: %s\n", argv[1], strerror(errno));
    exit(1);
  }

  scanfile(); // Acilan dosyayi token'lar icin tara
  exit(0);    // Programi basariyla sonlandir
}