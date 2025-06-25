# Part 1: Introduction to Lexical Scanning

**Lexical Scanning** (veya **Lexical Analysis**), bir derleyicinin ilk aşamasıdır ve bir programın kaynak kodunu analiz ederek onu daha küçük parçalara, yani **token**'lara ayırır. Bu işlem, derleyicinin girdi kodunu anlaması ve sonraki aşamalarda (örneğin, parsing ve semantic analysis) kullanması için gereklidir.

## Lexical Scanning'in Amacı

Lexical scanning'in temel amacı, kaynak koddaki karakterleri bir dizi **token**'a dönüştürmektir. **Token**'lar, dilin en küçük anlamlı birimleridir ve şunları içerebilir:

* **Tanımlayıcılar**: Değişken ve fonksiyon isimleri (x, y, myFunction).
* **Anahtar kelimeler**: if, else, while, return gibi.
* **Sabitler**: Sayılar (42, 3.14) veya karakterler ('a', "hello").
* **Operatörler**: +, -, \*, /, = gibi.
* **Ayırıcılar**: ;, {, }, (, ) gibi.

Lexical scanning, bu birimleri tanımlayarak dilin gramerine uygun olup olmadığını kontrol eder ve derleyicinin sonraki aşamalarında kullanılmak üzere bir **token listesi** oluşturur.

## Basit Lexical Scanner

Derleyici yazma yolculuğumuza basit bir lexical tarayıcı ile başlıyoruz. Bir önceki bölümde bahsettiğim gibi, tarayıcının görevi giriş dilindeki leksik öğeleri veya *token*ları tanımlamaktır.

Sadece 5 leksik öğeyle başlıyoruz:

* Dört temel matematiksel operatör: `*`,`/`, `+` ve `-`
* Bir veya daha fazla `0` ile `9` arası rakamdan oluşan ondalık tam sayılar

Taradığımız her token, bu yapıda (defs.h dosyasından) saklanacaktır:

```
// Token yapısı
struct token {
  int token;
  int intvalue;
};
```

Burada `token` alanı şu değerlerden biri olabilir (defs.h dosyasından):

```
// Tokenlar
enum {
  T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT
};
```

Token bir `T_INTLIT` (yani bir tam sayı) olduğunda, intvalue alanı taradığımız tam sayının değerini tutacaktır.

## `scan.c` Fonksiyonları

`scan.c` dosyası, leksik tarayıcımızın fonksiyonlarını barındırır. Girdi dosyamızdan her seferinde bir karakter okuyacağız. Ancak, girdi akışında çok ileri okuduğumuzda bir karakteri "geri koymamız" gereken zamanlar olacaktır. Ayrıca hata ayıklama mesajlarımızda satır numarasını yazdırabilmek için o anki satırı takip etmek istiyoruz. Bütün bunlar `next()` fonksiyonu tarafından yapılır:

```
// Girdi dosyasından bir sonraki karakteri alır.
static int next(void) {
  int c;

  if (Putback) {                // Geri konmuş bir karakter varsa,
    c = Putback;                // onu kullan.
    Putback = 0;
    return c;
  }

  c = fgetc(Infile);            // Girdi dosyasından oku
  if ('\n' == c)
    Line++;                     // Satır sayacını artır
  return c;
}
```

`Putback` ve `Line` değişkenleri, girdi dosya işaretçimizle birlikte `data.h` dosyasında tanımlanmıştır:

```
extern_ int     Line;
extern_ int     Putback;
extern_ FILE    *Infile;
```

