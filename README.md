# Goblins Keyboard Hitbox Pico W Oyun Kolu: USB (Kablolu) ve BLE (Kablosuz) Çift Mod

Raspberry Pi Pico W ile yapılmış, **hem USB HID hem de Bluetooth Low Energy (BLE) HID** olarak çalışan bir oyun kolu. Arduino IDE ve [arduino-pico](https://github.com/earlephilhower/arduino-pico) çekirdeği kullanılır.

## İçindekiler

- [Genel Bakış](#genel-bakış)
- [Gerekli Malzemeler](#gerekli-malzemeler)
- [Arduino IDE Kurulumu](#arduino-ide-kurulumu)
- [Donanım Bağlantıları](#donanım-bağlantıları)
- [Yazılım](#yazılım)
- [Yükleme](#yükleme)
- [Test](#test)
- [Sorun Giderme](#sorun-giderme)
- [Bilinen Sınırlamalar](#bilinen-sınırlamalar)
- [Hızlı Kontrol Listesi](#hızlı-kontrol-listesi)

---

## Genel Bakış

| Mod | Ne zaman | Nasıl görünür |
|---|---|---|
| **Kablolu** | Veri USB-C portu bilgisayara takılıyken | USB HID oyun kolu |
| **Kablosuz** | Kol pil ile açıldığında (USB yokken) | BLE HID oyun kolu |

- Kolda **iki USB-C portu** vardır: biri **yalnızca şarj** (TP4056 modülüne), diğeri **yalnızca veri** (Pico W'ye) içindir.
- Mod seçimi **açılışta**, Pico W'nin VBUS (USB gücü) girişinin okunmasıyla otomatik yapılır.

```mermaid
flowchart LR
    A[Açılış] --> B{VBUS var mı?}
    B -- Evet --> C[USB Joystick<br/>Kablolu mod]
    B -- Hayır --> D[JoystickBLE<br/>Kablosuz mod]
```

> [!IMPORTANT]
> **Kütüphane karışıklığı:** İnternette sıkça önerilen `BleGamepad` kütüphanesi **yalnızca ESP32 içindir** ve Pico W'de çalışmaz. `BTstackLib` ve `Adafruit_TinyUSB` de bu çözümde gerekmez.
> Gereken her şey arduino-pico kartı paketinin içinde hazır gelir (`Joystick` ve `JoystickBLE`). Kütüphane yöneticisinden ayrıca bir şey kurmanız gerekmez.

---

## Gerekli Malzemeler

- Raspberry Pi Pico W (1 adet)
- TP4056 şarj modülü, **koruma devreli** sürüm (DW01 + 8205A yongalı, genelde 6 pinli)
- 3.7 V Li-Po / Li-Ion pil (kapasiteye göre seçin)
- USB-C soket kartı, 2 adet (biri şarj, biri veri; veri için VBUS, GND, D+, D− pinleri çıkış veren tür)
- SPST (tek kutuplu) kaydırmalı anahtar, pil hattı için
- 16 adet buton (4 yön, 4 aksiyon, 2+2 omuz tuşu, 2 analog tuşu, Start, Select)
- Bağlantı kabloları, lehim malzemeleri

---

## Arduino IDE Kurulumu

### arduino-pico kart paketini ekleme

1. Arduino IDE'de **Dosya → Tercihler** menüsünü açın.
2. **Ek Kart Yöneticisi URL'leri** alanına şu adresi yapıştırın:
   ```
   https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
   ```
3. **Araçlar → Kart → Kart Yöneticisi** menüsünü açın.
4. Arama kutusuna `pico` veya `Earle Philhower` yazın.
5. **"Raspberry Pi Pico/RP2040" – Earle F. Philhower, III** paketini kurun.

> [!WARNING]
> "Arduino Mbed OS RP2040 Boards" paketi farklı bir çekirdektir. Bu proje için **onu kullanmayın**.

### Kart ve araç ayarları

| Ayar | Değer |
|---|---|
| Araçlar → Kart | Raspberry Pi Pico W |
| Araçlar → IP/Bluetooth Stack | Bluetooth etkin olan seçenek (ör. "IPv4 + Bluetooth") |
| Araçlar → USB Stack | Pico SDK (varsayılan; **Adafruit TinyUSB seçmeyin**) |

> [!NOTE]
> Bluetooth seçeneği açık değilse `JoystickBLE` derlenmez. USB Stack olarak Adafruit TinyUSB seçilirse yerleşik `Joystick` sınıfı çalışmaz.

### Yerleşik kütüphaneleri kontrol etme

Kütüphane Yöneticisi'nde `Joystick` araması yapmanıza gerek yoktur; bu kütüphaneler kart paketiyle birlikte gelir. Kontrol etmek için:

- **Taslak → Kütüphane Ekle** menüsünün altında Pico W'ye ait kütüphaneler listelenir.
- **Dosya → Örnekler** altında Pico W için Joystick ve Bluetooth örnekleri bulunur.

---

## Donanım Bağlantıları

### Buton pinleri

Tüm butonlar bir bacağı ilgili GPIO'ya, diğer bacağı GND'ye gelecek şekilde bağlanır. Kod dahili pull-up direnci kullanır; **dışarıdan direnç gerekmez**. Basılı = `LOW`.

| Fonksiyon | GPIO | | Fonksiyon | GPIO |
|---|---|---|---|---|
| Sol | 15 | | B | 20 |
| Aşağı | 13 | | X | 19 |
| Sağ | 14 | | Y | 18 |
| Yukarı | 12 | | Omuz L | 16 |
| Start | 2 | | Omuz R | 17 |
| Select | 3 | | Omuz 2 L | 4 |
| A | 5 | | Omuz 2 R | 27 |
| | | | Analog tuşu L | 26 |
| | | | Analog tuşu R | 22 |

### Güç ve şarj bağlantısı (iki USB-C port)

İki hat birbirinden ayrı olduğu için şarj kablosunu takmak Pico'yu kendiliğinden açmaz. Pil hattı bir anahtarla Pico'ya bağlanır.

| Bağlantı | Nereye |
|---|---|
| Şarj portu VBUS | TP4056 IN+ |
| Şarj portu GND | TP4056 IN− |
| Şarj portu D+ / D− | Bağlanmaz |
| TP4056 B+ / B− | Pil (kalıcı bağlı) |
| TP4056 OUT+ | Anahtar üzerinden → Pico **VSYS** (pin 39) |
| TP4056 OUT− | Ortak GND (Pico GND) |
| Veri portu VBUS | Doğrudan Pico **VBUS** (pin 40) |
| Veri portu GND | Pico GND |
| Veri portu D+ / D− | Pico USB D+ / D− (test noktaları veya mikro-USB breakout) |

> [!NOTE]
> Bu bölüm şu varsayıma dayanır: anahtar yalnızca pil hattında (TP4056 OUT+ ile VSYS arasında) bulunur ve veri portunun VBUS hattı anahtarsız Pico'nun VBUS pinine gider. Bağlantınız farklıysa bu bölümü ona göre düzenleyin.

### Çalışma durumları

| Durum | Sonuç |
|---|---|
| Anahtar kapalı, veri kablosu yok | Pico kapalı. Şarj portuna kablo takılıysa pil şarj olur. |
| Anahtar açık, veri kablosu yok | Pico pilden çalışır, VBUS düşük okunur, kol **BLE modunda** açılır. |
| Veri kablosu bilgisayara takılı | VBUS yüksek okunur, kol **kablolu modda** açılır. Pico bu durumda anahtardan bağımsız olarak VBUS'tan beslenir. |
| Şarj portu takılı | Pil şarj olur. Kol açıksa kablosuz modda oynanırken şarj devam eder. |

### Güvenlik notları

- Pico'nun kendi üzerinde VBUS ile VSYS arasında bir diyot vardır. Pilden USB portuna geri akım olmaması için ek diyot gerekmez.
- TP4056 modülünün **koruma devreli** sürümünü kullanın. Koruması olmayan 4 pinli sürüm, pil boşaldığında kesme yapmaz ve Li-Po'ya zarar verebilir.
- Veri portunun VBUS hattı Pico'nun VBUS pinine bağlı değilse kod daima BLE modunu seçer.
- Şarj portunun 5 V hattını Pico'nun VBUS veya VSYS pinine **bağlamayın**. Yalnızca TP4056 girişine gitmelidir.
- Lehim sonrası **kısa devre kontrolü** yapmadan pili bağlamayın.

---

## Yazılım

### Çalışma mantığı

1. Açılışta tüm buton pinleri `INPUT_PULLUP` olarak ayarlanır.
2. Pico W'nin kablosuz yongasındaki VBUS algılama pini (`CYW43_WL_GPIO_VBUS_PIN`) okunur.
3. VBUS varsa USB `Joystick`, yoksa `JoystickBLE` başlatılır.
4. Döngüde her 5 ms'de butonlar okunur; rapor **yalnızca bir buton veya D-pad durumu değiştiğinde** gönderilir.
5. D-pad dört yönden oluşan 8 yönlü "hat" değerine çevrilir.

### Tam kod

Aşağıdaki kodu yeni bir taslağa yapıştırın:

```cpp
#include <Arduino.h>
#include <Joystick.h>      // USB joystick (built into arduino-pico)
#include <JoystickBLE.h>   // BLE joystick (built into arduino-pico)
#include "pico/cyw43_arch.h"

// Pin layout
const int PIN_LEFT = 15, PIN_DOWN = 13, PIN_RIGHT = 14, PIN_UP = 12;
const int PIN_START = 2, PIN_SELECT = 3;
const int PIN_A = 5, PIN_B = 20, PIN_X = 19, PIN_Y = 18;
const int PIN_SHOULDER_L = 16, PIN_SHOULDER_R = 17;
const int PIN_SHOULDER_2_L = 4, PIN_SHOULDER_2_R = 27;
const int PIN_ANALOG_BTN_L = 26, PIN_ANALOG_BTN_R = 22;

// Buttons in order -> joystick buttons 1..12
const int buttonPins[] = {
  PIN_A, PIN_B, PIN_X, PIN_Y,
  PIN_SHOULDER_L, PIN_SHOULDER_R, PIN_SHOULDER_2_L, PIN_SHOULDER_2_R,
  PIN_ANALOG_BTN_L, PIN_ANALOG_BTN_R, PIN_START, PIN_SELECT
};
const int NUM_BUTTONS = sizeof(buttonPins) / sizeof(buttonPins[0]);

bool isWiredMode = false;
uint32_t lastState = 0xFFFFFFFF;  // forces one report at startup

// Returns hat angle in degrees, or -1 when centered
int readHat() {
  bool up    = digitalRead(PIN_UP)    == LOW;
  bool down  = digitalRead(PIN_DOWN)  == LOW;
  bool left  = digitalRead(PIN_LEFT)  == LOW;
  bool right = digitalRead(PIN_RIGHT) == LOW;

  if (up && right)   return 45;
  if (down && right) return 135;
  if (down && left)  return 225;
  if (up && left)    return 315;
  if (up)    return 0;
  if (right) return 90;
  if (down)  return 180;
  if (left)  return 270;
  return -1;
}

void setup() {
  const int allPins[] = {
    PIN_LEFT, PIN_DOWN, PIN_RIGHT, PIN_UP, PIN_START, PIN_SELECT,
    PIN_A, PIN_B, PIN_X, PIN_Y, PIN_SHOULDER_L, PIN_SHOULDER_R,
    PIN_SHOULDER_2_L, PIN_SHOULDER_2_R, PIN_ANALOG_BTN_L, PIN_ANALOG_BTN_R
  };
  for (int p : allPins) pinMode(p, INPUT_PULLUP);

  // Pico W: VBUS sense is on the wireless chip's GPIO2
  delay(100);  // let the radio chip finish starting
  isWiredMode = cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);

  if (isWiredMode) {
    Joystick.useManualSend(true);  // we call send_now() ourselves
    Joystick.begin();
  } else {
    JoystickBLE.useManualSend(true);
    JoystickBLE.begin();
  }
}

void loop() {
  int hat = readHat();

  // Pack buttons and hat into one number so changes are easy to detect
  uint32_t state = 0;
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) state |= (1UL << i);
  }
  state |= ((uint32_t)(hat + 1)) << 16;  // hat: -1..315 stored in upper bits

  // Only send a report when something actually changed
  if (state != lastState) {
    lastState = state;
    if (isWiredMode) {
      Joystick.hat(hat);
      for (int i = 0; i < NUM_BUTTONS; i++) {
        Joystick.button(i + 1, (state >> i) & 1);
      }
      Joystick.send_now();
    } else {
      JoystickBLE.hat(hat);
      for (int i = 0; i < NUM_BUTTONS; i++) {
        JoystickBLE.button(i + 1, (state >> i) & 1);
      }
      JoystickBLE.send_now();
    }
  }

  delay(5);  // scan inputs ~200 times per second
}
```

### Buton eşleşmesi

| Joystick butonu | Fiziksel tuş | | Joystick butonu | Fiziksel tuş |
|---|---|---|---|---|
| 1 | A | | 7 | Omuz 2 L |
| 2 | B | | 8 | Omuz 2 R |
| 3 | X | | 9 | Analog tuşu L |
| 4 | Y | | 10 | Analog tuşu R |
| 5 | Omuz L | | 11 | Start |
| 6 | Omuz R | | 12 | Select |

D-pad ayrı bir buton olarak değil, **"hat" (şapka)** girişi olarak raporlanır.

### Girişlerin donması sorunu ve çözümü

Kodun ilk sürümü her 10 ms'de bir (saniyede 100 kez), hiçbir şey değişmese bile rapor gönderiyordu. Kablosuz modda bir süre sonra butonlar tepki vermeyi bırakıyor, cihaz ise bağlı görünmeye devam ediyordu. Güncel kod raporu **yalnızca bir buton veya D-pad durumu değiştiğinde** gönderir ve bu sorunu giderdi.

> [!NOTE]
> Nedenin tam olarak rapor yoğunluğu olduğu kütüphane kaynak koduyla doğrulanmadı, ancak değişiklikten sonra sorun gözlenmedi.

---

## Yükleme

1. Pico W üzerindeki **BOOTSEL** düğmesine basılı tutarak USB'ye takın. Bilgisayarda `RPI-RP2` adlı bir sürücü görünür.
2. Arduino IDE'de **Araçlar → Port** menüsünden Pico'yu seçin (ilk yüklemede UF2 sürücüsü olarak da yükleme yapılabilir).
3. **Yükle** düğmesine basın.
4. İlk yüklemeden sonra çekirdeğin otomatik sıfırlama desteği sayesinde BOOTSEL'e genelde tekrar basmanız gerekmez.

---

## Test

### Kablolu mod

1. Veri USB-C portunu bilgisayara takın.
2. Windows'ta `joy.cpl` komutunu çalıştırın (`Win + R`). Cihaz bir oyun kolu olarak listelenir.
3. Özellikler penceresinde butonlara basarak her tuşun doğru numarada tepki verdiğini kontrol edin.
4. D-pad yönlerini kontrol edin.

### Kablosuz mod

1. Veri kablosunu çıkarın, anahtarı kapatıp açın (pil ile açılış, BLE modu).
2. Bilgisayarın veya telefonun Bluetooth ayarlarından yeni cihaz arayın.
3. Cihaz listede görünür (isim varsayılan olabilir; ayrıca bkz. [Bilinen Sınırlamalar](#bilinen-sınırlamalar)).
4. Eşleştirdikten sonra `joy.cpl` veya bir gamepad test sitesi ile butonları deneyin.

### Şarj

1. Şarj USB-C portunu şarj adaptörüne takın (anahtar açık veya kapalı olabilir).
2. TP4056 üzerindeki LED şarj sırasında kırmızı, şarj bitince mavi/yeşil yanar (modüle göre değişir).

---

## Sorun Giderme

| Belirti | Olası neden ve çözüm |
|---|---|
| `BleGamepad.h: No such file` hatası | Bu kütüphane ESP32 içindir. Koddan kaldırın ve bu kılavuzdaki kodu kullanın. |
| `Joystick.h` veya `JoystickBLE.h` bulunamıyor | arduino-pico paketi kurulu değil veya yanlış kart seçili. [Arduino IDE Kurulumu](#arduino-ide-kurulumu) bölümünü tekrar kontrol edin. |
| `JoystickBLE` derlenmiyor | IP/Bluetooth Stack ayarı Bluetooth'u etkinleştirmiyor. |
| `class JoystickBLE_ has no member named setManufacturer` / `connected` / başka bir üye hatası | Bu üyeler kütüphanede yok. Tam koddaki gibi yalnızca `begin`, `button`, `hat`, `useManualSend`, `send_now` kullanın. |
| `CYW43_WL_GPIO_VBUS_PIN` derleme hatası | `pico/cyw43_arch.h` dosyasının eklendiğinden ve kartın Pico W olduğundan emin olun. |
| Kol USB takılıyken BLE modunda açılıyor | Veri portunun VBUS hattı Pico'nun VBUS pinine (pin 40) bağlı değil. Bağlantıyı kontrol edin. |
| Kol şarj portu takılınca açılıyor | Şarj portunun 5 V hattı Pico'nun VBUS veya VSYS pinine de bağlı olabilir. Şarj portunu yalnızca TP4056 IN+ ve IN− pinlerine bağlayın. |
| Kol USB takıldıktan sonra anahtarla açılırsa mod hatalı | Mod yalnızca açılışta seçilir. USB'yi çıkarıp taktıktan sonra anahtarı kapatıp açın. |
| BLE cihazı eşleşmiyor veya kod değişince bağlanmıyor | Bilgisayarın Bluetooth listesindeki eski kaydı silin ve yeniden eşleştirin. |
| D-pad yönleri yanlış | `hat()` fonksiyonu derece yerine farklı bir değer bekliyor olabilir. Çekirdek klasöründeki `libraries/Joystick/src/HID_Joystick.h` dosyasındaki `hat()` açıklamasına bakın ve `readHat()` değerlerini buna göre değiştirin. |
| Hareketler kesik kesik veya düzensiz | `useManualSend(true)` satırlarını kaldırıp deneyin. |
| Pil çok çabuk bitiyor veya boşalıyor | Koruma devresiz TP4056 kullanıyor olabilirsiniz. Korumalı modülü kullanın. |
| Bir süre sonra girişler algılanmıyor, cihaz bağlı görünüyor | Eski kod her 10 ms'de rapor gönderiyordu. [Tam kod](#tam-kod) bölümündeki güncel kodu kullanın; rapor yalnızca durum değişince gönderilir. |

---

## Bilinen Sınırlamalar

- **Mod yalnızca açılışta seçilir.** Pico W'de Bluetooth yığını temiz biçimde yeniden başlatılamadığı için çalışırken kablolu ve kablosuz mod arasında geçiş yoktur.
- **BLE adı:** Bu kılavuzdaki kod `setName` kullanmaz, çünkü kütüphanede varlığı doğrulanamamıştır. Cihaz varsayılan adla görünebilir. Derleyici kabul ederse `JoystickBLE.begin("Pico W Gamepad");` yazımını deneyebilirsiniz.
- **Analog çubuk yoktur.** Analog tuş girişleri (L3/R3) yalnızca buton olarak okunur.
- **Kod doğrulaması:** Kod, yazarın kendi kartında derlenip çalıştığı doğrulanan sürümdür; farklı arduino-pico sürümlerinde metot adları değişebilir. Bir hata alırsanız çekirdeğin `libraries/Joystick` ve Bluetooth örnek taslaklarına bakın.

---

## Hızlı Kontrol Listesi

- [ ] arduino-pico paketi kuruldu ve **Raspberry Pi Pico W** seçildi
- [ ] IP/Bluetooth Stack: **Bluetooth etkin**
- [ ] USB Stack: **Pico SDK**
- [ ] Koddan `BleGamepad`, `BTstackLib`, `Adafruit_TinyUSB` kaldırıldı
- [ ] Korumalı TP4056 kullanılıyor
- [ ] Pil hattı anahtar üzerinden VSYS'ye bağlı; veri portunun VBUS hattı Pico VBUS pinine bağlı
- [ ] Şarj portu yalnızca TP4056 girişine bağlı (Pico'ya bağlı değil)
- [ ] Kablolu ve kablosuz modlar ayrı ayrı test edildi


A gamepad built on the Raspberry Pi Pico W that works as **both a USB HID and a Bluetooth Low Energy (BLE) HID** controller. It uses the Arduino IDE and the [arduino-pico](https://github.com/earlephilhower/arduino-pico) core.

## Table of Contents

- [Overview](#overview)
- [Parts List](#parts-list)
- [Arduino IDE Setup](#arduino-ide-setup)
- [Hardware Connections](#hardware-connections)
- [Software](#software)
- [Uploading](#uploading)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)
- [Known Limitations](#known-limitations)
- [Quick Checklist](#quick-checklist)

---

## Overview

| Mode | When | Appears as |
|---|---|---|
| **Wired** | Data USB-C port is plugged into a computer | USB HID gamepad |
| **Wireless** | Gamepad is powered on from the battery (no USB) | BLE HID gamepad |

- The gamepad has **two USB-C ports**: one **for charging only** (wired to the TP4056 module) and one **for data only** (wired to the Pico W).
- The mode is chosen automatically **at boot** by reading the Pico W's VBUS (USB power) input.

```mermaid
flowchart LR
    A[Boot] --> B{VBUS present?}
    B -- Yes --> C[USB Joystick<br/>Wired mode]
    B -- No --> D[JoystickBLE<br/>Wireless mode]
```

> [!IMPORTANT]
> **Library confusion:** The `BleGamepad` library often recommended online is **ESP32-only** and does not work on the Pico W. `BTstackLib` and `Adafruit_TinyUSB` are also not needed for this solution.
> Everything required ships inside the arduino-pico board package (`Joystick` and `JoystickBLE`). You do not need to install anything from the Library Manager.

---

## Parts List

- Raspberry Pi Pico W (1)
- TP4056 charging module, **protected version** (DW01 + 8205A chips, usually 6 pins)
- 3.7 V Li-Po / Li-Ion battery (choose capacity to suit)
- USB-C breakout boards, 2 pieces (one for charging, one for data; the data one must break out VBUS, GND, D+ and D−)
- SPST (single-pole) slide switch for the battery line
- 16 buttons (4 directions, 4 action, 2+2 shoulder, 2 analog-stick clicks, Start, Select)
- Hookup wire and soldering supplies

---

## Arduino IDE Setup

### Adding the arduino-pico board package

1. In the Arduino IDE, open **File → Preferences**.
2. Paste this into **Additional Boards Manager URLs**:
   ```
   https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
   ```
3. Open **Tools → Board → Boards Manager**.
4. Search for `pico` or `Earle Philhower`.
5. Install **"Raspberry Pi Pico/RP2040" by Earle F. Philhower, III**.

> [!WARNING]
> The "Arduino Mbed OS RP2040 Boards" package is a different core. **Do not use it** for this project.

### Board and tool settings

| Setting | Value |
|---|---|
| Tools → Board | Raspberry Pi Pico W |
| Tools → IP/Bluetooth Stack | An option with Bluetooth enabled (e.g. "IPv4 + Bluetooth") |
| Tools → USB Stack | Pico SDK (default; **do not select Adafruit TinyUSB**) |

> [!NOTE]
> If Bluetooth is not enabled, `JoystickBLE` will not compile. If you select Adafruit TinyUSB as the USB Stack, the built-in `Joystick` class will not work.

### Checking the built-in libraries

There is no need to search for `Joystick` in the Library Manager; these libraries come with the board package. To verify:

- Under **Sketch → Include Library**, the Pico W libraries are listed.
- Under **File → Examples**, there are Joystick and Bluetooth examples for the Pico W.

---

## Hardware Connections

### Button pins

Each button connects with one leg to its GPIO and the other leg to GND. The code uses the internal pull-up resistor, so **no external resistors are needed**. Pressed = `LOW`.

| Function | GPIO | | Function | GPIO |
|---|---|---|---|---|
| Left | 15 | | B | 20 |
| Down | 13 | | X | 19 |
| Right | 14 | | Y | 18 |
| Up | 12 | | Shoulder L | 16 |
| Start | 2 | | Shoulder R | 17 |
| Select | 3 | | Shoulder 2 L | 4 |
| A | 5 | | Shoulder 2 R | 27 |
| | | | Analog click L | 26 |
| | | | Analog click R | 22 |

### Power and charging (two USB-C ports)

Because the two lines are kept separate, plugging in the charging cable does not power on the Pico by itself. The battery line connects to the Pico through a switch.

| Connection | Goes to |
|---|---|
| Charge port VBUS | TP4056 IN+ |
| Charge port GND | TP4056 IN− |
| Charge port D+ / D− | Not connected |
| TP4056 B+ / B− | Battery (permanently connected) |
| TP4056 OUT+ | Through the switch → Pico **VSYS** (pin 39) |
| TP4056 OUT− | Common GND (Pico GND) |
| Data port VBUS | Directly to Pico **VBUS** (pin 40) |
| Data port GND | Pico GND |
| Data port D+ / D− | Pico USB D+ / D− (test pads or micro-USB breakout) |

> [!NOTE]
> This section assumes the switch is only on the battery line (between TP4056 OUT+ and VSYS), and that the data port's VBUS line goes, without a switch, to the Pico's VBUS pin. If your wiring differs, adapt this section accordingly.

### Operating states

| State | Result |
|---|---|
| Switch off, no data cable | Pico is off. If a cable is plugged into the charge port, the battery charges. |
| Switch on, no data cable | Pico runs from the battery, VBUS reads low, gamepad boots in **BLE mode**. |
| Data cable plugged into a computer | VBUS reads high, gamepad boots in **wired mode**. The Pico is powered from VBUS regardless of the switch. |
| Charge port plugged in | Battery charges. If the gamepad is on, charging continues while you play wirelessly. |

### Safety notes

- The Pico has its own diode between VBUS and VSYS, so no extra diode is needed to prevent back-feeding from the battery to the USB port.
- Use the **protected** version of the TP4056 module. The unprotected 4-pin version does not cut off when the battery is depleted and can damage the Li-Po.
- If the data port's VBUS line is not connected to the Pico's VBUS pin, the code will always select BLE mode.
- **Do not** connect the charge port's 5 V line to the Pico's VBUS or VSYS pin. It must go only to the TP4056 input.
- Do a **short-circuit check** after soldering and before connecting the battery.

---

## Software

### How it works

1. At boot, all button pins are set to `INPUT_PULLUP`.
2. The VBUS sense pin on the Pico W's wireless chip (`CYW43_WL_GPIO_VBUS_PIN`) is read.
3. If VBUS is present, the USB `Joystick` is started; otherwise `JoystickBLE` is started.
4. The loop reads the buttons every 5 ms; a report is sent **only when a button or D-pad state changes**.
5. The D-pad's four directions are converted to an 8-direction "hat" value.

### Full code

Paste the following into a new sketch:

```cpp
#include <Arduino.h>
#include <Joystick.h>      // USB joystick (built into arduino-pico)
#include <JoystickBLE.h>   // BLE joystick (built into arduino-pico)
#include "pico/cyw43_arch.h"

// Pin layout
const int PIN_LEFT = 15, PIN_DOWN = 13, PIN_RIGHT = 14, PIN_UP = 12;
const int PIN_START = 2, PIN_SELECT = 3;
const int PIN_A = 5, PIN_B = 20, PIN_X = 19, PIN_Y = 18;
const int PIN_SHOULDER_L = 16, PIN_SHOULDER_R = 17;
const int PIN_SHOULDER_2_L = 4, PIN_SHOULDER_2_R = 27;
const int PIN_ANALOG_BTN_L = 26, PIN_ANALOG_BTN_R = 22;

// Buttons in order -> joystick buttons 1..12
const int buttonPins[] = {
  PIN_A, PIN_B, PIN_X, PIN_Y,
  PIN_SHOULDER_L, PIN_SHOULDER_R, PIN_SHOULDER_2_L, PIN_SHOULDER_2_R,
  PIN_ANALOG_BTN_L, PIN_ANALOG_BTN_R, PIN_START, PIN_SELECT
};
const int NUM_BUTTONS = sizeof(buttonPins) / sizeof(buttonPins[0]);

bool isWiredMode = false;
uint32_t lastState = 0xFFFFFFFF;  // forces one report at startup

// Returns hat angle in degrees, or -1 when centered
int readHat() {
  bool up    = digitalRead(PIN_UP)    == LOW;
  bool down  = digitalRead(PIN_DOWN)  == LOW;
  bool left  = digitalRead(PIN_LEFT)  == LOW;
  bool right = digitalRead(PIN_RIGHT) == LOW;

  if (up && right)   return 45;
  if (down && right) return 135;
  if (down && left)  return 225;
  if (up && left)    return 315;
  if (up)    return 0;
  if (right) return 90;
  if (down)  return 180;
  if (left)  return 270;
  return -1;
}

void setup() {
  const int allPins[] = {
    PIN_LEFT, PIN_DOWN, PIN_RIGHT, PIN_UP, PIN_START, PIN_SELECT,
    PIN_A, PIN_B, PIN_X, PIN_Y, PIN_SHOULDER_L, PIN_SHOULDER_R,
    PIN_SHOULDER_2_L, PIN_SHOULDER_2_R, PIN_ANALOG_BTN_L, PIN_ANALOG_BTN_R
  };
  for (int p : allPins) pinMode(p, INPUT_PULLUP);

  // Pico W: VBUS sense is on the wireless chip's GPIO2
  delay(100);  // let the radio chip finish starting
  isWiredMode = cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);

  if (isWiredMode) {
    Joystick.useManualSend(true);  // we call send_now() ourselves
    Joystick.begin();
  } else {
    JoystickBLE.useManualSend(true);
    JoystickBLE.begin();
  }
}

void loop() {
  int hat = readHat();

  // Pack buttons and hat into one number so changes are easy to detect
  uint32_t state = 0;
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) state |= (1UL << i);
  }
  state |= ((uint32_t)(hat + 1)) << 16;  // hat: -1..315 stored in upper bits

  // Only send a report when something actually changed
  if (state != lastState) {
    lastState = state;
    if (isWiredMode) {
      Joystick.hat(hat);
      for (int i = 0; i < NUM_BUTTONS; i++) {
        Joystick.button(i + 1, (state >> i) & 1);
      }
      Joystick.send_now();
    } else {
      JoystickBLE.hat(hat);
      for (int i = 0; i < NUM_BUTTONS; i++) {
        JoystickBLE.button(i + 1, (state >> i) & 1);
      }
      JoystickBLE.send_now();
    }
  }

  delay(5);  // scan inputs ~200 times per second
}
```

### Button mapping

| Joystick button | Physical key | | Joystick button | Physical key |
|---|---|---|---|---|
| 1 | A | | 7 | Shoulder 2 L |
| 2 | B | | 8 | Shoulder 2 R |
| 3 | X | | 9 | Analog click L |
| 4 | Y | | 10 | Analog click R |
| 5 | Shoulder L | | 11 | Start |
| 6 | Shoulder R | | 12 | Select |

The D-pad is not reported as separate buttons but as a **"hat" (POV)** input.

### The input-freezing problem and its fix

The first version of the code sent a report every 10 ms (100 times per second), even when nothing had changed. In wireless mode, after a while the buttons would stop responding while the device still appeared connected. The current code sends a report **only when a button or D-pad state changes**, which resolved the problem.

> [!NOTE]
> That report frequency was the exact cause has not been verified against the library source code, but the problem was not observed after the change.

---

## Uploading

1. Hold the **BOOTSEL** button on the Pico W while plugging it into USB. A drive named `RPI-RP2` appears on your computer.
2. In the Arduino IDE, select the Pico under **Tools → Port** (for the first upload you can also upload via the UF2 drive).
3. Press **Upload**.
4. After the first upload, the core's auto-reset support usually means you no longer need to press BOOTSEL again.

---

## Testing

### Wired mode

1. Plug the data USB-C port into a computer.
2. On Windows, run `joy.cpl` (`Win + R`). The device is listed as a game controller.
3. In the Properties window, press the buttons and check that each key responds with the correct number.
4. Check the D-pad directions.

### Wireless mode

1. Unplug the data cable and turn the switch off and on (battery power, BLE mode).
2. Search for a new device in your computer's or phone's Bluetooth settings.
3. The device appears in the list (the name may be the default; see [Known Limitations](#known-limitations)).
4. After pairing, test the buttons with `joy.cpl` or a gamepad tester website.

### Charging

1. Plug the charge USB-C port into a charger (the switch can be on or off).
2. The LED on the TP4056 is red while charging and blue/green when done (varies by module).

---

## Troubleshooting

| Symptom | Likely cause and fix |
|---|---|
| `BleGamepad.h: No such file` error | That library is ESP32-only. Remove it from your code and use the code in this guide. |
| `Joystick.h` or `JoystickBLE.h` not found | The arduino-pico package is not installed or the wrong board is selected. Re-check [Arduino IDE Setup](#arduino-ide-setup). |
| `JoystickBLE` does not compile | The IP/Bluetooth Stack setting does not enable Bluetooth. |
| `class JoystickBLE_ has no member named setManufacturer` / `connected` / another member error | Those members do not exist in the library. Use only `begin`, `button`, `hat`, `useManualSend`, `send_now`, as in the full code. |
| `CYW43_WL_GPIO_VBUS_PIN` compile error | Make sure `pico/cyw43_arch.h` is included and the board is a Pico W. |
| Gamepad boots in BLE mode with USB plugged in | The data port's VBUS line is not connected to the Pico's VBUS pin (pin 40). Check the wiring. |
| Gamepad turns on when the charge port is plugged in | The charge port's 5 V line may also be connected to the Pico's VBUS or VSYS pin. Connect the charge port only to TP4056 IN+ and IN−. |
| Wrong mode if the gamepad is switched on after USB is plugged in | The mode is chosen only at boot. Unplug and replug USB, then turn the switch off and on. |
| BLE device will not pair, or will not connect after a code change | Delete the old entry in your computer's Bluetooth list and pair again. |
| D-pad directions are wrong | `hat()` may expect a value other than degrees. Check the `hat()` description in `libraries/Joystick/src/HID_Joystick.h` in the core folder and adjust the `readHat()` values accordingly. |
| Movement is choppy or irregular | Try removing the `useManualSend(true)` lines. |
| Battery drains too quickly or over-discharges | You may be using a TP4056 without protection. Use the protected module. |
| Inputs stop being detected after a while, device still appears connected | The old code sent a report every 10 ms. Use the current code from [Full code](#full-code); reports are sent only when state changes. |

---

## Known Limitations

- **The mode is chosen only at boot.** Because the Bluetooth stack on the Pico W cannot be restarted cleanly, there is no switching between wired and wireless mode while running.
- **BLE name:** The code in this guide does not use `setName`, because its existence in the library could not be verified. The device may appear with the default name. If the compiler accepts it, you can try `JoystickBLE.begin("Pico W Gamepad");`.
- **No analog stick.** The analog-stick click inputs (L3/R3) are read as buttons only.
- **Code verification:** The code is the version verified to compile and run on the author's board; method names may differ in other arduino-pico versions. If you get an error, look at the core's `libraries/Joystick` and Bluetooth example sketches.

---

## Quick Checklist

- [ ] arduino-pico package installed and **Raspberry Pi Pico W** selected
- [ ] IP/Bluetooth Stack: **Bluetooth enabled**
- [ ] USB Stack: **Pico SDK**
- [ ] `BleGamepad`, `BTstackLib`, `Adafruit_TinyUSB` removed from the code
- [ ] Protected TP4056 in use
- [ ] Battery line goes through the switch to VSYS; data port VBUS goes to the Pico's VBUS pin
- [ ] Charge port connected only to the TP4056 input (not to the Pico)
- [ ] Wired and wireless modes tested separately
