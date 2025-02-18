/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpi_device.h>
#include <acpi/acpigen.h>
#include <acpi/acpigen_pci.h>
#include <console/console.h>
#include <device/pci_ids.h>
#include <mtcl.h>
#include <sar.h>
#include <stdio.h>
#include <stdlib.h>
#include <wrdd.h>

#include "chip.h"
#include "wifi.h"
#include "wifi_private.h"

/* Domain type */
enum sar_domain {
	DOMAIN_TYPE_WIFI = 0x7,
	DOMAIN_TYPE_BLUETOOTH = 0x12
};

/* Maximum number DSM UUID bifurcations in _DSM */
#define MAX_DSM_FUNCS 2

/*
 * WIFI ACPI NAME = "WF" + hex value of last 8 bits of dev_path_encode + '\0'
 * The above representation returns unique and consistent name every time
 * generate_wifi_acpi_name is invoked. The last 8 bits of dev_path_encode is
 * chosen since it contains the bus address of the device.
 */
#define WIFI_ACPI_NAME_MAX_LEN 5

/* Unique ID for the WIFI _DSM */
#define ACPI_DSM_OEM_WIFI_UUID    "F21202BF-8F78-4DC6-A5B3-1F738E285ADE"

/* Unique ID for CnviDdrRfim entry in WIFI _DSM */
#define ACPI_DSM_RFIM_WIFI_UUID   "7266172C-220B-4B29-814F-75E4DD26B5FD"

__weak int get_wifi_sar_limits(union wifi_sar_limits *sar_limits)
{
	return -1;
}

/*
 * Function 1: Allow PC OEMs to set ETSI 5.8GHz SRD in Passive/Disabled ESTI SRD
 * Channels: 149, 153, 157, 161, 165
 * 0 - ETSI 5.8GHz SRD active scan
 * 1 - ETSI 5.8GHz SRD passive scan
 * 2 - ETSI 5.8GHz SRD disabled
 */
static void wifi_dsm_srd_active_channels(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->disable_active_sdr_channels);
}

/*
 * Function 2 : Supported Indonesia 5.15-5.35 GHz Band
 * 0 - Set 5.115-5.35GHz to Disable in Indonesia
 * 1 - Set 5.115-5.35GHz to Enable (Passive) in Indonesia
 * 2 - Reserved
 */
static void wifi_dsm_indonasia_5Ghz_band_enable(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->support_indonesia_5g_band);
}

/*
 * Function 3: Support Wi-Fi 6 11ax Rev 2 new channels on 6-7 GHz.
 * Bit 0:
 * 0 - No override; use device settings 0
 * 1 - Force disable all countries that are not defined in the following bits
 *
 * Bit 1:
 * 0 No override; USA 6GHz disable 0
 * 1 6GHz allowed in the USA (enabled only if the device is certified to the USA)
 */
static void wifi_dsm_supported_ultra_high_band(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->support_ultra_high_band);
}

/*
 * Function 4: Regulatory Special Configurations Enablements
 */
static void wifi_dsm_regulatory_configurations(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->regulatory_configurations);
}

/*
 * Function 5: M.2 UART Interface Configuration
 */
static void wifi_dsm_uart_configurations(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->uart_configurations);
}

/*
 * Function 6: Control Enablement 11ax on certificated modules
 * Bit 0 - Apply changes to country Ukraine. 11Ax Setting within module certification
 * 0 - None. Work with Wi-Fi FW/OTP definitions [Default]
 * 1 - Apply changes.
 *
 * Bit 1 - 11Ax Mode. Effective only if Bit 0 set to 1
 * 0 - Disable 11Ax on country Ukraine [Default]
 * 1 - Enable 11Ax on country Ukraine
 *
 * Bit 2 - Apply changes to country Russia. 11Ax Setting within module certification
 * 0 - None. Work with Wi-Fi FW/OTP definitions [Default]
 * 1 - Apply changes.
 *
 * Bit 3 - 11Ax Mode. Effective only if Bit 2 set to 1
 * 0 - Disable 11Ax on country Russia [Default]
 * 1 - Enable 11Ax on country Russia
 *
 * Bit 31:04 - Reserved
 *
 * Note: Assumed Russia Work with Wi-Fi FW/OTP definitions
 */
static void wifi_dsm_ukrane_russia_11ax_enable(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->enablement_11ax);
}

/*
 * Function 7: Control Enablement UNII-4 over certificate modules
 */
static void wifi_dsm_unii4_control_enable(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->unii_4);
}

