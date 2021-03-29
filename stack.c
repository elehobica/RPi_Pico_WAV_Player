#include <stdlib.h>
#include <stdio.h>
#include "stack.h"

// Modified from https://it-ojisan.tokyo/stack/

stack_t* stack_init()
{
    stack_t *pStack = (stack_t *) malloc(sizeof(stack_t));
	if (pStack == NULL) printf("malloc pStack failed\n\r");
	pStack->tail = -1;
    return pStack;
}

int stack_push(stack_t *pStack, stack_data_t *item)
{
	if (pStack->tail >= STACK_SIZE - 1) {
		return 0;
	} else {
		pStack->tail++;
		pStack->data[pStack->tail] = *item;
    }
    return 1;
}

int stack_pop(stack_t *pStack, stack_data_t *item)
{
	if (pStack->tail <= -1){
		return 0;
	} else {
        *item = pStack->data[pStack->tail];
		pStack->tail--;
	}
    return 1;
}

int stack_get_count(stack_t *pStack)
{
	return pStack->tail + 1;
}

void stack_delete(stack_t *pStack)
{
    free(pStack);
}