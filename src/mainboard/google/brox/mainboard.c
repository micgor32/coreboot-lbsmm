/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <acpi/acpigen.h>
#include <baseboard/gpio.h>
#include <baseboard/variants.h>
#include <device/device.h>
#include <drivers/wwan/fm/chip.h>
#include <ec/ec.h>
#include <fw_config.h>
#include <soc/gpio.h>
#include <soc/ramstage.h>
#include <stdio.h>

static void add_fw_config_oem_string(const struct fw_config *config, void *arg)
{
	struct smbios_type11 *t;
	char buffer[64];

	t = (struct smbios_type11 *)arg;

	snprintf(buffer, sizeof(buffer), "%s-%s", config->field_name, config->option_name);
	t->count = smbios_add_string(t->eos, buffer);
}

static void mainboard_smbios_strings(struct device *dev, struct smbios_type11 *t)
{
	fw_config_for_each_found(add_fw_config_oem_string, t);
}

void mainboard_update_soc_chip_config(struct soc_intel_alderlake_config *config)
{
	variant_update_soc_chip_config(config);
}

void __weak variant_update_soc_chip_config(struct soc_intel_alderlake_config *config)
{
	/* default implementation does nothing */
}

void __weak variant_init(void)
{
	/* default implementation does nothing */
}

void __weak fw_config_gpio_padbased_override(struct pad_config *padbased_table)
{
	/* default implementation does nothing */
}

void __weak variant_configure_pads(void)
{
	const struct pad_config *base_pads;
	const struct pad_config *override_pads;
	size_t base_num, override_num;

	base_pads = variant_gpio_table(&base_num);
	override_pads = variant_gpio_override_table(&override_num);
	gpio_configure_pads_with_override(base_pads, base_num, override_pads, override_num);
}

static void mainboard_init(void *chip_info)
{
	variant_configure_pads();
	variant_init();
	variant_devtree_update();
}

void __weak variant_devtree_update(void)
{
	/* Override dev tree settings per board */
}

static void mainboard_dev_init(struct device *dev)
{
	mainboard_ec_init();
}

static void mainboard_generate_s0ix_hook(void)
{
	acpigen_write_if_lequal_op_int(ARG0_OP, 1);
	{
		if (CONFIG(HAVE_SLP_S0_GATE))
			acpigen_soc_clear_tx_gpio(GPIO_SLP_S0_GATE);
		variant_generate_s0ix_hook(S0IX_ENTRY);
	}
	acpigen_write_else();
	{
		if (CONFIG(HAVE_SLP_S0_GATE))
			acpigen_soc_set_tx_gpio(GPIO_SLP_S0_GATE);
		variant_generate_s0ix_hook(S0IX_EXIT);
	}
	acpigen_write_if_end();
}

static void mainboard_fill_ssdt(const struct device *dev)
{
	/* for variant to fill additional SSDT */
	variant_fill_ssdt(dev);

	acpigen_write_scope("\\_SB");
	acpigen_write_method_serialized("MS0X", 1);
	mainboard_generate_s0ix_hook();
	acpigen_write_method_end(); /* Method */
	acpigen_write_scope_end(); /* Scope */
}

void __weak variant_fill_ssdt(const struct device *dev)
{
	/* Add board-specific SSDT entries */
}

void __weak variant_generate_s0ix_hook(enum s0ix_entry entry)
{
	/* Add board-specific MS0X entries */
	/*
	   if (s0ix_entry == S0IX_ENTRY) {
		implement variant operations here
	   }
	   if (s0ix_entry == S0IX_EXIT) {
		implement variant operations here
	   }
	 */
}

static void mainboard_enable(struct device *dev)
{
	dev->ops->init = mainboard_dev_init;
	dev->ops->get_smbios_strings = mainboard_smbios_strings;
	dev->ops->acpi_fill_ssdt = mainboard_fill_ssdt;
}


void __weak variant_finalize(void)
{
}

static void mainboard_final(void *chip_info)
{
	variant_finalize();
}

struct chip_operations mainboard_ops = {
	.init = mainboard_init,
	.enable_dev = mainboard_enable,
	.final = mainboard_final,
};