/*
 * Function 10: Energy Detection Threshold (EDT)
 * Bits 0-3: EDT revision - Default 0
 *
 * Bits 4-5: Reserved - Should be 0
 *
 * Bit 6: HB EDT Level. 5 GHz ETSI - EDT Level change - Default 0
 * 0 - Disable EDT optimization for ETSI HB
 * 1 - Enable EDT optimization for ETSI HB
 *
 * Bits 7-8: Reserved - Should be 0
 *
 * Bit 9: UHB EDT Level. 6 GHz FCC - EDT Level change - Default 0
 * 0 - Disable EDT optimization for FCC UHB
 * 1 - Enable EDT optimization for FCC UHB
 *
 * Bit 10-12: Reserved - Default 0
 *
 * Bit 13: EDT_En_HB_5G2/3 - Default 0
 * 0 - Disable EDT optimization for HB_5G2/3
 * 1 - Enable EDT optimization for HB_5G2/3
 *
 * Bit 14: EDT_En_HB_5G4 - Default 0
 * 0 - Disable EDT optimization for HB_5G4
 * 1 - Enable EDT optimization for HB_5G4
 *
 * Bit 15: EDT_En_HB_5G6 - Default 0
 * 0 - Disable EDT optimization for HB_5G6
 * 1 - Enable EDT optimization for HB_5G6
 *
 * Bit 16: EDT_En_HB_5G8/9 - Default 0
 * 0 - Disable EDT optimization for HB_5G8/9
 * 1 - Enable EDT optimization for HB_5G8/9
 *
 * Bit 17: EDT_En_UHB_6G1 - Default 0
 * 0 - Disable EDT optimization for UHB_6G1
 * 1 - Enable EDT optimization for UHB_6G1
 *
 * Bit 18: EDT_En_UHB_6G3 - Default 0
 * 0 - Disable EDT optimization for UHB_6G3
 * 1 - Enable EDT optimization for UHB_6G3
 *
 * Bit 19: EDT_En_UHB_6G5 - Default 0
 * 0 - Disable EDT optimization for UHB_6G5
 * 1 - Enable EDT optimization for UHB_6G5
 *
 * Bit 20: EDT_En_UHB_6G6 - Default 0
 * 0 - Disable EDT optimization for UHB_6G6
 * 1 - Enable EDT optimization for UHB_6G6
 *
 * Bit 21: EDT_En_UHB_6G8 - Default 0
 * 0 - Disable EDT optimization for UHB_6G8
 * 1 - Enable EDT optimization for UHB_6G8
 *
 * Bit 22: EDT_En_UHB_7G0 - Default 0
 * 0 - Disable EDT optimization for UHB_7G0
 * 1 - Enable EDT optimization for UHB_7G0
 *
 * Bits 23-31: Reserved - Should be 0
 */
static void wifi_dsm_energy_detection_threshold(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->energy_detection_threshold);
}

/*
 * Function 11: RFI mitigation
 * Bit 0:
 * 0 - DLVR RFIm enabled (default)
 * 1 - DLVR RFIm disabled
 *
 * Bit 1:
 * 0 - DDR RFIm enabled (default)
 * 1 - DDR RFIm disabled
 *
 * Bits 2-31: Reserved - Should be 0
 */

static void wifi_dsm_rfi_mitigation(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->rfi_mitigation);
}

/*
 * Function 12: Control Enablement 802.11be on certificated modules
 * Bit 0
 * 0 - 11BE disabled for China Mainland
 * 1 - 11BE enabled for China Mainland
 *
 * Bit 1
 * 0 - 11BE disabled for South Korea
 * 1 - 11BE enabled for South Korea
 *
 * Bit 2:27 - Reserved (shall be set to zeroes)
 *
 * Bit 28:31 - 11BE enablement revision
 *
 */
static void wifi_dsm_11be_country_enablement(void *args)
{
	struct dsm_profile *dsm_config = (struct dsm_profile *)args;

	acpigen_write_return_integer(dsm_config->enablement_11be);
}

static void wifi_dsm_ddrrfim_func3_cb(void *ptr)
{
	const bool is_cnvi_ddr_rfim_enabled = *(bool *)ptr;
	acpigen_write_return_integer(is_cnvi_ddr_rfim_enabled ? 0 : 1);
}

static void (*wifi_dsm_callbacks[])(void *) = {
	NULL,					/* Function 0 */
	wifi_dsm_srd_active_channels,		/* Function 1 */
	wifi_dsm_indonasia_5Ghz_band_enable,	/* Function 2 */
	wifi_dsm_supported_ultra_high_band,	/* Function 3 */
	wifi_dsm_regulatory_configurations,	/* Function 4 */
	wifi_dsm_uart_configurations,		/* Function 5 */
	wifi_dsm_ukrane_russia_11ax_enable,	/* Function 6 */
	wifi_dsm_unii4_control_enable,		/* Function 7 */
	NULL,					/* Function 8 */
	NULL,					/* Function 9 */
	wifi_dsm_energy_detection_threshold,	/* Function 10 */
	wifi_dsm_rfi_mitigation,		/* Function 11 */
	wifi_dsm_11be_country_enablement,	/* Function 12 */
};

/*
 * The current DSM2 table is only exporting one function (function 3), some more
 * functions are reserved so marking them NULL.
*/
static void (*wifi_dsm2_callbacks[])(void *) = {
	NULL,				/* Function 0 */
	NULL,				/* Function 1 */
	NULL,				/* Function 2 */
	wifi_dsm_ddrrfim_func3_cb,	/* Function 3 */
};

