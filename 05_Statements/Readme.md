## Bölüm 5: İfadeler (*Statements*)

Dilimizin dilbilgisine bazı "düzgün" ifadeler (statements) eklemenin zamanı geldi. Şuna benzer kod satırları yazabilmek istiyorum:

```
print 2 + 3 * 5;
print 17 - 8/2 + 5;
```

Elbette, boşlukları (*whitespace*) göz ardı ettiğimiz için, bir ifadenin tüm simgelerinin (*tokens*) aynı satırda olması zorunluluğu yoktur. Her ifade `print` anahtar kelimesiyle başlar ve noktalı virgül ile biter. Bu yüzden bunlar dilimize yeni simgeler (*tokens*) olarak eklenecek.

## Dilbilgisinin BNF Tanımı

İfadeler için BNF (*Backus-Naur Form*) gösterimini zaten görmüştük. Şimdi yukarıdaki türdeki ifadeler için BNF sözdizimini tanımlayalım:

```
statements: statement  
          | statement statements
          ;

statement: 'print' expression ';' 
          ;
```

Bir girdi dosyası birden fazla ifade içerir. Bunlar ya tek bir ifadedir ya da bir ifadeyi takiben daha fazla ifadeden oluşur. Her ifade `print` anahtar kelimesiyle başlar, ardından bir ifade (*expression*) gelir ve noktalı virgül ile biter.

## Sözcüksel Tarayıcıdaki (*Lexical Scanner*) Değişiklikler

Yukarıdaki sözdizimini ayrıştıran koda geçmeden önce, mevcut koda birkaç ek parça eklememiz gerekiyor. Sözcüksel tarayıcı (*lexical scanner*) ile başlayalım.

Noktalı virgüller için bir simge (*token*) eklemek kolay olacak. Şimdi, print anahtar kelimesi. Daha sonra dilimizde birçok anahtar kelime ve değişkenlerimiz için tanımlayıcılar olacak, bu yüzden onlarla başa çıkmamıza yardımcı olacak bazı kodlar eklememiz gerekecek.

`scan.c` içinde, SubC derleyicisinden ödünç aldığım bu kodu ekledim. Alfasayısal karakterleri, alfasayısal olmayan bir karaktere rastlayana kadar bir arabelleğe okur.

```
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
```

Ayrıca, dildeki anahtar kelimeleri tanımak için bir fonksiyona ihtiyacımız var. Birinci yol, anahtar kelimelerin bir listesini bulundurmak ve listeyi gezip `scanident()`'tan gelen arabellege karsi her birini `strcmp()` ile karsilastirmak olabilirdi. SubC'den gelen kodun şöyle bir optimizasyonu var: `strcmp()` yapmadan once ilk harfe gore eslestirme yapmak. Bu, onlarca anahtar kelimeye karsi karsilastirmayi hizlandirir. Su anda bu optimizasyona ihtiyacimiz yok ama daha sonrasi icin ekledim:

```
// Girdiden bir kelime verildiginde, eslesen
// anahtar kelime simge numarasini veya bir anahtar kelime degilse 0 dondur.
// Tum anahtar kelimelere karsi strcmp() yaparak zaman kaybetmemek icin
// ilk harfe gore kontrol yap.
static int keyword(char *s) {
  switch (*s) {
    case 'p': // Kelime 'p' ile basliyorsa kontrol et.
      if (!strcmp(s, "print")) // Kelime "print" ise.
        return (T_PRINT); // T_PRINT simgesini dondur.
      break;
  }
  return (0); // Anahtar kelime degilse 0 dondur.
}
```

Şimdi, `scan()` fonksiyonundaki `switch` ifadesinin altına, noktalı virgülleri ve anahtar kelimeleri tanımak için bu kodu ekliyoruz:

