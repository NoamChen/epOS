/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org>
*/

#include <mem/mem_pages.h>
#include <printk.h>
#include <OS_types.h>

#define PAGE_SIZE 4096

// These are defined in the linker script, with these we calculate the size of the kernel on runtime.
extern unsigned long kernel_start;
extern unsigned long kernel_end;
static uint32_t total_memory = 0;
static uintptr_t physical_start = 0;
static uintptr_t pages_start;

static uint32_t total_pages;
static uint32_t *page_bitmap = NULL;


#define KERNEL_VIRTUAL_BASE 0xC0000000
#define BIT_CHECK(a,b) ((a) & (1<<(b)))
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define IS_PAGE_ALIGNED(POINTER) \
	    (((uintptr_t)(const void *)(POINTER)) % (PAGE_SIZE) == 0)

#define PAGE_DIRECTORY_SIZE 12
#define PAGE_TABLE_SIZE 12


typedef struct page
{
   uint32_t present    : 1;   // Page present in memory
   uint32_t rw         : 1;   // Read-only if clear, readwrite if set
   uint32_t user       : 1;   // Supervisor level only if clear
   uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
   uint32_t dirty      : 1;   // Has the page been written to since last refresh?
   uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
   uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table
{
   page_t pages[PAGE_TABLE_SIZE];
} page_table_t;

typedef struct page_directory {
   /**
      Array of pointers to pagetables.
   **/
   uint32_t tables[1024];

} page_directory_t;


static page_directory_t __attribute__((aligned(0x1000))) kernel_pgd;
static page_table_t __attribute__((aligned(0x1000))) kernel_pgt[1024];
void mem_switch_page_directory(page_directory_t * dir)
{
	void * m = 0;
//	asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
	asm volatile ("invlpg (%0)" : : "b"(m) : "memory") ;
	uint32_t cr0;
//	asm volatile("mov %%cr0, %0": "=r"(cr0));
//	cr0 |= 0x80000000; // Enable paging!
//	asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void mem_init(multiboot_info_t *mbi)
{

	uint32_t reserved_pages;
	uint32_t kernel_size;
	uint32_t required_kernel_pages;

	printk("Memory init:\r\ndetecting physical memory.\r\n");


	for (int i=0; i < PAGE_DIRECTORY_SIZE; i++)
	{
		kernel_pgd.tables[i] = (uint32_t) &kernel_pgt[i] - KERNEL_VIRTUAL_BASE;
		printk("pagetable: %u physical: %x\r\n", i, (uint32_t)  kernel_pgd.tables[i]);
	}

	while(1);
	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)(mbi->mmap_addr + KERNEL_VIRTUAL_BASE);

	while (mmap < mbi->mmap_addr + KERNEL_VIRTUAL_BASE +  mbi->mmap_length)
	{
		mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(unsigned int));

		if (mmap->size == 0)
		{
			// That's the end of the list
			break;
		}

		printk("Memory region size: %u address: 0x%llx length: 0x%llx type: %u\r\n",
		       mmap->size, mmap->addr, mmap->len, mmap->type);

		// We only support one zone, we'll take the biggest.
		//
		if (mmap->type == 1)
		{
			if (mmap->len > total_memory)
			{
				total_memory = mmap->len;
				physical_start = mmap->addr;
			}
		}

	}

	if (total_memory == 0)
	{
		printk("No physical memory!\r\n");
		panic();
	}

	kernel_size = ((uint32_t) &kernel_end - (uint32_t) &kernel_start);
	required_kernel_pages = (kernel_size / PAGE_SIZE) + 1;

	printk("Kernel start: 0x%x, kernel end: 0x%x\r\n", &kernel_start, &kernel_end);
	printk("Kernel occupies 0x%x bytes, consuming %u pages\r\n", kernel_size, required_kernel_pages);

	physical_start += required_kernel_pages * PAGE_SIZE;
	total_memory -= required_kernel_pages * PAGE_SIZE;
	total_pages = total_memory / PAGE_SIZE;

	printk("Physical memory zone set to: 0x%x size: 0x%x\r\n", physical_start, total_memory);
	printk("Number of pages: %u\r\n", total_pages);

#if 0
	// Now we calculate how many pages we need to reserve to ourselves for physical allocator management.
	reserved_pages = (total_pages / 32 * sizeof(uint32_t)) / PAGE_SIZE;

	if ((total_pages / 32 * sizeof(uint32_t)) % PAGE_SIZE)
	{
		reserved_pages++;
	}

	printk("reserved_pages = %u\r\n", reserved_pages);

	page_bitmap = physical_start;
	pages_start = page_bitmap + (reserved_pages * PAGE_SIZE);

#endif

//	for (int i =0; i < required_kernel_pages;i++)
//		map_page(
//	 Map the kernel back again
	// Clear the memory bitmap
//	memset(page_bitmap, 0, reserved_pages * PAGE_SIZE);

}

void mem_page_free(addr_t page)
{
	int entry;
	int bit;

	// Sanity, check that the address given is page aligned.
	if (! IS_PAGE_ALIGNED(page))
	{
		printk("recieved unaligned page to free, that's a problem :(\r\n");
		panic();
	}

	page -= physical_start;
	entry = page / PAGE_SIZE;
	bit = page % PAGE_SIZE;

	// Sanity, check that the address given was actually alocated.
	if (! BIT_CHECK(page_bitmap[entry], bit))
	{
		printk("Trying to free an already free page, that's a problem :(\r\n");
		panic();
	}

	BIT_CLEAR(page_bitmap[entry], bit);


}

addr_t mem_page_get()
{
	uint32_t i = 0;
	uint32_t j = 0;
	bool found = false;

	for (i = 0; i < total_pages; i++)
	{
		if (page_bitmap[i])
		{
			// There's at least one free page in this int bitmap, find it and mark it.
			//
			//
			for (j = 0; j < 32; j++)
			{
				if (! BIT_CHECK(page_bitmap[i], j))
				{
					BIT_SET(page_bitmap[i], j);
					found = true;
					break;
				}
			}

			printk("ERROR:  internal error\r\n");
			panic();
		}

	}

	if (found)
	{
		return (addr_t)((i * 32 + j) * PAGE_SIZE) + physical_start;
	}
	else
	{
		return NULL;
	}
}
