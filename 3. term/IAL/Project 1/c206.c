	
/* c206.c **********************************************************}
{* Téma: Dvousmìrnì vázaný lineární seznam
**
**                   Návrh a referenèní implementace: Bohuslav Køena, øíjen 2001
**                            Pøepracované do jazyka C: Martin Tuèek, øíjen 2004
**                                            Úpravy: Bohuslav Køena, øíjen 2015
**
** Implementujte abstraktní datový typ dvousmìrnì vázaný lineární seznam.
** U¾iteèným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován promìnnou
** typu tDLList (DL znamená Double-Linked a slou¾í pro odli¹ení
** jmen konstant, typù a funkcí od jmen u jednosmìrnì vázaného lineárního
** seznamu). Definici konstant a typù naleznete v hlavièkovém souboru c206.h.
**
** Va¹ím úkolem je implementovat následující operace, které spolu
** s vý¹e uvedenou datovou èástí abstrakce tvoøí abstraktní datový typ
** obousmìrnì vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu pøed prvním pou¾itím,
**      DLDisposeList ... zru¹ení v¹ech prvkù seznamu,
**      DLInsertFirst ... vlo¾ení prvku na zaèátek seznamu,
**      DLInsertLast .... vlo¾ení prvku na konec seznamu, 
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek, 
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku, 
**      DLDeleteFirst ... zru¹í první prvek seznamu,
**      DLDeleteLast .... zru¹í poslední prvek seznamu, 
**      DLPostDelete .... ru¹í prvek za aktivním prvkem,
**      DLPreDelete ..... ru¹í prvek pøed aktivním prvkem, 
**      DLPostInsert .... vlo¾í nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vlo¾í nový prvek pøed aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... pøepí¹e obsah aktivního prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na dal¹í prvek seznamu,
**      DLPred .......... posune aktivitu na pøedchozí prvek seznamu, 
**      DLActive ........ zji¹»uje aktivitu seznamu.
**
** Pøi implementaci jednotlivých funkcí nevolejte ¾ádnou z funkcí
** implementovaných v rámci tohoto pøíkladu, není-li u funkce
** explicitnì uvedeno nìco jiného.
**
** Nemusíte o¹etøovat situaci, kdy místo legálního ukazatele na seznam 
** pøedá nìkdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodnì komentujte!
**
** Terminologická poznámka: Jazyk C nepou¾ívá pojem procedura.
** Proto zde pou¾íváme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozornìní na to, ¾e do¹lo k chybì.
** Tato funkce bude volána z nìkterých dále implementovaných operací.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální promìnná -- pøíznak o¹etøení chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L pøed jeho prvním pou¾itím (tzn. ¾ádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádìt nad ji¾ inicializovaným
** seznamem, a proto tuto mo¾nost neo¹etøujte. V¾dy pøedpokládejte,
** ¾e neinicializované promìnné mají nedefinovanou hodnotu.
**/
	L->First = NULL;
	L->Act = NULL;
	L->Last = NULL;
}

void DLDisposeList (tDLList *L) {
/*
** Zru¹í v¹echny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Ru¹ené prvky seznamu budou korektnì
** uvolnìny voláním operace free. 
**/
	//Mazu vzdy aktivni prvek
	L->Act = L->First;
	//Prochazim cely seznam
	while (L->First != NULL)
	{
		//Posunu zacatek seznamu
		L->First = L->Act->rptr;
		//Uvolnim aktivni prvek a posunu se na dalsi
		free(L->Act);
		L->Act = L->First;
	}
	L->Act = NULL;
	L->Last = NULL;
}

void DLInsertFirst (tDLList *L, int val) {
/*
** Vlo¾í nový prvek na zaèátek seznamu L.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
	//Promenna pro novy prvek
	tDLElemPtr aux = malloc(sizeof(struct tDLElem));
	if (aux == NULL)
		DLError();
	else
	{
		//Naplnim novy prvek daty
		aux->data = val;
		//Zaradim prvek na zacatek
		aux->rptr = L->First;
		aux->lptr = NULL;
		//Pokud je v seznamu jediny prvek je soucasne prvni i posledni, pokud ne nastavim ukazatel jeho nastupce
		if (L->First != NULL)
			L->First->lptr = aux;
		else
			L->Last = aux;
		L->First = aux;
	}
}

void DLInsertLast(tDLList *L, int val) {
/*
** Vlo¾í nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
	//Promenna pro novy prvek
	tDLElemPtr aux = malloc(sizeof(struct tDLElem));
	if (aux == NULL)
		DLError();
	else
	{
		//Naplnim novy prvek daty a zaradim jej na konec
		aux->data = val;
		aux->rptr = NULL;
		aux->lptr = L->Last;
		//Pokud je novy prvek jediny v seznamu je zaroven prvni i poledni, pokud neni upravim ukazatel jeho predchudce
		if (L->Last != NULL)
			L->Last->rptr = aux;
		else
			L->First = aux;
		L->Last = aux;
	}
}

void DLFirst (tDLList *L) {
/*
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný pøíkaz (nepoèítáme-li return),
** ani¾ byste testovali, zda je seznam L prázdný.
**/
	L->Act = L->First;
}

void DLLast (tDLList *L) {
/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný pøíkaz (nepoèítáme-li return),
** ani¾ byste testovali, zda je seznam L prázdný.
**/
	L->Act = L->Last;
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prostøednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
	if(L->First == NULL)
		DLError();
	else
		*val = L->First->data;
}

