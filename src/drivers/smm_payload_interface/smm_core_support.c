/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpi.h>
#include <arch/io.h>
#include <bootstate.h>
#include <console/console.h>
#include <cpu/intel/smm_reloc.h>
#include <cpu/x86/smm.h>
#include <smm_payload_interface.h>
#include <soc/pm.h>
#include <types.h>

/*
 * FIXME: For enhancing this to work on other platforms, such as AMD:
 * - (Payload requires support, then coreboot tables must be developed.)
 * - `get_smm_info()` implementations must handle `platform_get_smm_info()`.
 * - ACPI base region is SoC-specific.
 */

/* GUIDs in little-endian, so they can be used with memcmp() */
const uint8_t pld_s3_communication_guid[16] = {
	0xa1, 0x1b, 0xe3, 0x88, 0x56, 0x18, 0x8b, 0x4b,
	0xbb, 0xdf, 0xf8, 0x16, 0xdd, 0x94, 0x0a, 0xef,
};

/*
 * SMBASE relocation in C-environment shall succeed without using TSEG:
 * - Provide stack outside TSEG. Presumed safe: not stage-cached nor permanent.
 * - Stash the S3 info early. Once relocated/locked, ramstage cannot access SMRAM.
 */
static uint8_t smbase_relocation_cpu_stacks[CONFIG_MAX_CPUS * CONFIG_SMM_MODULE_STACK_SIZE];
static uint8_t have_saved_s3_info = 0;
static uint8_t s3_info_save[PLD_S3_COMMUNICATION_SIZE];

void pld_interface_get_reserved_region(uintptr_t *tseg_base, size_t *tseg_size)
{
	smm_region(tseg_base, tseg_size);
}

static void *find_s3_info(void)
{
	uintptr_t tseg_base;
	size_t tseg_size;
	const uint8_t *hob_guid;

	if (have_saved_s3_info == 1)
		return (void *)pld_hob_header_to_extension_hob((void *)s3_info_save);

	smm_region(&tseg_base, &tseg_size);

	hob_guid = pld_hob_header_to_struct((void *)tseg_base);
	if (pld_guid_compare(hob_guid, pld_s3_communication_guid)) {
		// Save data from SMRAM while still unlocked
		memcpy(s3_info_save, (void *)tseg_base, PLD_S3_COMMUNICATION_SIZE);
		have_saved_s3_info = 1;

		return (void *)pld_hob_header_to_extension_hob((void *)s3_info_save);
	}

	printk(BIOS_ERR, "%s(): Could not find S3 info!\n", __func__);
	return NULL;
}

enum platform_smm_status platform_get_smm_info(uintptr_t *perm_smbase, size_t *perm_smsize)
{
	// Initialise to "invalid" state
	*perm_smbase = 0;
	*perm_smsize = 0;

	// No SMM relocation needed on cold boot
	if (!acpi_is_wakeup_s3())
		return SKIP_SMM_INIT;

	*perm_smbase = (uintptr_t)smbase_relocation_cpu_stacks;
	*perm_smsize = sizeof(smbase_relocation_cpu_stacks);

	return MINIMAL_SMM_INIT;
}

u32 smm_get_cpu_smbase(unsigned int cpu_num)
{
	struct PLD_TO_BL_SMM_INFO *pld_to_bl_smm_info;

	pld_to_bl_smm_info = find_s3_info();
	if (pld_to_bl_smm_info == NULL)
		return 0;

	if (cpu_num >= pld_to_bl_smm_info->S3Info.CpuCount) {
		printk(BIOS_ERR, "Unexpected CPU %d/%d!\n", cpu_num,
		       pld_to_bl_smm_info->S3Info.CpuCount);
		return 0;
	}

	return pld_to_bl_smm_info->S3Info.SmmBase[cpu_num].SmmBase;
}

static void clear_s3_save_region(void)
{
	uintptr_t tseg_base;
	size_t tseg_size;

	smm_region(&tseg_base, &tseg_size);
	memset((void *)tseg_base, 0x0, PLD_S3_COMMUNICATION_SIZE);
}

static void trigger_payload_swsmi(void)
{
	struct PLD_TO_BL_SMM_INFO *pld_to_bl_smm_info;

	pld_to_bl_smm_info = find_s3_info();
	if (pld_to_bl_smm_info == NULL)
		return;

	printk(BIOS_DEBUG, "SwSmiTriggerValue: 0x%x\n",
	       pld_to_bl_smm_info->S3Info.SwSmiTriggerValue);
	outb(pld_to_bl_smm_info->S3Info.SwSmiTriggerValue, APM_CNT);
}

static void perform_save(void *unused)
{
	uint32_t reg32;

	// Payload does not enable sleep SMI
	reg32 = inl(ACPI_BASE_ADDRESS + SMI_EN);
	reg32 &= ~SLP_SMI_EN;
	outl(reg32, ACPI_BASE_ADDRESS + SMI_EN);

	// Enable SCI mode when there's no 'ACPI enable' SMI handler
	reg32 = inl(ACPI_BASE_ADDRESS + PM1_CNT);
	reg32 |= SCI_EN;
	outl(reg32, ACPI_BASE_ADDRESS + PM1_CNT);

	clear_s3_save_region();
}

static void perform_s3_restore(void *unused)
{
	uint32_t reg32;

	// Payload does not enable sleep SMI
	reg32 = inl(ACPI_BASE_ADDRESS + SMI_EN);
	reg32 &= ~SLP_SMI_EN;
	outl(reg32, ACPI_BASE_ADDRESS + SMI_EN);

	// Enable SCI mode when there's no 'ACPI enable' SMI handler
	reg32 = inl(ACPI_BASE_ADDRESS + PM1_CNT);
	reg32 |= SCI_EN;
	outl(reg32, ACPI_BASE_ADDRESS + PM1_CNT);

	trigger_payload_swsmi();
}

BOOT_STATE_INIT_ENTRY(BS_WRITE_TABLES, BS_ON_EXIT, perform_save, NULL);
BOOT_STATE_INIT_ENTRY(BS_OS_RESUME, BS_ON_ENTRY, perform_s3_restore, NULL);
