/* SPDX-License-Identifier: GPL-2.0-only */

#include <boot/coreboot_tables.h>
#include <commonlib/coreboot_tables.h>
#include <drivers/option/cfr_frontend.h>
#include <inttypes.h>
#include <intelblocks/pcie_rp.h>
#include <string.h>
#include <types.h>
#include <variants.h>

static const struct sm_object boot_option = SM_DECLARE_ENUM({
	.opt_name	= "boot_option",
	.ui_name	= "Boot Option",
	.ui_helptext	= "Change the boot device in the event of a failed boot",
	.default_value	= 0,
	.values		= (const struct sm_enum_value[]) {
				{ "Fallback",		0		},
				{ "Normal",		1		},
				SM_ENUM_VALUE_END			},
});

static const struct sm_object debug_level = SM_DECLARE_ENUM({
	.opt_name	= "debug_level",
	.ui_name	= "Debug Level",
	.ui_helptext	= "Set the verbosity of the debug output.",
	.default_value	= 0,
	.values		= (const struct sm_enum_value[]) {
				{ "Emergency",		0		},
				{ "Alert",		1		},
				{ "Critical",		2		},
				{ "Error",		3		},
				{ "Warning",		4		},
				{ "Notice",		5		},
				{ "Info",		6		},
				{ "Debug",		7		},
				{ "Spew",		8		},
				SM_ENUM_VALUE_END			},
});

#if CONFIG(SOC_INTEL_TIGERLAKE) || CONFIG(SOC_INTEL_ALDERLAKE) || CONFIG(SOC_INTEL_RAPTORLAKE)
static const struct sm_object gna = SM_DECLARE_BOOL({
	.opt_name	= "gna",
	.ui_name	= "Gaussian & Neural Accelerator",
	.ui_helptext	= "Enable or Disable the Gaussian & Neural Accelerator",
	.default_value	= false,
});
#endif

static const struct sm_object me_state = SM_DECLARE_ENUM({
	.opt_name	= "me_state",
	.ui_name	= "Intel Management Engine",
	.ui_helptext	= "Enable or disable the Intel Management Engine",
	.default_value	= 1,
	.values		= (const struct sm_enum_value[]) {
				{ "Disabled",		1		},
				{ "Enabled",		0		},
				SM_ENUM_VALUE_END			},
});

static const struct sm_object me_state_counter = SM_DECLARE_NUMBER({
	.opt_name	= "me_state_counter",
	.ui_name	= "ME State Counter",
	.flags		= CFR_OPTFLAG_SUPPRESS,
	.default_value	= 0,
});

static const struct sm_object power_on_after_fail = SM_DECLARE_BOOL({
	.opt_name	= "power_on_after_fail",
	.ui_name	= "Power on after failure",
	.ui_helptext	= "Automatically turn on after a power failure",
	.default_value	= false,
});

static const struct sm_object power_profile = SM_DECLARE_ENUM({
	.opt_name	= "power_profile",
	.ui_name	= "Power Profile",
	.ui_helptext	= "Select whether to maximize performance, battery life or both.",
	.default_value	= 1,
	.values		= (const struct sm_enum_value[]) {
				{ "Power Saver",	PP_POWER_SAVER	},
				{ "Balanced",		PP_BALANCED	},
				{ "Performance",	PP_PERFORMANCE	},
				SM_ENUM_VALUE_END			},
});

#if CONFIG(SOC_INTEL_ALDERLAKE)
static const struct sm_object pciexp_aspm = SM_DECLARE_ENUM({
	.opt_name	= "pciexp_aspm",
	.ui_name	= "PCI ASPM",
	.ui_helptext	= "Controls the Active State Power Management for PCI devices."
			  " Enabling this feature can reduce power consumption of"
			  " PCI-connected devices during idle times.",
	.default_value	= ASPM_L0S_L1,
	.values		= (const struct sm_enum_value[]) {
				{ "Disabled",		ASPM_DISABLE	},
				{ "L0s",		ASPM_L0S	},
				{ "L1",			ASPM_L1		},
				{ "L0sL1",		ASPM_L0S_L1	},
				SM_ENUM_VALUE_END			},
});

static const struct sm_object pciexp_clk_pm = SM_DECLARE_BOOL({
	.opt_name	= "pciexp_clk_pm",
	.ui_name	= "PCI Clock Power Management",
	.ui_helptext	= "Enables or disables power management for the PCI clock. When"
			  " enabled, it reduces power consumption during idle states."
			  " This can help lower overall energy use but may impact"
			  " performance in power-sensitive tasks.",
	.default_value	= true,
});

static const struct sm_object pciexp_l1ss = SM_DECLARE_ENUM({
	.opt_name	= "pciexp_l1ss",
	.ui_name	= "PCI L1 Substates",
	.ui_helptext	= "Controls deeper power-saving states for PCI devices."
			  " Enabling this feature allows supported devices to achieve"
			  " lower power states at the cost of slightly increased"
			  " latency when exiting these states.",
	.default_value	= L1_SS_L1_2,
	.values		= (const struct sm_enum_value[]) {
				{ "Disabled",		L1_SS_DISABLED	},
				{ "L1.1",		L1_SS_L1_1	},
				{ "L1.2",		L1_SS_L1_2	},
				SM_ENUM_VALUE_END			},
});
#endif

static const struct sm_object reboot_counter = SM_DECLARE_NUMBER({
	.opt_name	= "reboot_counter",
	.ui_name	= "Reboot Counter",
	.flags		= CFR_OPTFLAG_SUPPRESS,
	.default_value	= 0,
});

static const struct sm_object vtd = SM_DECLARE_BOOL({
	.opt_name	= "vtd",
	.ui_name	= "VT-d",
	.ui_helptext	= "Enable or disable Intel VT-d (virtualization)",
	.default_value	= true,
});

static struct sm_obj_form performance = {
	.ui_name = "Performance",
	.obj_list = (const struct sm_object *[]) {
		&power_profile,
		NULL
	},
};

static struct sm_obj_form processor = {
	.ui_name = "Processor",
	.obj_list = (const struct sm_object *[]) {
		&me_state,
		&me_state_counter,
		&vtd,
		NULL
	},
};

static struct sm_obj_form power = {
	.ui_name = "Power",
	.obj_list = (const struct sm_object *[]) {
		&power_on_after_fail,
		NULL
	},
};

static struct sm_obj_form devices = {
	.ui_name = "Devices",
	.obj_list = (const struct sm_object *[]) {
		#if CONFIG(SOC_INTEL_TIGERLAKE) || CONFIG(SOC_INTEL_ALDERLAKE) || CONFIG(SOC_INTEL_RAPTORLAKE)
		&gna,
		#endif
		NULL
	},
};

static struct sm_obj_form pci = {
	.ui_name = "PCI",
	.obj_list = (const struct sm_object *[]) {
		#if CONFIG(SOC_INTEL_ALDERLAKE)
		&pciexp_clk_pm,
		&pciexp_aspm,
		&pciexp_l1ss,
		#endif
		NULL
	},
};

static struct sm_obj_form coreboot = {
	.ui_name = "coreboot",
	.obj_list = (const struct sm_object *[]) {
		&boot_option,
		&debug_level,
		&reboot_counter,
		NULL
	},
};

static struct sm_obj_form *sm_root[] = {
	&performance,
	&processor,
	&power,
	&devices,
	&pci,
	&coreboot,
	NULL
};

void mb_cfr_setup_menu(struct lb_cfr *cfr_root)
{
	cfr_write_setup_menu(cfr_root, sm_root);
}