void DLCopyLast (tDLList *L, int *val) {
/*
** Prostøednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
	if(L->Last == NULL)
		DLError();
	else
		*val = L->Last->data;
}

void DLDeleteFirst (tDLList *L) {
/*
** Zru¹í první prvek seznamu L. Pokud byl první prvek aktivní, aktivita 
** se ztrácí. Pokud byl seznam L prázdný, nic se nedìje.
**/
	tDLElemPtr aux = L->First;
	if (aux != NULL)
	{
		//Posunu zacatek seznamu na nasledujici prvek
		L->First = aux->rptr;
		//Pokud zadny nasledujici neni seznam je prazdny, pokud je nastavim jeho ukazatel na predchudce
		if (L->First != NULL)
			L->First->lptr = NULL;
		else
			L->Last = NULL;
		free(aux);
		//Kontrola zda nedoslo ke ztrate aktivity
		if (aux == L->Act)
			L->Act = NULL;
	}
}	

void DLDeleteLast (tDLList *L) {
/*
** Zru¹í poslední prvek seznamu L. Pokud byl poslední prvek aktivní,
** aktivita seznamu se ztrácí. Pokud byl seznam L prázdný, nic se nedìje.
**/ 
	//Posunu konec seznamu na predesly prvek
	tDLElemPtr aux = L->Last;
	if (aux != NULL)
	{
		//Pokud zadny predchudce neni seznam je prazdny, pokud je nastavim jeho ukazatel na nasledovnika
		L->Last = aux->lptr;
		if (L->Last != NULL)
			L->Last->rptr = NULL;
		else
			L->First = NULL;
		free(aux);
		//Kontrola zda nedoslo ke ztrate aktivity
		if (aux == L->Act)
			L->Act = NULL;
	}
}

void DLPostDelete (tDLList *L) {
/*
** Zru¹í prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se nedìje.
**/
	tDLElemPtr aux = L->Act;
	//Seznam je aktivni a ma nasledovnika
	if (aux != NULL && aux->rptr != NULL)
	{
		//Mazany prvek
		aux = aux->rptr;
		//V aktivnim prvku upravim ukazatel na nastupce
		L->Act->rptr = aux->rptr;
		//Pokud byl mazany prvek posledni upravim ukazatel na konec seznamu, pokud nebyl zpetne spojim seznam
		if (aux->rptr != NULL)
			aux->rptr->lptr = L->Act;
		else
			L->Last = L->Act;
		free(aux);
	}
}

void DLPreDelete (tDLList *L) {
/*
** Zru¹í prvek pøed aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se nedìje.
**/
	tDLElemPtr aux = L->Act;
	//Seznam je aktivni a ma predcudce
	if (aux != NULL && aux->lptr != NULL)
	{
		aux = aux->lptr;		
		//V aktivnim prvku upravim ukazatel na predchudce
		L->Act->lptr = aux->lptr;
		//Pokud byl mazany prvek prvni upravim ukazatel na zacatek seznamu, pokud nebyl zpetne spojim seznam
		if (aux->lptr != NULL)
			aux->lptr->rptr = L->Act;
		else
			L->First = L->Act;
		free(aux);
	}
}

void DLPostInsert (tDLList *L, int val) {
/*
** Vlo¾í prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se nedìje.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
	if (L->Act != NULL)
	{
		//Novy prvek
		tDLElemPtr aux = malloc(sizeof(struct tDLElem));
		if (aux == NULL)
			DLError();
		else
		{
			//Naplnim novy prvek daty
			aux->data = val;
			aux->rptr = L->Act->rptr;
			aux->lptr = L->Act;
			//Pokud je novy prvek posledni v seznamu upravim ukazatel na konec seznamu, pokud ne upravim jeho nastupce
			if (aux->rptr != NULL)
				aux->rptr->lptr = aux;
			else
				L->Last = aux;
			//Novy prvek zaradim za aktualni
			L->Act->rptr = aux;
		}
	}
}

void DLPreInsert (tDLList *L, int val) {
/*
** Vlo¾í prvek pøed aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se nedìje.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
	if (L->Act != NULL)
	{
		//Novy prvek
		tDLElemPtr aux = malloc(sizeof(struct tDLElem));
		if (aux == NULL)
			DLError();
		else
		{
			//Naplnim novy prvek daty
			aux->data = val;
			aux->rptr = L->Act;
			aux->lptr = L->Act->lptr;
			//Pokud je novy prvek prvni v seznamu upravim ukazatel na zacatek seznamu, pokud ne upravim jeho predchudce
			if (aux->lptr != NULL)
				aux->lptr->rptr = aux;
			else
				L->First = aux;
			//Novy prvek zaradim pred aktualni
			L->Act->lptr = aux;
		}
	}
}

void DLCopy (tDLList *L, int *val) {
/*
** Prostøednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/
	if(L->Act == NULL)
		DLError();
	else
		*val = L->Act->data;
}

void DLActualize (tDLList *L, int val) {
/*
** Pøepí¹e obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedìlá nic.
**/
	if(L->Act != NULL)
		L->Act->data = val;
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedìlá nic.
** V¹imnìte si, ¾e pøi aktivitì na posledním prvku se seznam stane neaktivním.
**/
	if(L->Act != NULL)
		L->Act = L->Act->rptr;
}


void DLPred (tDLList *L) {
/*
** Posune aktivitu na pøedchozí prvek seznamu L.
** Není-li seznam aktivní, nedìlá nic.
** V¹imnìte si, ¾e pøi aktivitì na prvním prvku se seznam stane neaktivním.
**/
	if(L->Act != NULL)
		L->Act = L->Act->lptr;
}

int DLActive (tDLList *L) {
/*
** Je-li seznam L aktivní, vrací nenulovou hodnotu, jinak vrací 0.
** Funkci je vhodné implementovat jedním pøíkazem return.
**/
	return L->Act != NULL ? 1 : 0;
}

/* Konec c206.c*/
