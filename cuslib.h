#ifndef CUSLIB_H
#define CUSLIB_H

#include <stdio.h>
#include <stdlib.h>

#define HEAP_MAX_CAP 10000
#define MAX_CHUNKS 1000
#define MAX_FREE_CHUNKS 1000

void *custom_alloc(size_t required_size_bytes);
void custom_free(void *ptr);

extern uintptr_t heap[HEAP_MAX_CAP / sizeof(uintptr_t)];

typedef struct Chunk
{
    uintptr_t *start;
    size_t size;
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