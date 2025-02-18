/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <bootblock_common.h>
#include <superio/ite/common/ite.h>
#include <superio/ite/it8625e/it8625e.h>

#define UART_DEV PNP_DEV(0x2e, IT8625E_SP1)

void bootblock_mainboard_early_init(void)
{
	ite_enable_serial(UART_DEV, CONFIG_TTYS0_BASE);
}
