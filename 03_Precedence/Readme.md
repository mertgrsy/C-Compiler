# Bölüm 3: Operatör Önceliği

Derleyici yazma yolculuğumuzun bir önceki bölümünde, bir ayrıştırıcının (*parser*) dilimizin anlamsal özelliklerini mutlaka uygulayabildiğini görmüştük. Sadece dilbilgisinin sözdizimi ve yapısal kurallarını uyguluyordu.

`2 \* 3 + 4 \* 5` gibi ifadelerin yanlış değerini hesaplayan bir kodla sonlandırdık, çünkü kod aşağıdaki gibi bir AST oluşturmuştu:

```
     *
    / \
   2   +
      / \
     3   *
        / \
       4   5
```

Ancak bunun yerine şu şekilde oluşturmalıydı:

```
          +
         / \
        /   \
       /     \
      *       *
     / \     / \
    2   3   4   5
```

Bu sorunu çözmek için, operatör önceliğini gerçekleştirmek üzere ayrıştırıcımıza kod eklememiz gerekiyor. Bunu yapmanın (en az) iki yolu vardır:

* Operatör önceliğini dilin dilbilgisinde açıkça belirtmek
* Mevcut ayrıştırıcıyı bir operatör önceliği tablosuyla değiştirmek

## Operatör Önceliğini Açık Hale Getirme

Yolculuğun son bölümündeki dilbilgisimiz şöyleydi:

```
expression: number
          | expression '*' expression
          | expression '/' expression
          | expression '+' expression
          | expression '-' expression
          ;

number:  T_INTLIT
         ;
```

Dört matematiksel operatör arasında hiçbir farklılaşma olmadığına dikkat edin. Şimdi dilbilgisini, bir fark olacak şekilde ayarlayalım:

```
expression: additive_expression
    ;

additive_expression:
      multiplicative_expression
    | additive_expression '+' multiplicative_expression
    | additive_expression '-' multiplicative_expression
    ;

multiplicative_expression:
      number
    | number '*' multiplicative_expression
    | number '/' multiplicative_expression
    ;

number:  T_INTLIT
         ;
```

Şimdi iki tür ifademiz var: *toplamsal* ifadeler (*additive expressions*) ve *çarpımsal* ifadeler (*multiplicative expressions*). Dilbilgisinin artık sayıları yalnızca çarpımsal ifadelerin bir parçası olmaya zorladığını unutmayın. Bu, `*` ve `/` operatörlerini her iki taraftaki sayılara daha sıkı bağlamaya zorlar, böylece daha yüksek önceliğe sahip olurlar.

Herhangi bir toplamsal ifade aslında kendi başına bir çarpımsal ifade veya bir `+` veya `-` operatörü ve ardından başka bir çarpımsal ifade ile takip edilen toplamsal (yani çarpımsal) bir ifadedir. Toplamsal ifade artık çarpımsal ifadeden çok daha düşük bir önceliğe sahiptir.

## Özyinelemeli İnişli Ayrıştırıcıda Yukarıdakileri Yapmak

Dilbilgimizin yukarıdaki versiyonunu alıp özyinelemeli inişli ayrıştırıcımıza nasıl uygulayabiliriz? Bunu `expr2.c` dosyasında yaptım ve kodu aşağıda ele alacağım.

Cevap, `*` ve `/` operatörleriyle ilgilenmek için bir `multiplicative_expr()` işlevine ve daha düşük öncelikli `+` ve `-` operatörleriyle ilgilenmek için bir `additive_expr() `işlevine sahip olmaktır.

Her iki işlev de bir şeyi ve bir operatörü okuyacak. Daha sonra, aynı öncelikte takip eden operatörler varken, her işlev girdinin biraz daha fazlasını ayrıştıracak ve sol ve sağ yarıları ilk operatörle birleştirecektir.

Ancak, `additive_expr()` işlevi daha yüksek öncelikli `multiplicative_expr()` işlevine başvurmak zorunda kalacaktır. İşte bunun nasıl yapıldığı:

### `additive_expr()`