Tüm C dosyaları, `extern\_` yerine `extern` gelecek şekilde bu dosyayı içerecektir. Ancak `main.c`, `extern_` ifadesini kaldıracak; bu sayede bu değişkenler `main.c'ye "ait" olacaktır. Son olarak, bir karakteri girdi akışına nasıl geri koyarız?:

```
// İstenmeyen bir karakteri geri koyar
static void putback(int c) {
Putback = c;
}
```

### Boşlukları Yoksayma

Girdi akışında boşluk karakterlerini okuyup sessizce atlayarak, boşluk olmayan bir karaktere ulaşana kadar devam eden ve o karakteri döndüren bir fonksiyona ihtiyacımız var. İşte o fonksiyon:

```
// İlgilenmemiz gerekmeyen girdileri (yani boşluk, yeni satır) atlar.
// İlgilenmemiz gereken ilk karakteri döndürür.
static int skip(void) {
  int c;

  c = next(); // Bir sonraki karakteri al
  // Boşluk, tab, yeni satır, satır başı veya form besleme karakterleri olduğu sürece
  while (' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c) {
    c = next(); // Bir sonraki karakteri al ve atlamaya devam et
  }
  return (c); // Boşluk olmayan ilk karakteri döndür
}
```

### Token Tarama: `scan()` Fonksiyonu

Artık boşlukları atlayarak karakterleri okuyabiliyoruz; ayrıca bir karakter fazla okuduğumuzda onu geri de koyabiliyoruz. Şimdi ilk leksik tarayıcımızı yazabiliriz:

```
// Girdide bulunan bir sonraki token'ı tarar ve döndürür.
// Token geçerliyse 1, başka token kalmadıysa 0 döndürür.
int scan(struct token *t) {
  int c;

  // Boşlukları atla
  c = skip();

  // Girdi karakterine göre token'ı belirle
  switch (c) {
  case EOF: // Dosya sonu karakteri ise
    return (0); // Token kalmadı, 0 döndür
  case '+': // Toplama operatörü ise
    t->token = T_PLUS; // Token tipini T_PLUS olarak ayarla
    break;
  case '-': // Çıkarma operatörü ise
    t->token = T_MINUS; // Token tipini T_MINUS olarak ayarla
    break;
  case '*': // Çarpma operatörü ise
    t->token = T_STAR; // Token tipini T_STAR olarak ayarla
    break;
  case '/': // Bölme operatörü ise
    t->token = T_SLASH; // Token tipini T_SLASH olarak ayarla
    break;
  default:
    // Yakında buraya daha fazla kod eklenecek
  }

  // Bir token bulduk
  return (1); // 1 döndürerek geçerli bir token bulunduğunu belirt
}
```

Basit tek karakterli token'lar için bu kadar: tanınan her karakteri bir token'a dönüştürüyoruz. Şunu sorabilirsiniz: Neden sadece tanınan karakteri `struct token` içine koymadık? Cevap şu ki, daha sonra `==` gibi çok karakterli token'ları ve `if` ve `while` gibi anahtar kelimeleri tanımamız gerekecek. Bu nedenle, token değerlerinin numaralandırılmış bir listesine sahip olmak işleri kolaylaştıracaktır.

### Tam Sayı Değişmez Değerleri

Aslında, `3827` ve `87731` gibi tam sayı değişmez değerleri de tanımamız gerektiği için bu durumla şimdiden yüzleşmek zorundayız. `switch` ifadesindeki eksik `default` kodu şöyledir:

```
default:
    // Eğer bir rakamsa,
    // değişmez tam sayı değerini tara
    if (isdigit(c)) { // Karakter bir rakam mı?
      t->intvalue = scanint(c); // scanint() fonksiyonu ile tam sayı değerini tara
      t->token = T_INTLIT; // Token tipini T_INTLIT olarak ayarla
      break;
    }

    printf("Tanımlanamayan karakter %c, satır %d\n", c, Line); // Tanımlanamayan karakter hatası
    exit(1); // Programdan çık
  }
