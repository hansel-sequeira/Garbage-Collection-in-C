#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "cuslib.h"

uintptr_t heap[HEAP_MAX_CAP_WORDS] = {0};
uintptr_t *stack_base_ptr = 0;
uintptr_t *leaked_chunks[MAX_CHUNKS] = {0};
bool visited[] = {0};

Chunk_List allocated_chunk_list = {0};
Chunk_List freed_chunk_list = {
    .chunk_count = 1,
    .chunks[0] = {
        .size = sizeof(heap) / sizeof(uintptr_t),
        .start = heap}};

int chunk_comparator_size(const void *a, const void *b)
{
    const Chunk *a_chunk = a;
    const Chunk *b_chunk = b;
    return (a_chunk->size - b_chunk->size);
}

int chunk_comparator_address(const void *a, const void *b)
{
    const Chunk *a_chunk = a;
    const Chunk *b_chunk = b;
    return (a_chunk->start - b_chunk->start);
}

int chunk_list_search(Chunk_List *list, uintptr_t *ptr)
{
    const Chunk key_chunk = {
        .start = ptr};
    Chunk *ret = bsearch(&key_chunk, list->chunks, list->chunk_count, sizeof(list->chunks[0]),
                         chunk_comparator_address);
    if (ret == NULL)
        return -1;
    assert(ret >= list->chunks);
    return ((ret - list->chunks));
}

void chunk_list_compaction(Chunk_List *list)
{
    Chunk_List temp = {0};
    for (size_t i = 0; i < list->chunk_count; i++)
    {
        if (temp.chunk_count == 0)
            temp.chunks[temp.chunk_count++] = list->chunks[i];
        else
        {
            if (temp.chunks[temp.chunk_count - 1].start + temp.chunks[temp.chunk_count - 1].size == list->chunks[i].start)
                temp.chunks[temp.chunk_count - 1].size += list->chunks[i].size;
            else
                temp.chunks[temp.chunk_count++] = list->chunks[i];
        }
    }

    *list = temp;
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

void chunk_list_insert(Chunk_List *list, uintptr_t *ptr, size_t size)
{
    assert(list->chunk_count < MAX_CHUNKS);
    list->chunks[list->chunk_count].start = ptr;
    list->chunks[list->chunk_count].size = size;

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

void mark_chunks(uintptr_t *start, const uintptr_t *end)
{
    while (start <= end)
    {
        uintptr_t *word_val = (uintptr_t *)(*start);
        for (size_t i = 0; i < allocated_chunk_list.chunk_count; i++)
        {
            Chunk chunk = allocated_chunk_list.chunks[i];
            if (word_val >= chunk.start && word_val < chunk.start + chunk.size)
            {
                if (!visited[i])
                {
                    visited[i] = true;
                    mark_chunks(chunk.start, chunk.start + chunk.size);
                }
            }
        }
        start++;
    }
}

void *custom_alloc(size_t required_size_bytes)
{
    chunk_list_compaction(&freed_chunk_list);

    size_t required_size_words = (required_size_bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
    if (required_size_words == 0)
        return NULL;

    for (size_t i = 0; i < freed_chunk_list.chunk_count; i++)
    {
        if (freed_chunk_list.chunks[i].size >= required_size_words)
        {
            Chunk victim_free_chunk = freed_chunk_list.chunks[i];
            uintptr_t *ptr = victim_free_chunk.start;
            int remaining_size_words = victim_free_chunk.size - required_size_words;
            chunk_list_insert(&allocated_chunk_list, ptr, required_size_words);
            chunk_list_free(&freed_chunk_list, i);
            if (remaining_size_words > 0)
                chunk_list_insert(&freed_chunk_list, ptr + required_size_words, remaining_size_words);

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
    qsort(&(freed_chunk_list.chunks), freed_chunk_list.chunk_count, sizeof(freed_chunk_list.chunks[0]), chunk_comparator_size);
    chunk_list_free(&allocated_chunk_list, idx);
}

void custom_gc()
{
    uintptr_t *curr_ptr = (uintptr_t *)__builtin_frame_address(0);
    size_t leaked_chunks_cnt = 0;
    memset(visited, false, sizeof(visited));
    memset(leaked_chunks, 0, sizeof(leaked_chunks));
    mark_chunks(curr_ptr, stack_base_ptr + 1);

    for (size_t i = 0; i < allocated_chunk_list.chunk_count; i++)
    {
        if (!visited[i])
        {
            printf("Freeing: %p\n", allocated_chunk_list.chunks[i].start);
            leaked_chunks[leaked_chunks_cnt++] = allocated_chunk_list.chunks[i].start;
        }
    }

    for (size_t i = 0; i < leaked_chunks_cnt; i++)
        custom_free(leaked_chunks[i]);
}
