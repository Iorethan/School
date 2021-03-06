	
/* c206.c **********************************************************}
{* T�ma: Dvousm�rn� v�zan� line�rn� seznam
**
**                   N�vrh a referen�n� implementace: Bohuslav K�ena, ��jen 2001
**                            P�epracovan� do jazyka C: Martin Tu�ek, ��jen 2004
**                                            �pravy: Bohuslav K�ena, ��jen 2015
**
** Implementujte abstraktn� datov� typ dvousm�rn� v�zan� line�rn� seznam.
** U�ite�n�m obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datov� abstrakce reprezentov�n prom�nnou
** typu tDLList (DL znamen� Double-Linked a slou�� pro odli�en�
** jmen konstant, typ� a funkc� od jmen u jednosm�rn� v�zan�ho line�rn�ho
** seznamu). Definici konstant a typ� naleznete v hlavi�kov�m souboru c206.h.
**
** Va��m �kolem je implementovat n�sleduj�c� operace, kter� spolu
** s v��e uvedenou datovou ��st� abstrakce tvo�� abstraktn� datov� typ
** obousm�rn� v�zan� line�rn� seznam:
**
**      DLInitList ...... inicializace seznamu p�ed prvn�m pou�it�m,
**      DLDisposeList ... zru�en� v�ech prvk� seznamu,
**      DLInsertFirst ... vlo�en� prvku na za��tek seznamu,
**      DLInsertLast .... vlo�en� prvku na konec seznamu, 
**      DLFirst ......... nastaven� aktivity na prvn� prvek,
**      DLLast .......... nastaven� aktivity na posledn� prvek, 
**      DLCopyFirst ..... vrac� hodnotu prvn�ho prvku,
**      DLCopyLast ...... vrac� hodnotu posledn�ho prvku, 
**      DLDeleteFirst ... zru�� prvn� prvek seznamu,
**      DLDeleteLast .... zru�� posledn� prvek seznamu, 
**      DLPostDelete .... ru�� prvek za aktivn�m prvkem,
**      DLPreDelete ..... ru�� prvek p�ed aktivn�m prvkem, 
**      DLPostInsert .... vlo�� nov� prvek za aktivn� prvek seznamu,
**      DLPreInsert ..... vlo�� nov� prvek p�ed aktivn� prvek seznamu,
**      DLCopy .......... vrac� hodnotu aktivn�ho prvku,
**      DLActualize ..... p�ep��e obsah aktivn�ho prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na dal�� prvek seznamu,
**      DLPred .......... posune aktivitu na p�edchoz� prvek seznamu, 
**      DLActive ........ zji��uje aktivitu seznamu.
**
** P�i implementaci jednotliv�ch funkc� nevolejte ��dnou z funkc�
** implementovan�ch v r�mci tohoto p��kladu, nen�-li u funkce
** explicitn� uvedeno n�co jin�ho.
**
** Nemus�te o�et�ovat situaci, kdy m�sto leg�ln�ho ukazatele na seznam 
** p�ed� n�kdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodn� komentujte!
**
** Terminologick� pozn�mka: Jazyk C nepou��v� pojem procedura.
** Proto zde pou��v�me pojem funkce i pro operace, kter� by byly
** v algoritmick�m jazyce Pascalovsk�ho typu implemenov�ny jako
** procedury (v jazyce C procedur�m odpov�daj� funkce vracej�c� typ void).
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozorn�n� na to, �e do�lo k chyb�.
** Tato funkce bude vol�na z n�kter�ch d�le implementovan�ch operac�.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* glob�ln� prom�nn� -- p��znak o�et�en� chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L p�ed jeho prvn�m pou�it�m (tzn. ��dn�
** z n�sleduj�c�ch funkc� nebude vol�na nad neinicializovan�m seznamem).
** Tato inicializace se nikdy nebude prov�d�t nad ji� inicializovan�m
** seznamem, a proto tuto mo�nost neo�et�ujte. V�dy p�edpokl�dejte,
** �e neinicializovan� prom�nn� maj� nedefinovanou hodnotu.
**/
	L->First = NULL;
	L->Act = NULL;
	L->Last = NULL;
}

