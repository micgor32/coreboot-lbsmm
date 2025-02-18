/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _SOC_PANTHERLAKE_TCSS_H_
#define _SOC_PANTHERLAKE_TCSS_H_

/* Thunderbolt firmware IMR status */
#define IOM_CSME_IMR_TBT_STATUS			0x14
#define TBT_VALID_AUTHENTICATION		BIT(30)

/* IOM aux bias control registers in REGBAR MMIO space */
#define IOM_AUX_BIAS_CTRL_PULLUP_OFFSET_0	0x1070
#define IOM_AUX_BIAS_CTRL_PULLUP_OFFSET(x)	(IOM_AUX_BIAS_CTRL_PULLUP_OFFSET_0 + (x) * 4)
#define IOM_AUX_BIAS_CTRL_PULLDOWN_OFFSET_0	0x1088
#define IOM_AUX_BIAS_CTRL_PULLDOWN_OFFSET(x)	(IOM_AUX_BIAS_CTRL_PULLDOWN_OFFSET_0 + (x) * 4)

#define BIAS_CTRL_VW_INDEX_SHIFT 24
#define BIAS_CTRL_BIT_POS_SHIFT 16

enum {
	TCSS_TYPE_C_PORT_DISABLE,
	TCSS_TYPE_C_PORT_DP_ONLY,
	TCSS_TYPE_C_PORT_NO_TBT,
	TCSS_TYPE_C_PORT_NO_PCIE,
	TCSS_TYPE_C_PORT_FULL_FUN = 7
};

#endif
