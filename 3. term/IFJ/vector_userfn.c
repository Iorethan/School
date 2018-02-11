/*
 * vector_userfn.c
 * Autor: Tomas Zencak
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

#include "vector_userfn.h"
#include "error.h"
#include "cexception.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

int vector_UserFn_init(Vector_UserFn *vector){
	return vector_UserFn_init_sized(vector, VECTOR_UserFn_DEFAULT_SIZE);
}

int vector_UserFn_init_sized(Vector_UserFn *vector, int size){
	vector->last=SIZE_MAX;//basically -1. I can do this since unsigned overflow is defined.
	vector->size=size;
	if(size==0){
		vector->array=NULL;
		return RET_EOK;
	}
	if((vector->array=malloc(sizeof(UserFn)*size))==NULL){
		vector->size=0;
		return RET_INTERN;
	}
	else{
		return RET_EOK;
	}
}

void vector_UserFn_destroy(Vector_UserFn *vector){
	free(vector->array);
}

//If it returns an error, the vector remains unchanged.
static int grow_vector(Vector_UserFn *vector){
	size_t nsize;
	//if size of vector is lower than the default
	if(vector->size<VECTOR_UserFn_DEFAULT_SIZE*2){
		//just set default*2
		nsize=VECTOR_UserFn_DEFAULT_SIZE*2;
	}
	//otherwise double
	else{
		nsize=2*vector->size;
	}
	UserFn *tmp;
	if((tmp=realloc(vector->array, nsize*sizeof(UserFn)))==NULL){
		return RET_INTERN;
	}
	else{
		vector->size=nsize;
		vector->array=tmp;
	}
	return RET_EOK;
}

int vector_UserFn_push_back(Vector_UserFn *vector, UserFn value){
	if(++(vector->last)==vector->size){
		int retval=grow_vector(vector);
		if((retval!=RET_EOK)){
			//if growing fails, reset the vector to a valid state.
			vector->last--;
			//throw if possible
			Throw(retval);
			return RET_INTERN;
		}
	}
	vector->array[vector->last]=value;
	return RET_EOK;
}

UserFn vector_UserFn_pop_back(Vector_UserFn *vector){
	assert(vector->last!=SIZE_MAX);
	return vector->array[vector->last--];//note that this is postfix --
}

UserFn* vector_UserFn_back(Vector_UserFn *vector){
	assert(vector->last!=SIZE_MAX);
	return &vector->array[vector->last];
}

size_t vector_UserFn_count(Vector_UserFn *vector){
	return vector->last+1;
}

UserFn* vector_UserFn_at(Vector_UserFn *vector, size_t index){
	assert(vector->last>=index);
	return vector->array+index;
}

int vector_UserFn_insert_at(Vector_UserFn *vector, size_t index, UserFn value){
	int retval=vector_UserFn_push_back(vector, value);
	if(retval!=RET_EOK){
		//if an error occured and this is in a try block, push_back already threw
		//so there's no point in putting Throw here.
		return retval;
	}
	if(index==vector->last+1){
		//if we are inserting at position one past the end, it's just a push.
		return retval;
	}
	assert(vector->last>=index);
	memmove(vector->array+index+1, vector->array+index, (vector->last+1-index)*sizeof(UserFn));
	vector->array[index]=value;
	return RET_EOK;
}
