/* SPDX-License-Identifier: GPL-2.0-only */

#include <arch/stages.h>
#include <soc/clkbuf_ctl.h>
#include <soc/dvfs.h>
#include <soc/dvfsrc.h>
#include <soc/emi.h>
#include <soc/irq2axi.h>
#include <soc/modem_power_ctrl.h>
#include <soc/mt6316.h>
#include <soc/mt6363.h>
#include <soc/mt6373.h>
#include <soc/mt6685.h>
#include <soc/mt6685_rtc.h>
#include <soc/mtk_fsp.h>
#include <soc/pcie.h>
#include <soc/pll.h>
#include <soc/pmif.h>
#include <soc/pwrsel.h>
#include <soc/srclken_rc.h>
#include <soc/thermal.h>

static void raise_little_cpu_freq(void)
{
	mt6316_buck_set_voltage(SPMI_SLAVE_8, MT6316_BUCK_3, 1050000);
	printk(BIOS_INFO, "%s: little core voltage: %d\n",
	       __func__, mt6316_buck_get_voltage(SPMI_SLAVE_8, MT6316_BUCK_3));
	mt_pll_raise_little_cpu_freq(2400U * MHz);
}

void platform_romstage_main(void)
{
	irq2axi_disable();
	pwrsel_init();
	mtk_pmif_init();
	mt6363_init();
	mt6363_enable_vtref18(true);
	mt6363_set_vtref18_voltage(1800000);
	mt6316_init();
	raise_little_cpu_freq();
	mt6373_init();
	mtk_fsp_init(ROMSTAGE_INIT);
	mtk_fsp_load_and_run();
	mt6685_init();
	dvfsrc_opp_level_mapping();
	srclken_rc_init();
	clk_buf_init();
	if (CONFIG(RTC))
		rtc_boot();
	mtk_dram_init();
	modem_power_down();
	dvfs_init();
	thermal_init();

	if (CONFIG(PCI))
		mtk_pcie_deassert_perst();
}
