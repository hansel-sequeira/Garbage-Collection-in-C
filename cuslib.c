#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define HEAP_MAX_CAP 10000
#define MAX_CHUNKS 1000
#define MAX_FREE_CHUNKS 1000

char heap[HEAP_MAX_CAP] = {0};

typedef struct Chunk
{
    void *start;
    size_t size;
} Chunk;

typedef struct Chunk_List
{
    Chunk chunks[MAX_CHUNKS];
    size_t chunk_count;
} Chunk_List;

Chunk_List allocated_chunk_list = {0};
Chunk_List freed_chunk_list = {
    .chunk_count = 1,
    .chunks[0] = {
        .size = sizeof(heap),
        .start = heap}};

int chunk_comparator(const void *a, const void *b)
{
    const Chunk *a_chunk = a;
    const Chunk *b_chunk = b;
    return (a_chunk->start - b_chunk->start);
}

int chunk_list_search(Chunk_List *list, void *ptr)
{
    const Chunk key_chunk = {
        .start = ptr};
    Chunk *ret = bsearch(&key_chunk, list->chunks, list->chunk_count, sizeof(list->chunks[0]),
                         chunk_comparator);
    if (ret == NULL)
        return -1;
    assert(ret >= list->chunks);
    return ((ret - list->chunks));
}

void chunk_list_free(Chunk_List *list, size_t index)
{
    assert(index < list->chunk_count);
    for (size_t i = index; i < list->chunk_count - 1; i++)
    {
        list->chunks[i] = list->chunks[i + 1];
    }
    list->chunk_count--;
}

void chunk_list_dump(Chunk_List *list)
{
    printf("Chunks (%zu): \n", list->chunk_count);
    for (size_t i = 0; i < list->chunk_count; i++)
    {
        printf("\t %p \t %zu\n", list->chunks[i].start, list->chunks[i].size);
    }
}

void chunk_list_insert(Chunk_List *list, void *ptr, size_t size)
{
    assert(list->chunk_count < MAX_CHUNKS);
    list->chunks[list->chunk_count].start = ptr;
    list->chunks[list->chunk_count].size = size;

    // sort on insertion based on pointer value

    for (size_t i = list->chunk_count; i > 0 &&
                                       list->chunks[i].start < list->chunks[i - 1].start;
         i--)
    {
        Chunk temp = list->chunks[i];
        list->chunks[i] = list->chunks[i - 1];
        list->chunks[i - 1] = temp;
    }

    list->chunk_count++;
}

void *custom_alloc(size_t required_size)
{
    if (required_size == 0)
        return NULL;

    for (size_t i = 0; i < freed_chunk_list.chunk_count; i++)
    {
        if (freed_chunk_list.chunks[i].size >= required_size)
        { // first-fit allocation
            Chunk victim_free_chunk = freed_chunk_list.chunks[i];
            void *ptr = victim_free_chunk.start;
            int remaining_size = victim_free_chunk.size - required_size;
            chunk_list_insert(&allocated_chunk_list, ptr, required_size);
            chunk_list_free(&freed_chunk_list, i);
            if (remaining_size > 0)
            {
                chunk_list_insert(&freed_chunk_list, ptr + required_size, remaining_size);
            }

            return ptr;
        }
    }
    return NULL;
}

void custom_free(void *ptr)
{
    if (ptr == NULL)
        return;
    int idx = chunk_list_search(&allocated_chunk_list, ptr);
    assert(idx >= 0);
    // remove this from the allocated chunk list and insert in the freed chunk list
    chunk_list_insert(&freed_chunk_list, allocated_chunk_list.chunks[idx].start, allocated_chunk_list.chunks[idx].size);
    chunk_list_free(&allocated_chunk_list, idx);
}

int main()
{
    // for (int i = 0; i < 10; i++)
    // {
    //     void *ret = custom_alloc(i);
    //     if (i % 2 == 0)
    //         custom_free(ret);
    // }
    void *p1 = custom_alloc(1);
    void *p2 = custom_alloc(2);
    custom_free(p1);
    custom_free(p2);

    custom_alloc(3);
    printf("Allocated chunks..\n");
    chunk_list_dump(&allocated_chunk_list);
    printf("Freed chunks..\n");
    chunk_list_dump(&freed_chunk_list);
    return 0;
}