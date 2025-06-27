#include "defs.h"
#include "data.h"
#include "decl.h"

// Cesitli yardimci fonksiyonlar

// Mevcut simgenin t oldugundan emin ol,
// ve bir sonraki simgeyi al. Aksi halde
// bir hata firlat.
void match(int t, char *what) { // t: Beklenen simge turu, what: Hata mesajinda kullanilacak beklenen seyi aciklayan string.
  if (Token.token == t) { // Mevcut simge beklenen simge turuyle eslesiyorsa.
    scan(&Token); // Bir sonraki simgeyi oku.
  } else { // Eslesmiyorsa.
    printf("%s expected on line %d\n", what, Line); // Hata mesaji yazdir.
    exit(1); // Programi sonlandir.
  }
}

// Bir noktalı virgül eslestir ve bir sonraki simgeyi al.
void semi(void) {
  match(T_SEMI, ";"); // Sadece T_SEMI simgesinin beklenmesini belirten match fonksiyonunu cagir.
}