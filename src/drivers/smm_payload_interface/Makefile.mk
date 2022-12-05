## SPDX-License-Identifier: GPL-2.0-only

ramstage-$(CONFIG_SMM_PAYLOAD_INTERFACE) += cbtable.c
ramstage-$(CONFIG_SMM_PAYLOAD_INTERFACE_PAYLOAD_SMM) += smm_core_support.c
all-$(CONFIG_SMM_PAYLOAD_INTERFACE) += util.c
