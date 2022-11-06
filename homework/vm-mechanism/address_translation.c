// example of a dynamic relocation (base and bounds) memory translation
// gcc address_translation.c -o bin/address_translation

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// track valid/invalid memory accesses
uint32_t naccess = 0;
uint32_t nexception = 0;

const uint32_t NADDR = 100;
const uint32_t ADDR_SPACE = 10;

uint32_t memory[NADDR] = { 
  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 
  0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 
  0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 
  0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 
  0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 
  0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 
  0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 
  0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 
  0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 
  0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63
};

enum mode_t {
  KERNEL,
  USER,
};

typedef struct process {
  uint32_t pid;
  uint32_t program_counter;
} process_t;

typedef struct process_table {
  process_t *process;
  uint32_t process_base;
  uint32_t process_bounds;
  struct process_table *next;
} process_table_t;

typedef struct cpu {
  uint32_t process_base_register;
  uint32_t process_bounds_register;
  process_table_t *cur_process;
  enum mode_t os_mode;
} cpu_t;

process_t *create_process(uint32_t pid) {
  process_t *process = NULL;
  if ((process = malloc(sizeof(process_t))) == NULL) {
    fprintf(stderr, "Error creating process\n");
    exit(EXIT_FAILURE);
  }
  process->pid = pid;
  return process;
}

void append_process(process_table_t **process_table_head, process_t *process) {
  // create process table entry
  process_table_t *process_table_node = NULL;
  if ((process_table_node = malloc(sizeof(process_table_t))) == NULL) {
    fprintf(stderr, "Error creating process table entry\n");
    exit(EXIT_FAILURE);
  }

  process_table_node->process_base = process->pid * ADDR_SPACE;
  process_table_node->process_bounds = process_table_node->process_base + ADDR_SPACE;
  process_table_node->next = NULL;
  process_table_node->process = process;

  process_table_t **cur = process_table_head;
  while (*cur != NULL) {
    cur = &(*cur)->next;
  }
  *cur = process_table_node;
}

// very friendly exception handler
int32_t raise_exception() {
  nexception++;
  return -1;
}

// change the cpu's current process
void change_cpu_process(cpu_t *cpu, process_table_t *current_process) {
  if (cpu->os_mode != KERNEL) {
    raise_exception();
  }
  cpu->cur_process = current_process;
  cpu->process_base_register = current_process->process_base;
  cpu->process_bounds_register = current_process->process_bounds;
}

void trap_handler(cpu_t *cpu, process_table_t *process_table) {
  // trap handler operates in kernel mode
  cpu->os_mode = KERNEL;
  change_cpu_process(cpu, process_table);
  cpu->os_mode = USER;
}

int32_t access_memory_address(uint32_t addr, uint32_t memory[], cpu_t cpu) {
  if (cpu.process_base_register + addr >= cpu.process_bounds_register) {
    return raise_exception();
  }
  naccess++;
  return memory[cpu.process_base_register + addr];
}

int main(void) {

  cpu_t cpu = { 
    0x0,
    0xA,
    NULL,
    KERNEL
  };

  // create some processes and a process table list
  process_table_t *head = NULL;
  append_process(&head, create_process(1));
  append_process(&head, create_process(2));
  append_process(&head, create_process(3));

  process_table_t *current_process = head;
  while (current_process != NULL) {

    // set the current process table list item as the running process
    trap_handler(&cpu, current_process);

    fprintf(stdout, "PID: %i, base: %i, bounds: %i\n",
      cpu.cur_process->process->pid, 
      cpu.process_base_register,
      cpu.process_bounds_register
    );
    
    int32_t rv = 0;
    for (uint32_t i = 0; i < 100; i++) {
      // try to access each available memory location, valid accesses
      // return the value at the memory addr, illegal access returns -1
      rv = access_memory_address(i, memory, cpu);
      if (cpu.process_base_register + i >= cpu.process_bounds_register) {
        assert(rv == -1);
        continue;
      }
      // memory value is the same as it's index
      assert(rv == cpu.process_base_register + i);
    }

    current_process = current_process->next;
  }

  fprintf(stdout, "Total valid memory accesses: %i\n", naccess);
  assert(naccess == 30);
  fprintf(stdout, "Total illegal memory accesses: %i\n", nexception);
  assert(nexception == 270);
}
