/* SPDX-License-Identifier: GPL-2.0-only OR MIT */

#include <device/mmio.h>
#include <device/resource.h>
#include <gpio.h>
#include <soc/addressmap.h>
#include <soc/early_init.h>
#include <soc/pcie.h>
#include <soc/pcie_common.h>

#define PCIE_REG_BASE_PORT0	0x112f0000

static const struct pad_func pcie_pins[2][3] = {
	{
		PAD_FUNC_UP(PCIE_WAKE_N, WAKEN),
		PAD_FUNC_UP(PCIE_PERESET_N, PERSTN),
		PAD_FUNC_UP(PCIE_CLKREQ_N, CLKREQN),
	},
	{
		PAD_FUNC_UP(CMMCLK0, PERSTN_1),
		PAD_FUNC_UP(CMMCLK1, CLKREQN_1),
		PAD_FUNC_UP(CMMCLK2, WAKEN_1),
	},
};

static void mtk_pcie_set_pinmux(uint8_t port)
{
	const struct pad_func *pins = pcie_pins[port];
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pcie_pins[port]); i++) {
		gpio_set_mode(pins[i].gpio, pins[i].func);
		gpio_set_pull(pins[i].gpio, GPIO_PULL_ENABLE, pins[i].select);
	}
}

void mtk_pcie_pre_init(void)
{
	mtk_pcie_set_pinmux(0);

	/* Assert all reset signals at early stage */
	mtk_pcie_reset(PCIE_REG_BASE_PORT0, true);

	early_init_save_time(EARLY_INIT_PCIE);
}
