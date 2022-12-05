# Zeleznicni uzel Sargans

Firmware pro nadrazi z adventniho kalendare 2022.

## Popis funkci

navestidlo - mezi ctyrmi rezimy se prepina otacenim knofliku, vypina a zapina se stiskem tehoz
*MOC NEFUNGUJE* a priznam se, ze uplne nevim proc. je to zahadne naladovy.

hodiny: normalne ukazuji hodiny (s RTC)

LED displej - ukazuje linku a destinaci po prijezdu vagonku

vnitrni osvetleni - trvale zapnute

osvetleni koleje c. 3 - prepina se zlutym tlacitkem, navic blika pri prijezdu vagonku

reproduktor - hraje pri prijezdu vagonku

### Na koleji c.1/2

je 24V indukcni civka, rozsveci se svetylka na vagoncich

### Pri prijezdu vlaku na kolej c. 3

- rozblika se osvetleni koleje
- na LED banneru se zobrazi linka / destinace
- prehraje se prislusna zeleznicni znelka (SBB, CFF, FFS, SNCF, DB) + hlaseni

### Hlaseni

Osmnact jich odpovida skutecne odjizdejicim linkam ze Sargansu, ale jinak jsou ze 2/3 smyslena, aby bylo pokazde jine. Destinace pokryvaji siroke okoli i vzdalenejsi kouty Svycarska, je tam i par mezinarodnich vlaku. Jingle na zacatku odpovida cilove destinaci, jinak by samozrejme mela byt porad jen SBB.

Na displeji se ukaze jen linka a cil (popr. jedna nacestna stanice), v hlaseni je pak trasa podrobneji.

Do hlaseni jsou prigenerovane i nejake dalsi splechty ohledne sektoru trid, restauracniho vozu, absence rodinneho vozu a podobne.

## Hardware

Arduino Nano Every (5V)
https://docs.arduino.cc/hardware/nano-every

RFID ctecka 125 kHz RDM6300
https://www.laskakit.cz/rfid-ctecka-s-antenou-125khz-em4100-rdm6300/

LED maticovy displej 32x8 MAX7219
https://www.laskakit.cz/32x8-led-matice-s-max7219-3mm/
-- nasledne vymenen za stejny, ale sestimodulovy

Indukcni smycka na svetla na vagonky
https://www.adafruit.com/product/5141

RTC hodiny DS3231
https://www.gme.cz/modul-rtc-ds323-i2c-a-32kb-flash

Hodinovy displej TM1637
https://www.laskakit.cz/hodinovy-displej-0-36--tm1637/

LED pasky u 3. koleje - dva 12V stripy zapojene do serie (!)
SQ-300 https://www.t-led.cz/p/led-pasek-vnitrni-sq3-300-7101
ovladane pres MOSFET modul IRF520
https://www.laskakit.cz/mosfet-tlacitko--irf520--24vdc-5a/

LED pasky uvnitr - opet dva 12V stripy zapojene do serie (!)
SQ-600 https://www.t-led.cz/p/led-pasek-vnitrni-sq3-600-7301
pripojene primo

RFID karty EM4100 125 kHz
https://www.laskakit.cz/rfid-em4100-125khz-neprepisovatelna-karta/

MP3 prehravac MH2024K-24SS
https://www.laskakit.cz/audio-mini-mp3-prehravac/
logika je 3v3, takze na arduino TX - mp3 RX spojeni je 1.5k rezistor
pozor, oba gnd piny musi byt propojene

Reproduktor 3W / 4R
https://www.laskakit.cz/reproduktor-3w-4-40mm/
pripojeny na SPKR1/2 piny mp3 prehravace

### Napajeni

Spinany 5/24V zdroj Meanwell RD-65B:
https://www.czech-meanwell.cz/meanwell/RD-65B-Mean-Well-Spinany-zdroj-uzavreny-dvouhladino-d2182.htm

5V - napaji primo LED, ctecku, neopixely + Arduino pres 5V pin
-- pripojeno do patice v rohu desky

