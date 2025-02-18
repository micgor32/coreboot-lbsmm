/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SOC_MEDIATEK_MT8196_SPM_MTCMOS_H
#define SOC_MEDIATEK_MT8196_SPM_MTCMOS_H

#include <commonlib/list.h>

enum mtcmos_state {
	MTCMOS_POWER_DOWN,
	MTCMOS_POWER_ON,
};

enum mtcmos_id {
	MTCMOS_ID_SSUSB_DP_PHY_P0,
	MTCMOS_ID_SSUSB_P0,
	MTCMOS_ID_SSUSB_P1,
	MTCMOS_ID_SSUSB_P23,
	MTCMOS_ID_SSUSB_PHY_P2,
	MTCMOS_ID_UFS0_SHUTDOWN,
	MTCMOS_ID_UFS0_PHY,
	MTCMOS_ID_PEXTP_MAC0,
	MTCMOS_ID_PEXTP_MAC1,
	MTCMOS_ID_PEXTP_MAC2,
	MTCMOS_ID_PEXTP_PHY0,
	MTCMOS_ID_PEXTP_PHY1,
	MTCMOS_ID_PEXTP_PHY2,
	MTCMOS_ID_AUDIO,
	MTCMOS_ID_ADSP_TOP_SHUTDOWN,
	MTCMOS_ID_ADSP_INFRA,
	MTCMOS_ID_ADSP_AO,
	MTCMOS_ID_MM_PROC_SHUTDOWN,
	MTCMOS_ID_SSRSYS,
	MTCMOS_ID_DISP_VCORE,
	MTCMOS_ID_DIS0_SHUTDOWN,
	MTCMOS_ID_DIS1_SHUTDOWN,
	MTCMOS_ID_OVL0_SHUTDOWN,
	MTCMOS_ID_OVL1_SHUTDOWN,
	MTCMOS_ID_DISP_EDPTX_SHUTDOWN,
	MTCMOS_ID_DISP_DPTX_SHUTDOWN,
	MTCMOS_ID_MM_INFRA0,
	MTCMOS_ID_MM_INFRA1,
	MTCMOS_ID_MM_INFRA_AO,
	MTCMOS_ID_DSI_PHY0,
	MTCMOS_ID_DSI_PHY1,
	MTCMOS_ID_DSI_PHY2,
	MTCMOS_ID_NUM,
};

struct mtcmos_cb {
	int (*pre_on)(void);
	int (*pre_off)(void);
	int (*post_on)(void);
	int (*post_off)(void);
};

int mtcmos_cb_register(enum mtcmos_id id, const struct mtcmos_cb *cb);
void mtcmos_ctrl(enum mtcmos_id id, enum mtcmos_state state);
void mtcmos_init(void);
void mtcmos_post_init(void);

#endif
