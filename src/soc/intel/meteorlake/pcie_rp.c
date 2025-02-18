/* SPDX-License-Identifier: GPL-2.0-only */

#include <intelblocks/pcie_rp.h>
#include <soc/pci_devs.h>
#include <soc/pcie.h>
#include <soc/soc_info.h>

/*
 * Document #640228 Meteor Lake - U/H and U Type4 Processor - 2.3 Device IDs - Table 8 "Other
 * Device ID" specifies that the first Thunderbolt PCIe root port number is 16. TBT's LCAP
 * registers return port index which starts from 16 (usually for other PCIe root ports index
 * starts from 1). Thus, keeping lcap_port_base 16 for TBT, so that coreboot's PCIe remapping
 * logic can return a correct index (0-based).
 */

static const struct pcie_rp_group tbt_rp_groups[] = {
	{ .slot = PCI_DEV_SLOT_TBT, .count = CONFIG_MAX_TBT_ROOT_PORTS, .lcap_port_base = 16 },
	{ 0 }
};

static const struct pcie_rp_group mtlp_rp_groups[] = {
	{ .slot = PCI_DEV_SLOT_PCIE_1, .start = 0, .count = 8, .lcap_port_base = 1 },
	{ .slot = PCI_DEV_SLOT_PCIE_2, .start = 0, .count = 3, .lcap_port_base = 1 },
	{ .slot = PCI_DEV_SLOT_PCIE_3, .start = 0, .count = 1, .lcap_port_base = 1 },
	{ 0 }
};

const struct pcie_rp_group *get_pcie_rp_table(void)
{
	return mtlp_rp_groups;
}

const struct pcie_rp_group *get_tbt_pcie_rp_table(void)
{
	return tbt_rp_groups;
}

enum pcie_rp_type soc_get_pcie_rp_type(const struct device *dev)
{
	return PCIE_RP_PCH;
}

int soc_get_cpu_rp_vw_idx(const struct device *dev)
{
	return -1;
}
