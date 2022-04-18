# Zeleznicni uzel Sargans

Firmware pro nadrazi z adventniho kalendare 2022.

## Popis funkci

vjezdove navestidlo (c.1) - mezi zelenou a cervenou se prepina cervenym tlacitkem

odjezdove navestidlo (c.2) - mezi ctyrmi rezimy se prepina otacenim knofliku, vypina a zapina se stiskem tehoz

hodiny: normalne ukazuji hodiny (na stridacku s datem), pripadne cislo vagonku (1/2,mezera,dvojciferne cislo)

LED displej - ukazuje linku a destinaci po prijezdu vagonku

vnitrni osvetleni - trvale zapnute

osvetleni koleje c. 3 - prepina se zlutym tlacitkem, navic blika pri prijezdu vagonku

reproduktor - hraje pri prijezdu vagonku

### Na koleji c.1/2

je 24V indukcni civka, rozsveci se svetylka na vagoncich

### Pri prijezdu vlaku na kolej c. 3

- rozblika se osvetleni koleje
- na LED banneru se zobrazi linka / destinace
- zahraje se melodie na vagonku a to:
a) pokud je dnes den daneho vagonku, tak pro Vorsilu Old McDonald's a pro Vlka Tschipfu-tschi en Isebahn chunnt
b) v ostatni dny prislusna zeleznicni znelka (SBB nemecka/francouzska/italska, SNCF, Trenitalia)

## Hardware

Arduino Nano Every (5V)
https://docs.arduino.cc/hardware/nano-every

RFID ctecka 125 kHz RDM6300
https://www.laskakit.cz/rfid-ctecka-s-antenou-125khz-em4100-rdm6300/

LED maticovy displej 32x8 MAX7219
https://www.laskakit.cz/32x8-led-matice-s-max7219-3mm/

Indukcni smycka na svetla na vagonky
https://www.adafruit.com/product/5141

RTC hodiny DS3231
https://www.laskakit.cz/rtc-hodiny-realneho-casu-ds3231--integrovana-baterie/

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

Zesilovac PAM8403 (pouziva se levy kanal)
https://www.laskakit.cz/2x3w-digitalni-audio-zesilovac-pam8403/

Reproduktor 3W / 4R
https://www.laskakit.cz/reproduktor-3w-4-40mm/

### Napajeni

Spinany 5/24V zdroj Meanwell RD-65B:
https://www.czech-meanwell.cz/meanwell/RD-65B-Mean-Well-Spinany-zdroj-uzavreny-dvouhladino-d2182.htm

5V - napaji primo LED, ctecku, neopixely + Arduino pres 5V pin
-- pripojeno do patice v rohu desky

24V - napaji indukcni civku a LED pasky
-- rozvedene pres mirne predimenzovane svorkovnice

### Pinout pro arduino

```
SCK   SPI CLK (LED CLK)         CIPO  (NC)
3V3   (NC)                      COPI  SPI COPI (LED DATA)
AREF  (NC)                      D10~  LED CS
D14   neopixel                  D9~   MP3 RX
D15   rotary B                  D8/SS MP3 TX
D16   rotary A                  D7    signal1 red switch
D17   rotary switch             D6~   (NC)
SDA   I2C data (RTC)            D5~   (NC)
SDC   I2C clk (RTC)             D4    rfid
D20   clock-display clk         D3~   gleis 3 strip
D21   clock-display data        D2    strip yellow switch
5V    power bus +               GND   (NC)
RST   (NC)                      RST   (NC)
GND   power bus -               RX    (NC)
VIN   (NC)                      TX    (NC)
```

### Zapojeni komponent na desku + kabely

5V napajeni
v rohu

zlute tlacitko
R20-21

cervene tlacitko
R21-22

neopixel (semafory)
R25-27 (VCC,DATA,GND)

rotary
L19-L23 (GND,VCC,SWITCH,A,B cerna,zelena,cervena,modra,bila)

hodinovy displej
R53-56 (VCC,GND,DATA,CLK zelena,cervena,modra,cerna)

RTC
L52-55 (VCC,DATA,CLK,GND zluta,oranzova,cervena,hneda)
--- tohle je naprosty fail, mohlo to byt primo na desce

LED
L25-29 (VCC,GND,DATA,CS,CLK bila,modra,cervena,zelena,cerna)

RFID
- antenovy konektor R59-60 -- patice
- napajeci konektor R47-79 NC,VCC,GND
- datovy konektor L47-51 DATA,NC,NC,NC,NC

### Mechanicke poznamky

EIC napajeci zasuvka je pripojena pres vypinac (na L zile) na vstup AC/DC zdroje.

Micro USB vyvedeny na vnejsi stenu je primo vytazeny z arduina. Asi nejlepsi napad je vypnout 5V/24V napajeni pred pripojenim USB kabelu, i kdyz je mozne, ze to neni uplne nutne. Pri vypnuti napajeni je ovsem trochu problem, ze vsechny komponenty jsou napajeny skrz Arduino, coz neni nejzdravejsi (problem je zejmena LED displej, ktery je schopen pri maximalnim rozsviceni odberu 1500 mA).
* mozna by se dal displej udelat vypinaci

## Software

### Poznamky

nelze pouzit MD_Parola knihovnu na LED displej, bud s necim koliduje, nebo zacne prepisovat nejakou pamet, ale jakkoli 'funguje', tak zacne zpusobovat nepochopitelne problemy zabranujici nahravani sketche...

nastaveni RTC hodin neni udelane nijak chytre, v pripade potreby se provede jednorazovym odkomentovanim dvou radku v setup()

### Knihovny

RDM6300 - bundled

MD_MAX72XX - 3.3.0

Adafruit NeoPixel - 1.2.3

TM1637 - 1.2.0

InputDebounce - 1.6.0

Rotary - 1.0.0

DS3232RTC - 2.0.0

## Nadrazni hlaseni

viz adresar ansagen

skutecne linky 18x
vymyslene 30x

## Poznamky / WIP

zvuky

SBB https://www.mobiles24.co/search?q=SBB+CFF+FFS+Swiss+Railway+Station+Jingle+Chime&c=free-mp3-ringtones
SNCF
https://www.youtube.com/watch?v=NA5MwhuHWLo

TODO: speaker

TODO: napajeni 5v

TODO: vnitrni pasky

TODO: vymenit kablik na antenu

TODO: prehodit asi tu d8 na d7

VNITRNI ROZMER 120mm

VZDALENOST NAVESTIDEL 200mm