```
case ';':
      t->token = T_SEMI; // Simgeyi T_SEMI olarak ayarla.
      break;
    default:

      // Eger bir rakamsa,
      // tam sayi degerini tara.
      if (isdigit(c)) {
        t->intvalue = scanint(c); // Tam sayi degerini tara.
        t->token = T_INTLIT; // Simgeyi T_INTLIT olarak ayarla.
        break;
      } else if (isalpha(c) || '_' == c) {
        // Bir anahtar kelime veya tanimlayici oku.
        scanident(c, Text, TEXTLEN); // Tanimlayiciyi tara ve Text bufferina kaydet.

        // Eger taninan bir anahtar kelimeyse, o simgeyi dondur.
        if (tokentype = keyword(Text)) { // keyword fonksiyonu cagrilir, eger bir anahtar kelimeyse tokentype'a atanir.
          t->token = tokentype; // Simgeyi taninan anahtar kelime turune ayarla.
          break;
        }
        // Taninan bir anahtar kelime degil, simdilik hata ver.
        printf("Unrecognised symbol %s on line %d\n", Text, Line);
        exit(1);
      }
      // Karakter, taninan hicbir simgeye ait degilse hata ver.
      printf("Unrecognised character %c on line %d\n", c, Line);
      exit(1);
```

Ayrıca, anahtar kelimeleri ve tanımlayıcıları depolamak için global bir `Text` arabelleği ekledim:

```
#define TEXTLEN         512             // Girdideki sembollerin uzunlugu.
extern_ char Text[TEXTLEN + 1];         // Taranan son tanimlayici.
```

## İfade Ayrıştırıcısındaki (*Expression Parser*) Değişiklikler

Şimdiye kadar girdi dosyalarımız sadece tek bir ifade içeriyordu; bu nedenle, `binexpr()` ( `expr.c` içinde) bulunan Pratt ayrıştırıcı kodumuzda, ayrıştırıcıdan çıkmak için bu kod vardı:

```
  // Eger hic simge kalmadiysa, sadece sol dugumu dondur.
  tokentype = Token.token;
  if (tokentype == T_EOF) // Dosya sonu simgesi (T_EOF) goruldugunde.
    return (left); // Sadece sol dugumu dondur ve cik.
```

Yeni dilbilgimizle, her ifade noktalı virgül ile sonlandırılıyor. Bu nedenle, ifade ayrıştırıcısındaki kodu `T_SEMI` (noktalı virgül) simgelerini tanıması ve ifade ayrıştırmasından çıkması için değiştirmemiz gerekiyor:

```
// Koku ikili bir operator olan bir AST agaci dondur.
// ptp parametresi, onceki simgenin onceligidir.
struct ASTnode *binexpr(int ptp) {
  struct ASTnode *left, *right;
  int tokentype;

  // Soldaki tam sayi degerini al.
  // Ayni anda bir sonraki simgeyi de oku.
  left = primary();

  // Eger bir noktalı virgul ile karsilasirsak, sadece sol dugumu dondur.
  tokentype = Token.token;
  if (tokentype == T_SEMI) // Noktali virgul simgesi (T_SEMI) goruldugunde.
    return (left); // Sadece sol dugumu dondur ve cik.

    while (op_precedence(tokentype) > ptp) {
      // ... (aradaki kod degismedi) ...

          // Mevcut simgenin detaylarini guncelle.
    // Eger bir noktalı virgul ile karsilasirsak, sadece sol dugumu dondur.
    tokentype = Token.token;
    if (tokentype == T_SEMI) // Dongu icinde noktali virgul simgesi (T_SEMI) goruldugunde.
      return (left); // Sadece sol dugumu dondur ve cik.
    }
    // Geriye kalan tree yapisini dondur.
    return (left); // Oncelik dusukse veya dongu bittiyse olusan agaci dondur.
}
```

## Kod Üreticisindeki (*Code Generator*) Değişiklikler

`gen.c` içindeki genel kod üreticisini, `cg.c` içindeki CPU'ya özgü koddan ayrı tutmak istiyorum. Bu aynı zamanda, derleyicinin geri kalanının her zaman sadece `gen.c` içindeki fonksiyonları çağırması gerektiği ve sadece `gen.c`'nin `cg.c` içindeki kodu çağırması gerektiği anlamına geliyor.

Bu amaçla, `gen.c`'de bazı yeni front-end fonksiyonlar tanımladım:

