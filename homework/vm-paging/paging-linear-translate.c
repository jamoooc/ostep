#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*

  A processes virtual memory address space is split into fixed sized units,
  called `pages`. Physical memory is split into fixed sized units called 
  `page frames`. Each can contain a single virtual memory page. Some page 
  frames are reserved by the OS and the rest can be allocated to processes. 
  A processes physical pages are not necessarily allocated to contiguous 
  frames and are often assigned via a free list.

  The OS maintains a per-process `page table` to map, via address translations,
  where each virtual page resides in physical memory.

  To translate a processes virtual address it is split into two components,
  the `virtual page number` (VPN) and the `offset` into the page.

  E.g. for a 16 byte page in a 64 byte address space we need to be able to
  select 4 pages, requiring 2 bits of the address, and to 4 bits to index 
  into the page (0 - 15).

  Virtual address 21:

  |----VPN----|---------offset--------|
  |  0  |  1  |  0  |  1  |  0  |  1  |

  VPN: 1 (b01)
  Offset 5 (b0101)

  The `physical frame number` (PFN) is retrieved from the page table and 
  substituted for the VPN. The most simple page table is an array, indexed by 
  the VPN.

  // linear page table: array of page table entry (PTE) structs containing the PFN
  pagetable = [ 7, ... ]; 
  PFN = pagetable[VPN]; (b111 = pagetable[b01])

  |-------PFN-------|---------offset--------|
  |  1  |  1  |  1  |  1  |  0  |  1  |  0  |

  Virtual address 21 therefore maps to physical address 117.

  The page-table is located by the page-table base register. Thus:

  VPN = (virtual address & VPN_MASK) >> OFFSET_LEN;
  PTEAddr = PageTableBaseRegister + (VPN * sizeof(PTE))

  In the example above the VPN_MASK = 0x30 (b110000) and the OFFSET_LEN to 4. The 
  PTEAddr can then be use to index into the array of PTEs, extract the PFN and
  concatenate it with the offset to form the physical addr.

*/


// valid bit etc would probably be retrieved by masks instead of a separate field
typedef struct pte {
  u_int8_t addr;
  u_int8_t valid;
} pte_t;


int main(void) {

  u_int8_t virtual_address = 0x15; // 21
  // use 0 for the page-table base register as we have the page-table in memory
  u_int8_t page_table_base_register = 0x0; 
  u_int8_t VPN_MASK = 0x30;
  u_int8_t OFFSET_MASK = 0xf;
  u_int8_t OFFSET_SHIFT = 0x4;
  u_int8_t PFN_SHIFT = 0x4;
 
  // page-table with mock entry
  pte_t pte = { 0x7, 1 };
  pte_t linear_page_table[] = { pte };

  // extract the VPN from the virtual address
  u_int8_t virtual_page_number = (virtual_address & VPN_MASK) >> OFFSET_SHIFT;
  fprintf(stdout, "virtual_page_number: %d\n", virtual_page_number);
  assert(virtual_page_number == 1);

  // Form the address of the page-table entry (PTE).
  // If we weren't indexing we would use page_table_base_register + (vpn * sizeof(pte)) 
  // to get the address of the page-table entry.
  u_int8_t pte_address = page_table_base_register + virtual_page_number;
  fprintf(stdout, "pte_address: %d\n", pte_address);
  assert(pte_address == 1);

  // fetch the page-table entry
  pte_t page_table_entry = linear_page_table[pte_address - 1]; 
  fprintf(stdout, "page_table_entry.addr: %d\n", page_table_entry.addr);
  fprintf(stdout, "page_table_entry.valid: %d\n", page_table_entry.valid);
  assert(page_table_entry.addr == 0x7);
  assert(page_table_entry.valid == 0x1);

  // check the process can access the page etc. 
  if (!page_table_entry.valid) {
    exit(1);
  }

  // get the offset value to concatenate with the PFN
  u_int8_t offset = virtual_address & OFFSET_MASK;
  fprintf(stdout, "offset: %d\n", offset);
  assert(offset == 5);

  // left shift the PFN by the offset length and OR the offset to 
  // form the physical address
  u_int8_t physical_address = (pte.addr << PFN_SHIFT) | offset;
  fprintf(stdout, "physical_address: %d\n", physical_address);
  assert(physical_address == 117);

}
