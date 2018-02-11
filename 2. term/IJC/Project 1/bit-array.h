// bit-array.h
// Reseni IJC-DU1, priklad a), 20.3.2015
// Autor: Ondrej Vales, FIT
// Prelozeno: gcc 4.8.4

#ifndef BIT_ARRAY_H_INCLUDED
#define BIT_ARRAY_H_INCLUDED

#include <limits.h>

typedef unsigned long type;
typedef type BitArray_t[];

#define DU1_GET_BIT_(p,i)\
((((p)[((i)/(sizeof(type)*CHAR_BIT))+1])&(((type)1)<<((i)%(sizeof(type)*CHAR_BIT))))!=0)

#define DU1_SET_BIT_(p,i,b)\
(((b) == 0) ? ((p)[((i)/(sizeof(type)*CHAR_BIT))+1]&=~(((type)1)<<((i)%(sizeof(type)*CHAR_BIT)))) : (((p)[((i)/(sizeof(type)*CHAR_BIT))+1])|=(((type)1)<<((i)%(sizeof(type)*CHAR_BIT)))))

#define BA_create(jmeno_pole,velikost)\
type (jmeno_pole)[((velikost)/(sizeof(type)*CHAR_BIT))+(((velikost)%(sizeof(type)*CHAR_BIT))!=0)+1] = {0};\
(jmeno_pole)[0]=(velikost)


#ifndef USE_INLINE

#define BA_size(jmeno_pole)\
((jmeno_pole)[0])

#define BA_get_bit(jmeno_pole,index)\
(((long)(index) >= 0) ?\
    (((type)(index) < BA_size(jmeno_pole)) ?\
        (DU1_GET_BIT_(jmeno_pole,index)) :\
        (FatalError("Index %ld mimo rozsah 0..%ld", (long)index, (long)(BA_size(jmeno_pole) - 1)),0)) :\
    (FatalError("Index %ld mimo rozsah 0..%ld", (long)index, (long)(BA_size(jmeno_pole) - 1)),0))

#define BA_set_bit(jmeno_pole,index,vyraz)\
(((long)(index) >= 0) ?\
    (((type)(index) < BA_size(jmeno_pole)) ?\
        (DU1_SET_BIT_(jmeno_pole,index,vyraz)) :\
        (FatalError("Index %ld mimo rozsah 0..%ld", (long)index, (long)(BA_size(jmeno_pole) - 1)),0)) :\
    (FatalError("Index %ld mimo rozsah 0..%ld", (long)index, (long)(BA_size(jmeno_pole) - 1)),0))

#else

inline type BA_size(BitArray_t jmeno_pole)
{
  return jmeno_pole[0];
}

inline int BA_get_bit(BitArray_t jmeno_pole,type index)
{
  return ((long)index >= 0) ? ((index < BA_size(jmeno_pole)) ? (DU1_GET_BIT_(jmeno_pole,index)) : (FatalError("Index %ld mimo rozsah 0..%ld", (long)index, (long)(BA_size(jmeno_pole) - 1)),0)) : (FatalError("Index %ld mimo rozsah 0..%ld", (long)index, (long)(BA_size(jmeno_pole) - 1)),0);
}

inline int BA_set_bit(BitArray_t jmeno_pole,type index, int vyraz)
{
  return ((long)index >= 0) ? ((index < BA_size(jmeno_pole)) ? (DU1_SET_BIT_(jmeno_pole,index, vyraz)) : (FatalError("Index %ld mimo rozsah 0..%ld", (long)index, (long)(BA_size(jmeno_pole) - 1)),0)) : (FatalError("Index %ld mimo rozsah 0..%ld", (long)index, (long)(BA_size(jmeno_pole) - 1)),0);
}

#endif // USE_INLINE


#endif // BIT-ARRAY_H_INCLUDED
