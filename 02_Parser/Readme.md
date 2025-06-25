# Bölüm 2: Ayrıştırmaya Giriş

Derleyici yazma yolculuğumuzun bu bölümünde, bir ayrıştırıcının *parser* temellerini tanıtacağım. İlk bölümde bahsettiğim gibi, ayrıştırıcının görevi, girdinin sözdizimini ve yapısal öğelerini tanımak ve bunların dilin dilbilgisine *grammer* uygun olmasını sağlamaktır. Zaten tarayabildiğimiz birkaç dil öğemiz, yani belirteçlerimiz *tokens* var:

* Dört temel matematiksel operatör: \*, /, + ve -
* 1 veya daha fazla 0..9 rakamından oluşan ondalık tam sayılar

Şimdi ayrıştırıcımızın tanıyacağı dil için bir dilbilgisi tanımlayalım.

## BNF: Backus-Naur Formu

Bilgisayar dilleriyle uğraşmaya başlarsanız bir noktada [BNF](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form) kullanımıyla karşılaşacaksınız. Burada, tanımak istediğimiz dilbilgisini ifade etmek için yeterli BNF sözdizimini tanıtacağım.

Tam sayılarla matematiksel ifadeleri ifade edecek bir dilbilgisi istiyoruz. İşte dilbilgisinin BNF açıklaması:

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

Yukarıdaki dilbilgisinde dikey çizgiler ( | ) dilbilgisindeki seçenekleri ayırır, yani yukarıdaki ifade şunları söyler:

* Bir ifade sadece bir "number" olabilir, veya
* Bir ifade, bir '\*' belirteciyle ayrılmış iki ifadedir, veya
* Bir ifade, bir '/' belirteciyle ayrılmış iki ifadedir, veya
* Bir ifade, bir '+' belirteciyle ayrılmış iki ifadedir, veya
* Bir ifade, bir '-' belirteciyle ayrılmış iki ifadedir
* Bir sayı her zaman bir T\_INTLIT belirtecidir

Dilbilgisinin BNF tanımının özyinelemeli *recursive* olduğu oldukça açık olmalı: bir ifade diğer ifadelere başvurularak tanımlanır. Ancak özyinelemeyi "sonlandırmanın" bir yolu var: bir ifade bir sayı olduğunda, bu her zaman bir T\_INTLIT belirtecidir ve dolayısıyla özyinelemeli değildir.

BNF'de, "expression" ve "number"ın, dilbilgisindeki kurallar tarafından üretildikleri için terminal olmayan semboller (*non-terminal*) symbols olduğunu söyleriz. Ancak T\_INTLIT, herhangi bir kural tarafından tanımlanmadığı için bir terminal semboldür (*terminal symbol*) Bunun yerine, dilde zaten tanınmış bir belirteçtir. Benzer şekilde, dört matematik operatör belirteci de terminal sembollerdir.

## Özyinelemeli İnişli Ayrıştırma (*Recursive Descent Parsing*)

Dilimizin dilbilgisinin özyinelemeli olduğu göz önüne alındığında, onu özyinelemeli olarak ayrıştırmayı denemek mantıklıdır. Yapmamız gereken, bir belirteci okumak ve ardından bir sonraki belirtece bakmaktır. Bir sonraki belirtecin ne olduğuna bağlı olarak, girdiyi ayrıştırmak için hangi yolu izlememiz gerektiğine karar verebiliriz. Bu, zaten çağrılmış bir işlevi özyinelemeli olarak çağırmamızı gerektirebilir.

Bizim durumumuzda, herhangi bir ifadedeki ilk belirteç bir sayı olacak ve bunu bir matematik operatörü takip edebilir. Ondan sonra yalnızca tek bir sayı olabilir veya tamamen yeni bir ifadenin başlangıcı olabilir. Bunu özyinelemeli olarak nasıl ayrıştırabiliriz?

Şöyle görünen bir sözde kod yazabiliriz:

```
function expression() {
  Ilk belirtecin sayi olup olmadigini tara ve kontrol et. Degilse hata ver.
  Sonraki belirteci al.
  Eger girdinin sonuna ulastaysak, geri don, yani temel durum.

  Aksi takdirde, expression() fonksiyonunu cagir.
}
```

