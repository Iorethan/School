/*
 * ial.c
 * Autor: Ondrej Vales
 * Predmet: Formalni jazyky a prekladace
 * Projekt: Implementace interpretu jazyka IFJ15
 * Varianta zadani: a/2/II
 * Datum: 1. 10. 2015
 * Prelozeno: gcc 4.8
 * Tomas Zencak xzenca00
 * Ondrej Vales xvales03
 * Nikola Valesova xvales02
 * Radek Vit xvitra00
 */

#include "ial.h"
#include "cexception.h"

size_t hash_function(const char *data, unsigned htab_size){
	//An actually decent hash function.
	unsigned int hash=8161;//512th prime. pretty much anything should do, really.
	for(char c=*data; (c=*data); data++){
		hash=(c*hash)>>3;
	}
	return hash%htab_size;
}

int htab_init(HTab *table){
	return htab_init_sized(table, HTAB_DEFAULT_SIZE);
}

int htab_init_sized(HTab *table, size_t size){
	if(!(table->table=malloc(sizeof(HTItem)*size))){
		return RET_INTERN;
	}
	table->size=size;
	//set all pointers to NULL.
	memset(table->table, 0, sizeof(HTItem)*size);
	return RET_EOK;
}

int htab_insert(HTab *table, char* key, HTdata value){
	size_t index=hash_function(key, table->size);
	//this is a table with duplications, no need to check for existence.
	HTItem *nitem=malloc(sizeof(HTItem));
	if(!nitem){
		Throw(RET_INTERN);
		//if we are not inside of a try block, return instead.
		return RET_INTERN;
	}
	nitem->ptr=table->table[index];
	nitem->key=key;
	nitem->value=value;
	table->table[index]=nitem;
	return RET_EOK;
}

static HTItem **htab_search_internal(HTab *table, char *key){
	size_t index=hash_function(key, table->size);
	for(HTItem **item=&table->table[index]; *item; *item=(*item)->ptr){
		if(!strcmp(key, (*item)->key)){
			return item;
		}
	}
	//if cycle ends, nothing was found
	return NULL;
}

HTdata *htab_search(HTab *table, char *key){
	HTItem **search_result=htab_search_internal(table, key);
	if(search_result){
		return &(*search_result)->value;
	}
	return NULL;
}

void htab_remove(HTab *table, char* key){
	HTItem **search_result=htab_search_internal(table, key);
	if(search_result){
		HTItem *to_free=*search_result;
		*search_result=to_free->ptr;
		free(to_free);
	}
}

void htab_destroy(HTab *table){
	for(size_t i=0; i<table->size; i++){
		for(HTItem *tmp=table->table[i]; table->table[i]; tmp=table->table[i]){
			table->table[i]=tmp->ptr;
			free(tmp);
		}
	}
	free(table->table);
}

int find (char* s, char* search, int lens, int lense)
{
	if(lense == 0)
		//Empty string.
		return 0;
	
	//Create table containing backtracking information (change of position in s after failed match).
	int* table = malloc(lense * sizeof(int));
	if(table == NULL)
	{
		Throw(RET_INTERN);
		return -1;
	}
	table[0] = -1;
	
	int r;
	for (int k = 1; k < lense; k++)
	{
		r = table[k - 1];
		while (r > -1 && search[r] != search[k - 1])
			r = table[r];
		table[k] = r + 1;
	}
	
	int s_pos = 0;
	int search_pos = 0;
	//Search through string s.
	while (search_pos < lense && s_pos < lens)
	{
		if (search_pos == -1 || s[s_pos] == search[search_pos])
		{
			s_pos++;
			search_pos++;
		}
		else
		{
			search_pos = table[search_pos];
		}
	}
	free (table);
	//Check if match or end of string.
	if (search_pos == lense)
		return s_pos - search_pos;
	return -1;
}

/**
 * @brief takse root of the given heap and exchanges it with children until heap property is valid.
 * @param heap a pointer to the start of the heap.
 * @param start the root of the heap to fix.
 * @param length the length of \p heap
 */
void sift_root_downward(Heap_element* heap, int start, int length){
	//give ourselves a heap starting at index 1 to eliminate some math.
	//this also means that length is equal to the index of the last element
	heap--;
	int parentIndex=start+1;
	int childIndex=parentIndex*2;
	//save the element at the root for later
	Heap_element root_element=heap[parentIndex];
	//while the current node has children
	while(childIndex<=length){
		//if child isn't the last node, make sure we have the larger child.
		if(childIndex<length&&heap[childIndex+1]>heap[childIndex]){
			childIndex++;
		}
		//if the child is larger than the root element
		if(heap[childIndex]>root_element){
			//put the child in place of its parent
			heap[parentIndex]=heap[childIndex];
		}
		//otherwise the parent is larger and the heap property is valid.
		else{
			break;
		}
		//update iteration variables
		parentIndex=childIndex;
		childIndex=childIndex*2;
	}
	//put root element into its proper place
	heap[parentIndex]=root_element;
}

/**
 * @brief Makes \p arr into a max heap.
 * @param arr The array to heapify
 * @param length The length of \p arr
 */
void heapify(Heap_element *arr, int length){
	for(int i=length/2-1; i>=0; i--){
		sift_root_downward(arr, i, length);
	}
}

void heapsort(Heap_element* arr, int length){
	//heapify the array
	heapify(arr, length);
	//now the top of the array is the largest element, so swap the last element for it and restore the heap property in the remaining heap.
	for(int i=length-1; i>0; i--){
		Heap_element largest=arr[0];
		arr[0]=arr[i];
		arr[i]=largest;
		sift_root_downward(arr, 0, i);
	}
}
