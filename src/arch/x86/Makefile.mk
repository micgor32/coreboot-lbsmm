## SPDX-License-Identifier: GPL-2.0-only

ifeq ($(CONFIG_POSTCAR_STAGE),y)
$(eval $(call init_standard_toolchain,postcar))
endif

################################################################################
# i386 specific tools
NVRAMTOOL:=$(objutil)/nvramtool/nvramtool

OPTION_TABLE_H:=
ifeq ($(CONFIG_HAVE_OPTION_TABLE),y)

CMOS_LAYOUT_FILE := $(top)/$(call strip_quotes,$(CONFIG_CMOS_LAYOUT_FILE))

cbfs-files-y += cmos_layout.bin
cmos_layout.bin-file = $(obj)/cmos_layout.bin
cmos_layout.bin-type = cmos_layout

$(obj)/cmos_layout.bin: $(NVRAMTOOL) $(CMOS_LAYOUT_FILE)
	@printf "    OPTION     $(subst $(obj)/,,$(@))\n"
	$(NVRAMTOOL) -y $(CMOS_LAYOUT_FILE) -L $@

OPTION_TABLE_H:=$(obj)/option_table.h

$(OPTION_TABLE_H): $(NVRAMTOOL) $(CMOS_LAYOUT_FILE)
	@printf "    OPTION     $(subst $(obj)/,,$(@))\n"
	$(NVRAMTOOL) -y $(CMOS_LAYOUT_FILE) -H $@
endif # CONFIG_HAVE_OPTION_TABLE

stripped_vgabios_id = $(call strip_quotes,$(CONFIG_VGA_BIOS_ID))
cbfs-files-$(CONFIG_VGA_BIOS) += pci$(stripped_vgabios_id).rom
pci$(stripped_vgabios_id).rom-file := $(call strip_quotes,$(CONFIG_VGA_BIOS_FILE))
pci$(stripped_vgabios_id).rom-type := optionrom

stripped_second_vbios_id = $(call strip_quotes,$(CONFIG_VGA_BIOS_SECOND_ID))
cbfs-files-$(CONFIG_VGA_BIOS_SECOND) += pci$(stripped_second_vbios_id).rom
pci$(stripped_second_vbios_id).rom-file := $(call strip_quotes,$(CONFIG_VGA_BIOS_SECOND_FILE))
pci$(stripped_second_vbios_id).rom-type := optionrom

stripped_vgabios_dgpu_id = $(call strip_quotes,$(CONFIG_VGA_BIOS_DGPU_ID))
cbfs-files-$(CONFIG_VGA_BIOS_DGPU) += pci$(stripped_vgabios_dgpu_id).rom
pci$(stripped_vgabios_dgpu_id).rom-file := $(call strip_quotes,$(CONFIG_VGA_BIOS_DGPU_FILE))
pci$(stripped_vgabios_dgpu_id).rom-type := optionrom

# The AMD LPC SPI DMA controller requires source files to be 64 byte aligned.
ifeq ($(CONFIG_SOC_AMD_COMMON_BLOCK_LPC_SPI_DMA),y)
pci$(stripped_vgabios_id).rom-align := 64
pci$(stripped_second_vbios_id).rom-align := 64
pci$(stripped_vgabios_dgpu_id).rom-align := 64
endif # CONFIG_SOC_AMD_COMMON_BLOCK_LPC_SPI_DMA

###############################################################################
# bootblock
###############################################################################

ifeq ($(CONFIG_ARCH_BOOTBLOCK_X86_32)$(CONFIG_ARCH_BOOTBLOCK_X86_64),y)

bootblock-y += boot.c
bootblock-$(CONFIG_DEBUG_HW_BREAKPOINTS_IN_ALL_STAGES) += breakpoint.c
bootblock-y += post.c
bootblock-y += cpu_common.c
bootblock-$(CONFIG_IDT_IN_EVERY_STAGE) += exception.c
bootblock-$(CONFIG_IDT_IN_EVERY_STAGE) += idt.S
bootblock-y += memcpy.c
bootblock-y += memset.c
bootblock-$(CONFIG_ARCH_BOOTBLOCK_X86_32) += memmove_32.c
bootblock-$(CONFIG_ARCH_BOOTBLOCK_X86_64) += memmove_64.S
bootblock-$(CONFIG_COLLECT_TIMESTAMPS_TSC) += timestamp.c
bootblock-$(CONFIG_X86_TOP4G_BOOTMEDIA_MAP) += mmap_boot.c
bootblock-$(CONFIG_DEBUG_NULL_DEREF_BREAKPOINTS_IN_ALL_STAGES) += null_breakpoint.c
bootblock-$(CONFIG_DEBUG_STACK_OVERFLOW_BREAKPOINTS_IN_ALL_STAGES) += stack_canary_breakpoint.c
bootblock-$(CONFIG_BOOTBLOCK_NORMAL) += bootblock_normal.c
bootblock-y += gdt_init.S
bootblock-y += id.S
bootblock-$(CONFIG_HAVE_CF9_RESET) += cf9_reset.c
bootblock-y += bootblock.ld
bootblock-y += car.ld