Bu işlevi 2 + 3 - 5 T\_EOF girdisi üzerinde çalıştıralım; burada T\_EOF, girdinin sonunu yansıtan bir belirteçtir. (*expression*) işlevine yapılan her çağrıyı numaralandıracağım.

```
expression0:
  2'yi tarar, bir sayıdır
  Sonraki belirteci, +'yı alır, bu T_EOF değildir
  expression() çağrılır

    expression1:
      3'ü tarar, bir sayıdır
      Sonraki belirteci, -'yi alır, bu T_EOF değildir
      expression() çağrılır

        expression2:
          5'i tarar, bir sayıdır
          Sonraki belirteci, T_EOF'u alır 
        expression2'den geri döner
    expression1'den geri döner
expression0'dan geri döner
```

Evet, işlev 2 + 3 - 5 T\_EOF girdisini özyinelemeli olarak ayrıştırabildi.

Elbette, girdiyle hiçbir şey yapmadık, ancak bu ayrıştırıcının işi değil. Ayrıştırıcının işi girdiyi tanımak ve herhangi bir sözdizimi hatası hakkında uyarmaktır. Girdinin anlamsal analizini, yani bu girdinin anlamını anlama ve gerçekleştirme işini başkası yapacaktır.

> Daha sonra, bunun aslında doğru olmadığını göreceksiniz. Sözdizimi analizi ile anlamsal analizi birlikte yapmak genellikle daha mantıklıdır.

## Soyut Sözdizimi Ağaçları (*Abstract Syntax Trees*)