```c
void genpreamble()        { cgpreamble(); } // Genel baslangic kodunu uretmek icin CPU-ozgu baslangic kodunu cagir.
void genpostamble()       { cgpostamble(); } // Genel bitis kodunu uretmek icin CPU-ozgu bitis kodunu cagir.
void genfreeregs()        { freeall_registers(); } // Tum yazmaclari serbest birakmak icin CPU-ozgu fonksiyonu cagir.
void genprintint(int reg) { cgprintint(reg); } // Bir yazmaci yazdirmak icin CPU-ozgu fonksiyonu cagir.
```

## İfadeler (*Statements*) İçin Ayrıştırıcıyı (*Parser*) Ekleme

Yeni bir `stmt.c` dosyamız var. Bu dosya, dilimizdeki tüm ana ifadeler için ayrıştırma kodunu barındıracak. Şu anda, yukarıda verdiğim ifadeler için BNF dilbilgisini ayrıştırmamız gerekiyor. Bu tek bir fonksiyonla yapılır. Özyinelemeli tanımı bir döngüye dönüştürdüm:

```
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
```

Her döngüde, kod bir `T_PRINT` simgesi bulur. Ardından, ifadeyi ayrıştırmak için `binexpr()`'ı çağırır. Son olarak, `T_SEMI` simgesini bulur. Eğer bir `T_EOF` simgesi takip ediyorsa, döngüden çıkarız.

Her ifade ağacından sonra, ağacı assembly koduna dönüştürmek ve nihai değeri yazdırmak için assembly `printint()` fonksiyonunu çağırmak üzere `gen.c`'deki kod çağrılır.

## Bazı Yardımcı Fonksiyonlar

Yukarıdaki kodda, yeni bir dosya olan `misc.c`'ye koyduğum birkaç yeni yardımcı fonksiyon var:

```
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
```

Bunlar, ayrıştırıcıdaki sözdizimi kontrolünün bir parçasını oluşturur. Daha sonra, sözdizimi kontrolümüzü kolaylaştırmak için `match()`'i çağıracak daha kısa fonksiyonlar ekleyeceğim.

## `main()`'deki Değişiklikler

`main()` fonksiyonu eskiden, eski girdi dosyalarındaki tek ifadeyi ayrıştırmak için doğrudan `binexpr()`'i çağırırdı. Şimdi ise bunu yapıyor:

```
  scan(&Token);                 // Girdiden ilk simgeyi al.
  genpreamble();                // Baslangic kodunu uret.
  statements();                 // Girdideki ifadeleri ayristir.
  genpostamble();               // Bitis kodunu uret.
  fclose(Outfile);              // Cikti dosyasini kapat ve cik.
  exit(0);                      // Programi basarili bir sekilde sonlandir.
```

## Test Edelim

Yeni ve değiştirilmiş kod bu kadar. Yeni kodu bir deneyelim. İşte yeni girdi dosyası, `input01`:

```
print 44 * 2;
print 
   28 - 1
      * 8; print
1 + 2 +
  9 - 8/2 + 5*2;
```

Evet, simgelerin (*tokens*) birden fazla satıra yayıldığını kontrol etmeye karar verdim. Girdi dosyasını derlemek ve çalıştırmak için `make test` komutunu çalıştırın:

```
$ make test
./comp1 input01
cc -o out out.s
./out
88
20
18
```

Calışıyor!

## Sonuç ve Sıradaki Adımlar

Dilimize ilk "gerçek" ifade dilbilgisini ekledik. Bunu BNF (*Backus-Naur Form*) gösterimiyle tanımladım, ancak özyinelemeli olarak değil, bir döngü ile uygulamak daha kolay oldu. Merak etmeyin, yakında özyinelemeli ayrıştırmaya geri döneceğiz.

Bu süreçte, tarayıcıyı (*scanner*) değiştirmemiz, anahtar kelimeler ve tanımlayıcılar için destek eklememiz ve genel kod üreticisi ile CPU'ya özgü üreticiyi daha temiz bir şekilde ayırmamız gerekti.

Derleyici yazma yolculuğumuzun bir sonraki bölümünde, dile değişkenler ekleyeceğiz. Bu önemli miktarda çalışma gerektirecek. [Sonraki adım](../06_Variables/Readme.md)