```
// Koku bir '+' veya '-' ikili operator olan bir AST agaci dondur
struct ASTnode *additive_expr(void) {
  struct ASTnode *left, *right;
  int tokentype;

  // Sol alt-agaci bizden daha yuksek bir oncelikte al
  left = multiplicative_expr();

  // Eger token kalmadiysa, sadece sol dugumu dondur
  tokentype = Token.token;
  if (tokentype == T_EOF) return (left);

  // Kendi oncelik seviyemizdeki token uzerinde dongu
  while (1) {
    // Sonraki tam sayi literalini al
    scan(&Token);

    // Sag alt-agaci bizden daha yuksek bir oncelikte al
    right = multiplicative_expr();

    // Iki alt-agaci dusuk oncelikli operatorumuzle birlestir
    left = mkastnode(arithop(tokentype), left, right, 0);

    // Ve kendi onceligimizdeki sonraki tokeni al
    tokentype = Token.token;
    if (tokentype == T_EOF) break;
  }

  // Olusturdugumuz agaci dondur
  return (left);
}
```

En başta, ilk operatör yüksek öncelikli bir `*` veya `/` olması durumunda hemen multiplicative\_expr() işlevini çağırırız. Bu işlev, yalnızca düşük öncelikli bir `+` veya `-` operatörüyle karşılaştığında geri dönecektir.

Böylece, while döngüsüne girdiğimizde, bir `+` veya `-` operatörümüz olduğunu biliriz. Döngüyü, girdide token kalmayana kadar, yani T\_EOF tokenine ulaşana kadar sürdürürüz.

Döngünün içinde, gelecekteki operatörlerin bizden daha yüksek öncelikli olması ihtimaline karşı `multiplicative_expr()` işlevini tekrar çağırırız. Yine, bu, operatörler bizden daha yüksek öncelikli olmadığında geri dönecektir.

Sol ve sağ alt ağaçlara sahip olduğumuzda, bunları döngünün son turunda aldığımız operatörle birleştirebiliriz. Bu tekrarlar, böylece `2 + 4 + 6` gibi bir ifadeye sahip olsaydık, sonuç olarak şu AST ağacına sahip olurduk:

```
       +
      / \
     +   6
    / \
   2   4
```

Eğer ki `multiplicative_expr()` kendi daha öncelikli operatöre sahip olsaydı elimizdeki çok node'a sahip alt ağaçları birleştiriyor olacaktık.

### multiplicative_expr()

```
// Koku bir '*' veya '/' ikili operator olan bir AST agaci dondur
struct ASTnode *multiplicative_expr(void) {
  struct ASTnode *left, *right;
  int tokentype;

  // Soldaki tam sayi literalini al.
  // Ayni anda sonraki tokeni de al.
  left = primary();

  // Eger token kalmadiysa, sadece sol dugumu dondur
  tokentype = Token.token;
  if (tokentype == T_EOF) return (left);

  // Token bir '*' veya '/' oldugu surece
  while ((tokentype == T_STAR) || (tokentype == T_SLASH)) {
    // Sonraki tam sayi literalini al
    scan(&Token);
    right = primary();

    // Bunu soldaki tam sayi dugumu ile birlestir
    left = mkastnode(arithop(tokentype), left, right, 0);

    // Mevcut tokenin detaylarini guncelle.
    // Eger token kalmadiysa, sadece sol dugumu dondur
    tokentype = Token.token;
    if (tokentype == T_EOF) break;
  }

  // Olusturdugumuz agaci dondur
  return (left);
}
```

## Yukarıdaki Kodun Analizi

Kod, `additive_expr()` işlevine benzer, ancak gerçek tam sayı literallerini almak için `primary()` işlevini çağırmamız fark yaratır. Ayrıca, yalnızca yüksek öncelikli `*` ve `/` operatörlerimiz olduğunda döngüye devam ederiz. Düşük öncelikli bir operatöre rastladığımızda, o noktaya kadar oluşturduğumuz alt ağacı basitçe geri döndürürüz. Bu, düşük öncelikli operatörle ilgilenmek üzere `additive_expr()` işlevine geri döner.

## Yukarıdaki Yöntemin Dezavantajları

Operatör önceliğinin açıkça belirtildiği bir özyinelemeli inişli ayrıştırıcı (*recursive descent parser*) inşa etmenin yukarıdaki yolu, doğru öncelik seviyesine ulaşmak için gereken tüm işlev çağrıları nedeniyle verimsiz olabilir. Ayrıca, her bir operatör önceliği seviyesiyle ilgilenmek için işlevler olması gerekir, bu da çok sayıda kod satırı oluşmasına neden olur.

## Alternatif: Pratt Ayrıştırma (*Pratt Parsing*)

