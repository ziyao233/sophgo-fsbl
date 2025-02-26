#include <mmio.h>
#include <debug.h>
#include <assert.h>
#include <bl_common.h>
#include <platform.h>
#include <cpu.h>
#include "csr.h"

#include <arch_helpers.h>

void sync_cache(void)
{
	/* icache.iall */
	asm (".long 0x100000b");
	/* sync.i */
	asm (".long 0x1a0000b");
}

void cpu_report_exception(unsigned int exception_type)
{
}

void enable_cache(struct cache_map *map)
{
}
