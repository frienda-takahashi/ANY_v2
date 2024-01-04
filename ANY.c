//  ANY.c Created by SHINYA TAKAHASHI on 2023/08/19.

#include <string.h>     // memcpy
#include "S.h"          // list, mcc
#include "RIS.h"
#include "ANY.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmultichar"



static RIS *reuseCELLList = NULL;


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++ private ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void linkedListInsert(CELL **firstCell, CELL *insertCell, i64 idx)
{
    // idx <= 0、または、*firstCELLがnullなら*firstCELLにinsert
    // でなければ、idxの位置にinsert
    // でなければ、lastCELLの->nextにinsert
    
    if (firstCell && insertCell) {
        if (idx <= 0 || !*firstCell) {
            insertCell->next = *firstCell;
            *firstCell = insertCell;
            return;
        }
        
        CELL *cell = *firstCell, *lastCell = NULL;
        int i = 0;
        while (cell) {
            if (i == idx - 1) {
                insertCell->next = cell->next;
                cell->next = insertCell;
                return;
            }
            lastCell = cell;
            cell = cell->next;
            i++;
        }
        
        if (lastCell) {
            lastCell->next = insertCell;
            return;
        }
        
        printf("エラー %s &fc:%p fc:%p ic:%p idx:%lld\n", __func__, firstCell, *firstCell, insertCell, idx);
    }
}



static void linkedListRemove(CELL **firstCell, CELL *removeCell)
{
    if (firstCell && *firstCell && removeCell) {
        if (removeCell == *firstCell) {
            if (removeCell->next) *firstCell = removeCell->next;
            else                  *firstCell = NULL;
        }
        else {
            CELL *cell = *firstCell, *prevCell = *firstCell;
            while (cell) {
                if (cell == removeCell) {
                    prevCell->next = cell->next;
                    return;
                }
                prevCell = cell;
                cell = cell->next;
            }
        }
    }
    else {
        printf("エラー %s &fc:%p rc:%p\n", __func__, firstCell, removeCell);
    }
}



static void eraseCell(CELL *cell)
{
    if (cell) {
        if (cell->type == ELEMENT_TYPE_POINTER) {
            // keyのみfree
            MMFree((void **)&cell->key);
        }
        else if (cell->type == ELEMENT_TYPE_ANY) {
            // keyのみfree、ANYをRelease
            MMFree((void **)&cell->key);
            ANY *any_ = cell->value;
            ANYRelease(&any_);
        }
        else {
            // keyとvalueをfree
            MMFree((void **)&cell->key);
            MMFree((void **)&cell->value);
        }
    }
}