Anlamsal analizi yapmak için, tanınan girdiyi ya yorumlayan ya da başka bir formata, örneğin assembly koduna çeviren koda ihtiyacımız var. Yolculuğun bu bölümünde, girdi için bir yorumlayıcı (*interpreter*) inşa edeceğiz. Ancak oraya ulaşmak için önce girdiyi, AST olarak da bilinen bir [soyut sözdizimi ağacına (*abstract syntax tree*)](https://en.wikipedia.org/wiki/Abstract_syntax_tree) dönüştüreceğiz.

İnşa edeceğimiz AST'deki her düğümün yapısı `defs.h` dosyasında açıklanmıştır:

```
// AST dugum turleri
enum {
  A_ADD,        // Toplama islemi
  A_SUBTRACT,   // Cikarma islemi
  A_MULTIPLY,   // Carpma islemi
  A_DIVIDE,     // Bolme islemi
  A_INTLIT      // Tam sayi literafi
};

// Soyut Sozdizimi Agaci yapisi
struct ASTnode {
  int op;                               // Bu agac uzerinde yapilacak "islem"
  struct ASTnode *left;                 // Sol ve sag cocuk agaclar
  struct ASTnode *right;
  int intvalue;                         // A_INTLIT icin tam sayi degeri
};
```

A\_ADD ve A\_SUBTRACT `op` değerlerine sahip düğümler gibi bazı AST düğümleri, `left` ve `right` ile işaret edilen iki çocuk AST'ye sahiptir. Daha sonra, alt ağaçların değerlerini toplayacak veya çıkaracağız.

Alternatif olarak, `A_INTLIT` `op` değerine sahip bir AST düğümü, bir tam sayı değerini temsil eder. Hiçbir alt ağaç çocuğu yoktur, yalnızca `intvalue` alanında bir değeri vardır.

## AST Düğümleri ve Ağaçları Oluşturma

`tree.c` dosyasındaki kod, AST'leri inşa etmek için işlevlere sahiptir. En genel işlev olan `mkastnode()`, bir AST düğümündeki dört alanın tümü için değerler alır. Düğümü tahsis eder, alan değerlerini doldurur ve düğüme bir işaretçi döndürür:

```
// Genel bir AST dugumu olustur ve geri dondur
struct ASTnode *mkastnode(int op, struct ASTnode *left,
                          struct ASTnode *right, int intvalue) {
  struct ASTnode *n;

  // Yeni bir AST dugumu icin malloc yap
  n = (struct ASTnode *) malloc(sizeof(struct ASTnode));
  if (n == NULL) {
    fprintf(stderr, "Unable to malloc in mkastnode()\n");
    exit(1);
  }
  // Alan degerlerini kopyala ve geri dondur
  n->op = op;
  n->left = left;
  n->right = right;
  n->intvalue = intvalue;
  return (n);
}
```

Bu durum göz önüne alındığında, bir yaprak AST düğümü (yani hiç çocuğu olmayan bir düğüm) ve tek çocuklu bir AST düğümü oluşturan daha özel işlevler yazabiliriz:

```
// Bir AST yaprak dugumu yap
struct ASTnode *mkastleaf(int op, int intvalue) {
  return (mkastnode(op, NULL, NULL, intvalue));
}

// Tekli bir AST dugumu yap: sadece bir cocugu var
struct ASTnode *mkastunary(int op, struct ASTnode *left, int intvalue) {
  return (mkastnode(op, left, NULL, intvalue));
}
```

## AST'nin Amacı

AST'yi (*Abstract Syntax Tree*) her tanınan ifadeyi depolamak için kullanacağız. Daha sonra bu ağacı özyinelemeli olarak dolaşarak ifadenin nihai değerini hesaplayabileceğiz. Matematiksel operatörlerin öncelik sırasını da dikkate almak istiyoruz. İşte bir örnek:

**2 \* 3 + 4 \* 5** ifadesini ele alalım. Çarpma işlemi toplama işleminden daha yüksek önceliğe sahiptir. Bu nedenle, çarpma işlemlerinin operandlarını birbirine bağlamak ve bu işlemleri toplama işleminden önce gerçekleştirmek istiyoruz.

Eğer AST'yi şu şekilde oluşturursak:

```
          +
         / \
        /   \
       /     \
      *       *
     / \     / \
    2   3   4   5
```

Bu ağacı dolaşırken şu işlemleri yaparız:

1. İlk olarak `2 x 3` işlemini gerçekleştiririz.
2. Daha sonra `4 x 5` işlemini gerçekleştiririz.
3. Bu sonuçları ağacın köküne (`+`) iletiriz ve toplama işlemini yaparız.

Bu yöntem, operatör önceliğini doğru bir şekilde ele alarak ifadeyi doğru sırayla işler. AST'nin amacı, bu tür karmaşık ifadeleri yapılandırılmış bir şekilde temsil etmek ve daha sonra bu yapıyı kullanarak ifadeyi çözümlemektir.

## Basit Bir İfade Ayrıştırıcı

Bu bölümde, ifadeleri ayrıştırmak için basit bir ayrıştırıcı (*parser*) oluşturuyoruz. Ayrıştırıcı, sözdizimini kontrol eder ve bir AST (*Abstract Syntax Tree*) oluşturur. Ancak, operatör önceliğini ele almak gibi daha karmaşık anlamsal analizleri yapmaz.

### Token Değerlerini AST Düğüm Türlerine Dönüştürme

Token değerlerini AST düğüm türlerine dönüştürmek için bir işlev kullanıyoruz. Bu işlev, token değerlerini AST düğüm türlerine eşler (`expr.c`):

```
// Bir tokeni AST islemine donustur
int arithop(int tok) {
  switch (tok) {
    case T_PLUS:
      return (A_ADD);
    case T_MINUS:
      return (A_SUBTRACT);
    case T_STAR:
      return (A_MULTIPLY);
    case T_SLASH:
      return (A_DIVIDE);
    default:
      fprintf(stderr, "unknown token in arithop() on line %d\n", Line);
      exit(1);
  }
}
```

Bu işlevdeki `default` durumu, verilen tokenin bir AST düğüm türüne dönüştürülemediği durumlarda çalışır. Bu, ayrıştırıcıdaki sözdizimi kontrolünün bir parçasını oluşturur.

### Global Token Değişkeni ve scan() İşlevi

Ayrıştırıcı, girdiyi taramak ve her bir belirteci `token` işlemek için bir global `Token` değişkenine dayanır. Bu değişken, şu anda taranmış olan belirteci tutar. `scan()` işlevi ise girdiden bir sonraki belirteci alır ve `Token` değişkenini günceller.

Token değişkeni, `data.h` dosyasında şu şekilde tanımlanmıştır:

```
extern_ struct token Token;
```

Bu, Token değişkeninin başka dosyalarda tanımlandığını ve bu dosyada kullanılabileceğini belirtir. Token değişkeni, tarayıcı (*scanner*) tarafından doldurulur ve ayrıştırıcı (*parser*) tarafından kullanılır.

`main()` fonksiyonunda, ayrıştırıcı çalıştırılmadan önce ilk belirteç taranır ve ardından ifade ayrıştırılır:

```
scan(&Token);                 // Girdiden ilk belirteci al
n = binexpr();                // Dosyadaki ifadeyi ayrıştır
```

### Birincil Faktörleri Ayrıştırma

Bir tam sayı literali (*integer literal*) olup olmadığını kontrol eden ve bu değeri tutan bir AST düğümü oluşturan bir işlev:

```
// Birincil faktoru ayristir ve onu temsil eden bir AST dugumu dondur
static struct ASTnode *primary(void) {
  struct ASTnode *n;

  // INTLIT tokeni icin bir yaprak AST dugumu yap
  // ve sonraki tokeni tara. Diger token turleri icin
  // bir sozdizimi hatasi olustur.
  switch (Token.token) {
    case T_INTLIT:
      n = mkastleaf(A_INTLIT, Token.intvalue);
      scan(&Token);
      return (n);
    default:
      fprintf(stderr, "syntax error on line %d\n", Line);
      exit(1);
  }
}
```

Bu işlev, global bir `Token` değişkeni olduğunu ve bu değişkenin girdiden taranmış en son tokeni içerdiğini varsayar.

### İfade Ayrıştırıcı (*Expression Parser*)

Ayrıştırıcı, bir AST ağacı oluşturur ve kök düğümü bir ikili operatör (*binary operator*) olan bir AST ağacı döndürür:

```
// Koku bir ikili operator olan bir AST agaci dondur
struct ASTnode *binexpr(void) {
  struct ASTnode *n, *left, *right;
  int nodetype;

  // Soldaki tam sayi literali al.
  // Aynı anda sonraki tokeni al.
  left = primary();

  // Token kalmadıysa, sadece sol dugumu dondur
  if (Token.token == T_EOF)
    return (left);

  // Tokeni bir dugum turune donustur
  nodetype = arithop(Token.token);

  // Sonraki tokeni tara
  scan(&Token);

  // Sagdaki agaci ozyinelemeli olarak al
  right = binexpr();

  // Simdi iki alt agacla bir agac olustur
  n = mkastnode(nodetype, left, right, 0);
  return (n);
}
```

### Operatör Önceliği Sorunu

Bu basit ayrıştırıcı, operatör önceliğini dikkate almaz. Tüm operatörleri eşit öncelikli olarak ele alır. Örneğin, `2 * 3 + 4 * 5` ifadesini ayrıştırırken şu AST'yi oluşturur:

```
     *
    / \
   2   +
      / \
     3   *
        / \
       4   5
```

Bu AST, yanlış bir işlem sırasına sahiptir: 4 x 5 = 20 hesaplanır. Ardından 3 + 20 = 23 hesaplanır. Ancak doğru işlem sırası 2 x 3 = 6, ardından 6 + 20 = 26 olmalıdır.

### Neden Bu Yöntemi Kullandık?

Bu basit ayrıştırıcıyı yazmamızın amacı, bir ayrıştırıcı oluşturmanın kolay olduğunu, ancak anlamsal analizi (örneğin, operatör önceliğini ele almak) yapmanın daha zor olduğunu göstermektir. Daha karmaşık bir ayrıştırıcı, operatör önceliğini doğru bir şekilde ele alacak şekilde tasarlanmalıdır.

## Ağacı Yorumlama (*Interpreting the Tree*)

Artık (yanlış olsa da) AST ağacımızı oluşturduğumuza göre, onu yorumlamak için kod yazabiliriz. Bu işlem, ağacı özyinelemeli olarak dolaşarak her düğümdeki işlemi gerçekleştirmeyi içerir. İşte bu sürecin sözde kodu:

### Sözde Kod: `interpretTree`

```
interpretTree:
  1. Önce sol alt ağacı yorumla ve değerini al.
  2. Ardından sağ alt ağacı yorumla ve değerini al.
  3. Ağacın kökündeki düğümdeki işlemi, iki alt ağacın değerleri üzerinde gerçekleştir.
  4. Bu değeri döndür.
```

### Özyinelemeli Kod

Bu kod `interp.c` dosyasında bulunur ve yukarıdaki sözde kodu takip eder:

```
// Verilen bir AST'yi yorumla
// icindeki operatorleri ve bir nihai deger geri dondur.
int interpretAST(struct ASTnode *n) {
  int leftval, rightval;

  // Sol ve sag alt-agac degerlerini al
  if (n->left)
    leftval = interpretAST(n->left);
  if (n->right)
    rightval = interpretAST(n->right);

  switch (n->op) {
    case A_ADD:
      return (leftval + rightval);
    case A_SUBTRACT:
      return (leftval - rightval);
    case A_MULTIPLY:
      return (leftval * rightval);
    case A_DIVIDE:
      return (leftval / rightval);
    case A_INTLIT:
      return (n->intvalue);
    default:
      fprintf(stderr, "Unknown AST operator %d\n", n->op);
      exit(1);
  }
}
```

Yine, `switch` ifadesindeki default durumu, AST düğüm türünü yorumlayamadığımızda tetiklenir. Bu, ayrıştırıcımızdaki anlamsal kontrolün bir parçasını oluşturacaktır.

## Ayrıştırıcıyı (*Parser*) Build Alma

Burada başka kodlar ve `main()` içindeki yorumlayıcıya (*interpreter*) yapılan çağrı gibi kısımlar da bulunmaktadır:

```
  scan(&Token);                 // Girdiden ilk belirteci al
  n = binexpr();                // Dosyadaki ifadeyi ayristir
  printf("%d\n", interpretAST(n));      // Sonuc degeri hesapla
  exit(0);
```

Şimdi ayrıştırıcıyı aşağıdaki komutla derleyebilirsiniz:

```
$ make
cc -o parser -g expr.c interp.c main.c scan.c tree.c
```

Ayrıştırıcıyı test etmeniz için birkaç girdi dosyası sağladım, ancak tabii ki kendi dosyalarınızı da oluşturabilirsiniz. Unutmayın, hesaplanan sonuçlar hatalıdır, ancak ayrıştırıcı ardışık sayılar, ardışık operatörler ve girdinin sonunda eksik bir sayı gibi girdi hatalarını tespit etmelidir. Yorumlayıcıya hangi AST ağaç düğümlerinin hangi sırada değerlendirildiğini görmeniz için bazı hata ayıklama kodu da ekledim:

```
$ cat input01
2 + 3 * 5 - 8 / 3

$ ./parser input01
int 2
int 3
int 5
int 8
int 3
8 / 3
5 - 2
3 * 3
2 + 9
11

$ cat input02
12 +
456 * 2
/ 09

$ ./parser input02
int 12
int 456
int 2
int 9
2 / 9
456 * 0
12 + 0
12
```

## Sonuç ve Sonraki Adımlar

Bir ayrıştırıcı (*parser*), dilin dilbilgisini tanır ve derleyiciye verilen girdinin bu dilbilgisine uygun olup olmadığını kontrol eder. Eğer uygun değilse, ayrıştırıcı bir hata mesajı yazdırmalıdır. İfade dilbilgisimiz özyinelemeli olduğu için, ifadelerimizi tanımak amacıyla özyinelemeli inişli ayrıştırıcı (*recursive descent parser*) yazmayı tercih ettik.

Şu anda ayrıştırıcı, yukarıdaki çıktıda gösterildiği gibi çalışıyor, ancak girdinin anlamsal özelliklerini doğru bir şekilde alamıyor. Diğer bir deyişle, ifadelerin doğru değerini hesaplamıyor.

Derleyici yazma yolculuğumuzun bir sonraki bölümünde, ayrıştırıcıyı, doğru matematiksel sonuçları elde etmek için ifadelerin anlamsal analizini de yapacak şekilde değiştireceğiz.