24V - napaji indukcni civku a LED pasky
-- rozvedene pres mirne predimenzovane svorkovnice
-> indukcni civka
-> vnitrni LED stripy
-> mosfet pro LED strip na 3. koleji

### Pinout / zapojeni

#### Arduino Nano Every

(usb portem nahoru)
```
SCK   SPI CLK (LED CLK)         CIPO  (NC)
3V3   (NC)                      COPI  SPI COPI (LED DATA)
AREF  (NC)                      D10~  LED CS
D14   neopixel                  D9~   (NC)
D15   rotary B                  D8/SS (NC)
D16   rotary A                  D7    (NC)
D17   rotary switch             D6~   (NC)
SDA   I2C data (RTC)            D5~   (NC)
SDC   I2C clk (RTC)             D4    rfid
D20   clock-display clk         D3~   gleis 3 strip
D21   clock-display data        D2    strip yellow switch
5V    power bus +               GND   (NC)
RST   (NC)                      RST   (NC)
GND   power bus -               RX    MP3 TX
VIN   (NC)                      TX    MP3 RX+R
```

#### DFPlayer mini

(SD slotem dolu)
```
L19 VCC               x
L20 RX -> [TX]+R      x
L21 TX -> [RX]        x
L22 x                 x
L23 x                 x
L24 SPKR1             x
L25 GND               GND
L26 SPKR2             x
```

#### Speaker

L26 -> MP3 SPKR2
L27 -> MP3 SPKR1

#### RFID EM4100 RDM6300

(popiskem a antenou dolu)
```
L35 DATA -> [D4]    R35 x
L36 x               R36 VCC -> +
L37 x               R37 GND -> -
L38 x
L39 x
                    R47 ANT
                    R48 ANT
```

#### LED
(VCC,GND,DATA,CS,CLK bila,modra,cervena,zelena,cerna)
R28 VCC  -> +
R29 GND  -> -
R30 DATA -> [COPI] cervena
R31 CS   -> [D10] zelena
R32 CLK  -> [SCK] modra

#### hodinovy displej

(VCC,GND,DATA,CLK zelena,cervena,modra,cerna)

L29 VCC  -> +
L30 GND  -> -
L31 DATA -> [D21] modra
L32 CLK  -> [D20] cerna

#### MOSFET (gleis 3 strip)

zapojeny primo do patice (soucastkami dovnitr)

L40 GND -> -
L41 VCC -> +
L42 SIG -> [D3]

#### Neopixel

(VCC,DATA,GND cervena,hneda,cerna -> hneda, bila, cerna)
L50 VCC  -> +
L51 DATA -> [D14]
L52 GND  -> -

! prodluzka -- proto ty zmeny barev

#### Rotary switch
(GND,VCC,SWITCH,A,B cerna,zelena,cervena,modra,bila)

#### RTC modul

posledni dva jsou uplne nezapojene, vorsicht

GND -> -
VCC -> +
SDA -> [SDA]
SCL -> [SCL]
xxx
xxx

#### zlute tlacitko
L54 -> [D2]
L55 -> -

### Poznamky

EIC napajeci zasuvka je pripojena pres vypinac (na L zile) na vstup AC/DC zdroje.

Micro USB vyvedeny na vnejsi stenu je primo vytazeny z arduina. Asi nejlepsi napad je vypnout 5V/24V napajeni pred pripojenim USB kabelu, i kdyz je mozne, ze to neni uplne nutne. Pri vypnuti napajeni je ovsem trochu problem, ze vsechny komponenty jsou napajeny skrz Arduino, coz neni nejzdravejsi (problem je zejmena LED displej, ktery je schopen pri maximalnim rozsviceni odberu 1500 mA). Stejne tak MP3. Ale snad to neni megaproblem.

## Software

### Poznamky