$(call src-to-obj,bootblock,$(dir)/id.S): $(obj)/build.h

$(eval $(call link_stage,bootblock))

ifeq ($(CONFIG_BOOTBLOCK_IN_CBFS),y)
add_bootblock = \
	$(CBFSTOOL) $(1) add -f $(2) -n bootblock -t bootblock $(TXTIBB) \
	-b -$(call file-size,$(2)) \
	$(cbfs-autogen-attributes) $(TS_OPTIONS) $(CBFSTOOL_ADD_CMD_OPTIONS)
endif

ifneq ($(CONFIG_CBFS_VERIFICATION),y)
$(call src-to-obj,bootblock,$(dir)/walkcbfs.S): $(obj)/fmap_config.h
bootblock-y += walkcbfs.S
endif

endif # CONFIG_ARCH_BOOTBLOCK_X86_32 / CONFIG_ARCH_BOOTBLOCK_X86_64

###############################################################################
# verstage
###############################################################################

ifeq ($(CONFIG_ARCH_VERSTAGE_X86_32)$(CONFIG_ARCH_VERSTAGE_X86_64),y)

verstage-$(CONFIG_VBOOT_SEPARATE_VERSTAGE) += assembly_entry.S
verstage-y += boot.c
verstage-$(CONFIG_DEBUG_HW_BREAKPOINTS_IN_ALL_STAGES) += breakpoint.c
verstage-y += post.c
verstage-$(CONFIG_VBOOT_SEPARATE_VERSTAGE) += gdt_init.S
verstage-$(CONFIG_IDT_IN_EVERY_STAGE) += exception.c
verstage-$(CONFIG_IDT_IN_EVERY_STAGE) += idt.S
verstage-$(CONFIG_HAVE_CF9_RESET) += cf9_reset.c

verstage-y += cpu_common.c
verstage-y += memset.c
verstage-y += memcpy.c
verstage-$(CONFIG_ARCH_VERSTAGE_X86_32) += memmove_32.c
verstage-$(CONFIG_ARCH_VERSTAGE_X86_64) += memmove_64.S
verstage-$(CONFIG_X86_TOP4G_BOOTMEDIA_MAP) += mmap_boot.c
verstage-$(CONFIG_DEBUG_NULL_DEREF_BREAKPOINTS_IN_ALL_STAGES) += null_breakpoint.c
verstage-$(CONFIG_DEBUG_STACK_OVERFLOW_BREAKPOINTS_IN_ALL_STAGES) += stack_canary_breakpoint.c
# If verstage is a separate stage it means there's no need
# for a chipset-specific car_stage_entry() so use the generic one
# which just calls verstage().
verstage-$(CONFIG_VBOOT_SEPARATE_VERSTAGE) += verstage.c

verstage-$(CONFIG_COLLECT_TIMESTAMPS_TSC) += timestamp.c

verstage-y += car.ld

verstage-libs ?=

$(eval $(call link_stage,verstage))

endif # CONFIG_ARCH_VERSTAGE_X86_32 / CONFIG_ARCH_VERSTAGE_X86_64

###############################################################################
# romstage
###############################################################################

ifeq ($(CONFIG_ARCH_ROMSTAGE_X86_32)$(CONFIG_ARCH_ROMSTAGE_X86_64),y)

