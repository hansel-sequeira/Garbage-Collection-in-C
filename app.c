#include <stdio.h>
#include "./cuslib.h"

int main()
{

    void *ptr[11] = {0};
    for (int i = 0; i <= 10; i++)
    {
        ptr[i] = custom_alloc(i);
        if (i == 6 || i == 7)
            custom_free(ptr[i]);
    }

    printf("Allocated chunks..\n");
    chunk_list_dump(&allocated_chunk_list);
    printf("Freed chunks..\n");
    chunk_list_dump(&freed_chunk_list);
    return 0;
}