nelze pouzit MD_Parola knihovnu na LED displej, bud s necim koliduje, nebo zacne prepisovat nejakou pamet, ale jakkoli 'funguje', tak zacne zpusobovat nepochopitelne problemy zabranujici nahravani sketche...

nastaveni RTC hodin neni udelane nijak chytre, v pripade potreby se provede jednorazovym odkomentovanim dvou radku v setup()

trochu zrada je, ze jak mp3, tak rfid pouzivaji uart. vzhledem k tomu, ze nejde pouzit (snadno) dva softwarove uarty najednou, je mp3 pripojeny na hw uart (piny rx+tx), rfid pak potrebuje jen rx pin a ten je SW.

### Knihovny

boards package: megaAVR

RDM6300 - bundled

MD_MAX72XX - 3.3.0

Adafruit NeoPixel - 1.10.5

TM1637 - 1.2.0

InputDebounce - 1.6.0

Rotary - 1.0.0

DS3232RTC - 2.0.1

DFPlayer Mini Mp3 by Makuna - 1.1.0

## Nadrazni hlaseni

Adresar `ansagen` obsahuje dva seznamy `departures-actual_sargans` (skutecne vlaky ze Sargansu) a `departures-extra` (vymyslene vlaky ze Sargansu, primerene zajimave trasy pro hledani na mape), coz jsou pseudoCSV ve formatu LINKA | TRASA | HLASENI | JINGLE. O dalsi postup se staraji rake tasky:

### timetable

Generuje `departures.json`, ktery obsahuje uz konkretne pripraveny 48-prvkovy dataset pro jednotlive vagonky.

Texty jsou ve variante `text` pro displej a `voice` pro text-to-speech syntetizer (format SSML).

Jingle je nazev znelky, viz `ansagen/resources/jingles/*.mp3` -- pozor, ffmpeg si s nimi neporadil, pokud to nebylo 44kHz. Ale jiste by to slo poladit, kdyby si nekdo ffmpeg poradne nastudoval...

```
  "0001": { # ID
    "train": { # oznaceni vlaku
      "text": "EC 193",
      "voice": "EuroCity 193"
    },
    "destination": { # trasa
      "text": "Memmingen, München Hbf",
      "voice": "Sankt Margarethen, Bregenz, Memmingen, Buchloe, München Hauptbahnhof"
    },
    "jingle": "db", # znelka
    "departure": [ # cas odjezdu H:M
      13,
      27
    ],
    "extras": [
      "Erste klasse Sektoren A und B, Zweite klasse sektoren B, C, D, E, F und G",
      "<lang xml:lang=\"fr-FR\">Restaurant</lang> Sektor B"
    ]
  },
```

### arduino

Generuje `sargans-arduino/destinations.h`, sadu retezcu pro LED banner.

### synth

Generuje MP3 hlaseni v adresari `ansagen/mp3`. Postup je nasledujici:
* sestaveni textu hlaseni
* synteza hlasu -- Amazon Polly (AWS)
* konverze na mp3 (soubory generovane primo polly nejak neprochazely pres ffmpeg)
* spojeni s prislusnym jinglem -- ffmpeg
* pridani reverb efektu -- ffmpeg

Je potreba mit nakonfigurovany AWS CLI, resp. vygenerovany konfigurak (pomoci `aws configure` nebo rucne).

## Jak to otevrit

1) k zakladne je pevne pripojena mala cast s napajeci zasuvkou

2) cela boudicka je pripevnena k zasuvce pouze dvema vruty zboku, jinak je to tam vlastne jen polozene -- je to ale sestavene dost na treni k tem vnejsim hranolkum podel sten, tak by to nemelo vadit

3) dreveny L-profil se zlutym tlacitkem a rotary switchem je ke stenam pripevnen opet dvema vruty

4) proskleny strop jde snadno vyndat. ano, je problem, ze az prilis snadno, drazky jsou moc hluboke, takze se to da shodit dovnitr. nejspis by stalo za to pridat nejake zajisteni na ten dil s tlacitky, ale zjistil jsem to uz moc pozde.