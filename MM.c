//  MM.c Created by SHINYA TAKAHASHI on 2023/09/03.

#include "MM.h"
#include <stdlib.h>     // malloc
#include <string.h>     // memcpy
#include <math.h>       // fmod,floor


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmultichar"



typedef struct MMInfo {
    void *address;
    i32   size;
    i32   type;
} MMInfo;



#define MM_TYPE_MMINFO 'MMIF'



i64_2 MMRangeLimited(i64_2 range, i64_2 limit)
{
    i64 loc1 = range.loc < limit.loc ? limit.loc : range.loc;
    i64 loc2 = range.loc + range.len > limit.loc + limit.len ? limit.loc + limit.len : range.loc + range.len;
    return (i64_2){loc1, loc2 - loc1};
}



static void *listNewReplace(void *src, i64 len, size_t elementSize, i64_2 rep, void *with, i64 withLen, i64 *nlen)
{
    if (nlen) {
        rep = MMRangeLimited(rep, (i64_2){0,len});
        i64_2 srcbwd = { rep.loc + rep.len, len - (rep.loc + rep.len) };
        *nlen = rep.loc + withLen + srcbwd.len;
        char *dst = calloc(*nlen + 1, elementSize);
        
        if (src)  MMCopyWithElementSize(dst, src, rep.loc, elementSize);
        if (with) MMCopyWithElementSize(&dst[rep.loc * elementSize], with, withLen, elementSize);
        if (src)  MMCopyWithElementSize(&dst[(rep.loc + withLen) * elementSize], &src[srcbwd.loc * elementSize], srcbwd.len, elementSize);

        return dst;
    }
    printf("%s %d err src:%p, nlen:%p", __func__, __LINE__, src, nlen);
    return NULL;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++ Memory Managing +++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static MMInfo *memoryList = NULL;
static i64 memoryListLen = 0;



static void STRFromMcc(int mcc, char outstr[5])
{
    //[=][+][0][0] -> [+][=][0][0][0]
    
    if (outstr) {
        memset(&outstr[0], 0, 5);
        char *str = (char *)&(mcc);
        int j=0;
        for (int i=3; i>=0; i--) {
            if (str[i] != 0) {
                outstr[j] = str[i];
                j++;
            }
        }
        for (; j<5; j++) {
            outstr[j] = 0;
        }
    }
}



void *MMCalloc(size_t count, size_t size, i32 type)
{
    void *p = calloc(count, size);
    if (!p) {
        // ERROR HANDLING
    }
    MMInfo info = {
        .address = p,
        .type = type,
        .size = (i32)(count * size),
    };
    
    void *newList = listNewReplace(memoryList, memoryListLen, sizeof(MMInfo), (i64_2){memoryListLen, 0}, &info, 1, &memoryListLen);
    free(memoryList);
    memoryList = newList;
    return p;
}



void MMFree(void **p)
{
    // memoryListをsortし、pがmemoryListに有ればfree、memoryListLenから削除
    if (memoryListLen > 0 && p && *p && *p != NULL) {
        MMSort(memoryList, memoryListLen, MM_TYPE_MMINFO);
        i64_2 range = MMGetRange(memoryList, memoryListLen, MM_TYPE_MMINFO, *p);
        if (range.len > 0) {
            free(*p);
            *p = NULL;
            void *newList = listNewReplace(memoryList, memoryListLen, sizeof(MMInfo), range, NULL, 0, &memoryListLen);
            free(memoryList);
            memoryList = newList;
        }
    }
}



void MMCopyWithElementSize(void *dst, void *src, i64 copyLen, size_t elementSize)
{
    if (dst && src && copyLen > 0) {
        memcpy(dst, src, copyLen * elementSize);
    }
    //else printf("err UTMemcpyWithMcc dst:%p, src:%p, copyLen:%ld\n", dst, src, copyLen);
}



void MMPrint(void)
{
    printf("MMPrint memoryListLen:%lld\n", memoryListLen);
    for (i64 i=0; i<memoryListLen; i++) {
        char s[5];
        STRFromMcc(memoryList[i].type, s);
        printf("%p type:%s size:%d\n", memoryList[i].address, s, memoryList[i].size);
    }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++ qsort callbacks +++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int qsortCBAscendent_MMInfo(const void *a, const void *b)
{
    const MMInfo da = *(MMInfo *)a, db = *(MMInfo *)b;
    if      (da.address > db.address) return 1;
    else if (da.address < db.address) return -1;
    else                              return 0;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++ get range ++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static i64_2 getRange_MMInfoAddress(MMInfo *list, i64 length, void *value)
{
    if (list && value) {
        void *_value = value;
        if (_value > list[length - 1].address) return (i64_2){length, 0};
        i64 L = 0, remain = length, loc1 = L + floor(remain/2);
        while (1) {
            // value <= list[i]なら左へ、value > list[i]なら右へ
            if (_value <= list[loc1].address) { L = L; remain = loc1 - L; loc1 = L + floor(remain/2); }
            else if (_value > list[loc1].address) { remain = L + remain - (loc1 + 1); L = loc1 + 1; loc1 = L + floor(remain/2); }
            if (remain == 0) break;
        }
        
        i64 R = loc1, remainR = length - loc1, loc2 = R + floor(remainR/2);
        while (1) {
            // value >= list[loc2] なら右へ、value < list[loc2] なら左へ
            if (_value >= list[loc2].address) { R = loc2 + 1; remainR = length - R; loc2 = R + floor(remainR/2); }
            else if (_value < list[loc2].address) { R = R; remainR = loc2 - R; loc2 = R + floor(remainR/2); }
            if (remainR == 0) break;
        }
        return (i64_2){loc1, loc2 - loc1};
    }
    return (i64_2){0, 0};
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++ interface ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

// リストユーティリティ

void *MMNewReplace(size_t elementSize, void *src, i64 *len, i64_2 range, void *with, i64 withLen)
{
    if (len) {
        // src:NULLならlen:0,range{0,0}に修正
        if (!src) {
            *len = 0;
            range = (i64_2){0,0};
        }
        range = MMRangeLimited(range, (i64_2){0, *len});
        i64_2 srcbwd = { range.loc + range.len, *len - (range.loc + range.len) };
        *len = range.loc + withLen + srcbwd.len;
        char *dst = MMCalloc(*len + 1, elementSize , 'MMRP');
        
        if (src)  MMCopyWithElementSize(dst, src, range.loc, elementSize);
        if (with) MMCopyWithElementSize(&dst[range.loc * elementSize], with, withLen, elementSize);
        if (src)  MMCopyWithElementSize(&dst[(range.loc + withLen) * elementSize], &src[srcbwd.loc * elementSize], srcbwd.len, elementSize);

        return dst;
    }
    printf("%s %d ERROR len:%p", __func__, __LINE__, len);
    return NULL;
}



void *MMNewJoin(size_t elementSize, void *list1, i64 len1, void *list2, i64 len2)
{
    if (len1 + len2 > 0) {
        void *res = MMCalloc(len1 + len2 + 1, elementSize, 'MMNJ');
        if (len1 > 0) MMCopyWithElementSize(res, list1, len1, elementSize);
        if (len2 > 0) MMCopyWithElementSize(&res[len1], list2, len2, elementSize);
        return res;
    }
    return NULL;
}



void MMReallocJoin(size_t elementSize, void **list1, i64 len1, void *list2, i64 len2)
{
    // *list1またはlist2がNULLでも動作するように
    if (len1 + len2 > 0 && list1) {
        void *res = MMNewJoin(elementSize, *list1, len1, list2, len2);
        MMFree(&(*list1));
        *list1 = res;
    }
}



void MMSort(void *list, i64 len, i32 elementType)
{
    // 対応しない型の場合はptrとして比較? (any,list,mesh,cell)
    switch (elementType) {
        case MM_TYPE_MMINFO: qsort(list, len, sizeof(MMInfo), qsortCBAscendent_MMInfo); break;
        default: {
            printf("MMSort ERROR UNKNOWN TYPE\n");
        }
    }
}



i64_2 MMGetRange(void *sortedList, i64 len, i32 elementType, void *value)
{
    switch (elementType) {
        case MM_TYPE_MMINFO: return getRange_MMInfoAddress(sortedList, len, value);
        default: {
            printf("MMSearch ERROR UNKNOWN TYPE\n");
            return (i64_2){0,0};
        }
    }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++++ test +++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void MMTestReplace(void)
{
    // NULLリスト,超過len,越境rangeを渡して確保
    i64 len = 100;
    void *list = MMNewReplace(sizeof(void *), NULL, &len, (i64_2){0,300}, NULL, 100);
    printf("%p %lld\n", list, len);
    MMFree(&list);
    
    // 上記に加え短いwith
    void *with = MMCalloc(3, sizeof(void *), 'MMTS');
    i64 len2 = 100;
    void *list2 = MMNewReplace(sizeof(void *), NULL, &len, (i64_2){0,300}, with, 200);
    printf("%p %lld\n", list2, len2);
    MMFree(&list2);
    MMFree(&with);
    
    // 文字列の確保
    i64 len3 = 0;
    char *str = MMNewReplace(sizeof(char), NULL, &len3, (i64_2){0,0}, "ABCD", strlen("ABCD"));
    printf("%s %lld\n", str, len3);
    MMFree((void **)&str);
    
    // ランダムに追加と削除
    i64 len4 = 0;
    char *src = NULL;
    for (i32 i=0; i<5; i++) {
        void *dst = MMNewReplace(sizeof(char), src, &len4, (i64_2){1,2}, "ABCD", strlen("ABCD"));
        src = dst;
        printf("%s %lld\n", src, len4);
        MMFree(&dst);
    }
    MMFree((void **)&src);
    
    for (i64 len_=0; len_<6; len_++) {
        for (i64 loc_=0; loc_<6; loc_++) {
            i64 len5 = strlen("abcd");
            char *s = MMNewReplace(sizeof(char), "abcd", &len5, (i64_2){loc_,len_}, "123", strlen("123"));
            printf("{%lld,%lld} %s\n", loc_, len_, s);
            MMFree((void **)&s);
        }
    }
    MMPrint();
}



void MMTestMemory(void)
{
    void *p = MMCalloc(1, sizeof(i64_2), 'rnge');
    void *q = MMCalloc(1, sizeof(f64_4), 'rect');
    MMPrint();
    
    for (i32 i=0; i<10; i++) {
        printf("p:%p\n", p);
        MMFree(&p);
        printf("p:%p\n", p);
        MMPrint();
    }
    MMFree(&q);
    MMPrint();
}



void MMTestJoin(void)
{
    // char*でテスト
    char *s1 = "1234";
    char *s2 = "abcd";
    char *s3 = MMNewJoin(sizeof(char), s1, strlen(s1), s2, strlen(s2));
    printf("%s\n", s3);
    
    MMReallocJoin(sizeof(char), (void **)&s3, strlen(s3), s2, strlen(s2));
    printf("%s\n", s3);
    
    MMReallocJoin(sizeof(char), NULL, 0, s3, strlen(s3));
    printf("%s\n", s3);
    
    MMReallocJoin(sizeof(char), (void **)&s3, strlen(s3), NULL, 0);
    printf("%s\n", s3);
    
    MMFree((void **)&s3);
    
    MMPrint();
}



void MMTestReallocJoin(void)
{
    void *d1 = MMCalloc(1, sizeof(void *), 'MMR1');
    void *d2 = MMCalloc(5, sizeof(void *), 'MMR2');
    MMReallocJoin(sizeof(void *), &d1, 1, d2, 5);
    MMFree(&d1);
    MMFree(&d2);
    MMPrint();
}



#pragma clang diagnostic pop
