NFORenum v3.4.3
Nástroj pro přečíslování a kontrolu NFO souborů


Kompilace
=========

NFORenum lze kompilovat pomocí g++ 3.4.4, g++ 4.0.2, a VS .NET v případě,
že máte nainstalovanou knihovnu Boost. Knihovna Boost je dostupná ke stažení
na stránkách www.boost.org.
Potřebný .h soubor z knihovny Boost je přiložen. Jestliže nemáte zbytek
knihovny, make by to měl automaticky detekovat a odstranit závislost
na knihovně.
Více informací o kompilaci programu naleznete ve zdrojovém kódu v souboru
0compile.txt. 

Instalace
=========

Nástroj nainstalujete překopírováním souboru renum.exe do jakéhokoliv adresáře
na disku, pokud možno, tak do adresáře, který je ve standardní cestě
(proměnná prostředí PATH). NFORenum si, jestliže bude třeba, vytvoří pracovní
adresář se jménem .renum, kam bude ukládat veškeré datové soubory.
Při upgradu programu není třeba tento adresář odstraňovat - program
automaticky obnoví zastaralé soubory.
K odinstalování stačí smazat renum.exe a všechny datové .renum adresáře.


Užití
=====

Kompletní seznam s popisem všech možností programu získáte spuštěním příkazu
renum -? nebo renum --help.

Pro přečíslování NFO souborů (např. MujGRF1.NFO a MujGRF2.NFO) použijte

> renum [možnosti] MujGRF1 MujGRF2

Tento příkaz automaticky přečísluje všechny sprity, opraví velikosti pseudo-
spritů a hodnotu pseudo-spritu č.0 a provede částečnou kontrolu syntaxe kódu.
V případě, že soubor neobsahuje NFO hlavičku, bude tato doplněna. Pakliže
soubor nezačíná číslicí, hvězdičkou, mezerou (tabulátorem, novým řádkem),
nebo jedním ze třech znaků označujících komentáře (/, ;, #) a program nebyl
spuštěn s parametrem -f/--force, soubor nebude zpracován.

Čísla spritů je nutné zadat pouze u real-spritů, v případech, když název
PCX souboru začíná číslicí.

NFORenum vypisuje zprávy jak do konzole, tak do výstupního NFO souboru.
Detailní popis všech chybových zpráv najdete v souboru SANITY.cz.txt.

Sprity, které z nějakého důvodu nemohou být zpracovány jsou umístěny do
komentáře a analýza pokračuje na dalším spritu.

NFORenum hledá NFO soubory v následujících místech a pořadí:
  <název-souboru>
  <název-souboru>.nfo
  sprites/<název-souboru>
  sprites/<název-souboru>.nfo

V případě, že na příkazové řádce nebyly specifikovány žádné parametry, NFORenum
čte ze standardního vstupu a zapisuje do standardního výstupu.

Standardní cesty použité při hledání/vytváření datového adresáře .renum jsou
uvedeny v následujícím seznamu:
  . (stávající adresář)
  $HOME
  $HOMEDRIVE$HOMEPATH
V případě, že adresář .renum není nalezen ani v jednom z těchto míst, NFORenum
se jej bude pokoušet vytvořit na každém z míst na tomto seznamu v opačném pořadí
dokud se to nezdaří. Toto chování lze změnit pomocí parametru --data.

NFORenum po může po ukončení vrátit jeden z následujících chybových kódů: 
(V případě několika různých chyb je vždy vracena nejvyšší hodnota.)
8: Kritická chyba programu. (Stalo se něco, co se stát nemělo.)
7: Chyba datového souboru. (Nezdařilo se vytvořit datový adresář
                            nebo otevřít soubor).
6: Chyba NFO souboru. (Nezdařilo se otevřít soubor, nebo přejmenovat
                       dočasný soubor.)
5: Chyba analýzy NFO souboru. (Sprite s chybnými znaky či délkou, nebo příliš
                               vysoká verze NFO kódu.)
4: Chyba při kontrole kódu.
3: Varování při kontrole kódu.
0: Bezchybné provedení programu.

Chybové kódy č. 7 a 8 representují chyby, ke kterým by při běhu programu nemělo
nikdy dojít - v případě, že k nim dojde, NFORenum vypíše do konzole chybové
hlášení a ihned ukončí běh.
Chybové kódy č. 1 a 2 jsou rezervované. Více informací je na stránkách
http://tinyurl.com/acudt.


Známé problémy
==============

- Akce 6 měnící jakýkoliv kontrolovaný sprite může způsobit chybná chybová
  hlášení.
  Stav: Nízká priorita, je těžké to ošetřit správně.
  Dočasné řešení: Použijte příkazy @@USEID2 a @@USESET k potlačení chybných
  zpráv.
- @@VERSIONCHECK nedává svou druhou akci 9 na správné místo.
  Řešení: Manuálně tuto akci 9 přesuňte za akci 8.
