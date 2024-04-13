#include <stdio.h>
#include <assert.h>

#define HEAP_MAX_CAP 10000
#define MAX_ALLOC_CHUNKS 1000
#define MAX_FREE_CHUNKS 1000

char heap[HEAP_MAX_CAP] = {0};
int heap_allocated_bytes = 0;

typedef struct Chunk
{
    void *start;
    size_t size;
} Chunk;

Chunk allocated_chunk_list[MAX_ALLOC_CHUNKS] = {0};
size_t allocated_chunk_count = 0;

Chunk freed_chunk_list[MAX_FREE_CHUNKS] = {0};
size_t freed_chunk_count = 0;
void *custom_alloc(size_t required_size)
{
    if (required_size == 0)
        return NULL;
    assert(heap_allocated_bytes + required_size <= HEAP_MAX_CAP);
    void *ptr = heap + heap_allocated_bytes;
    heap_allocated_bytes += required_size;

    Chunk allocated_chunk = {
        .start = ptr,
        .size = required_size};

    allocated_chunk_list[allocated_chunk_count++] = allocated_chunk;

    return ptr;
}

void dump_allocated_chunk_list()
{
    printf("List of allocated chunks (%zu):\n", allocated_chunk_count);
    for (size_t i = 0; i < allocated_chunk_count; i++)
    {
        printf("\t %p \t %zu\n", allocated_chunk_list[i].start, allocated_chunk_list[i].size);
    }
}

int main()
{
    for (int i = 0; i < 10; i++)
        custom_alloc(i);
    dump_allocated_chunk_list();
    return 0;
}