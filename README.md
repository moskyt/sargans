# Zeleznicni uzel Sargans

Firmware pro nadrazi z adventniho kalendare 2022.

## Hardware

Arduino Nano Every (5V)
https://docs.arduino.cc/hardware/nano-every

RFID ctecka 125 kHz RDM6300
https://www.laskakit.cz/rfid-ctecka-s-antenou-125khz-em4100-rdm6300/

LED maticovy displej 32x8 MAX7219
https://www.laskakit.cz/32x8-led-matice-s-max7219-3mm/

Indukcni smycka na svetla na vagonky
https://www.adafruit.com/product/5141

LED pasky u koleje - dva 12V stripy zapojene do serie (!), zapojene pres MOSFET modul

### Napajeni

Spinany 5/24V zdroj Meanwell RD-65B:
https://www.czech-meanwell.cz/meanwell/RD-65B-Mean-Well-Spinany-zdroj-uzavreny-dvouhladino-d2182.htm

5V - napaji primo LED, ctecku, neopixely + Arduino pres 5V pin

24V - napaji indukcni civku a LED pasky

### Mechanicke poznamky

EIC napajeci zasuvka je pripojena pres vypinac (na L zile) na vstup AC/DC zdroje.

Micro USB vyvedeny na vnejsi stenu je primo vytazeny z arduina. Asi nejlepsi napad je vypnout 5V/24V napajeni pred pripojenim USB kabelu, i kdyz je mozne, ze to neni uplne nutne. Pri vypnuti napajeni je ovsem trochu problem, ze vsechny komponenty jsou napajeny skrz Arduino, coz neni nejzdravejsi (problem je zejmena LED displej, ktery je schopen pri maximalnim rozsviceni odberu 1500 mA).
* mozna by se dal displej udelat vypinaci

## Knihovny

RDM6300 - bundled

MD_Parola 3.5.6

MD_MAX72XX 3.3.0

Adafruit NeoPixel 1.2.3