static void cellSetData(CELL *cell, i32 type, void *key, i64 kSize, void *val, i32 vSize)
{
    if (cell) {
        cell->type = type;
        cell->vSize = vSize;
        cell->key = MMCalloc(1, kSize, 'CSDK');
        memcpy(cell->key, key, kSize);
        
        cell->value = MMCalloc(1, vSize, 'CSDV');
        memcpy(cell->value, val, vSize);
    }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++ ANY basic interface ++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

ANY *ANYNew(void)
{
    ANY *any = MMCalloc(1, sizeof(ANY), 'ANY');
    any->retainCount = 1;
    return any;
}



ANY *ANYCopy(ANY *any)
{
    i64 len = 0;
    void *data = ANYNewToData(any, &len);
    ANY *copy = ANYNewFromData(data, len);
    MMFree(&data);
    return copy;
}



void ANYRetain(ANY *any)
{
    if (any) any->retainCount++;
}



void ANYRelease(ANY **any)
{
    if (any && *any) {
        (*any)->retainCount--;
        if ((*any)->retainCount == 0) {
            ANYRemoveAll(*any);
            MMFree((void **)any);
        }
    }
}



void ANYInsert(ANY *any, i64 idx, i32 elementType, char *key, void *value, i64 valueSize)
{
    // key,value共にコピーして保持
    // ptr値は代入、ANYならretain、SLISTはコピー
    if (any && key && value) {
        // サイズチェック
        i64 keylen = SLen(key);
        keylen    = keylen    > MAX_SIZE ? MAX_SIZE : keylen;
        valueSize = valueSize > MAX_SIZE ? MAX_SIZE : valueSize;
        CELL *cell = RISGet(&reuseCELLList, sizeof(CELL));
        cellSetData(cell, elementType, key, keylen, value, (i32)valueSize);
        
        switch (elementType) {
            case ELEMENT_TYPE_POINTER: {
                MMFree((void **)&cell->value);
                cell->value = value;
                break;
            }
            case ELEMENT_TYPE_ANY: {
                MMFree((void **)&cell->value);
                cell->value = value;
                ANYRetain((ANY *)value);
                break;
            }
        }
        linkedListInsert((CELL **)&any->rootCell, (CELL *)cell, idx);
        any->count++;
    }
    else printf("ANYInsert ERROR any:%p, key:%s, value:%p\n", any, key, value);
}



void ANYAdd(ANY *any, i32 elementType, char *key, void *value)
{
    i64 vSize = elementType == ELEMENT_TYPE_STRING ? SLen(value) + 1 : sizeof(void *);
    ANYInsert(any, any->count, elementType, key, value, vSize);
}



void ANYSet(ANY *any, i32 elementType, char *key, void *value)
{
    // keyの重複を許可しない追加. keyのcellを全削除して追加
    ANYRemove(any, key);
    i64 vSize = elementType == ELEMENT_TYPE_STRING ? SLen(value) + 1 : sizeof(void *);
    ANYInsert(any, any->count, elementType, key, value, vSize);
}



void *ANYGet(ANY *any, char *key)
{
    if (any && key) {
        CELL *cell = any->rootCell;
        while (cell) {
            if (strcmp(cell->key, key) == 0) return cell->value;
            cell = cell->next;
        }
    }
    return NULL;
}



void ANYRemove(ANY *any, char *key)
{
    //　keyのcellを全削除
    if (any && key) {
        CELL *cell = any->rootCell;
        while (cell) {
            // keyがポインタなら==で比較、文字列ならstrcmpで比較
            CELL *next = cell->next;
            if (strcmp(cell->key, key) == 0) {
                eraseCell(cell);
                linkedListRemove(&any->rootCell, cell);
                RISAdd(&reuseCELLList, cell, sizeof(CELL));
                any->count--;
            }
            cell = next;
        }
    }
}



void ANYRemoveAll(ANY *any)
{
    if (any) {
        CELL *cell = any->rootCell;
        while (cell) {
            CELL *next = cell->next;
            eraseCell(cell);
            linkedListRemove(&any->rootCell, cell);
            RISAdd(&reuseCELLList, cell, sizeof(CELL));
            any->count--;
            cell = next;
        }
    }
}



void ANYPrint(ANY *any)
{
    printf("ANYPrint %p", any);
    if (any) {
        printf(" count:%lld retainCount:%lld ", any->count, any->retainCount);
        CELL *cell = any->rootCell;
        while (cell) {
            switch (cell->type) {
                case ELEMENT_TYPE_POINTER:
                    printf("{%s,PTR,%d %p} ", cell->key, cell->vSize, cell->value);
                    break;
                case ELEMENT_TYPE_STRING:
                    printf("{%s,STR,%d %s} ", cell->key, cell->vSize, (char *)cell->value);
                    break;
                case 'i642': {
                    i64_2 range = *(i64_2 *)cell->value;
                    printf("{%s,RANGE,%d {%lld,%lld} } ", cell->key, cell->vSize, range.loc, range.len);
                    break;
                }
                case 'f644': {
                    f64_4 rect = *(f64_4 *)cell->value;
                    printf("{%s,RECT,%d {%.2f,%.2f,%.2f,%.2f} } ", cell->key, cell->vSize, rect.x, rect.y, rect.w, rect.h);
                    break;
                }
                case ELEMENT_TYPE_ANY: {
                    printf("{%s,ANY,%d %p}\n  ", cell->key, cell->vSize, cell->value);
                    ANY *any_ = (ANY *)cell->value;
                    ANYPrint(any_);
                    break;
                }
                default: {
                    char type[5] = {0};
                    SFromMcc(cell->type, type);
                    printf("{%s,%s,size:%d} ", cell->key, type, cell->vSize);
                    break;
                }
            }
            cell = cell->next;
        }
    }
    printf("\n");
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++ serialize +++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void *newDataFromCell(CELL *cell, i64 *outSize)
{
    // cellSize type keySize valueSize key value , cellSize mcc keySize valueSize key value , ...
    // 4        4    4       4         va  va
    // <----- fix length 16bytes ---->
    // <-------------- cell data -------------->
    
    void *data = NULL;
    if (cell && outSize) {
        i32 keySize = (i32)SLen(cell->key);
        i32 valueSize = cell->vSize;
        void *value = cell->value;
        i32 needsFree = 0;
        
        switch (cell->type) {
            case ELEMENT_TYPE_STRING:
                valueSize = (i32)SLen(cell->value);
                break;
            case ELEMENT_TYPE_ANY: {
                i64 valueSize_ = 0;
                value = ANYNewToData(cell->value, &valueSize_);
                valueSize = (i32)valueSize_;
                needsFree = 1;
                break;
            }
            default: {
                if (cell->type == ELEMENT_TYPE_POINTER) {
                    printf("newDataFromCell エラー：ポインタ値はシリアライズ不可です\n");
                    char mcc[5];
                    SFromMcc(cell->type, mcc);
                    printf("type:%s, keySize:%d, valueSize:%d, key:%s, value:%s\n", mcc, keySize, valueSize, cell->key, (char *)cell->value);
                    return NULL;
                }
                break;
            }
        }

        *outSize = 16 + keySize + valueSize;
        data = MMCalloc(*outSize, sizeof(char), 'NDFC');
        memcpy(data, outSize, sizeof(i32));
        memcpy(&data[4], &cell->type, sizeof(i32));
        memcpy(&data[8], &keySize, sizeof(i32));
        memcpy(&data[12], &valueSize, sizeof(i32));
        memcpy(&data[16], cell->key, keySize);
        memcpy(&data[16 + keySize], value, valueSize);
        if (needsFree) MMFree(&value);
    }
    return data;
}



static void *newDataOfCellSequenceFromANY(ANY *any, i64 *outLen)
{
    void *cellDataSequence = NULL;
    i64 cellDataSequenceLen = 0;
    CELL *cell = any->rootCell;
    while (cell) {
        i64 cellDataLen = 0;
        void *data = newDataFromCell(cell, &cellDataLen);
        MMReallocJoin(sizeof(uint8_t), &cellDataSequence, cellDataSequenceLen, data, cellDataLen);
        MMFree(&data);
        cellDataSequenceLen += cellDataLen;
        cell = cell->next;
    }
    *outLen = cellDataSequenceLen;
    return cellDataSequence;
}



void *ANYNewToData(ANY *any, i64 *outLength)
{
    // ANY data
    // size cellData cellData ...
    // 4    va       va
    
    i64 cellDataSequenceLen = 0;
    void *cellDataSequence = newDataOfCellSequenceFromANY(any, &cellDataSequenceLen);
    
    void *data = MMCalloc(4, 1, 'ANTD');
    MMReallocJoin(sizeof(uint8_t), &data, 4, cellDataSequence, cellDataSequenceLen);
    MMFree(&cellDataSequence);
    
    i32 size = 4 + (i32)cellDataSequenceLen;
    memcpy(&data[0], &size, 4);
    if (outLength) *outLength = (i64)size;
    return data;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++ restore ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static CELL *newCellFromData(void *data, i64 *outSize)
{
    if (data && outSize) {
        memcpy(outSize, &data[0], sizeof(i32));
        if (*outSize < 4) return NULL;
        
        CELL *cell = MMCalloc(1, sizeof(CELL), 'NCF1');
        memcpy(&cell->type, &data[4], sizeof(i32));

        i32 keySize = 0;
        memcpy(&keySize, &data[8], sizeof(i32));
        
        //i32 valueSize = 0;
        memcpy(&cell->vSize, &data[12], sizeof(i32));
        
        cell->key = MMCalloc(keySize + 1, sizeof(char), 'NCF2');
        memcpy(cell->key, &data[16], keySize);

        // type == ANY ならvalueにANYを復元してリテイン
        if (cell->type == ELEMENT_TYPE_ANY) {
            cell->value = ANYNewFromData(&data[16 + keySize], cell->vSize);
        }
        else {
            cell->value = MMCalloc(cell->vSize + 1, sizeof(char), 'NCF3');
            memcpy(cell->value, &data[16 + keySize], cell->vSize);
        }

        return cell;
    }
    return NULL;
}



ANY *ANYNewFromData(void *data, i64 length)
{
    // dataからANYを復元して返す
    if (data) {
        ANY *any = ANYNew();
        for (i64 i=4; i<length; ) {
            
            // cellを復元、type:ANYならretain
            i64 cellSize = 0;
            CELL *cell = newCellFromData(&data[i], &cellSize);
            if (!cell) break;

            if (cell->type == ELEMENT_TYPE_ANY) {
                ANYAdd(any, cell->type, cell->key, cell->value);
            }
            else {
                ANYInsert(any, any->count, cell->type, cell->key, cell->value, cell->vSize);
            }

            eraseCell(cell);
            MMFree((void **)&cell);
            i += cellSize;
        }
        return any;
    }
    return NULL;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*++++++++++++++++++++++++++++++ test +++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void ANYUsage(void)
{
    // ANYは基本的にNew,Add,Get,Remove,Releaseの関数で使います。
    
    // ANYオブジェクト生成：参照カウント1のオブジェクトのポインタが返ります。
    ANY *any = ANYNew();
    
    // 要素追加：ANYオブジェクトのポインタとエレメントタイプを指定してキーとバリューのポインタを渡します。
    // カスタム型を追加する場合はANYInsertを使い、型のサイズを渡します。
    i64_2 range1 = {150, 380};
    ANYInsert(any, any->count, 'i642', "サバ", &range1, sizeof(i64_2));
    
    i64_2 range2 = {180, 320};
    ANYInsert(any, any->count, 'i642', "サーモン", &range2, sizeof(i64_2));
    
    // 異なるエレメントタイプを混在させられます。スカラー型を入れる場合は変数のポインタを渡します。
    ANYAdd(any, ELEMENT_TYPE_STRING, "説明", "寿司ネタ別の価格帯");
    
    i64_2 range4 = {250, 380};
    ANYInsert(any, any->count, 'i642', "ハマチ", &range4, sizeof(i64_2));
    
    // 出力
    ANYPrint(any);
    
    // 要素取得：指定したキーのバリューのポインタが返ります。キーが重複する場合は最初にヒットした要素が返ります。
    i64_2 *range = (i64_2 *)ANYGet(any, "サーモン");
    
    // (取得要素を確認してみる)
    printf("取得要素を確認 %s\n", (char *)ANYGet(any, "説明"));
    if (range) printf("サーモン {%lld,%lld}\n", range->loc, range->len);
    
    // 高速列挙：ANY->rootCellからnextを辿る事で高速に列挙できます。
    CELL *cell = any->rootCell;
    while (cell) {
        if (cell->type == 'i642') {
            i64_2 *range = cell->value;
            if (range) printf("高速列挙 %s {%lld,%lld}\n", cell->key, range->loc, range->len);
        }
        cell = cell->next;
    }
    
    // 要素削除：指定したキーの要素を削除します。重複するキーの要素は全て削除されます。
    ANYRemove(any, "サーモン");
    
    // (削除の状態を確認してみる)
    ANYPrint(any);
    
    // 要素全削除
    ANYRemoveAll(any);
    
    // (削除の状態を確認してみる)
    ANYPrint(any);
    
    // ANYオブジェクト解放：ANYオブジェクトのポインタ変数のポインタを渡します。
    // 参照カウントが0ならオブジェクトは解放され、呼び出し元のポインタ変数にはNULLが代入されます。
    ANYRelease(&any);
}



void ANYTest(void)
{
    // str,i64_2,any,ptrの混在anyを生成、print,serialize,restore,leakのテスト
    for (int i=0; i<1; i++) {
        ANY *any = ANYNew();
        ANYAdd(any, ELEMENT_TYPE_STRING, "key1", "abc123");
        i64_2 range = {5,10};
        ANYInsert(any, any->count, 'i642', "key2", &range, sizeof(i64_2));
        ANY *subany = ANYNew();
        ANYAdd(subany, ELEMENT_TYPE_STRING, "key3", "def456");
        ANYAdd(any, ELEMENT_TYPE_ANY, "key4", subany);
        ANYRelease(&subany);
        void *p = MMCalloc(1, sizeof(void *), 'ATES');
        ANYAdd(any, ELEMENT_TYPE_POINTER, "key6", p);
        ANYPrint(any);
        
        printf("\nシリアライズ、リストア\n");
        i64 len = 0;
        void *data = ANYNewToData(any, &len);
        
        ANY *restore = ANYNewFromData(data, len);
        ANYPrint(restore);
        
        ANYRelease(&restore);
        MMFree(&data);
        MMFree(&p);
        ANYRelease(&any);
    }
    MMPrint();
    RISPrint(reuseCELLList, (char *)__func__, NULL);
}



#pragma clang diagnostic pop
