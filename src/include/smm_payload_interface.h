/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SMM_PAYLOAD_INTERFACE_H
#define SMM_PAYLOAD_INTERFACE_H

#include <commonlib/coreboot_tables.h>
#include <types.h>

#if CONFIG(SMM_PAYLOAD_INTERFACE)

void pld_interface_get_reserved_region(uintptr_t *tseg_base, size_t *tseg_size);

struct pld_hob_header {
	uint16_t type;
	uint16_t length;
};

#define PLD_S3_COMMUNICATION_SIZE 4096

bool pld_guid_compare(const uint8_t guid1[16], const uint8_t guid2[16]);
const void *pld_hob_header_to_struct(const struct pld_hob_header *hob);
const void *pld_hob_header_to_extension_hob(const struct pld_hob_header *hob);

#endif

uint8_t payload_mm_exec_interface(uint8_t sub_command, uint32_t argument);
void mm_call_entrypoint(void);

void lb_save_restore(struct lb_header *header);

#if CONFIG(SMM_PAYLOAD_INTERFACE_PAYLOAD_SMM)

/*
 * The data below is communicated between bootloader and payload.
 * It describes the data required by the bootloader to perform
 * SMM relocation for the payload and pass control to its SMI handler.
 *
 * The payload saves it to the SMM communication area on a normal boot.
 * On the S3 resume path, the bootloader restores these SMM bases and
 * triggers the SwSmiTriggerValue SWSMI to let the payload restore for S3.
 */

struct CPU_SMMBASE {
	uint32_t ApicId;
	uint32_t SmmBase;
};

struct SMM_S3_INFO {
	uint8_t SwSmiData;
	uint8_t SwSmiTriggerValue;
	uint16_t Reserved;
	uint32_t CpuCount;
	struct CPU_SMMBASE SmmBase[0];
};

struct PLD_TO_BL_SMM_INFO {
	// EFI_HOB_GUID_TYPE Header;
	struct SMM_S3_INFO S3Info;
};

#endif

#endif
