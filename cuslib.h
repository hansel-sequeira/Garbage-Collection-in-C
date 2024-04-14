#ifndef CUSLIB_H
#define CUSLIB_H

#include <stdio.h>
#include <stdlib.h>

#define HEAP_MAX_CAP_BYTES 10000
#define HEAP_MAX_CAP_WORDS (HEAP_MAX_CAP_BYTES / sizeof(uintptr_t))
#define MAX_CHUNKS 1000

void *custom_alloc(size_t required_size_bytes);
void custom_free(void *ptr);
void custom_gc();

extern uintptr_t heap[HEAP_MAX_CAP_WORDS];
extern uintptr_t *stack_base_ptr;

typedef struct Chunk
{
    uintptr_t *start;
    size_t size; // size in words
} Chunk;

typedef struct Chunk_List
{
    Chunk chunks[MAX_CHUNKS];
    size_t chunk_count;
} Chunk_List;

extern Chunk_List allocated_chunk_list;
extern Chunk_List freed_chunk_list;

int chunk_list_search(Chunk_List *list, uintptr_t *ptr);
void chunk_list_compaction(Chunk_List *list);
void chunk_list_free(Chunk_List *list, size_t index);
void chunk_list_dump(Chunk_List *list);
void chunk_list_insert(Chunk_List *list, uintptr_t *ptr, size_t size);

#endif // CUSLIB_H