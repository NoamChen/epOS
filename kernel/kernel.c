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

#include <types.h>
#include <boot/multiboot.h>
#include <video/VIDEO_textmode.h>
#include <isr.h>
#include <printk.h>
#include <mem/memory.h>
#include <cpu.h>
#include <serial.h>
#include <lib/list.h>
#include <apic.h>
#include <acpica/acpi.h>
#include <kbd.h>
#include <mmodules.h>

void kmain(void)
{
	extern uint32_t magic;
	extern void *mbd;
	multiboot_info_t *mbi = mbd;


	init_serial();
	printk("EP-OS by Ramon Fried, all rights reservered.\r\n");

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		/* Something went not according to specs. Print an error */
		/* message and halt, but do *not* rely on the multiboot */
		/* data structure. */
		pr_fatal("Multiboot integrity check failed\r\n");
		panic();
	}

	printk("EP-OS by Ramon Fried, all rights reservered.\r\n");
	cpu_init();
	mem_init(mbi);

//	ksymbol_init(mbi);

	VIDEO_init();
	VIDEO_clear_screen();

	acpi_early_init();
	enable_irq();

	ticks_init();

	if (kbd_8042_avail())
	{
		pr_info("8042 keyboard detected\r\n");
		kbd_8042_init();
	}

//	mmodules_parse(mbi);

	for (int i = 0 ; i < 1000; i++)
	{
		printk("Trying to allocate %u pages\r\n", i);
		mem_page_map_kernel(0x1000, i, 0);
	}

	printk("Bla Bla\r\n");

	while (1);
}
