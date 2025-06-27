#include "defs.h"
#include "data.h"
#include "decl.h"

// Assembly kodu uretimi

// Mevcut registerların listesi ve adları 
static int freereg[4]; // Her bir registerin bos olup olmadigini gosteren dizi. 1: bos, 0: dolu.
// Kullanilacak ARM64 genel amacli registerlerin adlari (genellikle gecici degerler icin x8-x15 kullanilir).
static char *reglist[4] = { "x8", "x9", "x10", "x11" };

// Tum registerleri musait olarak ayarla
void freeall_registers(void) {
    for (int i = 0; i < 4; i++) {
        freereg[i] = 1; // Tüm registerler boş
    }
}

// Bos bir register tahsis et. registerin numarasini dondur.
// Musait register yoksa programi sonlandir.
static int alloc_register(void) 
{
    for (int i = 0; i < 4; i++) {
        if (freereg[i]) {
            freereg[i] = 0; // Registeri ayir
            return i; // Kullanilan registerin numarasini dondur
        }
    }
    fprintf(stderr, "No free registers available\n");
    exit(1);
}

// Bir registeri musait registerlar listesine geri dondur.
// Zaten orada olup olmadigini kontrol et.
static void free_register(int reg) 
{
    if (freereg[reg] != 0) {
        fprintf(stderr, "Error trying to free register %d\n", reg);
        exit(1);
    }
    freereg[reg] = 1; // Registeri geri ver
}

// Assembly başlangıç kodunu yazdır.
void cgpreamble() {
    freeall_registers();
    fputs(
        // Veri bölümü: String sabitleri buraya gelir.
        "\t.text\n" // Kod bölümünün başlangıcı.
        "\t.section\t__TEXT,__cstring,cstring_literals\n" // MacOS/ARM64'te stringler için bölüm.
        ".L.str:\n" // printf için format string etiketi.
        "\t.asciz\t\"%d\\n\"\n" // "%d\n" string sabiti.

        // Kod bölümü: Fonksiyonlar buraya gelir.
        "\t.section\t__TEXT,__text,regular,pure_instructions\n"
        "\t.globl\t_printint\n" // printint fonksiyonunu global olarak görünür yap (MacOS'ta C fonksiyonları _ ile başlar).
        "\t.p2align 2\n" // 4 bayt (2^2) hizalama.
        "_printint:\n"
        // Standart fonksiyon başlangıcı (prologue)
        "\tstp\tx29, x30, [sp, #-16]!\n" // x29 ve x30'u yığına kaydet.
        "\tmov\tx29, sp\n" // x29'u yığın işaretçisine ayarla.

        // printf gibi variadic fonksiyonlar için yığında ek alan ayır.
        "\tsub\tsp, sp, #16\n"

        // Yazdırılacak değer bize x0'da geliyor. Onu yığına koymalıyız.
        "\tstr\tx0, [sp]\n"

        // Şimdi format string'inin adresini x0'a yükleyebiliriz.
        "\tadrp\tx0, .L.str@GOTPAGE\n" // Format string'in adresini yükle.
        "\tldr\tx0, [x0, .L.str@GOTPAGEOFF]\n" // Adres ofsetini yükle.

        // printf'i çağır. x0'da format string var, diğer argüman yığında.
        "\tbl\t_printf\n" // printf fonksiyonunu çağır.

        // Standart fonksiyon bitişi (epilogue)
        "\tmov\tsp, x29\n" // Yığın işaretçisini eski haline getir.
        "\tldp\tx29, x30, [sp], #16\n" // x29 ve x30'u yığından geri yükle.
        "\tret\n" // Fonksiyondan dön.

        "\n"
        "\t.globl\t_main\n" // _main fonksiyonunu global olarak görünür yap.
        "\t.p2align 2\n" // 4 bayt hizalama.
        "_main:\n"
        // _main fonksiyonu başlangıcı.
        "\tstp\tx29, x30, [sp, #-16]!\n" // x29 ve x30'u yığına kaydet.
        "\tmov\tx29, sp\n" // x29'u yığın işaretçisine ayarla.
        "\tsub\tsp, sp, #16\n",
        Outfile);
}

