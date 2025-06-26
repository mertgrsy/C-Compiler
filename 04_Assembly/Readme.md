# Bölüm 4: Gerçek Bir Derleyici

Artık bir derleyici yazma vaadimi gerçekleştirmenin tam zamanı geldi. Bu yolculuğun bu bölümünde, programımızdaki yorumlayıcıyı, M1 Mac işlemci için gerekli instructions üreten kod ile değiştireceğiz.

## Yorumlayıcıyı Gözden Geçirme

Bunu yapmadan önce, interp.c içindeki yorumlayıcı kodunu tekrar gözden geçirmek faydalı olacaktır:

```c
int interpretAST(struct ASTnode *n) {
  int leftval, rightval;

  // Eger sol alt dugum varsa, onu ozyinelemeli olarak yorumla ve degerini al.
  if (n->left) leftval = interpretAST(n->left);
  // Eger sag alt dugum varsa, onu ozyinelemeli olarak yorumla ve degerini al.
  if (n->right) rightval = interpretAST(n->right);

  // Mevcut AST dugumunun operator tipine gore islem yap.
  switch (n->op) {
    case A_ADD:      return (leftval + rightval); // Toplama islemi yap.
    case A_SUBTRACT: return (leftval - rightval); // Cikarma islemi yap.
    case A_MULTIPLY: return (leftval * rightval); // Carpma islemi yap.
    case A_DIVIDE:   return (leftval / rightval); // Bolme islemi yap.
    case A_INTLIT:   return (n->intvalue); // Tam sayi degeri dondur.

    default:
      // Bilinmeyen bir AST operatoru ile karsilasildiysa hata ver.
      fprintf(stderr, "Unknown AST operator %d\n", n->op);
      exit(1);
  }
}
```

`interpretAST()` fonksiyonu, verilen AST ağacını derinlemesine (*depth-first*) gezer. Önce varsa sol alt ağacı, sonra sağ alt ağacı değerlendirir. Son olarak, mevcut ağacın tabanındaki `op` değerini kullanarak bu alt düğümler üzerinde işlem yapar.

`op` değeri dört matematik operatöründen biriyse, ilgili matematiksel işlem gerçekleştirilir. Eğer `op` değeri, düğümün basitçe bir tam sayı sabiti olduğunu gösteriyorsa, sabitin değeri döndürülür.

Fonksiyon, bu ağaç için nihai değeri döndürür. Özyinelemeli olduğu için, bütün bir ağacın nihai değerini her seferinde bir alt-alt ağaç hesaplayarak bulacaktır.

## Assembly Kod Üretimine Geçiş

Genel (*generic*) bir assembly kodu üreticisi yazacağız. Bu üretici, sırasıyla, işlemciye özgü (*CPU-specific*) kod üretim fonksiyonlarını çağıracak.

İşte `gen.c` içindeki genel assembly kodu üreticisi:

```c
// Bir AST verildiginde,
// ozyinelemeli olarak assembly kodu uret.
static int genAST(struct ASTnode *n) {
  int leftreg, rightreg;

  // Sol ve sag alt agac degerlerini al.
  if (n->left) leftreg = genAST(n->left);
  if (n->right) rightreg = genAST(n->right);

  switch (n->op) {
    case A_ADD:      return (cgadd(leftreg,rightreg)); // Toplama islemi icin kod uret ve sonucu bir kayitcida tut.
    case A_SUBTRACT: return (cgsub(leftreg,rightreg)); // Cikarma islemi icin kod uret ve sonucu bir kayitcida tut.
    case A_MULTIPLY: return (cgmul(leftreg,rightreg)); // Carpma islemi icin kod uret ve sonucu bir kayitcida tut.
    case A_DIVIDE:   return (cgdiv(leftreg,rightreg)); // Bolme islemi icin kod uret ve sonucu bir kayitcida tut.
    case A_INTLIT:   return (cgload(n->intvalue));     // Tam sayi degerini bir kayitciya yukle ve kayitci kimligini dondur.

    default:
      // Bilinmeyen bir AST operatoru ile karsilasildiysa hata ver.
      fprintf(stderr, "Unknown AST operator %d\n", n->op);
      exit(1);
  }
}
```

Tanıdık geldi, değil mi?! Yine aynı derinlemesine (*depth-first*) ağaç dolaşımını yapıyoruz. Bu sefer:

