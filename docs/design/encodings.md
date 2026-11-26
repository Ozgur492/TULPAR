# TULPAR bayt düzeyi kodlamalar: öneri

**Durum:** TASLAK, onay bekliyor (Özgür Can Güngör, Cemre Karabulut). Onaylanmadan bu belgeye göre kod yazılmaz.
**İş:** TULPAR-160, "Freeze byte-level encodings of records, bundles and messages".
**Kaynak:** RSD v1.1, Tablo 4.26 (Bölüm 4.9), Tablo 2.1, Bölüm 4.3, 4.4, 4.5, 4.6, 4.8, 5.1, 6.5.

RSD bayt düzeyini bilerek Faz 1'e bırakır: *"Field sizes are given where the architecture gives them; byte-level encodings are frozen in Phase 1."* (Tablo 4.26 açıklaması). Bu belge o dondurmanın önerisidir.

## 1. İşaretler

| İşaret | Anlamı |
|---|---|
| ✔ | Boyut Tablo 4.26'da açıkça verilmiş. |
| † | Boyut RSD'nin başka bir kuralından türetilmiş; kaynak satırda yazılı. Onay gerekir. |
| öneri | RSD boyut vermiyor; bu belgenin önerisi. Onay gerekir. |
| ek alan | Tablo 4.26'da yok ama başka bir RSD gereksinimi bu alanı zorunlu kılıyor. |

Her öneri satırında seçilen genişliğin alternatifi ve neden seçilmediği yazılıdır.

## 2. Kanonik kodlama kuralları

Bu kurallar her kayıt, demet ve çerçeve için geçerlidir. Kanonik kodlamayı ihlal eden imzalı bir kayıt G3 kanıtıdır (RSD-CMC-02a). Bu yüzden her kural, ihlal edildiğinde "malformed" sonucunu veren kesin bir denetime karşılık gelir.

