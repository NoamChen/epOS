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


/*
 * Memory map:
 *
 * 0x00000000-0xC0000000 User space usage
 * 0xC0000000-0xD0000000 Kernel (256 MB to grow to....)
 * 0xD0000000-0xE0000000 Kernel heap
 * 0xE0000000........... Unused (for future use)
 *
 */

#include <kernel.h>
#include <types.h>
#include <mem/memory.h>
#include <printk.h>
#include <lib/kmalloc.h>
#include <lib/string.h>

void test_heap();

void mem_heap_init(heap_t *heap, addr_t vir_start, size_t size)
{

	FUNC_ENTER();

	heap->total_pages =  size / PAGE_SIZE;
	heap->location = vir_start;
	heap->initalized = true;


//	test_heap();
	FUNC_LEAVE();
}

int mem_heap_free(heap_t *heap, void *addr , int count)
{
	FUNC_ENTER();


	assert(heap->initalized);
	//todo: implement
	addr = addr;
	count = count;
	FUNC_LEAVE();
	return 0;
}

int mem_heap_lock(heap_t *heap)
{
	//todo: implement when necessary
	return 0;
}

int mem_heap_unlock(heap_t *heap)
{
	//todo: implement when necessary
	return 0;
}

void *mem_heap_alloc(heap_t *heap, size_t count)
{
	addr_t ret_address;
	addr_t physical;
	FUNC_ENTER();

	assert(heap->initalized);

	if (heap->total_pages - heap->used_pages < count)
	{
		printk("not enough physical pages\r\n");
		panic();
	}

	physical = mem_get_pages(count);

	if (!physical)
	{
		goto error;
	}

	for (size_t i = 0; i < count; i++)
	{
		mem_page_map(physical + i * (PAGE_SIZE), heap->location + i * (PAGE_SIZE), 0);
	}

	ret_address = heap->location;
	heap->location += count * PAGE_SIZE;
	heap->used_pages += count;

	FUNC_LEAVE();
	return (void *) ret_address;

error:
	printk("Can't allocate HEAP\r\n");
	panic();

	return NULL;
}



void test_heap()
{

	char *a , *b, *c, *d, *e;

	a = kmalloc(10);
	b = kmalloc(20);
	c = kmalloc(30);
	kfree(c);
	d = kmalloc(15);
	e = kmalloc(12);

	memset(a, 1, 10);
	memset(b, 0, 20);
	memset(d, 1, 15);
	memset(e, 1, 12);

	kfree(a);
	kfree(b);
	kfree(d);
	kfree(e);

}
