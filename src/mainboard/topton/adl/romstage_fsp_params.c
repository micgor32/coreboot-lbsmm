/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <fsp/api.h>
#include <soc/meminit.h>
#include <soc/romstage.h>

void mainboard_memory_init_params(FSPM_UPD *memupd)
{
	static const struct mb_cfg ddr5_mem_config = {
		.UserBd = BOARD_TYPE_MOBILE,
		.type = MEM_TYPE_DDR5,
		.ect = true,
		.LpDdrDqDqsReTraining = 1,
	};

	const struct mem_spd dimm_module_spd_info = {
		.topo = MEM_TOPO_DIMM_MODULE,
		.smbus = {
			[0] = { .addr_dimm[0] = 0x50, },
		},
	};

	const bool half_populated = false;

	memcfg_init(memupd, &ddr5_mem_config, &dimm_module_spd_info, half_populated);
}
