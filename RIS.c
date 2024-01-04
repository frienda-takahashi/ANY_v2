#include <string.h>     // memcpy,memset
#include "MM.h"
#include "RIS.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmultichar"



typedef struct RISCell {
    struct RISCell *prev;
    void           *item;
    i32             alive;
} RISCell;



typedef struct RIS {
    RISCell *last, *first;
    i32 created, alive;
} RIS;



static RISCell *searchAndReconnect(RISCell *last, void *item)
{
    // itemを持つCellを返却、返却Cellの後ろを前に接続
    RISCell *next = NULL;
    while (last) {
        if (last->item == item) {
            if (next) next->prev = last->prev;
            return last;
        }
        next = last;
        last = last->prev;
    }
    return NULL;
}



void *RISGet(RIS **stack, size_t size)
{
    // *stackが無ければ生成
    // lastが無ければ、cell生成、cellをfirstとlastに、aliveしてitemを返す
    // lastがaliveなら、cell生成、cell->prev = last, last = cell、aliveしてitemを返す
    // lastがaliveで無ければ、lastをfirstに移動、aliveしてitemを返す
    
    if (!*stack) *stack = MMCalloc(1, sizeof(RIS), 'RIS0');
    
    if (!(*stack)->last) {
        RISCell *cell = MMCalloc(1, sizeof(RISCell), 'RIS1');
        cell->item = MMCalloc(1, size, 'RIS2');
        (*stack)->first = (*stack)->last = cell;
        cell->alive = 1;
        (*stack)->created++;
        (*stack)->alive++;
        return cell->item;
    }
    else if ((*stack)->last->alive) {
        RISCell *cell = MMCalloc(1, sizeof(RISCell), 'RIS3');
        cell->item = MMCalloc(1, size, 'RIS4');
        cell->prev = (*stack)->last;
        (*stack)->last = cell;
        cell->alive = 1;
        (*stack)->created++;
        (*stack)->alive++;
        return cell->item;
    }
    else if ((*stack)->last->alive == 0) {
        (*stack)->first->prev = (*stack)->last;
        (*stack)->last = (*stack)->last->prev;
        (*stack)->first = (*stack)->last;
        (*stack)->first->prev = NULL;
        (*stack)->last->alive = 1;
        (*stack)->alive++;
        return (*stack)->first->item;
    }

    printf("ERROR %s %d is empty\n", __func__, __LINE__);
    return NULL;
}



void RISAdd(RIS **stack, void *item, size_t size)
{
    if (stack && item) {
        // itemをmemset(0)、aliveを0セットしcellをlastに移動、連結リスト再接続
        RISCell *cell = searchAndReconnect((*stack)->last, item);
        if (cell) {
            memset(cell->item, 0, size);
            cell->prev = (*stack)->last;
            cell->alive = 0;
            (*stack)->alive--;
            (*stack)->last = cell;
            return;
        }
        printf("ERROR %s %d no stacked cell for item:%p. stack:%p\n", __func__, __LINE__, item, stack);
    }
    printf("ERROR %s %d stack:%p item:%p\n", __func__, __LINE__, stack, item);
}



void RISPrint(RIS *stack, char *comment, void(*cb)(void *item))
{
    if (stack) {
        printf("RISPrint %s created:%d alive:%d\n", comment, stack->created, stack->alive);
        if (cb) {
            RISCell *cell = stack->last;
            while (cell) {
                cb(cell->item);
                cell = cell->prev;
            }
        }
    }
    else {
        printf("ERROR %s %d %s NO STACK\n", __func__, __LINE__, comment);
    }
}



void RISPrintCells(RIS *stack)
{
    if (stack) {
        RISCell *cell = stack->last;
        while (cell) {
            printf("%p:%d, ", cell->item, cell->alive);
            cell = cell->prev;
        }
        printf("\n");
    }
}



void RISTest(void)
{
    // get
    static RIS *stack = NULL;
    const i32 cnt = 3;
    i64_2 *list[cnt] = {0};
    for (i32 i=0; i<cnt; i++) {
        list[i] = RISGet(&stack, sizeof(i64_2));
        list[i]->loc = 1;
    }
    RISPrintCells(stack);
    
    // add
    for (i32 i=0; i<cnt-1; i++) {
        RISAdd(&stack, list[i], sizeof(i64_2));
    }
    RISPrintCells(stack);
    RISPrint(stack, NULL, NULL);
    MMPrint();
}



#pragma clang diagnostic pop
















