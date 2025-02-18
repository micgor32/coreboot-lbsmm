/* SPDX-License-Identifier: GPL-2.0-only */

#include <boot/coreboot_tables.h>
#include <cpu/x86/smm.h>
#include <device/pci_ops.h>
#include <lib.h>
#include <smm_payload_interface.h>
#include <soc/pci_devs.h>
#if CONFIG(SMM_PAYLOAD_INTERFACE_PAYLOAD_SMM)
#include <soc/pm.h>
#include <soc/systemagent.h>
#endif

void lb_save_restore(struct lb_header *header)
{
	uintptr_t tseg_base;
	size_t tseg_size;

	/*
	 * SMM info tables
	 */
	/* SMI registers */
	struct lb_pld_smm_registers *smm_reg = (struct lb_pld_smm_registers *)lb_new_record(header);

	smm_reg->tag = LB_TAG_PLD_SMM_REGISTER_INFO;
	smm_reg->revision = 0;
	smm_reg->count = 0;

#if CONFIG(SMM_PAYLOAD_INTERFACE_PAYLOAD_SMM)
	smm_reg->registers[smm_reg->count].register_id = REGISTER_ID_SMI_GBL_EN;
	smm_reg->registers[smm_reg->count].address_space_id = PLD_EFI_ACPI_3_0_SYSTEM_IO;
	smm_reg->registers[smm_reg->count].register_bit_width = 1;
	smm_reg->registers[smm_reg->count].register_bit_offset = log2(GBL_SMI_EN);
	smm_reg->registers[smm_reg->count].value = 1;
	smm_reg->registers[smm_reg->count].address = ACPI_BASE_ADDRESS + SMI_EN;
	smm_reg->count++;

	smm_reg->registers[smm_reg->count].register_id = REGISTER_ID_SMI_GBL_EN_LOCK;
	smm_reg->registers[smm_reg->count].address_space_id = PLD_EFI_ACPI_3_0_SYSTEM_MEMORY;
	smm_reg->registers[smm_reg->count].register_bit_width = 1;
	smm_reg->registers[smm_reg->count].register_bit_offset = log2(SMI_LOCK);
	smm_reg->registers[smm_reg->count].value = 1;
	smm_reg->registers[smm_reg->count].address = PCH_PWRM_BASE_ADDRESS + GEN_PMCON_B;
	smm_reg->count++;

	smm_reg->registers[smm_reg->count].register_id = REGISTER_ID_SMI_EOS;
	smm_reg->registers[smm_reg->count].address_space_id = PLD_EFI_ACPI_3_0_SYSTEM_IO;
	smm_reg->registers[smm_reg->count].register_bit_width = 1;
	smm_reg->registers[smm_reg->count].register_bit_offset = log2(EOS);
	smm_reg->registers[smm_reg->count].value = 1;
	smm_reg->registers[smm_reg->count].address = ACPI_BASE_ADDRESS + SMI_EN;
	smm_reg->count++;

	smm_reg->registers[smm_reg->count].register_id = REGISTER_ID_SMI_APM_EN;
	smm_reg->registers[smm_reg->count].address_space_id = PLD_EFI_ACPI_3_0_SYSTEM_IO;
	smm_reg->registers[smm_reg->count].register_bit_width = 1;
	smm_reg->registers[smm_reg->count].register_bit_offset = log2(APMC_EN);
	smm_reg->registers[smm_reg->count].value = 1;
	smm_reg->registers[smm_reg->count].address = ACPI_BASE_ADDRESS + SMI_EN;
	smm_reg->count++;

	smm_reg->registers[smm_reg->count].register_id = REGISTER_ID_SMI_APM_STS;
	smm_reg->registers[smm_reg->count].address_space_id = PLD_EFI_ACPI_3_0_SYSTEM_IO;
	smm_reg->registers[smm_reg->count].register_bit_width = 1;
	smm_reg->registers[smm_reg->count].register_bit_offset = APM_STS_BIT;
	smm_reg->registers[smm_reg->count].value = 1;
	smm_reg->registers[smm_reg->count].address = ACPI_BASE_ADDRESS + SMI_STS;
	smm_reg->count++;
#endif

#ifdef SMRAM
	// TODO: Consider incrementing `revision` and individual open/close/lock fields
	// - Or drop this, have coreboot handle it
	smm_reg->registers[smm_reg->count].register_id = REGISTER_ID_SMRAMC;
	smm_reg->registers[smm_reg->count].address_space_id = PLD_EFI_ACPI_3_0_PCI_CONFIGURATION_SPACE;
	smm_reg->registers[smm_reg->count].register_bit_width = 8;
	smm_reg->registers[smm_reg->count].register_bit_offset = 0;
	smm_reg->registers[smm_reg->count].address = PCI_BDF(SA_DEV_ROOT) + SMRAM;
	smm_reg->count++;
#endif
	smm_reg->size = sizeof(*smm_reg) + smm_reg->count * sizeof(struct lb_pld_generic_register);

	/* SMRAM ranges */
	struct lb_pld_smram_descriptor_block *smram_desc = (struct lb_pld_smram_descriptor_block *)lb_new_record(header);

	smram_desc->tag = LB_TAG_PLD_SMM_SMRAM;
	smram_desc->size = sizeof(*smram_desc) + sizeof(struct lb_pld_smram_descriptor);
	smram_desc->number_of_smm_regions = 2;

	pld_interface_get_reserved_region(&tseg_base, &tseg_size);

	smram_desc->descriptor[0].physical_start = tseg_base;
	smram_desc->descriptor[0].physical_size = PLD_S3_COMMUNICATION_SIZE;
	smram_desc->descriptor[0].region_state = 0x10; // EFI_ALLOCATED

	smram_desc->descriptor[1].physical_start = tseg_base + PLD_S3_COMMUNICATION_SIZE;
	smram_desc->descriptor[1].physical_size = tseg_size - PLD_S3_COMMUNICATION_SIZE;
	smram_desc->descriptor[1].region_state = 0;

	/* Fixed communication range. Payload informs bootloader of SWSMI number */
	struct lb_pld_s3_communication *s3_comm = (struct lb_pld_s3_communication *)lb_new_record(header);

	s3_comm->tag = LB_TAG_PLD_S3_COMMUNICATION;
	s3_comm->size = sizeof(*s3_comm);

	s3_comm->comm_buffer.physical_start = tseg_base;
	s3_comm->comm_buffer.physical_size = PLD_S3_COMMUNICATION_SIZE;
	s3_comm->pld_acpi_s3_enable = 0;

	/*
	 * SPI info tables
	 */
	/* SPI registers */
	struct lb_pld_spi_flash_info *spi_info = (struct lb_pld_spi_flash_info *)lb_new_record(header);

	spi_info->tag = LB_TAG_PLD_SPI_FLASH_INFO;
	spi_info->size = sizeof(*spi_info);
	spi_info->revision = 0;

	spi_info->flags = 0;
	if (CONFIG(BOOTMEDIA_SMM_BWP))
		spi_info->flags |= FLAGS_SPI_DISABLE_SMM_WRITE_PROTECT;

	spi_info->spi_address.address_space_id = PLD_EFI_ACPI_3_0_PCI_CONFIGURATION_SPACE;
	spi_info->spi_address.register_bit_width = 32;
	spi_info->spi_address.register_bit_offset = 0;
	// FIXME: Avoid hard-coding
#ifdef PCH_DEV_SPI
	spi_info->spi_address.address = CONFIG_ECAM_MMCONF_BASE_ADDRESS + PCI_BDF(PCH_DEV_SPI);
#else
	#error "BUGBUG: PCH_DEV_SPI not defined!"
#endif
}