* `A\_INTLIT`: Sabit değeri bir register'a yükle
* Diğer operatörler: Sol alt-çocuğun ve sağ alt-çocuğun değerlerini tutan iki register üzerinde bir matematiksel işlem gerçekleştir.

Değerleri aktarmak yerine, `genAST()` içindeki kod yazmaç (*register*) tanımlayıcılarını aktarıyor. Örneğin `cgload()`, bir değeri bir register'a yükler ve yüklenen değere sahip register'ın kimliğini döndürür.

`genAST()`'in kendisi, o noktada ağacın nihai değerini tutan register'ın kimliğini döndürür. Bu yüzden yukarıdaki kod register kimliklerini alıyor:

```c
  // Sol alt dugum varsa, ozyinelemeli olarak kod uret ve kayitci kimligini al.
  if (n->left) leftreg = genAST(n->left);
  // Sag alt dugum varsa, ozyinelemeli olarak kod uret ve kayitci kimligini al.
  if (n->right) rightreg = genAST(n->right);
```

## `genAST()` Çağrısı

`genAST()` sadece kendisine verilen ifadenin değerini hesaplayacak. Bu nihai hesaplamayı ekrana yazdırmamız gerekiyor. Ayrıca, üreteceğimiz assembly kodunu bazı başlangıç kodu (*preamble*) ve bazı bitiş kodu (*postamble*) ile sarmalamamız gerekecek. Bu, `gen.c` içindeki diğer fonksiyon ile yapılır:

```c
void generatecode(struct ASTnode *n) {
  int reg;

  cgpreamble();         // Baslangic kodunu uret.
  reg = genAST(n);      // Ifadenin degerini hesapla ve sonucu registirde tut.
  cgprintint(reg);      // Registirdeki sonucu int olarak yazdir.
  cgpostamble();        // Bitis kodunu uret.
}
```

## M1 Mac (ARM64) Kod Üreticisi

Genel kod üreticisini geride bıraktık. Şimdi gerçek assembly kodu üretimine bakmamız gerekiyor. Şimdilik, M1 Mac (ARM64) CPU'yu hedefliyorum, çünkü elimde bu var. O zaman, `cg.c` dosyasını açalım ve göz atmaya başlayalım.

### Register Tahsisi

Her CPU'nun sınırlı sayıda register'ı vardır. Tam sayı sabit değerlerini ve bunlar üzerinde gerçekleştirdiğimiz herhangi bir hesaplamayı tutmak için bir register tahsis etmemiz gerekecek. Ancak, bir değeri kullandıktan sonra, değeri genellikle atabilir ve dolayısıyla onu tutan register'ı serbest bırakabiliriz. Daha sonra bu register'ı başka bir değer için yeniden kullanabiliriz.

Register tahsisiyle ilgilenen üç fonksiyon bulunmaktadır:

* `freeall_registers()`: Tüm register'ları kullanılabilir olarak ayarla
* `alloc_register()`: Boş bir register tahsis et
* `free_register()`: Tahsis edilmiş bir register'ı serbest bırak

Kodun üzerinden geçmeyeceğim çünkü basit, ancak bazı hata kontrolleri içeriyor. Şu anda, eğer register'lar biterse program çökecek. Daha sonra, boş register kalmadığında ne yapacağımızı ele alacağız.

Kod, genel register'lar üzerinde çalışır: r0, r1, r2 ve r3. Gerçek register adlarını içeren bir dizge (*string*) tablosu vardır:

```c
// ARM64 mimarisi icin yaygin genel amacli yazmaclar (64-bit).
// Genellikle x0-x7 fonksiyon argumanlari ve donus degerleri icin kullanilir.
// x8 ve sonrasi gecici degerler icin daha uygundur.
static char *reglist[4]= { "x8", "x9", "x10", "x11" };
```

Bu, bu fonksiyonları CPU mimarisinden oldukça bağımsız hale getirir.

### Register Yükleme

Bu işlem `cgload()` içinde yapılır: bir register tahsis edilir, ardından bir tam sayı sabit değeri tahsis edilen registera yüklenir.

```c
// Bir tam sayi sabit degerini bir kayitciya yukle.
// Kayitcinin numarasini dondur.
int cgload(int value) {

  // Yeni bir kayitci al.
  int r = alloc_register();

  // Onu baslatmak icin kodu yazdir.
  // ARM64'te 'mov' dogrudan sabit degeri bir kayitciya yukleyebilir.
  fprintf(Outfile, "\tmov\t%s, #%d\n", reglist[r], value);
  return (r);
}
```

