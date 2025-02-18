/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __CPU_INTEL_MODEL_206AX_CHIP_H__
#define __CPU_INTEL_MODEL_206AX_CHIP_H__

#include "stdbool.h"

/* Keep this in sync with acpi.c */
enum cpu_acpi_level {
	CPU_ACPI_DISABLED = 0,
	CPU_ACPI_C1,
	CPU_ACPI_C2,
	CPU_ACPI_C3,
	CPU_ACPI_C6,
	CPU_ACPI_C7,
	CPU_ACPI_C7S,
};

/* VR12 PSI codes */
enum vr12_phases {
	VR12_KEEP_DEFAULT = 0,	/* For device-trees missing the setting */
	VR12_ALL_PHASES,
	VR12_2_PHASES,
	VR12_1_PHASE,
	VR12_LIGHT_LOAD,
};

/* VR12 power state listing */
enum vr12_psi {
	VR12_PSI1 = 0,
	VR12_PSI2,
	VR12_PSI3,
	VR12_PSI_MAX,
};

struct psi_state {
	enum vr12_phases phases;
	int current;			/* In Amps */
};

union turbo_ratio_limits {
		/* Limit for 1, 2, 3 and 4 active cores respectively */
		struct {
			int limit_1c;
			int limit_2c;
			int limit_3c;
			int limit_4c;
		};
		int raw[4];
};

struct cpu_intel_model_206ax_config {
	enum cpu_acpi_level acpi_c1;
	enum cpu_acpi_level acpi_c2;
	enum cpu_acpi_level acpi_c3;

	int tcc_offset;		/* TCC Activation Offset */

	unsigned int pl1_mw; /* Long-term power limit in milliwatts */
	bool pl1_clamp; /* Long-term power limit clamping limitation */
	unsigned int pl2_mw; /* Short-term power limit in milliwatts */
	bool pl2_clamp; /* Short-term power limit clamping limitation */

	int pp0_current_limit;	/* Primary Plane Current Limit (Icc) in Amps */
	int pp1_current_limit;	/* Secondary Plane Current Limit (IAXG) in Amps */

	/* PSI states only have an effect when in Package C3 or higher */
	struct psi_state pp0_psi[3];	/* Power states for Primary Plane (Icc) */
	struct psi_state pp1_psi[3];	/* Power states for Secondary Plane (IAXG) */

	union turbo_ratio_limits turbo_limits; /* Turbo ratio limits depending on the number of active cores */
};

#endif /* __CPU_INTEL_MODEL_206AX_CHIP_H__ */
