
#pragma once
#include <stdio.h>


// Reusable Item Stack : RIS
// stackが無ければ生成
// getで空のitemを返す、空のitemが無ければsize分確保して返す
// addでitemを0クリアしてスタック



typedef struct RIS RIS;



void  RISAdd(RIS **stack, void *item, size_t size);
void *RISGet(RIS **stack, size_t size);
void  RISPrint(RIS *stack, char *comment, void(*cb)(void *item));
void  RISTest(void);

