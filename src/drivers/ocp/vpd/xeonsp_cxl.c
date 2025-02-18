/* SPDX-License-Identifier: GPL-2.0-only */

#include <drivers/ocp/include/vpd.h>
#include <soc/config.h>

enum xeonsp_cxl_mode get_cxl_mode(void)
{
	int ocp_cxl_mode = get_cxl_mode_from_vpd();
	switch (ocp_cxl_mode) {
	case CXL_SYSTEM_MEMORY:
		return XEONSP_CXL_SYS_MEM;
	case CXL_SPM:
		return XEONSP_CXL_SP_MEM;
	default:
		return XEONSP_CXL_DISABLED;
	}
}
