// address translation with segmentaion
// gcc segmentation.c -o bin/segmentation

// an example of virtual address translation using the top 2 bits
// to specify the memory segment to apply the address offset

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum memory_location {
  CODE,
  STACK,
  HEAD
};

#define SEG_MASK 0x3000
#define SEG_SHIFT 0xC
#define OFFSET_MASK 0xFFF

// the top two bits are set to 00 for program code, 
// 01 for the heap and 11 for the stack.
#define HEAP_MASK 0x1000
#define STACK_MASK 0x3000

void print_binary(uint16_t n) {
  fprintf(stdout, "Binary: ");
  for (int i = 15; i >= 0; i--) {
    fprintf(stdout, "%i", (n >> i) & 1);
    if (i % 4 == 0 && i != 0) {
      fprintf(stdout, " "); // space separate every 4 digits
    }
  }
  fprintf(stdout, ", Hex: 0x%X, Dec: %d\n", n, n);
}

char *print_segment(uint16_t n) {
  switch (n) {
    case 0: {
      return "CODE";
    }
    case 1: {
      return "HEAP";
    }
    case 2: {
      return "STACK";
    }
    default: {
      return "";
    }
  }
}

int main(void) {
  // take the 12 bit virtual address offset 104 (decimal).
  // without applying the heap or stack mask this can be 
  // assumed to be program code segment
  uint16_t addr_segment_offset = 0x068;

  fprintf(stdout, "Address offset:\n");
  print_binary(addr_segment_offset);

  // bitwise OR the offset with the heap segment mask
  uint16_t virtual_addr = addr_segment_offset | HEAP_MASK;
  
  fprintf(stdout, "\nVirtual address:\n");
  print_binary(virtual_addr);

  // get the memory segment by right shifting the bottom 
  // 12 bits of the segment offset
  uint16_t segment = (virtual_addr & SEG_MASK) >> SEG_SHIFT; 
  assert(segment == 1);
  fprintf(stdout, "\nAddress segment: %s\n", print_segment(segment));
  print_binary(segment);
  
  // get the offset from the virtual address
  uint16_t offset = virtual_addr & OFFSET_MASK;
  assert(offset == addr_segment_offset);
  fprintf(stdout, "\nAddress offset: %d\n", offset);
  print_binary(offset);

  // bitwise OR the offset with the stack segment mask
  virtual_addr = addr_segment_offset | STACK_MASK;
  fprintf(stdout, "\nVirtual address\n");
  print_binary(virtual_addr);

  segment = (virtual_addr & SEG_MASK) >> SEG_SHIFT;
  assert(segment == 3);
  fprintf(stdout, "\nAddress segment: %s\n", print_segment(segment));
  print_binary(segment);

  // we would add the base and check bounds before accessing any memory addr
}

