#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct block_t block_t;

struct block_t {
    size_t size;
    uint8_t is_free;
    block_t* next;
};

static block_t* free_list = NULL;

void* init_allocator(size_t size) {
    void* memory = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if(memory == NULL) {
        perror("Failed to allocate memory 1");
        return NULL;
    }

    block_t* block = (block_t*)memory;
    block->size = size - sizeof(block_t);
    block->is_free = 1;
    block->next = NULL;

    free_list = block;
    return memory;
}

void* mem_alloc(size_t size) {
    if(free_list == NULL && init_allocator((size > 1024 * 1024) ? size * 2 : 1024 * 1024) == NULL) {
        perror("Failed to allocate memory 2");
        return NULL;
    }

    block_t* current = free_list;
    block_t* previous = NULL;

    while(current != NULL) {
        if(current->is_free == 1 && current->size >= size) {
            current->is_free = 0;

            if(current->size > size + sizeof(block_t) + 16) {
                block_t* next_block = (block_t*)((char*)(current + 1) + size);
                next_block->is_free = 1;
                next_block->size = current->size - size - sizeof(block_t);

                current->size = size;
                current->next = next_block;
            }

            return (void*)(current + 1);
        }

        previous = current;
        current = current->next;
    }

    perror("Failed to allocate memory 3");
    return NULL;
}

int main(int argc, char* argv[]) {
    int* a = mem_alloc(sizeof(int));
    *a = 2;

    char* b = (char*)mem_alloc(sizeof(char) * 20);
    strcpy(b, "Hello, world!");

    printf("%p, %d\n", a, *a);
    printf("%p, %s\n", b, b);

    return 0;
}
