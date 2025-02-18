/* SPDX-License-Identifier: GPL-2.0-only */

#include <chip.h>
#include <cpu/intel/turbo.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <option.h>
#include <static.h>
#include <types.h>
#include <variants.h>

void devtree_update(void)
{
	config_t *cfg = config_of_soc();

	struct soc_intel_common_config *common_config;
	common_config = chip_get_common_soc_structure();

	struct soc_power_limits_config *soc_conf_20core =
		&cfg->power_limits_config[MTL_P_682_482_CORE];

	struct device *gna_dev = pcidev_on_root(0x08, 0);

	uint8_t performance_scale = 100;

	/* Set PL4 to 1.0C */
	soc_conf_20core->tdp_pl4			= 65;

	/* Set PL1 to 50% of PL2 */
	soc_conf_20core->tdp_pl1_override = (soc_conf_20core->tdp_pl2_override / 2) & ~1;

	/* Scale PL1 & PL2 based on CMOS settings */
	switch (get_power_profile(PP_POWER_SAVER)) {
	case PP_POWER_SAVER:
		performance_scale			-= 50;
		common_config->pch_thermal_trip		= 30;
		break;
	case PP_BALANCED:
		performance_scale			-= 25;
		common_config->pch_thermal_trip		= 25;
		break;
	case PP_PERFORMANCE:
		/* Use the Intel defaults */
		common_config->pch_thermal_trip		= 20;
		break;
	}

	soc_conf_20core->tdp_pl1_override = (soc_conf_20core->tdp_pl1_override * performance_scale) / 100;
	soc_conf_20core->tdp_pl2_override = (soc_conf_20core->tdp_pl2_override * performance_scale) / 100;

	/* Enable/Disable Bluetooth based on CMOS settings */
	if (get_uint_option("wireless", 1) == 0)
		cfg->usb2_ports[9].enable = 0;

	/* Enable/Disable Webcam based on CMOS settings */
	if (get_uint_option("webcam", 1) == 0)
		cfg->usb2_ports[CONFIG_CCD_PORT].enable = 0;

	/* Enable/Disable Card Reader based on CMOS Settings */
	if (get_uint_option("card_reader", 1) == 0)
		cfg->usb2_ports[3].enable = 0;

	/* Enable/Disable GNA based on CMOS settings */
	if (get_uint_option("gna", 0) == 0)
		gna_dev->enabled = 0;
}