Kod miktarını azaltmanın bir yolu, dilbilgisinde açık önceliği yineleyen işlevlere sahip olmak yerine, her belirteçle *token* ilişkili öncelik değerlerinin bir tablosuna sahip olan bir [Pratt ayrıştırıcı](https://en.wikipedia.org/wiki/Pratt_parser) kullanmaktır.

## `expr.c`: Pratt Ayrıştırma

`expr2.c` yerine doğrudan kullanılabilen Pratt Ayrıştırmayı `expr.c` koduna uyguladım. Şimdi bu yöntemi incelemeye başlayalım.

İlk olarak, her belirteç *token* için öncelik seviyelerini belirlemek üzere bazı kodlara ihtiyacımız var:

```
// Her token icin operator onceligi
static int OpPrec[] = { 0, 10, 10, 20, 20,    0 };
//                     EOF  +   -   *   /  INTLIT

// Ikili operator olup olmadigini kontrol et ve
// onceligini dondur.
static int op_precedence(int tokentype) {
  int prec = OpPrec[tokentype];
  if (prec == 0) {
    fprintf(stderr, "sozdizimi hatasi, satir %d, token %d\n", Line, tokentype);
    exit(1);
  }
  return (prec);
}
```

Daha yüksek sayılar (örneğin 20), daha düşük sayılardan (örneğin 10) daha yüksek öncelik anlamına gelir.

Şimdi sorabilirsiniz: `OpPrec[]` adında bir arama tablonuz varken neden bir fonksiyona ihtiyacınız var? Cevap: sözdizimi hatalarını tespit etmek için.

`234 101 + 12` gibi bir girdiyi düşünün. İlk iki belirteci tarayabiliriz. Ancak, ikinci 101 belirtecinin önceliğini almak için sadece `OpPrec[]` kullanırsak, bunun bir operatör olmadığını fark etmeyiz. Dolayısıyla, `op\_precedence()` işlevi doğru dilbilgisi sözdizimini zorlar.

Şimdi, her öncelik seviyesi için bir fonksiyona sahip olmak yerine, operatör öncelikleri tablosunu kullanan tek bir ifade (*expression*) fonksiyonumuz var:

```
// Koku ikili operator olan bir AST agaci dondur.
// ptp parametresi onceki tokenin onceligi.
struct ASTnode *binexpr(int ptp) {
  struct ASTnode *left, *right;
  int tokentype;

  // Soldaki tam sayi literalini al.
  // Ayni anda sonraki tokeni de al.
  left = primary();

  // Eger token kalmadiysa, sadece sol dugumu dondur
  tokentype = Token.token;
  if (tokentype == T_EOF) return (left);

  // Mevcut tokenin onceligi,
  // onceki token onceliginden daha buyuk oldugu surece
  while (op_precedence(tokentype) > ptp) {
    // Sonraki tam sayi literalini al
    scan(&Token);

    // Tokenimizin onceligiyle bir alt agac olusturmak icin
    // binexpr() fonksiyonunu ozyinelemeli olarak cagir
    right = binexpr(OpPrec[tokentype]);

    // Bu alt agaci bizimkiyle birlestir. 
    // Tokeni de aynı anda bir AST islemine donustur.
    left = mkastnode(arithop(tokentype), left, right, 0);

    // Mevcut tokenin detaylarini guncelle.
    // Eger token kalmadiysa, sadece sol dugumu dondur
    tokentype = Token.token;
    if (tokentype == T_EOF) return (left);
  }

  // Oncelik ayni veya daha dusuk oldugunda olusturdugumuz agaci dondur
  return (left);
}
```

Öncelikle, bunun önceki ayrıştırıcı fonksiyonları gibi hala özyinelemeli olduğuna dikkat edin. Bu sefer, çağrılmadan önce bulunan belirtecin öncelik seviyesini alırız. `main()` bizi en düşük öncelik olan 0 ile çağıracak, ancak biz kendimizi daha yüksek değerlerle çağıracağız.

Kodun `multiplicative_expr()` fonksiyonuna oldukça benzediğini de fark etmelisiniz: bir tam sayı literalini oku, operatörün belirteç türünü al, sonra bir ağaç oluşturarak döngüye gir.

Fark, döngü koşulu ve gövdesindedir:

```
multiplicative_expr():
  while ((tokentype == T_STAR) || (tokentype == T_SLASH)) {
    // Sonraki tam sayi literalini al
    scan(&Token);
    right = primary();

    // Soldaki tam sayi dugumu ile birlestir
    left = mkastnode(arithop(tokentype), left, right, 0);

    // Mevcut tokenin detaylarini guncelle.
    // Eger token kalmadiysa, sol dugumu dondur
    tokentype = Token.token;
    if (tokentype == T_EOF) return (left);
  }

binexpr():
  // Bu tokenin onceligi, onceki token onceliginden daha buyuk oldugu surece
  while (op_precedence(tokentype) > ptp) {
    // Sonraki tam sayi literalini al
    scan(&Token);
    // binexpr() fonksiyonunu tokenimizin onceligiyle cagir
    right = binexpr(OpPrec[tokentype]);

    // Alt agaci birlestir ve AST islemine donustur
    left = mkastnode(arithop(tokentype), left, right, 0);

    // Mevcut tokenin detaylarini guncelle.
    // Eger token kalmadiysa, sol dugumu dondur
    tokentype = Token.token;
    if (tokentype == T_EOF) return (left);
  }
```

Pratt ayrıştırıcı ile, bir sonraki operatörün mevcut belirtecimizden daha yüksek bir önceliği olduğunda, sadece `primary()` ile bir sonraki tam sayı literalini almak yerine, operatör önceliğini yükseltmek için kendimizi `binexpr(OpPrec[tokentype])` ile çağırırız.

Kendi öncelik seviyemizde veya daha düşük bir belirtece ulaştığımızda, basitçe:

```
return (left);
```

Bu, ya bizi çağıran operatörden daha yüksek öncelikte birçok düğüm ve operatör içeren bir alt ağaç olacak, ya da bizimle aynı öncelikteki bir operatör için tek bir tam sayı literafi olacaktır.

Şimdi ifade ayrıştırmasını yapmak için tek bir fonksiyonumuz var. Operatör önceliğini zorlamak için küçük bir yardımcı fonksiyon kullanır ve böylece dilimizin anlamsal özelliklerini uygular.

## Her İki Ayrıştırıcıyı (*Parser*)'ı da Kullanıma Sokma

```
$ make parser                                        # Pratt Ayrıştırıcı
cc -o parser -g expr.c interp.c main.c scan.c tree.c

$ make parser2                                       # Oncelik Tirmanma (Precedence Climbing)
cc -o parser2 -g expr2.c interp.c main.c scan.c tree.c
```

Ayrıca, yolculuğumuzun önceki bölümündeki aynı girdi dosyalarıyla her iki ayrıştırıcıyı da test edebilirsiniz:

```
$ make test

(./parser input01; \
 ./parser input02; \
 ./parser input03)

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
int 12
int 456
int 2
456 * 2
int 9
912 / 9
12 + 101
113
make: [test] Error 1 (ignored)

$ make test2
(./parser2 input01; \
./parser2 input02; \
./parser2 input03)

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
int 12
int 456
int 2
456 * 2
int 9
912 / 9
12 + 101
113
make: [test2] Error 1 (ignored)
```

Sonuç ve Sıradaki Adımlar

Sanırım biraz geri çekilip nereye geldiğimize bakmanın tam zamanı. Elimizde şimdi şunlar var:

* dilimizdeki simgeleri tanıyan ve döndüren bir tarayıcı (*scanner*)
* dilbilgimizi tanıyan, syntax hatalarını raporlayan ve bir Soyut Sözdizimi Ağaçları (AST) oluşturan bir ayrıştırıcı (*parser*)
* dilimizin semantiğini uygulayan, ayrıştırıcı için bir öncelik tablosu
* Soyut Sözdizimi Ağacını derinlemesine gezen ve girdideki ifadenin sonucunu hesaplayan bir yorumlayıcı (*interpreter*)

Henüz sahip olmadığımız şey ise bir derleyici. Ama ilk derleyicimizi yapmaya o kadar yakınız ki!

Derleyici yazma yolculuğumuzun bir sonraki bölümünde, yorumlayıcıyı değiştireceğiz. Onun yerine, matematik operatörü içeren her AST düğümü için **M1 Mac işlemci için gerekli instructions** üreten bir çevirici (translator) yazacağız. Ayrıca, üreticinin çıkardığı kodu desteklemek için bazı başlangıç (*preamble*) ve bitiş (*postamble*) kodları da üreteceğiz. [Siradaki adim](../04_Assembly/Readme.md)
