/*
 * help_czech.h
 * Program help in Czech language.
 *
 * Copyright 2006 by Dan Masek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

START_HELP_TEXT(RL_CZECH)
"Pouziti: nforenum [moznosti] [soubor1 [soubor2 [soubor3 [...]]]]\n"
"   Je mozne specifikovat libovolne mnozstvi vstupnich souboru.\n"
"   V pripade, ze nebyly specifikovany zadne soubory ani prepinace\n"
"   NFORenum bude cist ze standardniho vstupniho proudu a zapisovat\n"
"   do standardniho vystupniho proudu.\n"
"Moznosti:\n"
"   --comments=<typ> -c <typ>\n"
"       <typ> je znak (bud /, ;, nebo #), a specifikuje styl komentaru,\n"
"       ktery bude NFORenum pouzivat. Toto nastaveni neovlivni uvodni\n"
"       hlavicku souboru kvuli pozadavkum programu grfcodec.\n"
"   --data[=<adresar>] -D[<adresar>]\n"
"       Jestlize je specifikovan <adresar>, NFORenum v nem bude hledat\n"
"       adresar .nforenum a v pripade, ze neexistuje jej vytvori.\n"
"       Jestlize neni specifikovan <adresar>, NFORenum vypise cestu\n"
"       k stavajicimu datovemu adresari.\n"
"       V kazdem pripade pouziti tohoto parametru odstrani 5 vterinovou\n"
"       pauzu po vytvoreni adresare .nforenum urcenou pro to, aby uzivatele,\n"
"       kteri nespousteji program z prikazove radky, meli moznost spatrit\n"
"       zpravu o vytvoreni adresare.\n"
"       Standardni chovani: NFORenum hleda adresar .nforenum ve stavajicim\n"
"       adresari a potom v adresari nastavenem v promenne prostredi HOME\n"
"       (je-li definovana). V pripade, ze adresar neni nalezen, NFORenum\n"
"       jej nejprve zkusi vytvorit v HOME a pote ve stavajicim adresari.\n"
"   --force -f\n"
"       Nucene zpracovani souboru, ktere nevypadaji jako NFO.\n"
"       Standardni chovani: NFORenum zpracuje tyto soubory jako by mely\n"
"       specifikovanou prilis vysokou verzi NFO.\n"
"   --help -h\n"
"       Zobrazi tuto napovedu.\n"
"   --lock\n"
"       Uzamkne stavajici stav komentarovych prikazu. Prikazy budou\n"
"       analyzovany obvyklym zpusobem (takze NOPRESERVE bude platit,\n"
"       @@DIFF bude odstranen, atd) ale nebudou vykonavat zadnou funkci.\n"
"   --no-replace --keep-old -k\n"
"       Nenahrazovat puvodni soubor; NFORenum vytvori pro vystup novy\n"
"       soubor se jmenem ve stylu soubor.new.nfo.\n"
"       Standardni chovani: NFORenum pouzije soubor soubor[.nfo].new jako\n"
"       docasny a po ukonceni zpracovani jej prejmenuje na soubor[.nfo]\n"
"   --auto-correct -a\n"
"       NFORenum provede zakladni automatickou opravu chybnych pseudo-\n"
"       spritu. V pripade, ze je tento prepinac zadan dvakrat, NFORenum\n"
"       bude provadet i opravy, u nichz je vyssi sance, ze budou chybne.\n"
"\n"
"Nasledujici prepinace zpusobi, ze se NFORenum bude chovat, jako by\n"
"vsechny vstupni soubory zacinaly odpovidajicim komentarovym prikazem.\n"
"Kompletni popis vsech komentarovych prikazu je v souboru README.\n"
"Dodatecne informace pro prikazy ktere je vyzaduji, je mozne zadat\n"
"jako argument prepinace. S vyjimkou prepinace -L/--let NFORenum\n"
"u argumentu nerozlisuje mezi velkymi a malymi pismeny.\n"
"\"ON\" a \"OFF\" lze take zadat jako \"+\" and \"-\".\n"
"   --beautify -b               @@BEAUTIFY\n"
"   --diff -d                   @@DIFF\n"
"   --let -L                    @@LET\n"
"   --lint -l                   @@LINT\n"
"   --preserve-messages -p      @@PRESERVEMESSAGES\n"
"   --real-sprites -r           @@REALSPRITES\n"
"   --use-old-nums -o           @@USEOLDSPRITENUMS\n"
"   --warning-disable -w        @@WARNING DISABLE\n"
"   --warning-enable -W         @@WARNING ENABLE\n"
"       Jako argument prepinacu -w a -W (a jejich dlouhych variant)\n"
"       lze take zadat seznam carkou oddelenych ID zprav, ktere maji byt\n"
"       aktivovany ci deaktivovany.\n"
"\n"
"NFORenum - Copyright 2004-2009 Dale McCoy\n"
"Tento program lze kopirovat a sirit dle podminek GNU General Public\n"
"License obsazenych v souboru 'COPYING'.\n"
"\n"
"V pripade, ze jste nestihli precist celou zpravu, zkuste pouzit\n"
"   nforenum -h | more\n"
END_HELP_TEXT()
