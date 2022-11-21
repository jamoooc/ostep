// free list example
// gcc allocator.c -o bin/allocator

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

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
  struct free_list *prev;
} free_list_t;

void prepend_free_list_node(free_list_t **free_list, free_list_t *node) {
  if (*free_list) {
    (*free_list)->prev = node;
  }
  node->next = *free_list;
  node->prev = NULL;
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
  node->prev = NULL;
  node->addr = addr;
  node->size = size;
  return node;
}

void print_free_list(free_list_t **free_list) {
  free_list_t *node = *free_list;
  while (node != NULL) {
    fprintf(stdout, "\t-> addr: %i, size: %i, next: %p, prev:%p\n", node->addr, node->size, node->next, node->prev);
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
  ptr->addr = addr;
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
      break;
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

free_list_t *coalesce_free_list(free_list_t **free_list) {
  fprintf(stdout, "Coalescing free list...\n");

  free_list_t *i = *free_list;
  
  top:
  while (i != NULL) {
    free_list_t *j = *free_list;
    while (j != NULL) {
      if (i->addr + i->size == j->addr) {
        fprintf(stdout, 
          "Found adjacent nodes: \n\ti -> addr: %d, size: %d.\n\tj -> addr: %d, size: %d.\n",
          i->addr, i->size,
          j->addr, j->size
        );

        // increase the address size of i by j->size
        i->size += j->size;

        // if j is the head of the free_list set the head node to i
        if (*free_list == j) {
          fprintf(stdout, "j is HEAD\n");
          *free_list = j->next;
        }

        // if j is not the last node, set the prev ptr
        // of the next node, to the prev ptr of j
        if (j->next != NULL) {
          j->next->prev = j->prev;
        }

        // if j is not the head node, set the next ptr
        // of the prev node, to the next ptr of j
        if (j->prev != NULL) {
          j->prev->next = j->next;
        }

        free(j);

        // skip i = i->next if we remove a node and restart the loop
        i = *free_list;
        goto top; 
      }
      j = j->next;
      // sleep(1);
    }
    i = i->next;
    // sleep(1);
  }
  return *free_list;
}

void free_pointer(free_list_t **free_list, pointer_t *ptr) {
  prepend_free_list_node(
    free_list, 
    create_free_list_node(ptr->addr, ptr->hptr->size + sizeof(header_t))
  );
  free(ptr->hptr);
  free(ptr);
  // coalesce_free_list(free_list);
}

int main(void) {

  // create and initialise a free list with a heap memory of 4KB
  free_list_t *free_list = NULL;
  prepend_free_list_node(&free_list, create_free_list_node(1, 4096));

  fprintf(stdout, "Initial free list:\n"); 
  print_free_list(&free_list);

  // request chunks of memory and print the free list state
  pointer_t *mem1 = NULL;
  if ((mem1 = request_memory(&free_list, 64)) == NULL) {
    fprintf(stderr, "No memory location found.\n"); 
  }
  fprintf(stdout, "Received memory -> addr: %i, size: %d.\n", mem1->addr, mem1->hptr->size);

  pointer_t *mem2 = NULL;
  if ((mem2 = request_memory(&free_list, 128)) == NULL) {
    fprintf(stderr, "No memory location found.\n"); 
  }
  fprintf(stdout, "Received memory -> addr: %i, size: %d.\n", mem2->addr, mem2->hptr->size);

  pointer_t *mem3 = NULL;
  if ((mem3 = request_memory(&free_list, 256)) == NULL) {
    fprintf(stderr, "No memory location found.\n"); 
  }
  fprintf(stdout, "Received memory -> addr: %i, size: %d.\n", mem3->addr, mem3->hptr->size);

  pointer_t *mem4 = NULL;
  if ((mem4 = request_memory(&free_list, 512)) == NULL) {
    fprintf(stderr, "No memory location found.\n"); 
  }
  fprintf(stdout, "Received memory -> addr: %i, size: %d.\n", mem4->addr, mem4->hptr->size);

  pointer_t *mem5 = NULL;
  if ((mem5 = request_memory(&free_list, 1024)) == NULL) {
    fprintf(stderr, "No memory location found.\n"); 
  }
  fprintf(stdout, "Received memory -> addr: %i, size: %d.\n", mem5->addr, mem5->hptr->size);
  
  fprintf(stdout, "Free list before freeing requested memory:\n");
  print_free_list(&free_list);

  free_pointer(&free_list, mem2);
  free_pointer(&free_list, mem5);
  free_pointer(&free_list, mem3);
  free_pointer(&free_list, mem4);
  free_pointer(&free_list, mem1);
  
  fprintf(stdout, "Free list after freeing requested memory:\n");
  print_free_list(&free_list);

  coalesce_free_list(&free_list);

  fprintf(stdout, "Free list after coalescing:\n");
  print_free_list(&free_list);
}
