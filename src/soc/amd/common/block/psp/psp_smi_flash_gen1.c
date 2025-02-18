/* SPDX-License-Identifier: GPL-2.0-only */

#include <device/mmio.h>
#include <types.h>
#include "psp_def.h"
#include "psp_smi_flash.h"

bool is_valid_psp_spi_info(struct mbox_psp_cmd_spi_info *cmd_buf)
{
	return true;
}

bool is_valid_psp_spi_read_write(struct mbox_psp_cmd_spi_read_write *cmd_buf)
{
	return true;
}

bool is_valid_psp_spi_erase(struct mbox_psp_cmd_spi_erase *cmd_buf)
{
	return true;
}

uint64_t get_psp_spi_info_id(struct mbox_psp_cmd_spi_info *cmd_buf)
{
	return SMI_TARGET_NVRAM;
}

void set_psp_spi_info(struct mbox_psp_cmd_spi_info *cmd_buf, uint64_t lba, uint64_t block_size,
		      uint64_t num_blocks)
{
	write64(&cmd_buf->req.lba, lba);
	write64(&cmd_buf->req.block_size, block_size);
	write64(&cmd_buf->req.num_blocks, num_blocks);
}

void get_psp_spi_read_write(struct mbox_psp_cmd_spi_read_write *cmd_buf,
			    uint64_t *target_nv_id, uint64_t *lba, uint64_t *offset,
			    uint64_t *num_bytes, uint8_t **data)
{
	*target_nv_id = SMI_TARGET_NVRAM;
	*lba = read64(&cmd_buf->req.lba);
	*offset = read64(&cmd_buf->req.offset);
	*num_bytes = read64(&cmd_buf->req.num_bytes);
	*data = cmd_buf->req.buffer;
}

void get_psp_spi_erase(struct mbox_psp_cmd_spi_erase *cmd_buf, uint64_t *target_nv_id,
		       uint64_t *lba, uint64_t *num_blocks)
{
	*target_nv_id = SMI_TARGET_NVRAM;
	*lba = read64(&cmd_buf->req.lba);
	*num_blocks = read64(&cmd_buf->req.num_blocks);
}
