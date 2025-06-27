#include "defs.h"
#include "data.h"
#include "decl.h"

// Ifadeler (statements) icin ayristirici (parser)

// statements: statement
//      | statement statements
//      ;
//
// statement: 'print' expression ';'
//      ;

// Bir veya daha fazla ifadeyi ayristir.
void statements(void) {
  struct ASTnode *tree; // Soyut Sozdizimi Agaci (AST) icin isaretci.
  int reg;              // Register numarasi (uretilen kodun sonucunu tutacak).

  while (1) { // Sonsuz bir dongu, ta ki acikca cikana kadar.
    // Ilk simge olarak 'print' kelimesini eslestir.
    match(T_PRINT, "print"); // Mevcut simgenin T_PRINT olmasini bekler, degilse hata verir.

    // Takip eden ifadeyi ayristir ve
    // assembly kodunu uret.
    tree = binexpr(0); // Ifadeyi ayristirir ve bir AST agaci olusturur. (0: en dusuk oncelik).
    reg = genAST(tree); // AST agacindan assembly kodu uretir ve sonucu tutan registerin numarasini dondurur.
    genprintint(reg); // Registerdaki degeri yazdiran assembly kodunu uretir.
    genfreeregs(); // Kullanilan tum registerleri serbest birakir.

    // Takip eden noktalı virgul'u eslestir
    // ve Eger dosya sonunda isek dur.
    semi(); // Mevcut simgenin T_SEMI (noktali virgul) olmasini bekler, degilse hata verir.
    if (Token.token == T_EOF) // Eger bir sonraki simge dosya sonu (EOF) ise.
      return; // Fonksiyondan cik, ifadeleri ayrıştırma bitti.
  }
}