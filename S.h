//  S.h Created by SHINYA TAKAHASHI on 2023/09/06.

#pragma once
#include "MM.h"


// basics
char *SCopy(char *s);
char *SNewJoin(char *s1, char *s2);
char *SNewReplace(char *s, i64_2 range, char *with, i64 wlen);
char *SNewSubString(char *s, i64_2 range);
char *SNewUUID(void);
i64   SLen(char *str);

// with int32_t
void  SFromMcc(i32 mcc, char outstr[5]);
char *SFromMccStatic(i32 mcc);
i32   SToMcc(char *mcc);
i32   SToMccWithRange(char *str, i64_2 range);

// test
void STest(void);