romstage-$(CONFIG_SEPARATE_ROMSTAGE) += assembly_entry.S
romstage-$(CONFIG_SEPARATE_ROMSTAGE) += romstage.c
romstage-y += boot.c
romstage-$(CONFIG_DEBUG_HW_BREAKPOINTS_IN_ALL_STAGES) += breakpoint.c
romstage-y += post.c
romstage-$(CONFIG_SEPARATE_ROMSTAGE) += gdt_init.S
romstage-y += cpu_common.c
romstage-$(CONFIG_IDT_IN_EVERY_STAGE) += exception.c
romstage-$(CONFIG_IDT_IN_EVERY_STAGE) += idt.S
romstage-y += memcpy.c
romstage-$(CONFIG_ARCH_ROMSTAGE_X86_32) += memmove_32.c
romstage-$(CONFIG_ARCH_ROMSTAGE_X86_64) += memmove_64.S
romstage-y += memset.c
romstage-$(CONFIG_X86_TOP4G_BOOTMEDIA_MAP) += mmap_boot.c
romstage-$(CONFIG_DEBUG_NULL_DEREF_BREAKPOINTS_IN_ALL_STAGES) += null_breakpoint.c
romstage-$(CONFIG_DEBUG_STACK_OVERFLOW_BREAKPOINTS_IN_ALL_STAGES) += stack_canary_breakpoint.c
romstage-y += postcar_loader.c
romstage-$(CONFIG_COLLECT_TIMESTAMPS_TSC) += timestamp.c
romstage-$(CONFIG_HAVE_CF9_RESET) += cf9_reset.c
romstage-$(CONFIG_COOP_MULTITASKING) += thread.c
romstage-$(CONFIG_COOP_MULTITASKING) += thread_switch.S
romstage-y += car.ld

romstage-srcs += $(wildcard $(src)/mainboard/$(MAINBOARDDIR)/romstage.c)
romstage-libs ?=

$(eval $(call link_stage,romstage))

# Compiling crt0 with -g seems to trigger https://sourceware.org/bugzilla/show_bug.cgi?id=6428
romstage-S-ccopts += -g0

endif # CONFIG_ARCH_ROMSTAGE_X86_32 / CONFIG_ARCH_ROMSTAGE_X86_64

###############################################################################
# postcar
###############################################################################

ifeq ($(CONFIG_ARCH_POSTCAR_X86_32),y)
$(eval $(call create_class_compiler,postcar,x86_32))
else
$(eval $(call create_class_compiler,postcar,x86_64))
endif
postcar-generic-ccopts += -D__POSTCAR__

postcar-y += boot.c
postcar-$(CONFIG_DEBUG_HW_BREAKPOINTS_IN_ALL_STAGES) += breakpoint.c
postcar-y += post.c
postcar-y += gdt_init.S
postcar-y += cpu_common.c
postcar-$(CONFIG_IDT_IN_EVERY_STAGE) += exception.c
postcar-$(CONFIG_IDT_IN_EVERY_STAGE) += idt.S
postcar-y += exit_car.S
postcar-y += memcpy.c
postcar-$(CONFIG_ARCH_POSTCAR_X86_32) += memmove_32.c
postcar-$(CONFIG_ARCH_POSTCAR_X86_64) += memmove_64.S
postcar-y += memset.c
postcar-$(CONFIG_X86_TOP4G_BOOTMEDIA_MAP) += mmap_boot.c
postcar-$(CONFIG_DEBUG_NULL_DEREF_BREAKPOINTS_IN_ALL_STAGES) += null_breakpoint.c
postcar-$(CONFIG_DEBUG_STACK_OVERFLOW_BREAKPOINTS_IN_ALL_STAGES) += stack_canary_breakpoint.c
postcar-y += postcar.c
postcar-$(CONFIG_COLLECT_TIMESTAMPS_TSC) += timestamp.c
postcar-$(CONFIG_HAVE_CF9_RESET) += cf9_reset.c

LDFLAGS_postcar += -Map $(objcbfs)/postcar.map

$(eval $(call link_stage,postcar))

$(objcbfs)/postcar.elf: $(objcbfs)/postcar.debug.rmod
	cp $< $@

# Add postcar to CBFS
cbfs-files-$(CONFIG_POSTCAR_STAGE)  += $(CONFIG_CBFS_PREFIX)/postcar
$(CONFIG_CBFS_PREFIX)/postcar-file := $(objcbfs)/postcar.elf
$(CONFIG_CBFS_PREFIX)/postcar-type := stage
$(CONFIG_CBFS_PREFIX)/postcar-compression := none

###############################################################################
# ramstage
###############################################################################

ifeq ($(CONFIG_ARCH_RAMSTAGE_X86_32)$(CONFIG_ARCH_RAMSTAGE_X86_64),y)

