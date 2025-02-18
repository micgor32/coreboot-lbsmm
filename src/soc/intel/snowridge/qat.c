/* SPDX-License-Identifier: GPL-2.0-only */

#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>

#include "ramstage.h"

static struct device_operations snr_qat_ops = {
	.read_resources = pciexp_pf_read_resources,
	.set_resources = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.final = pci_dev_request_bus_master,
	.ops_pci = &pci_dev_ops_pci,
};

static const unsigned short snr_qat_ids[] = {
	PCI_DID_INTEL_SNR_QAT_1_7,
	PCI_DID_INTEL_SNR_QAT_1_8,
	0
};

static const struct pci_driver snr_qat_driver __pci_driver = {
	.ops = &snr_qat_ops,
	.vendor = PCI_VID_INTEL,
	.devices = snr_qat_ids
};