void DLDisposeList (tDLList *L) {
/*
** Zru�� v�echny prvky seznamu L a uvede seznam do stavu, v jak�m
** se nach�zel po inicializaci. Ru�en� prvky seznamu budou korektn�
** uvoln�ny vol�n�m operace free. 
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
** Vlo�� nov� prvek na za��tek seznamu L.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
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
** Vlo�� nov� prvek na konec seznamu L (symetrick� operace k DLInsertFirst).
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
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
** Nastav� aktivitu na prvn� prvek seznamu L.
** Funkci implementujte jako jedin� p��kaz (nepo��t�me-li return),
** ani� byste testovali, zda je seznam L pr�zdn�.
**/
	L->Act = L->First;
}

void DLLast (tDLList *L) {
/*
** Nastav� aktivitu na posledn� prvek seznamu L.
** Funkci implementujte jako jedin� p��kaz (nepo��t�me-li return),
** ani� byste testovali, zda je seznam L pr�zdn�.
**/
	L->Act = L->Last;
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prost�ednictv�m parametru val vr�t� hodnotu prvn�ho prvku seznamu L.
** Pokud je seznam L pr�zdn�, vol� funkci DLError().
**/
	if(L->First == NULL)
		DLError();
	else
		*val = L->First->data;
}

void DLCopyLast (tDLList *L, int *val) {
/*
** Prost�ednictv�m parametru val vr�t� hodnotu posledn�ho prvku seznamu L.
** Pokud je seznam L pr�zdn�, vol� funkci DLError().
**/
	if(L->Last == NULL)
		DLError();
	else
		*val = L->Last->data;
}

void DLDeleteFirst (tDLList *L) {
/*
** Zru�� prvn� prvek seznamu L. Pokud byl prvn� prvek aktivn�, aktivita 
** se ztr�c�. Pokud byl seznam L pr�zdn�, nic se ned�je.
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
** Zru�� posledn� prvek seznamu L. Pokud byl posledn� prvek aktivn�,
** aktivita seznamu se ztr�c�. Pokud byl seznam L pr�zdn�, nic se ned�je.
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
** Zru�� prvek seznamu L za aktivn�m prvkem.
** Pokud je seznam L neaktivn� nebo pokud je aktivn� prvek
** posledn�m prvkem seznamu, nic se ned�je.
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
** Zru�� prvek p�ed aktivn�m prvkem seznamu L .
** Pokud je seznam L neaktivn� nebo pokud je aktivn� prvek
** prvn�m prvkem seznamu, nic se ned�je.
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
** Vlo�� prvek za aktivn� prvek seznamu L.
** Pokud nebyl seznam L aktivn�, nic se ned�je.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
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
** Vlo�� prvek p�ed aktivn� prvek seznamu L.
** Pokud nebyl seznam L aktivn�, nic se ned�je.
** V p��pad�, �e nen� dostatek pam�ti pro nov� prvek p�i operaci malloc,
** vol� funkci DLError().
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
** Prost�ednictv�m parametru val vr�t� hodnotu aktivn�ho prvku seznamu L.
** Pokud seznam L nen� aktivn�, vol� funkci DLError ().
**/
	if(L->Act == NULL)
		DLError();
	else
		*val = L->Act->data;
}

void DLActualize (tDLList *L, int val) {
/*
** P�ep��e obsah aktivn�ho prvku seznamu L.
** Pokud seznam L nen� aktivn�, ned�l� nic.
**/
	if(L->Act != NULL)
		L->Act->data = val;
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na n�sleduj�c� prvek seznamu L.
** Nen�-li seznam aktivn�, ned�l� nic.
** V�imn�te si, �e p�i aktivit� na posledn�m prvku se seznam stane neaktivn�m.
**/
	if(L->Act != NULL)
		L->Act = L->Act->rptr;
}


void DLPred (tDLList *L) {
/*
** Posune aktivitu na p�edchoz� prvek seznamu L.
** Nen�-li seznam aktivn�, ned�l� nic.
** V�imn�te si, �e p�i aktivit� na prvn�m prvku se seznam stane neaktivn�m.
**/
	if(L->Act != NULL)
		L->Act = L->Act->lptr;
}

int DLActive (tDLList *L) {
/*
** Je-li seznam L aktivn�, vrac� nenulovou hodnotu, jinak vrac� 0.
** Funkci je vhodn� implementovat jedn�m p��kazem return.
**/
	return L->Act != NULL ? 1 : 0;
}

/* Konec c206.c*/
