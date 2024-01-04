//  ANY.h Created by SHINYA TAKAHASHI on 2023/06/17.

#pragma once
#include "MM.h"


// 格納出来るエレメントタイプの定義（必要に応じてスカラー型や独自の構造体など追加のこと）
#define ELEMENT_TYPE_POINTER         'ptr'
#define ELEMENT_TYPE_STRING          'str' //special value size
#define ELEMENT_TYPE_ANY             'any'


// 格納出来る文字列のバイト数の上限を設定（必要に応じて変更のこと）
#define MAX_SIZE 1000


// オブジェクトの定義（高速列挙の為に透過型）
typedef struct CELL {
    i32   type, vSize;
    char *key;
    void *value;
    struct CELL *next;
} CELL;



typedef struct ANY {
    i64   retainCount, count;
    CELL *rootCell;
} ANY;



// 基本インターフェース（ANYAdd出来る要素数の上限無し）
ANY  *ANYNew(void);
ANY  *ANYCopy(ANY *any);
void  ANYRetain(ANY *any);
void  ANYRelease(ANY **any);
void  ANYInsert(ANY *any, i64 idx, i32 elementType, char *key, void *value, i64 valueSize);
void  ANYAdd(ANY *any, i32 elementType, char *key, void *value);
void  ANYSet(ANY *any, i32 elementType, char *key, void *value);
void *ANYGet(ANY *any, char *key);
void  ANYRemove(ANY *any, char *key);
void  ANYRemoveAll(ANY *any);
void  ANYPrint(ANY *any);


// シリアライズのインターフェース
void *ANYNewToData(ANY *any, i64 *outLength);
ANY  *ANYNewFromData(void *data, i64 length);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++++ test +++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void ANYUsage(void);
void ANYTest(void);



