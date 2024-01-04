// Created by SHINYA TAKAHASHI on 2023/09/03.

#pragma once
#include <stdio.h>


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++ definitions ++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

typedef char    i8;
typedef int32_t i32;
typedef int64_t i64;
typedef double  f64;



typedef struct f64_2 {
    union {
        struct { f64 x, y; };
        struct { f64 w, h; };
    };
} f64_2;



typedef struct f64_4 {
    union {
        struct { f64 x, y, w, h; };
        struct { f64 x1, x2, y1, y2; };
        struct { f64 r, g, b, a; };
        struct { f64 hsb_h, hsb_s, hsb_b, hsb_a; };
    };
} f64_4;



typedef struct i64_2 {
    union {
        struct { i64 x, y; };
        struct { i64 loc, len; };
        struct { i64 sec, nsec; };
    };
} i64_2;



#define f64_4Zero (f64_4){0,0,0,0}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++ basic interfaces +++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void *MMCalloc(size_t count, size_t size, i32 type);
void  MMFree(void **p);
void  MMCopyWithElementSize(void *dst, void *src, i64 copyLen, size_t elementSize);
void  MMPrint(void);


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++ shorten, extend +++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void  *MMNewReplace(size_t elementSize, void *src, i64 *len, i64_2 range, void *with, i64 withLen);
void  *MMNewJoin(size_t elementSize, void *list1, i64 len1, void *list2, i64 len2);
void   MMReallocJoin(size_t elementSize, void **list1, i64 len1, void *list2, i64 len2);
void   MMSort(void *list, i64 len, i32 elementType);
i64_2  MMGetRange(void *sortedList, i64 len, i32 elementType, void *value);


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++++ test +++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void MMTestReplace(void);
void MMTestMemory(void);
void MMTestJoin(void);
void MMTestReallocJoin(void);