### İki Registerı Toplama

`cgadd()` iki register numarasını alır ve bunları toplamak için kodu üretir. Sonuç, iki registerdan birine kaydedilir ve diğeri gelecekteki kullanım için serbest bırakılır:

```c
// Iki kayitciyi topla ve sonucu iceren
// kayitcinin numarasini dondur.
int cgadd(int r1, int r2) {
  // ARM64'te 'add' komutu: dest = src1 + src2.
  // r1 ve r2'yi topla, sonucu r2'ye yaz.
  fprintf(Outfile, "\tadd\t%s, %s, %s\n", reglist[r2], reglist[r1], reglist[r2]);
  free_register(r1); // r1'i serbest birak.
  return (r2); // Sonucu iceren kayitciyi dondur.
}
```

### İki Registerı Çarpma

Bu, toplamaya çok benzer ve yine işlem **değişme özelliğine sahiptir**, bu yüzden herhangi bir register döndürülebilir:

```c
// Iki kayitciyi carp ve sonucu iceren
// kayitcinin numarasini dondur.
int cgmul(int r1, int r2) {
  // ARM64'te 'mul' komutu: dest = src1 * src2.
  // r1 ve r2'yi carp, sonucu r2'ye yaz.
  fprintf(Outfile, "\tmul\t%s, %s, %s\n", reglist[r2], reglist[r1], reglist[r2]);
  free_register(r1); // r1'i serbest birak.
  return (r2); // Sonucu iceren kayitciyi dondur.
}
```

### İki Registerı Çıkarma

Çıkarma **değişme özelliğine sahip değildir**: sırayı doğru ayarlamamız gerekir. İkinci register birinciden çıkarılır, bu yüzden birinciyi döndürürüz ve ikinciyi serbest bırakırız:

```c
// Ilk kayitcidan ikinciyi cikar ve
// sonucu iceren kayitcinin numarasini dondur.
int cgsub(int r1, int r2) {
  // ARM64'te 'sub' komutu: dest = src1 - src2.
  // r1'den r2'yi cikar, sonucu r1'e yaz.
  fprintf(Outfile, "\tsub\t%s, %s, %s\n", reglist[r1], reglist[r1], reglist[r2]);
  free_register(r2); // r2'yi serbest birak.
  return (r1); // Sonucu iceren kayitciyi dondur.
}
```

### İki Registerı Bölme

