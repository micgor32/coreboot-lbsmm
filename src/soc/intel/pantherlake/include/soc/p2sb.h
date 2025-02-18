/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _SOC_PANTHERLAKE_P2SB_H_
#define _SOC_PANTHERLAKE_P2SB_H_

#define HPTC_OFFSET			0x60
#define  HPTC_ADDR_ENABLE_BIT		BIT(7)

#define PCH_P2SB_EPMASK0		0x220

extern struct device_operations pcd_p2sb_ops;
extern struct device_operations pcd_p2sb_2_ops;

#endif
