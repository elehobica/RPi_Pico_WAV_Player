#ifndef _STACK_H_
#define _STACK_H_

// Modified from https://it-ojisan.tokyo/stack/

#include <stdint.h>
#define STACK_SIZE 5

typedef struct {
    uint16_t head;
    uint16_t column;
} stack_data_t;

typedef struct {
	stack_data_t data[STACK_SIZE];
	int tail;
} stack_t;

#ifdef __cplusplus
extern "C" {
#endif

stack_t* stack_init();
int stack_push(stack_t *pStack, stack_data_t *item);
int stack_pop(stack_t *pStack, stack_data_t *item);
int stack_get_count(stack_t *pStack);
void stack_delete(stack_t *pStack);

#ifdef __cplusplus
}
#endif

#endif /* _STACK_H_ */