```

Bir ondalık rakam karakteriyle karşılaştığımızda, bu ilk karakterle birlikte yardımcı fonksiyon `scanint()`'i çağırırız. Bu fonksiyon, taranan tam sayı değerini döndürecektir. Bunu yapmak için, her karakteri sırayla okuması, geçerli bir rakam olup olmadığını kontrol etmesi ve nihai sayıyı oluşturması gerekir. İşte kod:

```
// Girdi dosyasından bir tam sayı değişmez değeri tarar ve döndürür.
static int scanint(int c) {
  int k, val = 0;

  // Her karakteri bir int değere dönüştür
  while ((k = chrpos("0123456789", c)) >= 0) { // Karakter "0123456789" içinde mi?
    val = val * 10 + k; // Değeri 10 kat artır ve yeni basamağı ekle
    c = next(); // Bir sonraki karakteri al
  }

  // Tam sayı olmayan bir karakterle karşılaştık, onu geri koy.
  putback(c); // Son okunan (tam sayı olmayan) karakteri geri koy
  return val; // Oluşturulan tam sayı değerini döndür
}
```

Sıfır `val` değeriyle başlarız. Her seferinde `0` ile `9` arasındaki bir karakter aldığımızda, `chrpos()` ile bunu bir `int` değere dönüştürürüz. `val` değerini 10 kat büyütür ve sonra bu yeni rakamı ekleriz.

Örneğin, `3`, `2`, `8` karakterleri varsa:

* `val = 0 * 10 + 3`, yani `3`
* `val = 3 * 10 + 2`, yani `32`
* `val = 32 * 10 + 8`, yani `328`

En sonda, `putback(c)` çağrısını fark ettiniz mi? Bu noktada ondalık bir rakam olmayan bir karakter bulduk. Bunu basitçe atamayız, ancak şansımıza daha sonra tüketilmek üzere girdi akışına geri koyabiliriz.

Bu noktada şunu da sorabilirsiniz: Neden sadece `c`'den `'0'` ASCII değerini çıkararak onu bir tam sayıya dönüştürmedik? Cevap, daha sonra onaltılık basamakları da dönüştürmek için `chrpos("0123456789abcdef")` yapabileceğiz.

`chrpos()`'un kodu burada:

```
// Karakter c'nin s dizisindeki konumunu döndürür
// veya c bulunamazsa -1 döndürür
static int chrpos(char *s, int c) {
  char *p;

  p = strchr(s, c); // s dizisinde c karakterini ara
  return (p ? p - s : -1); // Bulunursa konumunu, bulunamazsa -1 döndür
}
```

Şimdilik `scan.c` dosyasındaki leksik tarayıcı kodu bu kadar.

## Tarayıcıyı Çalıştırma

`main.c` dosyasındaki kod, yukarıdaki tarayıcıyı çalıştırır. `main()` fonksiyonu bir dosyayı açar ve ardından token'lar için tarar:

```
void main(int argc, char *argv[]) {
  // ... (diğer başlangıç kodları)
  init(); // Başlangıç ayarlarını yapar
  // ... (diğer kodlar)
  Infile = fopen(argv[1], "r"); // Komut satırı argümanı olarak verilen dosyayı okuma modunda açar
  // ... (diğer hata kontrolleri)
  scanfile(); // Dosyayı tarayarak token'ları işler
  exit(0); // Programı başarıyla sonlandırır
}
```

Ve `scanfile()` fonksiyonu, yeni bir token olduğu sürece döngüye girer ve token'ın ayrıntılarını yazdırır:

```
// Yazdırılabilir token'ların listesi
char *tokstr[] = { "+", "-", "*", "/", "intlit" };

// Girdi dosyasındaki tüm token'ları tarayan döngü.
// Bulunan her token'ın ayrıntılarını yazdırır.
static void scanfile() {
  struct token T; // Bir token yapısı tanımla

  while (scan(&T)) { // scan() fonksiyonu bir token döndürdüğü sürece (yani dosya sonu değilse)
    printf("Token %s", tokstr[T.token]); // Token tipini yazdır (örn. "+", "intlit")
    if (T.token == T_INTLIT) // Eğer token bir tam sayı değişmezi ise
      printf(", value %d", T.intvalue); // Değerini yazdır
    printf("\n"); // Yeni satıra geç
  }
}
```

## Bazı Örnek Girdi Dosyaları

Tarayıcının her bir dosyada hangi token'ları bulduğunu ve hangi girdi dosyalarını reddettiğini görebilmeniz için bazı örnek girdi dosyaları sağladım.

İlk olarak, kodu derliyoruz:

```
$ make
cc -o scanner -g main.c scan.c

$ cat input01
2 + 3 * 5 - 8 / 3

$ ./scanner input01
Token intlit, deger 2
Token +
Token intlit, deger 3
Token *
Token intlit, deger 5
Token -
Token intlit, deger 8
Token /
Token intlit, deger 3

$ cat input03
44234 - 34
* 67.4 + 32 

$ ./scanner input03
Token intlit, deger 44234
Token -
Token intlit, deger 34
Token *
Token intlit, deger 67
Tanimlanamayan karakter ., satir 2
```

## Sonuç ve Sıradaki Adımlar

Küçük adımlarla başladık ve dört temel matematik operatörünü ile tam sayı değişmez değerlerini tanıyabilen basit bir leksik tarayıcı geliştirdik. Girdiyi çok fazla okuduğumuzda boşlukları atlamanın ve karakterleri geri koymanın gerekli olduğunu gördük.

Tek karakterli token'ları taramak kolaydı, ancak çok karakterli token'lar biraz daha zorlayıcı olacaktır. Nihayetinde, `scan()` fonksiyonumuz, girdi dosyasından bir sonraki token'ı struct token yapısı içinde döndürüyor:

```
struct token {
  int token;
  int intvalue;
};
```

Derleyici yazma yolculuğumuzun bir sonraki bölümünde, girdi dosyalarımızın dilbilgisini (gramerini) yorumlamak ve her dosya için nihai değeri hesaplayıp ekrana yazdırmak üzere özyinelemeli bir ayrıştırıcı (recursive descent parser) inşa edeceğiz.

[Bir Sonraki Adım](https://github.com/mertgrsy/C-Compiler/02_Parser/Readme.md)
