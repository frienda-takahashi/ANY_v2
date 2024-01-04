//  S.c Created by SHINYA TAKAHASHI on 2023/09/06.

#include <stdlib.h>     // malloc, rand
#include <string.h>     // memcpy
#include "S.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmultichar"


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++ basics ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

char *SCopy(char *s)
{
    if (s) {
        char *dst = SNewReplace(s, (i64_2){0,0}, NULL, 0);
        return dst;
    }
    return NULL;
}



char *SNewJoin(char *s1, char *s2)
{
    char *dst = SNewReplace(s1, (i64_2){s1 ? strlen(s1) : 0, 0}, s2, s2 ? strlen(s2) : 0);
    return dst;
}



char *SNewReplace(char *s, i64_2 range, char *with, i64 wlen)
{
    i64 len = s ? strlen(s) : 0;
    char *dst = MMNewReplace(sizeof(char), s, &len, range, with, wlen);
    return dst;
}



char *SNewSubString(char *s, i64_2 range)
{
    if (s) {
        char *s3 = SNewReplace(s, (i64_2){range.loc + range.len, strlen(s) - (range.loc + range.len)}, NULL, 0);
        char *dst = SNewReplace(s3, (i64_2){0,range.loc}, NULL, 0);
        MMFree((void **)&s3);
        return dst;
    }
    return NULL;
}



i64 SLen(char *str)
{
    long res = str ? strlen(str) : 0;
    return res;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++ with int32_t +++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void SFromMcc(i32 mcc, char outstr[5])
{
    // Multi-Character-Constant to String
    // [=][+][0][0] -> [+][=][0][0][0]
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



char *SFromMccStatic(int mcc)
{
    static char s[5] = {0};
    memset(s, 0, 5);
    SFromMcc(mcc, s);
    return s;
}



i32 SToMcc(char *str)
{
    // String to int32_t
    // [+][=][0][] -> [=][+][0][0]
    
    long len = SLen(str);
    if (len > 4) len = 4;
    if (len == 0) return 0;
    
    char c[4] = {0};
    long j = len - 1;
    for (int i=0; i<len; i++) {
        c[j] = str[i];
        j--;
    }
    
    int outmcc = 0;
    memcpy(&outmcc, c, 4);
    return outmcc;
}



i32 SToMccWithRange(char *str, i64_2 range)
{
    char s[range.len + 1];
    memcpy(s, &str[range.loc], range.len);
    s[range.len] = 0;
    i32 mcc = SToMcc(s);
    return mcc;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++++ test +++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void STest(void)
{
    char *dst = SNewReplace("123456", (i64_2){1,2}, "xxx", 3);
    printf("%s\n", dst);
    MMFree((void **)&dst);
    
    dst = SNewSubString("123456", (i64_2){3,2});
    printf("%s\n", dst);
    MMFree((void **)&dst);
    
    dst = SCopy("123456");
    printf("%s\n", dst);
    MMFree((void **)&dst);
    
    char *s = SNewJoin(NULL, "A");
    printf("%s\n", s);
    MMFree((void **)&s);
    
    s = SNewJoin("B", NULL);
    printf("%s\n", s);
    MMFree((void **)&s);
    
    s = SNewJoin(NULL, NULL);
    printf("%s\n", s);
    MMFree((void **)&s);

    MMPrint();
}



#pragma clang diagnostic pop