ramstage-y += acpi.c
ramstage-$(CONFIG_HAVE_ACPI_RESUME) += acpi_s3.c
ramstage-$(CONFIG_ACPI_BERT) += acpi_bert_storage.c
ramstage-y += boot.c
ramstage-y += post.c
ramstage-y += c_start.S
ramstage-y += cpu.c
ramstage-y += cpu_common.c
ramstage-$(CONFIG_DEBUG_HW_BREAKPOINTS) += breakpoint.c
ramstage-y += ebda.c
ramstage-y += exception.c
ramstage-y += idt.S
ramstage-$(CONFIG_IOAPIC) += ioapic.c
ramstage-y += dma.c
ramstage-y += memcpy.c
ramstage-$(CONFIG_ARCH_RAMSTAGE_X86_32) += memmove_32.c
ramstage-$(CONFIG_ARCH_RAMSTAGE_X86_64) += memmove_64.S
ramstage-y += memset.c
ramstage-$(CONFIG_X86_TOP4G_BOOTMEDIA_MAP) += mmap_boot.c
ramstage-$(CONFIG_GENERATE_MP_TABLE) += mpspec.c
ramstage-$(CONFIG_DEBUG_NULL_DEREF_BREAKPOINTS) += null_breakpoint.c
ramstage-$(CONFIG_DEBUG_STACK_OVERFLOW_BREAKPOINTS) += stack_canary_breakpoint.c
ramstage-$(CONFIG_GENERATE_PIRQ_TABLE) += pirq_routing.c
ramstage-y += rdrand.c
ramstage-$(CONFIG_GENERATE_SMBIOS_TABLES) += smbios.c
ramstage-y += tables.c
ramstage-$(CONFIG_COOP_MULTITASKING) += thread.c
ramstage-$(CONFIG_COOP_MULTITASKING) += thread_switch.S
ramstage-$(CONFIG_COLLECT_TIMESTAMPS_TSC) += timestamp.c
ramstage-$(CONFIG_HAVE_ACPI_RESUME) += wakeup.S
ramstage-$(CONFIG_HAVE_CF9_RESET) += cf9_reset.c

rmodules_x86_32-y += memcpy.c
rmodules_x86_32-y += memmove_32.c
rmodules_x86_32-y += memset.c

rmodules_x86_64-y += memcpy.c
rmodules_x86_64-y += memmove_64.S
rmodules_x86_64-y += memset.c

ifeq ($(CONFIG_ARCH_RAMSTAGE_X86_32),y)
target-objcopy=-O elf32-i386 -B i386
endif
ifeq ($(CONFIG_ARCH_RAMSTAGE_X86_64),y)
target-objcopy=-O elf64-x86-64 -B i386:x86-64
endif

# Make sure generated code does not use XMMx and MMx registers
CFLAGS_x86_32 += -mno-mmx -mno-sse
CFLAGS_x86_64 += -mno-mmx -mno-sse

ramstage-srcs += $(wildcard src/mainboard/$(MAINBOARDDIR)/mainboard.c)
ifeq ($(CONFIG_GENERATE_MP_TABLE),y)
ifneq ($(wildcard src/mainboard/$(MAINBOARDDIR)/mptable.c),)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/mptable.c
endif
endif
ifeq ($(CONFIG_GENERATE_PIRQ_TABLE),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/irq_tables.c
endif

ramstage-libs ?=

$(eval $(call link_stage,ramstage))

$(objcbfs)/ramstage.elf: $(objcbfs)/ramstage.debug.rmod
	cp $< $@

endif # CONFIG_ARCH_RAMSTAGE_X86_32 / CONFIG_ARCH_RAMSTAGE_X86_64

smm-$(CONFIG_DEBUG_HW_BREAKPOINTS_IN_ALL_STAGES) += breakpoint.c
smm-$(CONFIG_IDT_IN_EVERY_STAGE) += exception.c
smm-$(CONFIG_IDT_IN_EVERY_STAGE) += idt.S
smm-y += memcpy.c
smm-$(CONFIG_ARCH_RAMSTAGE_X86_32) += memmove_32.c
smm-$(CONFIG_ARCH_RAMSTAGE_X86_64) += memmove_64.S
smm-y += memset.c
smm-$(CONFIG_X86_TOP4G_BOOTMEDIA_MAP) += mmap_boot.c
smm-$(CONFIG_DEBUG_NULL_DEREF_BREAKPOINTS_IN_ALL_STAGES) += null_breakpoint.c

smm-srcs += $(wildcard src/mainboard/$(MAINBOARDDIR)/smihandler.c)

ifneq ($(CONFIG_HAVE_CONFIGURABLE_APMC_SMI_PORT),y)
ramstage-y += apmc_smi_port.c
smm-y += apmc_smi_port.c
endif
