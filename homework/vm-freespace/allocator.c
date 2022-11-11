// free list example
// gcc allocator.c -o bin/allocator

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

typedef struct header {
  uint32_t size;
} header_t;

typedef struct pointer {
  header_t *hptr;
  uint32_t addr;
} pointer_t;

typedef struct free_list {
  // we would have a header etc. here if the free list was
  // allocated on the heap with other memory requests
  uint32_t addr;
  uint32_t size;
  struct free_list *next;
} free_list_t;

void prepend_free_list_node(free_list_t **free_list, free_list_t *node) {
  node->next = *free_list;
  *free_list = node;
}

free_list_t *create_free_list_node(uint32_t addr, uint32_t size) {
  free_list_t *node = NULL;
  if ((node = malloc(sizeof(free_list_t))) == NULL) {
    fprintf(stderr, "Error creating free list node.\nErr %i: %s.", 
      errno, strerror(errno)
    );
    exit(EXIT_FAILURE);
  }
  node->next = NULL;
  node->addr = addr;
  node->size = size;
  return node;
}

void print_free_list_list(free_list_t **free_list) {
  fprintf(stdout, "Free list:\n"); 
  free_list_t *node = *free_list;
  while (node != NULL) {
    fprintf(stdout, "\t-> Addr: %i, size: %i\n", node->addr, node->size); 
    node = node->next;
  }
}

header_t *create_header(uint32_t size) {
  header_t *header = NULL;
  if ((header = malloc(sizeof(header_t))) == NULL) {
    fprintf(stderr, "Error creating pointer header.\nErr %i: %s.", 
      errno, strerror(errno)
    );
    exit(EXIT_FAILURE);
  }
  header->size = size;
  return header;
}

pointer_t *create_pointer(uint32_t addr, uint32_t size) {
  header_t *header = create_header(size);
  pointer_t *ptr = NULL;
  if ((ptr = malloc(sizeof(pointer_t))) == NULL) {
    fprintf(stderr, "Error creating free list node.\nErr %i: %s.", 
      errno, strerror(errno)
    );
    exit(EXIT_FAILURE);
  }
  ptr->hptr = header;
  ptr->addr = addr + sizeof(header_t);
  return ptr;
}

// find a free memory slice with a 'first fit' strategy
uint32_t find_free_memory_slice(free_list_t **free_list, uint32_t request_size) {
  free_list_t *node = *free_list;
  uint32_t rv = 0;
  while (node != NULL) {
    if (node->size > request_size) {
      // split the free list block by subtracting the size of the 
      // requested region, (the split block doesn't become 'two' until 
      // the requested memory is freed and returned to the list) and
      // increment the addr of the given slice by the request size
      rv = node->addr;
      node->size -= (sizeof(header_t) + request_size);
      node->addr += (sizeof(header_t) + request_size);
      return rv;
    }
    // we can coalesce as we step through, or as we free, or both?
    node = node->next;
  }
  return rv; // 0 if no slice >= required size
}

pointer_t *request_memory(free_list_t **free_list, uint32_t size) {
  pointer_t *requested_memory_location = NULL;
  uint32_t free_addr = 0; 
  if ((free_addr = find_free_memory_slice(free_list, size)) == 0) {
    return requested_memory_location; // NULL
  }
  // if we find a slice of adequate size create a ptr
  // with the addr location and a header with the size
  if ((requested_memory_location = create_pointer(free_addr, size)) == NULL) {
    return requested_memory_location; // NULL
  }
  return requested_memory_location;
}

void free_pointer(free_list_t **free_list, pointer_t *ptr) {
  prepend_free_list_node(
    free_list, 
    create_free_list_node(ptr->addr, ptr->hptr->size + sizeof(header_t))
  );
  free(ptr); // for real
}

void coalesce_free_list() {
  // todo 
}

int main(void) {

  // create and initialise a free list with a heap memory of 4KB
  free_list_t *free_list = NULL;
  prepend_free_list_node(&free_list, create_free_list_node(1, 4096));
  print_free_list_list(&free_list);

  // request chunks of memory and print the free list state
  pointer_t *mem1 = NULL;
  if ((mem1 = request_memory(&free_list, 64)) == NULL) {
    fprintf(stderr, "No memory location found.\n"); 
  }
  fprintf(stdout, "Received: addr: %i, size: %d.\n", mem1->addr, mem1->hptr->size);

  pointer_t *mem2 = NULL;
  if ((mem2 = request_memory(&free_list, 128)) == NULL) {
    fprintf(stderr, "No memory location found.\n"); 
  }
  fprintf(stdout, "Received: addr: %i, size: %d.\n", mem2->addr, mem2->hptr->size);

  pointer_t *mem3 = NULL;
  if ((mem3 = request_memory(&free_list, 256)) == NULL) {
    fprintf(stderr, "No memory location found.\n"); 
  }
  fprintf(stdout, "Received: addr: %i, size: %d.\n", mem3->addr, mem3->hptr->size);
  print_free_list_list(&free_list);
  assert(free_list->size == 4096 - (sizeof(header_t) * 3 + 64 + 128 + 256));

  free_pointer(&free_list, mem2);
  print_free_list_list(&free_list);
  assert(free_list->size == 128 + sizeof(header_t)); // freed mem is head of list



}
