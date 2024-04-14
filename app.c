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

    custom_gc();

    printf("Allocated chunks..\n");
    chunk_list_dump(&allocated_chunk_list);
    printf("Freed chunks..\n");
    chunk_list_dump(&freed_chunk_list);
    return 0;
}