| No | Kural (öneri) | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|
| K1 | Bütün tamsayılar big-endian (ağ bayt sırası) yazılır. | ns-3 `Buffer::Iterator::WriteHtonU*` kuralı; mevcut `HelloHeader` bu kuralı kullanıyor. | Little-endian: x86 için doğal olurdu, ama ns-3 başlık kuralından ayrılır ve PHY izlerinin okunmasını zorlaştırır. |
| K2 | Yalnızca sabit genişlikli tamsayılar kullanılır (u8, u16, u32, u64; konum için i32). Değişken uzunluklu tamsayı (varint) yoktur. | Bir değerin tek bir kodlaması olur; kanonik olmayan varint biçimleri (ör. fazladan sıfır baytlar) sorunu ortadan kalkar (RSD-CMC-02a). | LEB128 varint: birkaç bayt kazandırır, ama her alana "en kısa biçim" denetimi ekler ve sabit boyut sınırlarını (RSD-CMC-01c) zorlaştırır. |
| K3 | Kayan nokta yoktur. Sürekli büyüklükler, belirtilen ölçekle tamsayıya nicemlenir. Nicemleme üreticide bir kez yapılır; alıcı yeniden nicemlemez. | RSD-EVD-03b: "platform-dependent rounding" yasak. Olay günlüğü de tamsayı nanosaniye kullanır. | IEEE-754 binary32/64: NaN, -0 ve birden fazla bit deseni kanonikliği bozar. |
| K4 | Nicemleme `q = llround(x * ölçek)` ile yapılır (yarım değerler sıfırdan uzağa yuvarlanır), sonra alanın aralığına doyurulur (saturate). | `std::llround`, IEEE-754 double için her platformda aynı sonucu verir. | `floor`: negatif koordinatlarda sistematik sapma yaratır. |
| K5 | Listeler önce bir sayı alanı (u8; gerekiyorsa u16), sonra öğeler. Her liste türünün en büyük öğe sayısı sabittir ve aşılması "malformed" demektir. | RSD-CMC-01c: "Every ingress parser shall enforce fixed size limits on its input." | Sonlandırıcı (sentinel) değer: sınır denetimini yalnızca ayrıştırma sonunda mümkün kılar. |
| K6 | Küme anlamındaki listeler (parents, common-cause tags, parent epochs) kesin artan sırada kodlanır; tekrar yasaktır. | Aynı küme tek bir bayt dizisine kodlanır; tanımlayıcı içerikten türetilir (RSD-EVD-02a). | Üretici sırası: aynı içerik iki farklı tanımlayıcı üretir. |
| K7 | Değişken uzunluklu bayt dizileri u16 uzunluk önekiyle yazılır; gömülü kanıt (embedded proof) u32 uzunluk önekiyle. | Bir kayıt 65 535 baytı geçmez. Bir eşdeğerlilik (equivocation) kanıtı iki demet içerir (yaklaşık 11 kB, Bölüm 4.10'daki "G3 proof of about 10.5 kB"). | Her yerde u32: kayıt başına 2 bayt gereksiz. |
| K8 | İsteğe bağlı alan: 1 baytlık varlık işareti (0 veya 1), ardından değer yalnızca işaret 1 ise. 0 ve 1 dışındaki değerler malformed. | Tek kodlama. | "Tüm sıfırlar yok demektir": geçerli bir değerle karışabilir ve hash alanında 48 bayt boşa harcar. |
| K9 | Sıralı türler (enum) u8'dir; tanımsız bir değer malformed. | RSD'deki kapalı kümeler (RSD-HOT-02a, RSD-OBS-03a). | u16: kapalı kümeler 256 değerin çok altında. |
| K10 | Roster kümeleri bit eşlemi olarak yazılır: uzunluk ceil(N/8) bayt (N: provizyon paketindeki roster büyüklüğü), bit i = roster indeksi i, her baytta en anlamlı bit önce. Kullanılmayan dolgu bitleri 0 olmalı. | En fazla 20 düğüm (Bölüm 1.4); N=20 için 3 bayt. Digest bütçesi yaklaşık 370 B (Tablo 4.29). | u16 kimlik listesi: 20 düğümde 41 bayta kadar çıkar. |
| K11 | Her kodlama tam olarak tüketilir. Sonda fazla bayt varsa ya da beklenen uzunluk tutmuyorsa girdi reddedilir. | Bir bayt dizisinin tek bir ayrıştırması olur (RSD-CMC-02b: her derlemede aynı karar). | Fazla baytı yok saymak: aynı kayıt için birden fazla bayt dizisi oluşur ve imzalı eşdeğerlilik tespiti bozulur. |
| K12 | Hash ve imza girdilerinin önüne ASCII bir alan etiketi eklenir, ör. `"TULPAR/1/record"`, `"TULPAR/1/bundle"`, `"TULPAR/1/hello"`. | Bir kaydın imzası bir çerçeve ya da el sıkışma imzası olarak yeniden kullanılamaz. | Etiketsiz: türler arası karışma (cross-protocol) riskini kodlama düzeninin şansına bırakır. |
| K13 | Kayıt tanımlayıcısı = SHA-384(`"TULPAR/1/record"` ‖ tanımlayıcı dışındaki alanların kanonik kodlaması). | RSD-EVD-02a: "the SHA-384 hash of the canonical encoding of all its other fields". | — (RSD tarafından belirlenmiş). |

## 3. Ortak tipler

| Tip | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| NodeId (origin, sender, vehicle, producer) | 2 | u16, roster indeksi; 0xFFFF ayrılmış | öneri | Araç kimliği LSC beacon'ında 2 B (RSD-SAF-02d); mevcut `MeshHello::sender` u16. 0xFFFF yayın adresi ve "headquarters" için ayrılır. | u8: 20 düğüme yeter, ama beacon'la tutarsız olur ve Bölüm 6.6'daki büyük filo gelecek işini kapatır. |
| Hash48 | 48 | bayt dizisi | † | SHA-384 özeti 48 B (Tablo 2.1, RSD-CRY-01d). | — |
| RecordId | 48 | Hash48 | † | RSD-EVD-02a. | 16 B kesik: tanımlayıcı içerik adreslemesidir; doğum günü çakışması saldırısına açık olur. |
| Signature | 4627 | bayt dizisi | ✔ / † | ML-DSA-87 imzası 4627 B (Tablo 2.1, Tablo 4.26, RSD-CRY-01b). | — |
| MlDsaPublicKey | 2592 | bayt dizisi | † | Tablo 2.1. | — |
| MlKemEncapsKey, MlKemCiphertext | 1568 | bayt dizisi | ✔ / † | Tablo 2.1, RSD-CRY-01e. | — |
| GcmTag | 16 | bayt dizisi | ✔ | Tablo 2.1, Tablo 4.26 (overlay header "16-byte tag"). | — |
| Window | 4 | u32, görev başından itibaren pencere indeksi | öneri | Pencere 5 s (Tablo 1.7); referans görev 30 dk = 360 pencere, ama görev süresini görev yapılandırması belirler. | u16: 91 saate kadar yeter, ama görev süresi RSD'de sınırlanmamış; 2 bayt farkı ömür boyu bir sarmalanma (wrap) denetiminden ucuz. |
| SimTime | 8 | u64, simülasyon zamanı, tamsayı nanosaniye | öneri | Olay günlüğü kuralı (CLAUDE.md: "integer nanoseconds"); RSD-SIM-01. | u32 milisaniye: daha küçük, ama sistemde ikinci bir zaman birimi açar. |
| EpochId | 6 | u16 üretici NodeId ‖ u32 o üreticinin epoch announcement kaydının record sequence numarası | öneri | Bölünmüş bileşenlerde iki üretici aynı anda epoch üretebilir; kimlik küresel olarak tekil olmalı (RSD-CRY-03h ebeveyn epoch karşılaştırması). Aynı (üretici, sıra) çifti iki farklı kayıtta görülürse bu zaten eşdeğerliliktir (G3, RSD-SEC-01d). Başlangıç epoch'u (provizyon) = 0xFFFF ‖ 0. | u32 sayaç: iki bileşen aynı numarayı üretir. 48 B RecordId: her kayıt başlığında ve HELLO'da 42 B fazla. 8 B hash öneki: içerik öğütülerek 2^32 işlemde çakıştırılabilir. |
| CommandId | 6 | u16 konsol kimliği ‖ u32 konsol sıra numarası | öneri | Tablo 4.26: "command id (console, console sequence)". | u16 sıra: tek konsolun ömrü boyunca yeterli olabilir, ama sarmalanma RSD-HOT-06g tekilleştirmesini bozar. |
| CertIndex | 2 | u16, provizyon paketindeki sertifika listesinde indeks | öneri | Provizyon paketi görev boyunca sabit ve imzalı (RSD-ORG-05a). | 48 B sertifika hash'i: tekil ama referans başına 46 B fazla; operatör komutunda iki kez yer alır. |
| Position | 12 | 3 × i32: x doğu, y kuzey, z yukarı; görev yapılandırmasındaki başlangıç noktasına göre yerel ENU; santimetre | öneri | LSC beacon'ında "position and altitude (12 B)" (RSD-SAF-02d), yani 3 × 4 B. ns-3 MobilityModel kartezyen metre verir. i32 cm aralığı ±21 474 km; Marmara alanı yaklaşık 17 × 10 km. M8 kinematik denetimi trigonometri olmadan tamsayıyla yapılabilir (RSD-EVD-03b). | Enlem/boylam i32 1e-7 derece (yaklaşık 1,1 cm): mesafe hesabı trigonometri ister; sonuç platforma bağlı yuvarlamaya açık olur. Desimetre: daha hassas değil, aynı boyut. |
| PositionSource | 1 | u8: 0 GNSS, 1 radar fix, 2 mutual tracking, 3 other | ✔ küme / öneri kodlar | RSD-OBS-03a kapalı kümesi. | — |
| ErrorBound | 2 | u16, desimetre, 6553,5 m'de doyar | öneri | GNSS hatası metre, radar fix hatası onlarca metre mertebesinde; 0,1 m çözünürlük yeterli. | u32 cm: 2 B fazla ve sensör doğruluğunun ötesinde çözünürlük. |
| Etx | 2 | u16, Q8.8 sabit noktalı (değer × 256), 65535'te doyar | öneri | ETX ≥ 1; kayan nokta yok (K3). | u8 Q4.4: 1/16 çözünürlük rota seçiminde eşitlikleri artırır. |
| SchemaVersion | 2 | u16 | ek alan | RSD-CMC-02c: "A record that carries a schema version other than the mission's shall be rejected". Tablo 4.26 bu alanı listelemiyor. | u8: yeterli, ama sürüm alanının ileride taşması dondurulmuş biçimi kırar; maliyeti 1 B. |

## 4. Kayıtlar ve çerçeveler

Alanlar Tablo 4.26'daki sırayla verilmiştir. "Ek alan" satırları sıranın başına eklenir.

### 4.1 Common record header (RSD-EVD-02, RSD-EVD-05, RSD-OBS-03)

| Alan | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| schema version | 2 | SchemaVersion | ek alan | RSD-CMC-02c. | Bkz. Bölüm 3. |
| identifier | 0 (hatta taşınmaz) | RecordId | † (48 B) | Alıcı K13 ile hesaplar. Taşınmadığı için "taşınan tanımlayıcı içerikle uyuşmuyor" diye ayrı bir hata durumu oluşmaz. | Hatta taşımak: kayıt başına 48 B; ayrıca uyuşmazlığın G3 sayılıp sayılmayacağı tanımsız kalır. **Karar gerekir (S1).** |
| origin | 2 | NodeId | öneri | Bölüm 3. | — |
| record sequence number | 4 | u32 | öneri | Kaynağa özgü zincir konumu. Referans görevde 360 pencere; pencere başına yüzlerce kayıt mümkün. | u16: 65 535 kayıt / 360 pencere = pencere başına 182; daha uzun görevlerde taşar. u64: 4 B fazla. |
| previous-record hash | 48 | Hash48 | † | SHA-384 (RSD-CRY-01d); zincir bütünlüğü (RSD-EVD-05a). | Kesik hash: zincir bütünlüğü doğrudan güvenlik özelliği. |
| window | 4 | Window | öneri | Bölüm 3. | — |
| epoch | 6 | EpochId | öneri | Bölüm 3. | — |
| type | 1 | u8 kayıt türü kodu | öneri | Kapalı küme; yeni bir tür eklemek zaten bir şema sürümü değişikliği demektir (RSD-CMC-02c). Mevcut tür sayısı 30 civarında. | u16: 256 türe yaklaşılmıyor. |
| uniqueness key | 16 | bayt dizisi: SHA-384(türe özgü anahtar alanları) ilk 16 bayt | öneri | **RSD bu alanın anlamını tanımlamıyor.** Varsayım: (origin, type, uniqueness key) üçlüsü mantıksal bir kaydı adlandırır; aynı üçlüyle iki farklı kayıt ya supersedes ilişkisi kurar ya da eşdeğerliliktir. Yalnızca kaynağın kendi kayıtları arasında karşılaştırıldığı için kesiklik başka bir aracı etkilemez. | 48 B: her kayıtta 32 B fazla. 8 B: kaynağın kendi kayıtlarında 2^32 sonrası kaza çakışması. **Anlam onayı gerekir (S2).** |
| supersedes | 1 + 0/48 | isteğe bağlı RecordId (K8) | öneri | Düzeltilen teklif önceki teklifin yerini alır (RSD-MIS: "a corrected bid supersedes the earlier one"). | Her zaman 48 B: kayıtların çoğunda boş. |
| parents | 1 + 48·n | u8 sayı + RecordId listesi, K6, n ≤ 16 | öneri | Bağımlılık kenarları (Tablo 1.7, Evidence Graph). | u16 sayı: 255'ten fazla ebeveyn RSD'nin hiçbir yerinde gerekmiyor. **Üst sınır 16 bir öneri; RSD'de değer yok (S3).** |
| common-cause tags | 1 + 2·n | u8 sayı + u16 etiket kodu listesi (görev yapılandırmasındaki etiket tablosunda indeks), K6, n ≤ 8 | öneri | RSD-OBS-03d: etiketler görev yapılandırmasından gelir; kayıt yalnızca indeks taşır. | Metin etiketleri: değişken uzunluk ve kanoniklik sorunu. **Üst sınır 8 bir öneri (S3).** |
| policy hash | 48 | Hash48 | † | SHA-384; Tablo 4.26 "policy hash". | — |

Ebeveyn, supersedes ve etiket olmadan sabit kısım: 2+2+4+48+4+6+1+16+1+1+1+48 = **134 B**.

Gövde: başlığın ardından türe özgü gövde gelir. Kayıt kendi başına imza taşımaz; demetin imzası altındadır (RSD-EVD-01a).

### 4.2 Signed bundle (RSD-EVD-01, RSD-CRY-01b)

| Alan | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| schema version | 2 | SchemaVersion | ek alan | RSD-CMC-02c, demet düzeyinde erken ret. | Yalnızca kayıtlarda tutmak: imza doğrulamadan önce yapılan ucuz denetimi (RSD-EVD-01g) kaybettirir. |
| origin | 2 | NodeId | öneri | — | — |
| window | 4 | Window | öneri | — | — |
| first record sequence | 4 | u32 | öneri | Kayıt sıra numarasıyla aynı genişlik. | — |
| last record sequence | 4 | u32 | öneri | Kayıt sayısı = last − first + 1; ayrı bir sayı alanı yok. Kayıtlar ardışık; aradaki her sıra numarası tam bir kez bulunmalı. | Ayrı sayı alanı: aynı bilgiyi iki kez taşır ve uyuşmazlık durumu açar. |
| Merkle root | 48 | Hash48 | † | SHA-384. Ağaç kuralı: RFC 9162 tarzı; yaprak = SHA-384(0x00 ‖ kayıt), iç düğüm = SHA-384(0x01 ‖ sol ‖ sağ), tek kalan düğüm bir üst seviyeye taşınır. | Düz liste hash'i: parça düzeyinde çekmeyi (RSD-EVD-01d) kayıt düzeyinde doğrulanamaz kılar. |
| records | Σ (2 + Lᵢ) | her kayıt için u16 uzunluk + kanonik kayıt (K7) | öneri | — | — |
| signature | 4627 | Signature | ✔ | İmzalanan mesaj = `"TULPAR/1/bundle"` ‖ imza dışındaki bütün alanlar. RSD-EVD-01a "the header, the records and their Merkle root under one signature" ifadesini harfiyen karşılar. | Yalnızca başlık ve kök: kriptografik olarak yeterli, ama RSD-EVD-01a kabul testinin harfi "covers ... the records" der. |

Sabit kısım (kayıtlar hariç): 2+2+4+4+4+48+4627 = **4691 B**. Bütçedeki 5,5 kB demet için kayıtlara yaklaşık 800 B kalır.

### 4.3 Digest (RSD-EVD-01c, RSD-EVD-03)

Digest bir kayıt değildir; swarm key altında bir çerçeve yüküdür (RSD-CRY-03c).

| Alan | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| window | 4 | Window | ek alan (öneri) | RSD-EVD-03c ayrışmayı "with both hashes and the window" ile kaydeder. Tablo 4.26 bu alanı listelemiyor. | Pencereyi alış zamanından çıkarmak: alış zamanı değerlendirmeye girmemeli (RSD-EVD-03b). |
| origin set | ceil(N/8) | roster bit eşlemi (K10) | öneri | Hangi kaynaklar için girdi olduğunu gösterir; girdiler roster indeksine göre artan sırada gelir. | Girdi başına u16 kaynak: 20 kaynakta 40 B fazla; bütçe yaklaşık 370 B. |
| per origin: highest record sequence | 4 | u32 | öneri | Kayıt sıra numarasıyla aynı. | — |
| per origin: chain-head prefix | 8 | bayt dizisi | ✔ | Tablo 4.26 "8-byte chain-head prefix". | — |
| evidence-set hash | 48 | Hash48 | † | SHA-384. Kanıt kümesi hash'inin nasıl hesaplanacağı (sıralı kayıt tanımlayıcıları üzerinde mi, Merkle kökü mü) ayrı bir karar. | — |
| state hash | 48 | Hash48 | † | SHA-384. | — |
| policy version | 2 | u16, imzalı policy set içindeki sürüm numarası | öneri | RSD "policy version" (RSD-EVD-01c, Bölüm 4.10 Auditability) ile "policy hash"i ayrı adlandırıyor. | 48 B policy hash: digest'i yaklaşık 394 B'ye çıkarır (bütçe yaklaşık 370 B). **Policy set biçimi provizyon tarafında; sürüm alanı orada tanımlanmalı (S4).** |
| nodes heard directly | ceil(N/8) | roster bit eşlemi | öneri | K10. | u16 liste: 38 B'ye kadar. |

N = 20 ve 20 kaynaklı tam digest: 4 + 3 + 20·12 + 48 + 48 + 2 + 3 = **348 B** (Tablo 4.29: "about 370 B").

### 4.4 Overlay frame header (RSD-NET-01, RSD-CRY-01g)

| Alan | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| version | 1 | u8 | öneri | Çerçeve biçimi sürümü. | Version ve suite'i 4+4 bit tek baytta toplamak: 1 B kazanç, ama bit alanı ayrıştırması ve 16 değerlik sınır getirir. |
| suite | 1 | u8 kripto takım kimliği | öneri | RSD-CRY-01a "suite identifier". CNSA 2.0 takımı = 1; P-384 karşılaştırma tabanı (Tablo 2.1) = 2. | — |
| frame type | 1 | u8 enum | öneri | Kapalı küme: HELLO, HS1, HS2, HS3, hop ACK, telemetry, digest, bundle fragment, pull, C1, C2, ... | — |
| sender | 2 | NodeId | öneri | — | — |
| next hop or broadcast | 2 | NodeId; 0xFFFF = broadcast | öneri | Her çerçeve MAC yayınıdır (RSD-NET-01b); hedef seçimi overlay alanındadır. | Ayrı bir yayın bayrağı: 1 B fazla ve kimlikle tutarsız durumlar açar. |
| counter | 8 | u64 | ✔ | Tablo 4.26 "8-byte counter"; IV kuralının girdisi (RSD-CRY-01g). | — |
| message id | 4 | u32, gönderene özgü | öneri | Parçaları bir mesajda toplar; (sender, message id) tekildir. | u16: saniyede 10 mesajla yaklaşık 1,8 saatte sarmalanır; görev süresi sınırlanmamış. |
| fragment index | 1 | u8 | öneri | Mesaj en fazla 255 parça × 1363 B ≈ 347 kB. En büyük tekil mesajlar: G3 kanıtı yaklaşık 10,5 kB; iki imzalı komut yaklaşık 9,3 kB. | u16: hiçbir RSD mesajı 255 parçaya yaklaşmıyor. |
| fragment count | 1 | u8 | öneri | Aynı. | — |
| tag | 16 | GcmTag | ✔ | Hop ya da swarm-key etiketi (RSD-NET-01f). Tablo sırasına uymak için başlıkta, son alan olarak yer alır; AES-GCM AAD'si = tag hariç başlık baytları. | Etiketi çerçeve sonuna (trailer) koymak: GCM'de alışılmış yer, ama Tablo 4.26'daki sırayı bozar. |

Başlık **37 B** (21 B + 16 B etiket). RSD-NET-01f'deki "fragments of at most 1400 bytes" için öneri: 1400 B, başlık dahil overlay çerçevesinin tamamıdır, yani parça yükü ≤ 1363 B. Alternatif olarak 1400 B yalnızca yük sayılabilir; ama Tablo 4.29'daki airtime hesabı ("A 1400-byte fragment takes 1.99 ms") çerçevenin tamamını ölçer.

### 4.5 HELLO (RSD-NET-02d, RSD-CRY-03a)

| Alan | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| node id | 2 | NodeId | öneri | Mevcut `MeshHello::sender` u16. | — |
| epoch | 6 | EpochId | öneri | Bölüm 3. | — |
| sequence number | 4 | u32 | öneri | Mevcut `MeshHello::sequence` u32; 2 s aralıkla sarmalanma yok. | u16: 36 saatte sarmalanır. |
| timestamp | 8 | SimTime | öneri | Tamsayı ns. | — |
| HMAC-SHA-384 (truncated) | 16 | bayt dizisi | ✔ | Tablo 4.26 "truncated to 16 B"; anahtar: discovery key (RSD-CRY-03a). Girdi = `"TULPAR/1/hello"` ‖ HMAC hariç bütün alanlar. | — |

**Overlay başlığıyla ilişkisi: karar gerekir (S5).** Önerim (A): HELLO, frame type = HELLO ve next hop = broadcast olan bir overlay çerçevesidir. Başlığın 16 B tag alanı HMAC'i taşır; gövde node id, epoch, sequence number ve timestamp'tir. node id = başlıktaki sender; ikisi farklıysa çerçeve malformed sayılır. Toplam 37 + 20 = **57 B**. node id gövdeden çıkarılırsa 55 B olur.
Alternatif (B): tag alanı ve HMAC alanı ayrı ayrı taşınır: 37 + 36 = 73 B. Fazladan 16 B taşır ve tag alanı HELLO'da anlamsız kalır.
Tablo 4.29 HELLO için "48 B" varsayıyor; (A) ile HELLO kanal payı %0,32'den yaklaşık %0,38'e çıkar.

### 4.6 Handshake HS1 / HS2 / HS3 (RSD-NET-02a, RSD-CMC-04a)

Tablo 4.26 alanları üç mesajın toplamı olarak listeliyor; **hangi alanın hangi mesajda yer aldığı RSD'de yok (S6).** Önerim:

| Mesaj | Alanlar (sırayla) | Genişlik |
|---|---|---|
| HS1 | suite id (1), nonce (32), ML-KEM encapsulation key (1568 †) | 1601 B |
| HS2 | suite id (1), nonce (32), ML-KEM ciphertext (1568 †), transcript signature (4627 †), key confirmation (48), boot quote | 6276 + quote |
| HS3 | transcript signature (4627 †), key confirmation (48), boot quote | 4675 + quote |

| Alan | Genişlik | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|
| nonce | 32 | öneri | 256 bit; CNSA 2.0 güvenlik düzeyiyle aynı. | 16 B: oturum tazeliği için yeterli olabilir, ama quote nonce'u da bu (RSD-CMC-04a). |
| key confirmation | 48 | öneri | Tam HMAC-SHA-384 (RSD-CRY-01d). | 16 B kesik: el sıkışma kanal payı zaten küçük; tam etiket daha güçlü. |
| suite id | 1 | öneri | Overlay başlığındaki suite ile aynı kodlar. | — |
| boot quote | 1 + 48 + quote imzası | öneri | build id (u8: E, I, R; RSD-CMC-05b), boot measurement (SHA-384, 48 B), session nonce imzalanır ama tekrar taşınmaz (RSD-CMC-04a). | — |

**Bütçe tutarsızlığı (S7):** Quote imzası ML-DSA-87 ise (RSD-CRY-01b "every other signed record") el sıkışma toplamı yaklaşık 21,9 kB olur. Tablo 4.29 ise "about 12.4 kB per handshake" varsayıyor ve bu değer quote imzası olmadan hesaplanan yaklaşık 12,6 kB'a uyuyor. Attestation anahtarının algoritması RSD'de belirtilmemiş. Quote ML-DSA-87 ise el sıkışma kanal payı %0,58'den yaklaşık %1,0'a çıkar.

### 4.7 Link-state advertisement (RSD-NET-02)

Common record header + gövde (I2).

| Alan | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| sequence number | 4 | u32 | öneri | Kesin artan (RSD-NET-02e); kayıt sıra numarasından ayrıdır çünkü yalnızca LSA'ları sayar. | Kayıt sıra numarasını kullanmak: Tablo 4.26 alanı ayrıca listeliyor; RSD-NET-02e LSA'lar arasında karşılaştırma yapıyor. |
| quantized position | 12 | Position | öneri | Bölüm 3. | — |
| position source | 1 | PositionSource | ✔ küme | RSD-OBS-03a. | — |
| error bound | 2 | ErrorBound | öneri | Bölüm 3. | — |
| neighbor list | 1 + 8·n | u8 sayı + {neighbor NodeId 2, ETX forward 2, ETX reverse 2, age 2}, NodeId'ye göre artan, n ≤ 19 | öneri | 20 düğümlü filoda en fazla 19 komşu. age: u16 milisaniye, 65 535'te doyar; ölçüm penceresi 10 s (RSD-NET-02a). | age için u64 ns: komşu başına 6 B fazla; 10 s'lik pencerede ms çözünürlüğü yeterli. |

Backhaul bilgisi taşımaz (RSD-HOT-04b).

### 4.8 Epoch announcement and revocation record (RSD-CRY-03, RSD-ORG-03d)

Common record header + gövde. Revocation, aynı türün "excluded" alanları dolu biçimidir.

| Alan | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| epoch id | 6 | EpochId | öneri | = (header origin, header record sequence); ikisi farklıysa malformed. | — |
| parent epochs | 1 + 6·n | u8 sayı + EpochId listesi, K6, 1 ≤ n ≤ 2 | öneri | Birleşmede iki ebeveyn (RSD-CRY-03i). | n ≤ 4: üç bileşenin aynı anda birleşmesi RSD'de tanımlı değil. **Üst sınır onayı gerekir (S3).** |
| recipient list | ceil(N/8) | roster bit eşlemi | öneri | K10. | — |
| hash of the new key | 48 | Hash48 | † | SHA-384 (RSD-CRY-03d). | — |
| producer | 0 | — | öneri | = header origin; tekrar taşınmaz. | Taşımak: uyuşmazlık durumu açar. |
| excluded certificate id | 1 + 0/2 | isteğe bağlı CertIndex | öneri | Yalnızca revocation'da var. | — |
| embedded proof | 0 / 4 + L | isteğe bağlı, u32 uzunluk + kanıt kodlaması | öneri | **Kanıtın kendi biçimi (dört G3 türü, RSD-SEC-01d) Tablo 4.26'da yok (S8).** | — |

### 4.9 Load receipt (RSD-ORG-05)

| Alan | Genişlik | İşaret | Gerekçe |
|---|---|---|---|
| vehicle id | 2 | öneri | NodeId. |
| mission manifest hash | 48 | † | SHA-384; görev kimliği = manifest hash'i (RSD-HOT-02d). |
| device envelope hash | 48 | † | SHA-384 (RSD-ORG-05b). |
| boot quote | 1 + 48 + 4627 | öneri / † | Bölüm 4.6 ile aynı yapı; görev öncesinde nonce = mission manifest hash. |
| signature | 4627 | † | ML-DSA-87 (RSD-CRY-01b). |

### 4.10 Operator command (RSD-HOT-02, RSD-HOT-06)

Operatör anahtarlarıyla imzalanır ve araç zincirinde değildir; bu yüzden common record header taşımaz. İçerik hash'i (RSD-HOT-06g) = SHA-384(`"TULPAR/1/command"` ‖ imzalar hariç bütün alanlar).

| Alan | Genişlik | Tür | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|---|
| mission id | 48 | Hash48 | † | "the mission identifier, which is the mission manifest hash" (RSD-HOT-02d). | — |
| command id | 6 | CommandId | öneri | Bölüm 3. | — |
| type | 1 | u8: 1 ABORT, 2 HOLD, 3 RESUME, 4 PRIORITY, 5 SUPPORT | ✔ küme / öneri kodlar | RSD-HOT-02a: tam olarak beş tür. ABORT'un immediate/recall biçimi parametrede. | ABORT için iki tür kodu: RSD-HOT-02a'daki "exactly five" kuralını ihlal eder. |
| target | ceil(N/8) | roster bit eşlemi | öneri | RSD-HOT-06e: "target does not include it". | Grup kodu: tek bir aracı hedeflemeyi engeller. |
| parameters | 2 + L | u16 uzunluk + türe özgü yapı | öneri | ABORT: u8 form (0 immediate, 1 recall). HOLD ve RESUME: boş. **PRIORITY ve SUPPORT parametreleri RSD'de yok (S9).** | — |
| issue window | 4 | Window | öneri | — | — |
| duration | 2 | u16 saniye | öneri | HOLD: varsayılan 600 s, en fazla 1800 s (RSD-HOT-02e); 0 ve 1800'ün üstü malformed. RSD birimi saniye. | Pencere sayısı: RSD saniyeyle tanımlıyor. |
| validity | 2 | u16 saniye | öneri | RESUME varsayılanı 120 s (RSD-HOT-02f). | — |
| precondition | 1 | u8 yüklem kodu | öneri | "closed set of predicates over the evidence" (RSD-HOT-02f). **Küme RSD'de tanımlı değil (S9).** | — |
| basis | 1 + 48·n | u8 sayı + RecordId listesi, K6, n ≤ 4 | öneri | **"basis" RSD'de tanımlı değil;** varsayım: operatörün dayandığı kanıt kayıtları. | — |
| signatures | 1 + 4629·n | u8 sayı (1 ya da 2) + {CertIndex 2, Signature 4627}, CertIndex'e göre artan, farklı sertifikalar | ✔ / † | RSD-HOT-02b/c, RSD-HOT-06c. | — |

### 4.11 Command receipt (RSD-HOT-01m)

Common record header + gövde; üreticinin bir sonraki demetinde taşınır.

| Alan | Genişlik | İşaret | Gerekçe |
|---|---|---|---|
| command id | 6 | öneri | CommandId. |
| vehicle | 0 | öneri | = header origin. |
| result | 1 | ✔ küme / öneri kodlar | u8: 1 APPLIED, 2 VOID, 3 REJECTED. |
| window | 0 | öneri | = header window. |

Komut içerik hash'inin (48 B) de makbuza eklenmesini öneririm: aynı command id ile farklı içerikler RSD-HOT-06g'de ayrıştırılıyor. Bu Tablo 4.26'da olmayan bir ek alan olur; **karar gerekir (S10).**

### 4.12 Group Status Summary (RSD-HOT-03)

Common record header + gövde; üreticinin demetinde taşınır (RSD-HOT-03c). **Bu kaydın on beş alanının çoğunun anlamı ve birimi RSD'de tanımlı değil;** aşağıdakiler yalnızca genişlik önerisidir.

| Alan | Genişlik | İşaret | Gerekçe ve varsayım |
|---|---|---|---|
| group | 1 | öneri | u8 grup indeksi (üç grup). |
| producer, window | 0 | öneri | = header origin, header window. |
| members covered with compact positions | 1 + 23·n | öneri | n ≤ 20; {vehicle 2, position 12 (Position, altitude dahil), time of fix 8 (SimTime), security state 1} (RSD-HOT-03j). "vehicles per security state" bilgisi üye başına durum olarak taşınır. |
| energy | n | öneri | Üye başına u8 yüzde. **Enerji modeli RSD'de yok.** |
| channels OK per modality | 3 | öneri | EO, IR, radar için u8 sayı. |
| coverage | 2 | öneri | u16 binde. **Tanım RSD'de yok.** |
| regime per sector | 1 + s | öneri | u8 sayı + u8 (1 S1, 2 S2, 3 S3) (RSD-OBS-04a). |
| top backlog items | 1 + 49·k | öneri | k ≤ 5; {RecordId 48, durum 1}. **k sınırı RSD'de yok.** |
| contacts | 1 + 29·c | öneri | c ≤ 16; {contact id 16, Position 12, status 1}. **Contact kimliği RSD'de yok.** |
| alerts | 1 + 3·a | öneri | a ≤ 8; {kod 1, NodeId 2}. **Uyarı kodları RSD'de yok.** |
| jamming level | 1 | öneri | u8 0–4 = J0–J4 (Tablo 4.19). |
| commands applied | 1 + 6·m | öneri | m ≤ 8; CommandId listesi. |

Bütçe denetimi: 20 üye, 5 backlog öğesi, 16 temas ve 8 uyarıyla gövde yaklaşık 1,3 kB. Tablo 4.29 "about 1 kB" varsayıyor; temas sınırı 8'e indirilirse ≈ 1,04 kB olur.

### 4.13 GCS service certificate (RSD-HOT-05, RSD-HOT-06d)

| Alan | Genişlik | İşaret | Gerekçe | Alternatif ve neden seçilmedi |
|---|---|---|---|---|
| role | 1 | öneri | u8: 1 vehicle, 2 operator, 3 gcs. | ASCII "gcs": değişken uzunluk ve kanoniklik sorunu (K2). |
| mission id | 48 | † | RSD-HOT-02d. | — |
| ML-KEM-1024 public key | 1568 | † | Tablo 2.1. | — |
| ML-DSA-87 public key | 2592 | † | Tablo 2.1. | — |
| validity | 8 | öneri | not before ve not after, iki u32 Window. | SimTime: sertifika süresi pencere çözünürlüğünde yeterli. |
| headquarters root signature | 4627 | † | ML-DSA-87. | — |

### 4.14 Backhaul hello (RSD-HOT-04, RSD-HOT-07)

İmzasızdır ve mesh'e hiçbir zaman iletilmez.

| Alan | Genişlik | İşaret | Gerekçe |
|---|---|---|---|
| sender | 2 | öneri | NodeId; 0xFFFF = headquarters. |
| link id | 1 | öneri | u8; ağ geçidi başına en fazla birkaç backhaul. |
| window | 4 | öneri | Window. |
| sequence | 4 | öneri | u32. |

### 4.15 LSC beacon

Bu PR'ın konusu değil: LSC'nin kendi kaynak ağacında kalır (RSD-SAF-02d, RSD-SAF-02f).

## 5. Tablo 4.26'da olmayan ama gereken biçimler

Bu biçimler tasarlanmadı; ayrı işler olarak açılmalı.

| Biçim | Kaynak | Not |
|---|---|---|
| GCS receipt | RSD-HOT-05f, RSD-HOT-05h | "names the mission, the record identifier and the GCS window", GCS service key ile imzalı. |
| G3 kanıt kodlaması | RSD-SEC-01d, RSD-ORG-03d | Dört kanıt türü; revocation record'ın içine gömülür. |
| Hop acknowledgment, telemetry, pull, C1 ve C2 yükleri | RSD-NET-01d, Tablo 4.29 | Overlay çerçeve türleri; gövde biçimleri yok. |
| Provizyon paketi (roster, certificate, policy set, exclusion list) | RSD-ORG-05, RSD-CRY-02b | policy version alanı digest için gerekli (S4). |
| Kayıt türü gövdeleri (observation, task, bid, lease, contract, finding vb.) | Bölüm 4.2, 4.3, 4.6 | Tablo 4.26'da yok. |

## 6. Kod yaklaşımı (onaydan sonra)

RSD-CRY-02e: *"The canonical-encoding validator shall be generated from the same specification as the record decoders and be identical in every build."* Bu kural, kodlayıcıların ve doğrulayıcının tek bir makinece okunur şartnameden üretilmesini ister. Önerim: bu belgenin tabloları onaylanınca `docs/design/encodings.json` gibi bir şartnameye dönüşür; `common/formats/` altındaki kodlayıcılar elle yazılmak yerine bu şartnameden üretilir. **Karar gerekir (S11).** Elle yazılırsa üretece geçiş ayrı bir iş olur.

## 7. Onay gerektiren kararlar

| No | Karar | Önerim |
|---|---|---|
| S1 | Kayıt tanımlayıcısı hatta taşınacak mı? | Taşınmasın; alıcı hesaplar. |
| S2 | uniqueness key'in anlamı ve genişliği | Mimari belgesinden anlamı alınmalı; genişlik 16 B. |
| S3 | Liste üst sınırları (parents 16, tags 8, parent epochs 2, basis 4, GSS sınırları) | Belgede yazılı değerler; hepsi öneri. |
| S4 | Digest'teki policy version: u16 numara mı, 48 B hash mi? | u16; policy set biçiminde tanımlanır. |
| S5 | HELLO HMAC'i overlay tag alanında mı, ayrı bir alanda mı? | Tag alanında (A), 57 B. |
| S6 | HS1/HS2/HS3 arasında alan dağılımı | Bölüm 4.6'daki tablo. |
| S7 | Boot quote imza algoritması ve el sıkışma bütçesi (12,4 kB ile 21,9 kB) | RSD sahibine sorulmalı; ML-DSA-87 ise Tablo 4.29 güncellenmeli. |
| S8 | Gömülü kanıt kodlaması | Ayrı iş. |
| S9 | PRIORITY ve SUPPORT parametreleri, RESUME önkoşul kümesi | Ayrı iş; RSD'de tanımlı değil. |
| S10 | Command receipt'e komut içerik hash'i eklenmeli mi? | Eklensin (ek alan). |
| S11 | Kodlayıcılar elle mi yazılacak, şartnameden mi üretilecek (RSD-CRY-02e)? | Şartnameden üretim. |
| S12 | 1400 B parça sınırı başlığı içeriyor mu? | Evet (yük ≤ 1363 B). |
| S13 | Konum: yerel ENU, i32 cm | Evet. |
