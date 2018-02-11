Autor: Ondřej Valeš
Login: xvales03
Projekt: IPK 2 - Přenos souborů
Datum: 21. 4. 2016

Programy slouží pro přenos souborů od klienta na server a naopak. Využívají 
vlastní aplikační protokol popsaný v dokumentace.pdf. Přenášené soubory se 
ukládají do stejného adresáře ve kterém by program spuštěn (klient i server).
Z klienta lze odeslat i soubor, který se nachází v jiném adresáři, u serveru 
toto není možné (odesílá pouze soubory z aktuálního adresáře).

Návod k použití
	1. přeložit pomocí make (vyžaduje g++)
	2. přesunout server a client do požadovaného adresáře
	3. spustit server, n udává číslo portu
		>./server -p n
	4. spustit client pomocí jednoho z příkazů, host udává jméno serveru na 
	   kterém běží server, n je číslo portu (stejné jako v bodě 3), filename 
	   udává jméno souboru pro nahrání/stažení a přepínač -d/-u udává zda 
	   chceme stáhnout soubor ze serveru (-d) nebo nahrát soubor (-u)	   
		>./client -h host -p n -d filename
		>./client -h host -p n -u filename
	5. client se po provedení jedné akce ukončí, lze jej spouštět opakovaně
	6. server běží v nekonečné smyčce, ukončit pomocí kill -9 PID

Specifikace
Server i klient je možné bezpečně spustit v jednom adresáři. Pokud je v jednom 
adresáři spuštěno více clientů nebo serverů zaráz není zaručen výlučný zápis do
douborů. Výlučný zápis do souboru není zaručen ani při současnám příchodu 
více požadavků na zápis stejného souboru na server.
Server je schopen obsluhovat více požadavků naráz, vzužívá pro to c++ threads.