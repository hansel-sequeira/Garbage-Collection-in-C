#include <stdio.h>
#include "./cuslib.h"

typedef struct Node
{
    char data;
    struct Node *left;
    struct Node *right;
} Node;

Node *generate_tree(size_t level_cur)
{
    if (level_cur == 0)
        return NULL;

    Node *root = custom_alloc(sizeof(*root));
    root->data = level_cur + 'a';
    root->left = generate_tree(level_cur - 1);
    root->right = generate_tree(level_cur - 1);
    return root;
}

void dump_tree(Node *root, int level)
{
    if (root == NULL)
        return;
    for (int i = 1; i <= level; i++)
        printf(i == level ? "\t|" : "\t");
    printf("%d\n", root->data);
    dump_tree(root->left, level + 1);
    dump_tree(root->right, level + 1);
}

int main()
{

    stack_base_ptr = (uintptr_t *)__builtin_frame_address(0);

    Node *root = generate_tree(3);
    dump_tree(root, 0);

    size_t heap_ptrs_cnt = 0;
    // for (size_t i = 0; i < allocated_chunk_list.chunk_count; i++)
    // {
    //     Chunk chunk = allocated_chunk_list.chunks[i];
    //     // iterate for every word of this chunk.
    //     for (size_t j = 0; j < chunk.size; j++)
    //     {
    //         uintptr_t *ptr = (uintptr_t *)*(chunk.start + j);
    //         if (ptr >= heap && ptr < heap + HEAP_MAX_CAP_WORDS)
    //         {
    //             // valid pointer referencing some word on the heap
    //             printf("Captured heap pointer: %p\n", ptr);
    //             heap_ptrs_cnt++;
    //         }
    //     }
    // }
    // printf("No. of pointers to the heap: %zu\n", heap_ptrs_cnt);

    custom_gc();

    printf("Allocated chunks..\n");
    chunk_list_dump(&allocated_chunk_list);
    printf("Freed chunks..\n");
    chunk_list_dump(&freed_chunk_list);
    return 0;
}