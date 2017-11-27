# svdview

Při pokusech s programováním ARM Cortex-Mx procesorů v exotických jazycích
jako je např. rust jsem narazil na stránku https://github.com/posborne/cmsis-svd,
kde je v jazyce python napsán parser SVD (System View Description) souborů,
které lze získat pro poměrně dost vyráběných čipů. Na této stránce je možné
dohledat velkou sbírku dat pro nejrůznější typy procesorů. Někteří výrobci tuto
informaci poskytují přímo na svých stránkách a tedy tyto soubory by měly obsahovat
poněkud relevantnější informace. Zřejmě je to strojově generováno z výrobní dokumentace,
bohužel si to každý chápe po svém a vybrat z toho potřebné informace plně univerzálním
způsobem je skoro nemožné.

První co jsem z toho chtěl udělat je obecný prohlížeč toho svd souboru, který má dost
jednoduchý formát xml. Tohle se povedlo, protože je to jednoduché. Na druhou stranu
prohlížet registry tímto způsobem je dost nepohodlné a po pravdě řečeno nic moc nepřináší.

Druhá věc, když už je tohle hotové, je vyrobit z těchto dat strom popisující periferie
celého čipu a z tohoto stromu vygenerovat třeba hlavičku pro C/C++. To už taková legrace
není, protože někdo chápe šířku slova v bitech, někdo v bytech, jsou tam různě popisovaná
pole registrů, jejich seskupování do tříd si také dělá každý po svém, takže univerzální
metoda zřejmě neexistuje. Ale jsou k tomu zdrojáky, které se dají přiohnout a pak to může
fungovat skoro pro všechny čipy. Ale bacha - už v samotných svd souborech mohou být chyby.

Co je nesporná výhoda - formát té hlavičky si může každý přiohnout po svém. Pokud tam nechcete
ty uniony a bitfieldy, lze to přepnout do režimu klasického výpisu a lze si zvolit, zda konstanty
chcete jako statické nebo jako definice preprocesoru. Já třeba používám C++ a tak definice
registru jako struktury může obsahovat i metody. To se dá docela dobře použít např. pro nastavování
skupin bitů najednou postupem read-modify-write. Příklady jsou v adresáři ./tests/arm/.
V adresáři ./tests/f4Disco/work/ je přepsán starší projekt pro F4 Discovery - midi player,
což už je složitější a ukazuje, že vygenerovaná hlavička je zřejmě použitelná.

Funguje to tedy pro soubory od STM, pro některá LPC od NXP, málokterý Atmel a snad i pro některé
exotičtější typy. Freescale (a asi i jiní) má v svd souborech nelogické věci, asi by to šlo
zkorigovat, ale chce to RM a srovnat si co tam má opravdu být. Fakticky když programujeme
přímo do registrů, je dobré mít RM otevřen a údaje průběžně aslespoň trochu kontrolovat.
Některé věci v hlavičce lze pak snadno opravit, je to mnohem méně práce než to psát od podlahy
znova. A kvůli tomu jsem to dělal.

Celé je to uděláno v Qt ve verzi 4.x, Qt5 jsem netestoval, ale taky by neměly být větší problémy.
Pro testy je potřeba arm-none-eabi-gcc/g++, testováno s verzí 6.3.1, g++ musí umět rozšíření C++14.
Všechny Makefile jsou psány pro Linux, ve Win s tím budou potíže (příkazy rm, mv, wget ...).

Tento program je rozšiřován v naději, že bude užitečný, avšak BEZ JAKÉKOLIV ZÁRUKY. Neposkytují se ani odvozené záruky
PRODEJNOSTI anebo VHODNOSTI PRO URČITÝ ÚČEL. Další podrobnosti hledejte v Obecné veřejné licenci GNU (http://www.gnugpl.cz/).

