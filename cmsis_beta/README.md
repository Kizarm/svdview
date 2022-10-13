# SVD Viewwer BETA

## Generátor hlaviček
Původní Qt program byl udělán ad hoc. Tento pokus je o něco sofistikovanější,
snaží se dodržet standard CMSIS. Hlavička je generována výhradně pro C++,
kromě toho se generuje template pro startup kód, který využívá popis přerušení
v SVD souborech. Není a nemůže to být dokonalé, ale je to použitelné,
vzhledem k tomu, že je to C++, je možné zkontrolovat délky struktur už
při překladu. Protože v tom dělají problémy enumerace, default jsou enumerace
jen deklarovány, je však možné zapnout jejich použití ve starém stylu
(pokud arm-none-eabi-g++ máme ve verzi menší než 9.3), nebo v novém, kdy
se pro enum v bitovém poli používá správný typ proměnné. Clang a novější
g++ se s tím už vyrovnají, starší verze g++ vyhodí spoustu nesmyslných
varování, které nejdou vypnout (bug).

Template pro startup je nutné brát s rezervou, tabulka vektorů je úplná,
včetně všech systémových přerušení. To může dělat problémy, některé čipy
očekávají na určitých adresách určité hodnoty (typicky NXP LPC...),
jinak nefungují. Většině čipů ale nevadí, pokud v systémové části tabulky
má vektor obsazen i když daná periferie chybí.

Jediné externí knihovny jsou standardní C++ STL, default překladač je clang.
Je možné (úpravou Makefile) použít gcc/g++.

## TEST - Hello
Ukázka použití pro čip STM32F051. Bliká ledkou přes systémový časovač
a vypisuje na sériový port (přes přerušení) text "Hello world". Takže
je v tom vlastně skoro vše, co je tak v praxi potřeba a je vidět, že
dopsat se toho nemusí zase tak moc. Je to samozřejmě extrémní příklad
bare metal programování jaké nepředpokládá žádný cizí kód.

I tady je použit clang, pokud je nainstalováno LLVM binutils, včetně ld.lld,
nic jiného není potřeba. Je možné použít arm-none-eabi-gcc/g++/binutils.

## TEST - HTML
Zobrazení registrů ve webovém prohlížeči. Zde se používá emscripten, což
je také převlečený clang.

## Licence
Vzhledem k tomu, že jsem to dělat, abych se něco naučil a tohle bude mít zřejmě
velmi omezené použití, řekl bych dělejte si s tím co chcete. Jenže ARM
má licencční omezení jak na formát CMSIS, tak mají výrobci omezení na SVD
soubory (mívají vlastní licenci), je na to nutné brát ohled.
