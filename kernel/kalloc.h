#ifndef KALLOC_H
#define KALLOC_H


#include <stdint.h>

void kinit1(void *vstart, void *vend);
void kinit2(void *vstart, void *vend);
void freerange(void *vstart, void *vend);
void kfree(char *v);
char* kalloc(void);
void dump_freelist();

#endif /* KALLOC_H */
