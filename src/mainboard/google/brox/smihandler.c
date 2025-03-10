/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <acpi/acpi.h>
#include <cpu/x86/smm.h>
#include <ec/google/chromeec/ec.h>
#include <ec/google/chromeec/smm.h>
#include <elog.h>
#include <baseboard/variants.h>
#include <intelblocks/smihandler.h>
#include <intelblocks/xhci.h>
#include <variant/ec.h>

void __weak variant_smi_sleep(u8 slp_typ) {}

void mainboard_smi_sleep(u8 slp_typ)
{
	variant_smi_sleep(slp_typ);
	chromeec_smi_sleep(slp_typ, MAINBOARD_EC_S3_WAKE_EVENTS, MAINBOARD_EC_S5_WAKE_EVENTS);
	/*
	 * Workaround: Reset the XHCI controller prior to S5 to avoid
	 * PMC timeout error during poweron from S5.
	 */
	if (slp_typ == ACPI_S5)
		xhci_host_reset();
}

int mainboard_smi_apmc(u8 apmc)
{
	chromeec_smi_apmc(apmc, MAINBOARD_EC_SCI_EVENTS, MAINBOARD_EC_SMI_EVENTS);
	return 0;
}

void elog_gsmi_cb_mainboard_log_wake_source(void)
{
	google_chromeec_log_events(MAINBOARD_EC_LOG_EVENTS | MAINBOARD_EC_S0IX_WAKE_EVENTS);
}

void mainboard_smi_espi_handler(void)
{
	chromeec_smi_process_events();
}