// Assembly bitiş kodunu yazdır.
void cgpostamble() {
    fputs(
        "\tmov\tx0, #0\n" // Programın çıkış durumunu 0 olarak ayarla (x0: dönüş değeri kaydedici).
        "\tmov\tsp, x29\n" // Yığın işaretçisini eski haline getir.
        "\tldp\tx29, x30, [sp], #16\n" // x29 ve x30'u yığından geri yükle.
        "\tret\n", // Fonksiyondan dön (main fonksiyonunu sonlandırır).
        Outfile);
}

// Bir tam sayi sabit degerini bir registera yukle.
// Kayitcinin numarasini dondur.
int cgload(int value) 
{
    int reg = alloc_register(); // Bos bir register al.
      // ARM64'te 'mov' komutu, dogrudan sabit degeri bir kayitciya yukleyebilir.
    fprintf(Outfile, "\tmov\t%s, #%d\n", reglist[reg], value);
    return (reg); // Tahsis edilen kayitci numarasini dondur.
}

// Iki tam sayi registerini topla ve sonucu iceren
// registerin numarasini dondur.
int cgadd(int reg1, int reg2) 
{
    // ARM64 'add' komutu: dest = src1 + src2. Sonucu reg2'de tut.
    fprintf(Outfile, "\tadd\t%s, %s, %s\n", reglist[reg2], reglist[reg1], reglist[reg2]);
    free_register(reg1); // reg1'i serbest birak.
    return (reg2); // Sonucu iceren registeri (reg2) dondur.
}

// Ilk registerden ikinciyi cikar ve
// sonucu iceren registerin numarasini dondur.
int cgsub(int reg1, int reg2) {
  // ARM64 'sub' komutu: dest = src1 - src2. Sonucu reg1'de tut.
  fprintf(Outfile, "\tsub\t%s, %s, %s\n", reglist[reg1], reglist[reg1], reglist[reg2]);
  free_register(reg2); // reg2'yi serbest birak.
  return (reg1); // Sonucu iceren registeri (reg1) dondur.
}

// Iki registeri carp ve sonucu iceren
// registerin numarasini dondur.
int cgmul(int reg1, int reg2) {
  // ARM64 'mul' komutu: dest = src1 * src2. Sonucu reg2'de tut.
  fprintf(Outfile, "\tmul\t%s, %s, %s\n", reglist[reg2], reglist[reg1], reglist[reg2]);
  free_register(reg1); // reg1'i serbest birak.
  return (reg2); // Sonucu iceren registeri (reg2) dondur.
}

// Ilk registeri ikinciye bol ve
// sonucu iceren registerin numarasini dondur.
int cgdiv(int reg1, int reg2) {
  // ARM64'te dogrudan isaretli bolme (sdiv) talimati bulunur.
  // dest = src1 / src2. Sonucu reg1'de tut.
  fprintf(Outfile, "\tsdiv\t%s, %s, %s\n", reglist[reg1], reglist[reg1], reglist[reg2]);
  free_register(reg2); // reg2'yi serbest birak.
  return (reg1); // Sonucu iceren registeri (reg1) dondur.
}

// Verilen register ile printint() fonksiyonunu cagir.
void cgprintint(int reg) {
  // ARM64 ABI'ye gore, fonksiyonlara ilk arguman x0 register'ina yerlestirilir.
  fprintf(Outfile, "\tmov\tx0, %s\n", reglist[reg]); // Sonucu iceren register'i x0'a tasi.
  fprintf(Outfile, "\tbl\t_printint\n"); // _printint fonksiyonunu cagir (bl = branch with link).
  free_register(reg); // Register'i serbest birak.
}