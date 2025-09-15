#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct block block_t;
typedef struct memory_region memory_region_t;

struct block {
    size_t size;
    uint8_t is_free;
    block_t* next;
};

struct memory_region {
    void* address;
    size_t size;
    memory_region_t* next;
};

static block_t* free_list = NULL;
static memory_region_t* regions = NULL;

void* init_allocator(size_t size) {
    void* memory = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if(memory == NULL) {
        fprintf(stderr, "Failed to allocate memory. Error: %lu\n", GetLastError());
        return NULL;
    }

    memory_region_t* new_region = (memory_region_t*)malloc(sizeof(memory_region_t));
    new_region->address = memory;
    new_region->size = size;
    new_region->next = regions;
    regions = new_region;

    block_t* block = (block_t*)memory;
    block->size = size - sizeof(block_t);
    block->is_free = 1;
    block->next = NULL;

    free_list = block;
    printf("Allocated %zu bytes at %p\n", size, memory);
    return memory;
}

void* mem_alloc(size_t size) {
    if(free_list == NULL) {
        size_t pool_size = (size > 1024 * 1024) ? size * 2 : 1024 * 1024;
        if(init_allocator(pool_size) == NULL) return NULL;
    }

    block_t* current = free_list;
    while(current != NULL) {
        if(current->is_free && current->size >= size) {
            current->is_free = 0;

            if(current->size > size + sizeof(block_t) + 16) {
                block_t* next_block = (block_t*)((char*)(current + 1) + size);
                next_block->is_free = 1;
                next_block->size = current->size - size - sizeof(block_t);
                next_block->next = current->next;

                current->size = size;
                current->next = next_block;
            }

            return (void*)(current + 1);
        }

        current = current->next;
    }

    fprintf(stderr, "No suitable block found for size %zu\n", size);
    return NULL;
}

void mem_free(void* ptr) {
    if(ptr == NULL) return;

    block_t* block = (block_t*)ptr - 1;
    block->is_free = 1;

    block_t* current = free_list;
    while(current != NULL && current->next != NULL) {
        if(current->is_free && current->next->is_free) {
            current->size += current->next->size + sizeof(block_t);
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void destroy_allocator() {
    memory_region_t* current = regions;

    while(current != NULL) {
        VirtualFree(current->address, 0, MEM_RELEASE);
        memory_region_t* next = current->next;
        free(current);
        current = next;
    }

    regions = NULL;
    free_list = NULL;
}

int main(int argc, char* argv[]) {
    int* a = mem_alloc(sizeof(int));
    *a = 2;

    char* b = (char*)mem_alloc(sizeof(char) * 20);
    strcpy(b, "Hello, world!");

    printf("%p, %d\n", a, *a);
    printf("%p, %s\n", b, b);

    mem_free(a);
    printf("%p, %s\n", b, b);

    destroy_allocator();

    return 0;
}