Bölme de değişme özelliğine sahip değildir, bu yüzden önceki notlar geçerlidir. Ancak ARM64'te bu işlem x86-64'e göre çok daha basittir. Direkt bir `sdiv` (*signed divide*) talimatı kullanarak bölümü alabiliriz. Yani özel yazmaçlara (x86'daki `%rax` gibi) yükleme veya uzatma (*extension*) işlemlerine gerek yoktur.

```c
// Ilk kayitciyi ikinciye bol ve
// sonucu iceren kayitcinin numarasini dondur.
int cgdiv(int r1, int r2) {
  // ARM64'te 'sdiv' komutu: dest = src1 / src2 (isaretli bolme).
  // r1'i r2'ye bol, sonucu r1'e yaz.
  fprintf(Outfile, "\tsdiv\t%s, %s, %s\n", reglist[r1], reglist[r1], reglist[r2]);
  free_register(r2); // r2'yi serbest birak.
  return (r1); // Sonucu iceren kayitciyi dondur.
}
```

### Bir Registerı Yazdırma

Bir registerı ondalık sayı olarak yazdırmak için direkt bir ARM64 talimatı yoktur. Bu sorunu çözmek için, assembly başlangıç (*preamble*) kodu, bir register argümanı alan ve bunu ondalık olarak yazdırmak için `printf()`'i çağıran `printint()` adında bir fonksiyon içerir.

`cgpreamble()` içindeki kodu vermeyeceğim, ancak aynı zamanda `main()` için başlangıç kodunu da içerir, böylece çıktı dosyamızı eksiksiz bir program elde etmek için derleyebiliriz. Burada verilmeyen `cgpostamble()` için olan kod ise programı sonlandırmak için basitçe `exit(0)`'ı çağırır.

Ancak, işte `cgprintint()`:

```c
void cgprintint(int r) {
  // Linux ARM64 (AArch64) ABI'ye gore, fonksiyonlara ilk arguman
  // x0 kayitcisina yerlestirilir.
  fprintf(Outfile, "\tmov\tx0, %s\n", reglist[r]); // Sonucu tutan kayitciyi x0'a tasi.
  fprintf(Outfile, "\tbl\t_printint\n"); // _printint fonksiyonunu cagir (bl = branch with link).
                                          // Unix/Linux'ta C fonksiyonlari genellikle _ ile baslar.
  free_register(r); // Kullanilan kayitciyi serbest birak.
}
```

Linux ARM64 (*AArch64*) ABI'si, bir fonksiyona ilk argümanın `x0` yazmacında olmasını bekler, bu yüzden `_printint`'i çağırmadan önce yazmacımızı `x0`'a taşıyoruz.

## İlk Derlememizi Yapma

ARM64 kod üreticisi hakkında anlatacaklarım bu kadar. `main()` içinde, çıktı dosyamız olarak `out.s`'i açmak için bazı ek kodlar bulunuyor. Ayrıca, girdi ifadesi için assembly kodumuzun yorumlayıcı ile aynı cevabı hesapladığını doğrulamak amacıyla yorumlayıcıyı da programda bıraktım.

Şimdi derleyiciyi oluşturalım ve `input01` üzerinde çalıştıralım:

```c
$ make
cc -o comp1 -g cg.c expr.c gen.c interp.c main.c scan.c tree.c

$ make test
./comp1 input01
int 2
int 3
int 5
3 * 5
2 + 15
int 8
int 3
8 / 3
17 - 2
15
cc -o out out.s
./out
15
```

Evet! İlk 15, yorumlayıcının çıktısı. İkinci 15 ise assembly kodunun çıktısı.

## Assembly Çıktısının Değerlendirilmesi

İşte girdi dosyamızdaki `2 + 3 * 5 - 8 / 3` ifadesi için derleyicimizin ürettiği `out.s` dosyası, detaylı Türkçe yorumlarla birlikte Assembly çıktısı tam olarak şöyle:

```assembly
        .text                                   ; Kod bolumunun basladigini belirtir.
	.section	__TEXT,__cstring,cstring_literals ; MacOS'ta string sabitlerinin yer alacagi ozel bir bolum tanimlar.
.L.str:                                         ; ".L.str" adinda bir etiket (label) tanimlar.
	.asciz	"%d\n"                          ; Bu etiketin altinda, null-sonlandirilmali bir string ("%d\n") olusturur.

	.section	__TEXT,__text,regular,pure_instructions ; Calistirilabilir kodun yer alacagi ana bolume geri doner.
	.globl	_printint                       ; "_printint" etiketini (fonksiyon adini) diger dosyalardan erisilebilir (global) yapar.
	.p2align 2                              ; Sonraki talimati 4 byte (2^2) sinirina hizalar, bu performans icin iyidir.
_printint:                                      ; "_printint" fonksiyonunun baslangici.
	stp	x29, x30, [sp, #-16]!           ; Fonksiyon baslangici (prologue): x29 (Frame Pointer) ve x30'u (Link Register/Donus Adresi) yigina kaydet, yigin isaretcisini (sp) 16 byte azalt.
	mov	x29, sp                         ; Frame Pointer'i (x29) mevcut yigin isaretcisine ayarla. Bu, yigin cercevesini kurar.
	sub	sp, sp, #16                     ; C Kutuphanesi cagrisi (_printf) icin yiginda ek 16 byte'lik guvenli alan ayir.
	str	x0, [sp]                        ; Yazdirilacak degeri (x0 ile gelen) yiginin en ustune koy. printf bu degeri buradan okuyacak.
	adrp	x0, .L.str@GOTPAGE              ; Global Offset Table (GOT) uzerinden ".L.str" etiketinin sayfa adresini x0'a yukle.
	ldr	x0, [x0, .L.str@GOTPAGEOFF]     ; x0'daki sayfa adresine ofseti ekleyerek ".L.str" string'inin gercek adresini x0'a yukle.
	bl	_printf                         ; "_printf" fonksiyonunu cagir. x0'da format string adresi var, diger arguman yiginda.
	mov	sp, x29                         ; Fonksiyon bitisi (epilogue): Yigin isaretcisini, fonksiyonun basindaki temiz haline geri getir.
	ldp	x29, x30, [sp], #16             ; Yigindan x29 ve x30'u geri yukle, yigin isaretcisini 16 byte artir.
	ret                                     ; Fonksiyondan don (cagiran yere geri git).

	.globl	_main                           ; "_main" etiketini (programin ana fonksiyonu) global olarak gorunur yapar.
	.p2align 2                              ; Sonraki talimati 4 byte sinirina hizalar.
_main:                                          ; "_main" fonksiyonunun baslangici.
	stp	x29, x30, [sp, #-16]!           ; Fonksiyon baslangici (prologue): x29 ve x30'u yigina kaydet, sp'yi 16 byte azalt.
	mov	x29, sp                         ; Frame Pointer'i (x29) mevcut yigin isaretcisine ayarla.
	sub	sp, sp, #16                     ; Harici fonksiyon cagrilari icin yiginda ek 16 byte'lik guvenli alan ayir.
	mov	x8, #2                          ; x8 yazmacina 2 degerini yukle.
	mov	x9, #3                          ; x9 yazmacina 3 degerini yukle.
	mov	x10, #5                         ; x10 yazmacina 5 degerini yukle.
	mul	x10, x9, x10                    ; x9 ve x10'u carp, sonucu x10'a yaz (x10 = 3 * 5 = 15).
	add	x10, x8, x10                    ; x8 ve x10'u topla, sonucu x10'a yaz (x10 = 2 + 15 = 17).
	mov	x8, #8                          ; x8 yazmacina 8 degerini yukle.
	mov	x9, #3                          ; x9 yazmacina 3 degerini yukle.
	sdiv	x8, x8, x9                      ; x8'i x9'a bol (isaretli tam sayi bolmesi), sonucu x8'e yaz (x8 = 8 / 3 = 2).
	sub	x10, x10, x8                    ; x10'dan x8'i cikar, sonucu x10'a yaz (x10 = 17 - 2 = 15).
	mov	x0, x10                         ; Nihai sonucu (15) x0 yazmacina tasi. Bu, "_printint" fonksiyonuna arguman olarak gonderilecek.
	bl	_printint                       ; "_printint" fonksiyonunu cagir.
	mov	x0, #0                          ; Programin donus degerini 0 (basarili) olarak ayarla.
	mov	sp, x29                         ; Fonksiyon bitisi (epilogue): Yigin isaretcisini, fonksiyonun basindaki temiz haline geri getir.
	ldp	x29, x30, [sp], #16             ; Yigindan x29 ve x30'u geri yukle, sp'yi 16 byte artir.
	ret                                     ; Fonksiyondan don (programi sonlandirir).
```

Harika! Artık geçerli bir derleyiciye sahibiz: bir dildeki girdiyi alıp, bu girdinin başka bir dildeki çevirisini üreten bir program.

Çıktıyı makine koduna dönüştürmek ve destekleyici kütüphanelerle bağlamak hala yapmamız gereken bir şey, ancak şimdilik bunu manuel olarak gerçekleştirebiliriz. Daha sonra, bunu otomatik olarak yapacak bir kod yazacağız.

## Sonuç ve Sıradaki Adımlar

Yorumlayıcıdan genel bir kod üreticiye geçmek çok önemli değildi, ancak daha sonra gerçek assembly çıktısı üretmek için bazı kodlar yazmak zorunda kaldık. Bunu yapmak için, register'ları nasıl tahsis edeceğimizi düşünmem ve öğrenmem gerekti: şimdilik, basit bir çözümümüz var. Ayrıca, `GOTPAGE` talimatı ve `_printf` fonksiyonu gibi bazı Mac'te ARM64 tuhaflıklarıyla da uğraşmak zorunda kaldık.

Henüz değinmediğim bir şey var: Bir ifade için neden AST (Soyut Sözdizimi Ağacı) üretmekle uğraşalım ki? Elbette, Pratt ayrıştırıcımızda bir '`+`' simgesiyle karşılaştığımızda `cgadd()`'i çağırabilirdik, diğer operatörler için de aynısı geçerli. Bunu size düşündürmek için bırakıyorum, ancak bir veya iki adım sonra buna geri döneceğim.

Derleyici yazma yolculuğumuzun bir sonraki bölümünde, dilimize bazı ifadeler (*statements*) ekleyeceğiz, böylece dilimiz gerçek bir bilgisayar diline benzemeye başlayacak. [Sonraki adım](../05_Statements/Readme.md)
