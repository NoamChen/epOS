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

#include <apic.h>
#include <printk.h>
#include <kernel/bits.h>

// Access registers offsets

#define IOAPIC_IOREGSEL	0x00
#define IOAPIC_IOWIN		0x10

#define REG_IOAPICID	0x00
#define REG_IOAPICVER	0x01
#define REG_IOAPICARB	0x02
#define REG_IOREDTBL	0x10

#define IOAPIC_VERSION 0x11

typedef union
{
	struct
	{
		uint32_t INTVEC : 8,
		         DELMOD		: 3,
		         DESTMOD		: 1,
		         DELIVS		: 1,
		         INTPOL		: 1,
		         IRR			: 1,
		         TRIGGER_MODE: 1,
		         IRQ_MASK	: 1,
		         reserved_1	: 15;
		uint32_t reserved_2	: 24,
		         DESTFIELD	: 8;
	}  __attribute__((packed));

	struct
	{
		uint32_t hi;
		uint32_t low;
	};
} ioapic_redirect_t;

void ioapic_dump(ioapic_t *ioapic);
static uint32_t ioapic_readreg(ioapic_t *ioapic, addr_t reg)
{
	uint32_t *rsel = ioapic->v_addr + IOAPIC_IOREGSEL;
	uint32_t *rread = ioapic->v_addr + IOAPIC_IOWIN;
	*rsel = reg;
	return *rread;
}

static void ioapic_writereg(ioapic_t *ioapic, addr_t reg, uint32_t val)
{
	uint32_t *rsel = ioapic->v_addr + IOAPIC_IOREGSEL;
	uint32_t *rwrite = ioapic->v_addr + IOAPIC_IOWIN;
	*rsel = reg;
	*rwrite = val;
}

static void ioapic_read_redirect(ioapic_t *ioapic, ioapic_redirect_t *entry, uint8_t irq)
{

	entry->hi = ioapic_readreg(ioapic, REG_IOREDTBL + (irq * 2));
	entry->low = ioapic_readreg(ioapic, REG_IOREDTBL + (irq * 2) + 1);
}

static void ioapic_write_redirect(ioapic_t *ioapic, ioapic_redirect_t *entry, uint8_t irq)
{

	ioapic_writereg(ioapic, REG_IOREDTBL + (irq * 2), entry->hi);
	ioapic_writereg(ioapic, REG_IOREDTBL + (irq * 2) + 1, entry->low);
}

static void _ioapic_irq_mask_set(ioapic_t *ioapic, uint8_t irq, bool mask)
{
	ioapic_redirect_t entry;

	if (irq >= ioapic->max_redirect)
	{
		pr_error("IOAPIC can't handle this irq, %u\r\n", irq);
		return;
	}

	ioapic_read_redirect(ioapic, &entry, irq);
	entry.IRQ_MASK = mask;
	ioapic_write_redirect(ioapic, &entry, irq);

}

void ioapic_irq_mask(ioapic_t *ioapic, uint8_t irq)
{
	_ioapic_irq_mask_set(ioapic, irq, true);
}
void ioapic_irq_unmask(ioapic_t *ioapic, uint8_t irq)
{
	_ioapic_irq_mask_set(ioapic, irq, false);
}

void ioapic_santize(ioapic_t *ioapic)
{
	uint32_t id = ioapic_readreg(ioapic, REG_IOAPICID);
	uint32_t ver = ioapic_readreg(ioapic, REG_IOAPICVER);

	ioapic->max_redirect = BF_GET(ver, 16, 8);

	if (BF_GET(ver, 0, 7) != IOAPIC_VERSION)
	{
		pr_fatal("IOAPIC version doesn't match.");
		panic();
	}

	pr_info("IOAPIC ID = 0x%x, Version = 0x%x Max redirect = 0x%x\r\n",
	        BF_GET(id, 24, 4), BF_GET(ver, 0, 7), ioapic->max_redirect);

#ifdef DEBUG
	ioapic_dump(ioapic);
#endif
}

void ioapic_dump(ioapic_t *ioapic)
{
	ioapic_redirect_t entry;

	for (int i = 0; i < ioapic->max_redirect; i++)
	{
		ioapic_read_redirect(ioapic, &entry, i);
		printk("IOAPIC Entry %u intvec: %x delmod: %x destmod: %u delivs: %u intpol: %u irr: %u trigger_mode: %u irq_mask: %u destfield: %x\r\n",
		       i, entry.INTVEC, entry.DELMOD, entry.DESTMOD, entry.DELIVS, entry.INTPOL, entry.IRR, entry.TRIGGER_MODE, entry.IRQ_MASK, entry.DESTFIELD);
	}

}
void ioapic_apply_redirect(ioapic_t *ioapic)
{
}