static const uint8_t *sar_fetch_set(const struct sar_profile *sar, size_t set_num)
{
	const uint8_t *sar_table = &sar->sar_table[0];

	return sar_table + (sar->chains_count * sar->subbands_count * set_num);
}

static const uint8_t *wgds_fetch_set(struct geo_profile *wgds, size_t set_num)
{
	const uint8_t *wgds_table = &wgds->wgds_table[0];

	return wgds_table + (wgds->bands_count * set_num);
}

static const uint8_t *ppag_fetch_set(struct gain_profile *ppag, size_t set_num)
{
	const uint8_t *ppag_table = &ppag->ppag_table[0];

	return ppag_table + (ppag->bands_count * set_num);
}

static void sar_emit_wrds(const struct sar_profile *sar)
{
	int i;
	size_t package_size, table_size;
	const uint8_t *set;

	if (sar == NULL)
		return;

	/*
	 * Name ("WRDS", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,	// 0x7:WiFi
	 *     WiFi SAR BIOS,	// BIOS SAR Enable/disable
	 *     SAR Table Set	// Set#1 of SAR Table
	 *   }
	 * })
	 */
	if (sar->revision > MAX_SAR_REVISION) {
		printk(BIOS_ERR, "Invalid SAR table revision: %d\n", sar->revision);
		return;
	}

	acpigen_write_name("WRDS");
	acpigen_write_package(2);
	acpigen_write_dword(sar->revision);

	table_size = sar->chains_count * sar->subbands_count;
	/* Emit 'Domain Type' + 'WiFi SAR Enable' + Set#1 */
	package_size = 1 + 1 + table_size;
	acpigen_write_package(package_size);
	acpigen_write_dword(DOMAIN_TYPE_WIFI);
	acpigen_write_dword(1);

	set = sar_fetch_set(sar, 0);
	for (i = 0; i < table_size; i++)
		acpigen_write_byte(set[i]);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_ewrd(const struct sar_profile *sar)
{
	int i;
	size_t package_size, set_num, table_size;
	const uint8_t *set;

	if (sar == NULL)
		return;

	/*
	 * Name ("EWRD", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,		// 0x7:WiFi
	 *     Dynamic SAR Enable,	// Dynamic SAR Enable/disable
	 *     Extended SAR sets,	// Number of optional SAR table sets
	 *     SAR Table Set,		// Set#2 of SAR Table
	 *     SAR Table Set,		// Set#3 of SAR Table
	 *     SAR Table Set		// Set#4 of SAR Table
	 *   }
	 * })
	 */
	if (sar->revision > MAX_SAR_REVISION) {
		printk(BIOS_ERR, "Invalid SAR table revision: %d\n", sar->revision);
		return;
	}

	if (sar->dsar_set_count == 0) {
		printk(BIOS_WARNING, "DSAR set count is 0\n");
		return;
	}

	acpigen_write_name("EWRD");
	acpigen_write_package(2);
	acpigen_write_dword(sar->revision);

	table_size = sar->chains_count * sar->subbands_count;
	/*
	 * Emit 'Domain Type' + 'Dynamic SAR Enable' + 'Extended SAR sets count'
	 * + number of bytes for Set#2 & 3 & 4
	 */
	package_size = 1 + 1 + 1 + table_size * MAX_DSAR_SET_COUNT;
	acpigen_write_package(package_size);
	acpigen_write_dword(DOMAIN_TYPE_WIFI);
	acpigen_write_dword(1);
	acpigen_write_dword(sar->dsar_set_count);

	for (set_num = 1; set_num <= sar->dsar_set_count; set_num++) {
		set = sar_fetch_set(sar, set_num);
		for (i = 0; i < table_size; i++)
			acpigen_write_byte(set[i]);
	}

	/* wifi driver always expects 3 DSAR sets */
	for (i = 0; i < (table_size * (MAX_DSAR_SET_COUNT - sar->dsar_set_count)); i++)
		acpigen_write_byte(0);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_wgds(struct geo_profile *wgds)
{
	int i;
	size_t package_size, set_num;
	const uint8_t *set;

	if (wgds == NULL)
		return;

	/*
	 * Name ("WGDS", Package() {
	 *  Revision,
	 *  Package() {
	 *   DomainType,                         // 0x7:WiFi
	 *   WgdsWiFiSarDeltaGroup1PowerMax1,    // Group 1 FCC 2400 Max
	 *   WgdsWiFiSarDeltaGroup1PowerChainA1, // Group 1 FCC 2400 A Offset
	 *   WgdsWiFiSarDeltaGroup1PowerChainB1, // Group 1 FCC 2400 B Offset
	 *   WgdsWiFiSarDeltaGroup1PowerMax2,    // Group 1 FCC 5200 Max
	 *   WgdsWiFiSarDeltaGroup1PowerChainA2, // Group 1 FCC 5200 A Offset
	 *   WgdsWiFiSarDeltaGroup1PowerChainB2, // Group 1 FCC 5200 B Offset
	 *   WgdsWiFiSarDeltaGroup1PowerMax3,    // Group 1 FCC 6000-7000 Max
	 *   WgdsWiFiSarDeltaGroup1PowerChainA3, // Group 1 FCC 6000-7000 A Offset
	 *   WgdsWiFiSarDeltaGroup1PowerChainB3, // Group 1 FCC 6000-7000 B Offset
	 *   WgdsWiFiSarDeltaGroup2PowerMax1,    // Group 2 EC Jap 2400 Max
	 *   WgdsWiFiSarDeltaGroup2PowerChainA1, // Group 2 EC Jap 2400 A Offset
	 *   WgdsWiFiSarDeltaGroup2PowerChainB1, // Group 2 EC Jap 2400 B Offset
	 *   WgdsWiFiSarDeltaGroup2PowerMax2,    // Group 2 EC Jap 5200 Max
	 *   WgdsWiFiSarDeltaGroup2PowerChainA2, // Group 2 EC Jap 5200 A Offset
	 *   WgdsWiFiSarDeltaGroup2PowerChainB2, // Group 2 EC Jap 5200 B Offset
	 *   WgdsWiFiSarDeltaGroup2PowerMax3,    // Group 2 EC Jap 6000-7000 Max
	 *   WgdsWiFiSarDeltaGroup2PowerChainA3, // Group 2 EC Jap 6000-7000 A Offset
	 *   WgdsWiFiSarDeltaGroup2PowerChainB3, // Group 2 EC Jap 6000-7000 B Offset
	 *   WgdsWiFiSarDeltaGroup3PowerMax1,    // Group 3 ROW 2400 Max
	 *   WgdsWiFiSarDeltaGroup3PowerChainA1, // Group 3 ROW 2400 A Offset
	 *   WgdsWiFiSarDeltaGroup3PowerChainB1, // Group 3 ROW 2400 B Offset
	 *   WgdsWiFiSarDeltaGroup3PowerMax2,    // Group 3 ROW 5200 Max
	 *   WgdsWiFiSarDeltaGroup3PowerChainA2, // Group 3 ROW 5200 A Offset
	 *   WgdsWiFiSarDeltaGroup3PowerChainB2, // Group 3 ROW 5200 B Offset
	 *   WgdsWiFiSarDeltaGroup3PowerMax3,    // Group 3 ROW 6000-7000 Max
	 *   WgdsWiFiSarDeltaGroup3PowerChainA3, // Group 3 ROW 6000-7000 A Offset
	 *   WgdsWiFiSarDeltaGroup3PowerChainB3, // Group 3 ROW 6000-7000 B Offset
	 *  }
	 * })
	 */
	if (wgds->revision > MAX_GEO_OFFSET_REVISION) {
		printk(BIOS_ERR, "Invalid WGDS revision: %d\n", wgds->revision);
		return;
	}

	package_size = 1 + wgds->chains_count * wgds->bands_count;

	acpigen_write_name("WGDS");
	acpigen_write_package(2);
	acpigen_write_dword(wgds->revision);
	/* Emit 'Domain Type' +
	 * Group specific delta of power (6 bytes * NUM_WGDS_SAR_GROUPS)
	 */
	acpigen_write_package(package_size);
	acpigen_write_dword(DOMAIN_TYPE_WIFI);

	for (set_num = 0; set_num < wgds->chains_count; set_num++) {
		set = wgds_fetch_set(wgds, set_num);
		for (i = 0; i < wgds->bands_count; i++)
			acpigen_write_byte(set[i]);
	}

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_ppag(struct gain_profile *ppag)
{
	int i;
	size_t package_size, set_num;
	const uint8_t *set;

	if (ppag == NULL)
		return;

	/*
	 * Name ("PPAG", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,		// 0x7:WiFi
	 *     PPAG Mode,		// Defines the mode of ANT_gain control to be used
	 *     ANT_gain Table Chain A	// Defines the ANT_gain in dBi for chain A
	 *     ANT_gain Table Chain B	// Defines the ANT_gain in dBi for chain B
	 *   }
	 * })
	 */
	if (ppag->revision > MAX_ANT_GAINS_REVISION) {
		printk(BIOS_ERR, "Invalid PPAG revision: %d\n", ppag->revision);
		return;
	}

	package_size = 1 + 1 + ppag->chains_count * ppag->bands_count;

	acpigen_write_name("PPAG");
	acpigen_write_package(2);
	acpigen_write_dword(ppag->revision);
	acpigen_write_package(package_size);
	acpigen_write_dword(DOMAIN_TYPE_WIFI);
	acpigen_write_dword(ppag->mode);

	for (set_num = 0; set_num < ppag->chains_count; set_num++) {
		set = ppag_fetch_set(ppag, set_num);
		for (i = 0; i < ppag->bands_count; i++)
			acpigen_write_byte(set[i]);
	}

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_wtas(struct avg_profile *wtas)
{
	int i;
	size_t package_size;

	if (wtas == NULL)
		return;

	/*
	 * Name (WTAS, Package() {
	 * {
	 *   Revision,
	 *   Package()
	 *   {
	 *     DomainType,            // 0x7:WiFi
	 *     WifiTASSelection,      // Enable/Disable the TAS feature
	 *     WifiTASListEntries,    // No. of blocked countries not approved by OEM to
	 *     BlockedListEntry1,        support this feature
	 *     BlockedListEntry2,
	 *     BlockedListEntry3,
	 *     BlockedListEntry4,
	 *     BlockedListEntry5,
	 *     BlockedListEntry6,
	 *     BlockedListEntry7,
	 *     BlockedListEntry8,
	 *     BlockedListEntry9,
	 *     BlockedListEntry10,
	 *     BlockedListEntry11,
	 *     BlockedListEntry12,
	 *     BlockedListEntry13,
	 *     BlockedListEntry14,
	 *     BlockedListEntry15,
	 *     BlockedListEntry16,
	 *   }
	 * })
	 */
	package_size = 1 + 1 + 1 + MAX_DENYLIST_ENTRY;

	acpigen_write_name("WTAS");
	acpigen_write_package(2);
	acpigen_write_dword(wtas->revision);
	acpigen_write_package(package_size);
	acpigen_write_dword(DOMAIN_TYPE_WIFI);
	acpigen_write_byte(wtas->tas_selection);
	acpigen_write_byte(wtas->tas_list_size);
	for (i = 0; i < MAX_DENYLIST_ENTRY; i++)
		acpigen_write_word(wtas->deny_list_entry[i]);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_brds(const struct bsar_profile *bsar)
{
	size_t package_size, table_size;
	const uint8_t *set;

	if (bsar == NULL)
		return;

	/*
	 * Name ("BRDS", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,			// 0x12:Bluetooth
	 *     Bluetooth SAR BIOS,		// BIOS SAR Enable/disable
	 *     Bluetooth Increase Power Mode	// SAR Limitation Enable/disable
	 *     Bluetooth SAR Power Restriction,	// 00000000 - 0dBm
	 *					// 11111111 - 31.875dBm
	 *					// (Step 0.125dBm)
	 *     Bluetooth SAR Table		// SAR Tx power limit table
	 *   }
	 * })
	 */
	if (bsar->revision == 0 || bsar->revision > MAX_BSAR_REVISION) {
		printk(BIOS_ERR, "Unsupported BSAR table revision: %d\n",
		       bsar->revision);
		return;
	}

	acpigen_write_name("BRDS");
	acpigen_write_package(2);
	acpigen_write_dword(bsar->revision);

	if (bsar->revision == 1)
		table_size = sizeof(bsar->revs.rev1);
	else
		table_size = sizeof(bsar->revs.rev2);

	/*
	 * Emit 'Domain Type' + 'Dynamic SAR Enable' + 'Increase Power Mode'
	 * + ('SAR Power Restriction' + SAR table) | (Chain A and B SAR tables).
	 */
	package_size = 1 + 1 + 1 + table_size;
	acpigen_write_package(package_size);
	acpigen_write_dword(DOMAIN_TYPE_BLUETOOTH);
	acpigen_write_dword(1);
	acpigen_write_dword(bsar->increased_power_mode_limitation);

	set = (const uint8_t *)&bsar->revs;
	for (int i = 0; i < table_size; i++)
		acpigen_write_byte(set[i]);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_wbem(const struct wbem_profile *wbem)
{
	if (wbem == NULL)
		return;

	/*
	 * Name ("WBEM", Package() {
	 * {
	 *   Revision,
	 *   Package()
	 *   {
	 *     DomainType,				// 0x7:WiFi
	 *     bandwidth_320mhz_country_enablement	// 0 Disabled
	 *						// 1 Japan Enabled
	 *						// 2 South Korea Enabled
	 *						// 3 Japan + South Korea Enabled
	 *   }
	 } })
	 */
	if (wbem->revision != WBEM_REVISION) {
		printk(BIOS_ERR, "Unsupported WBEM table revision: %d\n",
		       wbem->revision);
		return;
	}

	acpigen_write_name("WBEM");
	acpigen_write_package(2);
	acpigen_write_dword(wbem->revision);

	acpigen_write_package(2);
	acpigen_write_dword(DOMAIN_TYPE_WIFI);
	acpigen_write_dword(wbem->bandwidth_320mhz_country_enablement);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_bpag(const struct bpag_profile *bpag)
{
	if (bpag == NULL)
		return;

	/*
	 * Name ("BPAG", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,			// 0x12:Bluetooth
	 *     Bluetooth Per-Platform Antenna Gain Mode
	 *   }
	 * })
	 */
	if (bpag->revision == 0 || bpag->revision > BPAG_REVISION) {
		printk(BIOS_ERR, "Unsupported BPAG table revision: %d\n",
		       bpag->revision);
		return;
	}

	acpigen_write_name("BPAG");
	acpigen_write_package(2);
	acpigen_write_dword(bpag->revision);

	/*
	 * Emit 'Domain Type' + 'Antenna Gain Mode'
	 */
	acpigen_write_package(2);
	acpigen_write_dword(DOMAIN_TYPE_BLUETOOTH);
	acpigen_write_dword(bpag->antenna_gain_country_enablement);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_bbfb(const struct bbfb_profile *bbfb)
{
	if (bbfb == NULL)
		return;

	/*
	 * Name ("BBFB", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,			// 0x12:Bluetooth
	 *     By Pass Enabled			// Enable ByPass
	 *   }
	 * })
	 */
	if (bbfb->revision != BBFB_REVISION) {
		printk(BIOS_ERR, "Unsupported BBFB table revision: %d\n",
		       bbfb->revision);
		return;
	}

	acpigen_write_name("BBFB");
	acpigen_write_package(2);
	acpigen_write_dword(bbfb->revision);

	/*
	 * Emit 'Domain Type' + 'Enable ByPass'
	 */
	acpigen_write_package(2);
	acpigen_write_dword(DOMAIN_TYPE_BLUETOOTH);
	acpigen_write_dword(bbfb->enable_quad_filter_bypass);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_bdcm(const struct bdcm_profile *bdcm)
{
	if (bdcm == NULL)
		return;

	/*
	 * Name ("BDCM", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,			// 0x12:Bluetooth
	 *     Dual Chain Mode
	 *   }
	 * })
	 */
	if (bdcm->revision != BDCM_REVISION) {
		printk(BIOS_ERR, "Unsupported BDCM table revision: %d\n",
		       bdcm->revision);
		return;
	}

	acpigen_write_name("BDCM");
	acpigen_write_package(2);
	acpigen_write_dword(bdcm->revision);

	/*
	 * Emit 'Domain Type' + 'Dual Chain Mode'
	 */
	acpigen_write_package(2);
	acpigen_write_dword(DOMAIN_TYPE_BLUETOOTH);
	acpigen_write_dword(bdcm->dual_chain_mode);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_bbsm(const struct bbsm_profile *bbsm)
{
	if (bbsm == NULL)
		return;

	/*
	 * Name ("BBSM", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,			// 0x12:Bluetooth
	 *     Bluetooth Bands Selection
	 *   }
	 * })
	 */
	if (bbsm->revision != BBSM_REVISION) {
		printk(BIOS_ERR, "Unsupported BBSM table revision: %d\n",
		       bbsm->revision);
		return;
	}

	acpigen_write_name("BBSM");
	acpigen_write_package(2);
	acpigen_write_dword(bbsm->revision);

	/*
	 * Emit 'Domain Type' + 'Bluetooth Bands Selection'
	 */
	acpigen_write_package(2);
	acpigen_write_dword(DOMAIN_TYPE_BLUETOOTH);
	acpigen_write_dword(bbsm->bands_selection);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_bucs(const struct bucs_profile *bucs)
{
	if (bucs == NULL)
		return;

	/*
	 * Name ("BUCS", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,			// 0x12:Bluetooth
	 *     UHB country selection bits
	 *   }
	 * })
	 */
	if (bucs->revision != BUCS_REVISION) {
		printk(BIOS_ERR, "Unsupported BUCS table revision: %d\n",
		       bucs->revision);
		return;
	}

	acpigen_write_name("BUCS");
	acpigen_write_package(2);
	acpigen_write_dword(bucs->revision);

	/*
	 * Emit 'Domain Type' + 'UHB country selection bits'
	 */
	acpigen_write_package(2);
	acpigen_write_dword(DOMAIN_TYPE_BLUETOOTH);
	acpigen_write_dword(bucs->uhb_country_selection);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_bdmm(const struct bdmm_profile *bdmm)
{
	if (bdmm == NULL)
		return;

	/*
	 * Name ("BDMM", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,			// 0x12:Bluetooth
	 *     Dual Mac Enable
	 *   }
	 * })
	 */
	if (bdmm->revision != BDMM_REVISION) {
		printk(BIOS_ERR, "Unsupported BDMM table revision: %d\n",
		       bdmm->revision);
		return;
	}

	acpigen_write_name("BDMM");
	acpigen_write_package(2);
	acpigen_write_dword(bdmm->revision);

	/*
	 * Emit 'Domain Type' + 'Dual Mac Enable'
	 */
	acpigen_write_package(2);
	acpigen_write_dword(DOMAIN_TYPE_BLUETOOTH);
	acpigen_write_dword(bdmm->dual_mac_enable);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_ebrd(const struct ebrd_profile *ebrd)
{
	if (ebrd == NULL)
		return;

	size_t package_size, table_size;
	const uint8_t *set;

	/*
	 * Name ("EBRD", Package () {
	 *   Revision,
	 *   Package () {
	 *     Domain Type,			// 0x12:Bluetooth
	 *     Dynamic SAR Enable,
	 *     Number of Optional SAR,
	 *     Three SAR Sets
	 *   }
	 * })
	 */
	if (ebrd->revision != EBRD_REVISION) {
		printk(BIOS_ERR, "Unsupported EBRD table revision: %d\n",
		       ebrd->revision);
		return;
	}

	acpigen_write_name("EBRD");
	acpigen_write_package(2);
	acpigen_write_dword(ebrd->revision);

	/*
	 * Emit 'Domain Type' + 'Dynamic Sar Enabled' + 'Number of Optional SAR' +
	 * 'Three SAR Sets'
	 */
	table_size = sizeof(*ebrd) - offsetof(struct ebrd_profile, sar_table_sets);
	package_size = 1 + 1 + 1 + table_size;
	acpigen_write_package(package_size);
	acpigen_write_dword(DOMAIN_TYPE_BLUETOOTH);
	acpigen_write_dword(ebrd->dynamic_sar_enable);
	acpigen_write_dword(ebrd->number_of_optional_sar);

	set = (const uint8_t *)&ebrd->sar_table_sets;
	for (size_t i = 0; i < table_size; i++)
		acpigen_write_byte(set[i]);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_wpfc(const struct wpfc_profile *wpfc)
{
	if (wpfc == NULL)
		return;

	/*
	 * Name ("WPFC", Package() {
	 * {
	 *   Revision,
	 *   Package()
	 *   {
	 *     DomainType,				// 0x7:WiFi
	 *     Chain A Filter Platform Configuration
	 *     Chain B Filter Platform Configuration
	 *     Chain C Filter Platform Configuration
	 *     Chain D Filter Platform Configuration
	 *   }
	 } })
	 */
	if (wpfc->revision != WPFC_REVISION) {
		printk(BIOS_ERR, "Unsupported WPFC table revision: %d\n",
		       wpfc->revision);
		return;
	}

	acpigen_write_name("WPFC");
	acpigen_write_package(2);
	acpigen_write_dword(wpfc->revision);

	acpigen_write_package(5);
	acpigen_write_dword(DOMAIN_TYPE_WIFI);
	acpigen_write_byte(wpfc->filter_cfg_chain_a);
	acpigen_write_byte(wpfc->filter_cfg_chain_b);
	acpigen_write_byte(wpfc->filter_cfg_chain_c);
	acpigen_write_byte(wpfc->filter_cfg_chain_d);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void sar_emit_dsbr(const struct dsbr_profile *dsbr, enum sar_domain domain)
{
	if (dsbr == NULL)
		return;

	/*
	 * Name ("DSBR", Package() {
	 * {
	 *   Revision,
	 *   Package()
	 *   {
	 *     DomainType,				// 0x7:WiFi
	 *     BRI Resistor Value			// in Ohm
	 *   }
	 } })
	 */
	if (dsbr->revision > DSBR_REVISION) {
		printk(BIOS_ERR, "Unsupported DSBR table revision: %d\n",
		       dsbr->revision);
		return;
	}

	acpigen_write_name("DSBR");
	acpigen_write_package(2);
	acpigen_write_dword(dsbr->revision);

	acpigen_write_package(2);
	acpigen_write_dword(domain);
	acpigen_write_dword(dsbr->bri_resistor_value);

	acpigen_write_package_end();
	acpigen_write_package_end();
}

static void emit_wifi_sar_acpi_structures(const struct device *dev,
					  union wifi_sar_limits *sar_limits)
{
	/*
	 * If device type is PCI, ensure that the device has Intel vendor ID. CBFS SAR and SAR
	 * ACPI tables are currently used only by Intel WiFi devices.
	 */
	if (dev->path.type == DEVICE_PATH_PCI && dev->vendor != PCI_VID_INTEL)
		return;

	sar_emit_wrds(sar_limits->sar);
	sar_emit_ewrd(sar_limits->sar);
	sar_emit_wgds(sar_limits->wgds);
	sar_emit_ppag(sar_limits->ppag);
	sar_emit_wtas(sar_limits->wtas);
	sar_emit_wbem(sar_limits->wbem);
	sar_emit_wpfc(sar_limits->wpfc);
	sar_emit_dsbr(sar_limits->dsbr, DOMAIN_TYPE_WIFI);
}

static void wifi_ssdt_write_device(const struct device *dev, const char *path)
{
	/* Device */
	acpigen_write_device(path);
	acpi_device_write_uid(dev);

	if (dev->chip_ops)
		acpigen_write_name_string("_DDN", dev->chip_ops->name);

	/* Address */
	acpigen_write_ADR_pci_device(dev);

	acpigen_pop_len(); /* Device */
}

static void wifi_ssdt_write_properties(const struct device *dev, const char *scope)
{
	const struct drivers_wifi_generic_config *config = dev->chip_info;

	bool is_cnvi_ddr_rfim_enabled = config && config->enable_cnvi_ddr_rfim;

	/* Scope */
	acpigen_write_scope(scope);

	if (config) {
		/* Wake capabilities */
		acpigen_write_PRW(config->wake, ACPI_S3);

		/* Add _DSD for DmaProperty property. */
		if (config->add_acpi_dma_property)
			acpi_device_add_dma_property(NULL);
	}

	/* Fill regulatory domain structure */
	if (CONFIG(HAVE_REGULATORY_DOMAIN)) {
		/*
		 * Name ("WRDD", Package () {
		 *   WRDD_REVISION, // Revision
		 *   Package () {
		 *     DOMAIN_TYPE_WIFI,        // Domain Type, 7:WiFi
		 *     wifi_regulatory_domain() // Country Identifier
		 *   }
		 * })
		 */
		acpigen_write_name("WRDD");
		acpigen_write_package(2);
		acpigen_write_integer(WRDD_REVISION);
		acpigen_write_package(2);
		acpigen_write_dword(DOMAIN_TYPE_WIFI);
		acpigen_write_dword(wifi_regulatory_domain());
		acpigen_pop_len();
		acpigen_pop_len();
	}

	struct dsm_uuid dsm_ids[MAX_DSM_FUNCS];

	/* Retrieve the SAR limits data */
	union wifi_sar_limits sar_limits = {0};
	bool sar_loaded = false;
	if (CONFIG(USE_SAR)) {
		if (get_wifi_sar_limits(&sar_limits) < 0)
			printk(BIOS_ERR, "failed getting SAR limits!\n");
		else
			sar_loaded = true;
	}

	/* Fill Wifi SAR related ACPI structures */
	uint8_t dsm_count = 0;
	if (sar_loaded) {
		emit_wifi_sar_acpi_structures(dev, &sar_limits);

		struct dsm_profile *dsm = sar_limits.dsm;
		if (dsm && dsm->supported_functions != 0) {
			for (int i = 1; i < ARRAY_SIZE(wifi_dsm_callbacks); i++)
				if (!(dsm->supported_functions & (1 << i)))
					wifi_dsm_callbacks[i] = NULL;

			dsm_ids[dsm_count].uuid = ACPI_DSM_OEM_WIFI_UUID;
			dsm_ids[dsm_count].callbacks = &wifi_dsm_callbacks[0];
			dsm_ids[dsm_count].count = ARRAY_SIZE(wifi_dsm_callbacks);
			dsm_ids[dsm_count].arg = dsm;
			dsm_count++;
		}
	}

	if (is_cnvi_ddr_rfim_enabled) {
		dsm_ids[dsm_count].uuid = ACPI_DSM_RFIM_WIFI_UUID;
		dsm_ids[dsm_count].callbacks = &wifi_dsm2_callbacks[0];
		dsm_ids[dsm_count].count = ARRAY_SIZE(wifi_dsm2_callbacks);
		dsm_ids[dsm_count].arg = &is_cnvi_ddr_rfim_enabled;
		dsm_count++;
	}

	acpigen_write_dsm_uuid_arr(dsm_ids, dsm_count);

	/*
	 * Fill MediaTek MTCL related ACPI structure iff the device type is PCI,
	 * the device has the MediaTek vendor ID, and the MTCL feature is
	 * configured.
	 */
	if (CONFIG(USE_MTCL)) {
		if (dev->path.type == DEVICE_PATH_PCI &&
		    dev->vendor == PCI_VID_MEDIATEK)
			write_mtcl_function();
	}

	acpigen_write_scope_end(); /* Scope */

	/* Fill Bluetooth companion SAR related ACPI structures */
	if (sar_loaded && is_dev_enabled(config->bluetooth_companion)) {
		const char *path = acpi_device_path(config->bluetooth_companion);
		if (path) {	/* Bluetooth device under USB Hub scope or PCIe root port */
			acpigen_write_scope(path);
			sar_emit_brds(sar_limits.bsar);
			sar_emit_bpag(sar_limits.bpag);
			sar_emit_bbfb(sar_limits.bbfb);
			sar_emit_bdcm(sar_limits.bdcm);
			sar_emit_bbsm(sar_limits.bbsm);
			sar_emit_bucs(sar_limits.bucs);
			sar_emit_bdmm(sar_limits.bdmm);
			sar_emit_ebrd(sar_limits.ebrd);
			sar_emit_dsbr(sar_limits.dsbr, DOMAIN_TYPE_BLUETOOTH);
			acpigen_write_scope_end();
		} else {
			printk(BIOS_ERR, "Failed to get %s Bluetooth companion ACPI path\n",
			       dev_path(dev));
		}
	}

	if (sar_loaded)
		free(sar_limits.sar);

	printk(BIOS_INFO, "%s: %s %s\n", scope, dev->chip_ops ? dev->chip_ops->name : "",
	       dev_path(dev));
}

void wifi_pcie_fill_ssdt(const struct device *dev)
{
	const char *path;

	path = acpi_device_path(dev);
	if (!path)
		return;

	wifi_ssdt_write_device(dev, path);
	wifi_ssdt_write_properties(dev, path);
}

const char *wifi_pcie_acpi_name(const struct device *dev)
{
	static char wifi_acpi_name[WIFI_ACPI_NAME_MAX_LEN];

	/* ACPI 6.3, ASL 20.2.2: (Name Objects Encoding). */
	snprintf(wifi_acpi_name, sizeof(wifi_acpi_name), "WF%02X",
		 (dev_path_encode(dev) & 0xff));
	return wifi_acpi_name;
}

void wifi_cnvi_fill_ssdt(const struct device *dev)
{
	const char *path;
	if (!dev)
		return;

	path = acpi_device_path(dev->upstream->dev);
	if (!path)
		return;

	wifi_ssdt_write_properties(dev, path);
}
