NFORenum -- RPN kalkulátor
==========================

Vetšina z vás zná infixovou notaci výrazu, např. "4 + 2" nebo "4 + (2 * 2)".
Infixový zápis lze jednoduše popsat jako "levá operátor pravá".
RPN (Reverse Polish Notation - postfixová Polská notace) pak používá princip
"levá pravá operátor", takže předchozí výrazy by byly napsány jako "4 2 +"
resp. "4 2 2 * +". Výhodou RPN je, že zápis výrazu nevyžaduje žádné závorky;
každý dobře formovaný výraz lze vyhodnotit pouze jedním zpusobem.

V NFORenumu lze začít výraz v RPN levou závorkou "(" a může využívat operátoru
+, -, * a /. Výraz je ukončen pravou závorkou ")" a po vyhodnocení je celý výraz
v souboru nahrazen výsledkem. Přebývající čísla jsou ignorována, např. výsledek
výrazu (1 2 3 +) se rovná 5. Je-li číslo uvedeno na konci výrazu, je použito
jako výsledek, tzn. výsledek výrazu (1 2 + 3) a (3) je 3. NFORenum podporuje
pouze dělení celých čísel a výsledek je vždy zaokrouhlen dolů; např. výsledek
výrazu (5 3 /) je 1.

V RPN výrazech lze nahradit jakékoliv číslo názvem proměnné, která již byla v
souboru dříve definována pomocí příkazu @@LET. Proměnné lze používat jak v real-
spritech, tak v příkazech @@LET. Název Proměnné by mel být platný identifikátor
jazyka C (první znak musí být A..Z, a..z nebo _, všechny další musí být A..Z,
a..z, 0..9 nebo _); NFORenum rozlišuje v názvech proměnných velikost písmen.
Názvy proměnných, které začínají dvěma podtržítky jsou rezervované pro budoucí
použití programem.

Výrazy RPN lze použít v real-spritech na jakémkoliv místě, kde je očekáváno celé
číslo v desítkové soustavě anebo na pravé straně v příkazu @@LET.

Vnořené RPN výrazy nejsou povoleny.

Špatně zapsaný RPN výraz způsobí umístění celého spritu, jenž jej obsahuje, do
komentáře, jako by v něm chyběla metadata. NFORenum se pokusí co nejlépe
informovat o tom, co bylo špatně.

Záporná čísla prozatím nelze v RPN výrazech přímo používat. V případě, že chcete
zadat záporné číslo, např. "-5", použijte "0 5 -". Druhou možností je definovat
proměnnou "@@LET prom=-5", a pak použít "prom" namísto záporného čísla.

Analyzér RPN výrazu má následující známá omezení:
1) Nelze zadat záporná čísla. (použijte "0 <číslo> -", nebo proměnnou)
2) Pouze 4 základní aritmetické operace: +, -, *, /
