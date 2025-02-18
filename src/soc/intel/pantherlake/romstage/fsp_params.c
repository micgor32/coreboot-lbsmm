/* SPDX-License-Identifier: GPL-2.0-only */

#include <cpu/intel/common/common.h>
#include <cpu/x86/msr.h>
#include <fsp/debug.h>
#include <fsp/fsp_debug_event.h>
#include <fsp/util.h>
#include <intelblocks/cpulib.h>
#include <soc/iomap.h>
#include <soc/msr.h>
#include <soc/pcie.h>
#include <soc/romstage.h>
#include <static.h>

#define FSP_CLK_NOTUSED		0xff
#define FSP_CLK_LAN		0x70
#define FSP_CLK_FREE_RUNNING	0x80

static void fill_fspm_igd_params(FSP_M_CONFIG *m_cfg,
				 const struct soc_intel_pantherlake_config *config)
{
	const struct ddi_port_upds {
		uint8_t *ddc;
		uint8_t *hpd;
	} ddi_port_upds[] = {
		[DDI_PORT_A] = {&m_cfg->DdiPortADdc, &m_cfg->DdiPortAHpd},
		[DDI_PORT_B] = {&m_cfg->DdiPortBDdc, &m_cfg->DdiPortBHpd},
		[DDI_PORT_C] = {&m_cfg->DdiPortCDdc, &m_cfg->DdiPortCHpd},
		[DDI_PORT_1] = {&m_cfg->DdiPort1Ddc, &m_cfg->DdiPort1Hpd},
		[DDI_PORT_2] = {&m_cfg->DdiPort2Ddc, &m_cfg->DdiPort2Hpd},
		[DDI_PORT_3] = {&m_cfg->DdiPort3Ddc, &m_cfg->DdiPort3Hpd},
		[DDI_PORT_4] = {&m_cfg->DdiPort4Ddc, &m_cfg->DdiPort4Hpd},
	};
	m_cfg->InternalGraphics = !CONFIG(SOC_INTEL_DISABLE_IGD) && is_devfn_enabled(PCI_DEVFN_IGD);
	if (m_cfg->InternalGraphics) {
		/* IGD is enabled, set IGD stolen size to 128MB. */
		m_cfg->IgdDvmt50PreAlloc = IGD_SM_128MB;
		/* DP port config */
		m_cfg->DdiPortAConfig = config->ddi_port_A_config;
		m_cfg->DdiPortBConfig = config->ddi_port_B_config;
		for  (size_t i = 0; i < ARRAY_SIZE(ddi_port_upds); i++) {
			*ddi_port_upds[i].ddc = !!(config->ddi_ports_config[i] &
						   DDI_ENABLE_DDC);
			*ddi_port_upds[i].hpd = !!(config->ddi_ports_config[i] &
						   DDI_ENABLE_HPD);
		}
		/* Disable memory bandwidth compression */
		m_cfg->MemoryBandwidthCompression = 0;
	} else {
		/* IGD is disabled, skip IGD init in FSP. */
		m_cfg->IgdDvmt50PreAlloc = 0;
		/* DP port config */
		m_cfg->DdiPortAConfig = 0;
		m_cfg->DdiPortBConfig = 0;
		for  (size_t i = 0; i < ARRAY_SIZE(ddi_port_upds); i++) {
			*ddi_port_upds[i].ddc = 0;
			*ddi_port_upds[i].hpd = 0;
		}
	}
}

static void fill_fspm_mrc_params(FSP_M_CONFIG *m_cfg,
				 const struct soc_intel_pantherlake_config *config)
{
	m_cfg->SaGv = config->sagv;
	if (m_cfg->SaGv) {
		/*
		 * Set SaGv work points after reviewing the power and performance impact
		 * with SaGv set to 1 (Enabled) and various work points between 0-3 being
		 * enabled.
		 */
		if (config->sagv_wp_bitmap)
			m_cfg->SaGvWpMask = config->sagv_wp_bitmap;
		else
			m_cfg->SaGvWpMask = SAGV_POINTS_0_1_2_3;
	}

	if (config->max_dram_speed_mts)
		m_cfg->DdrFreqLimit = config->max_dram_speed_mts;

	m_cfg->RMT = config->rmt;
	m_cfg->MrcFastBoot = 1;
	m_cfg->LowerBasicMemTestSize = config->lower_basic_mem_test_size;
}

static void fill_fspm_cpu_params(FSP_M_CONFIG *m_cfg,
				 const struct soc_intel_pantherlake_config *config)
{
	/* CpuRatio Settings */
	if (config->cpu_ratio_override)
		m_cfg->CpuRatio = config->cpu_ratio_override;
	else
		/* Set CpuRatio to match existing MSR value */
		m_cfg->CpuRatio = (rdmsr(MSR_FLEX_RATIO).lo >> 8) & 0xff;

	m_cfg->PrmrrSize = get_valid_prmrr_size();
	m_cfg->TsegSize = CONFIG_SMM_TSEG_SIZE;
	m_cfg->SmmRelocationEnable = 0;
}

static void fill_fspm_security_params(FSP_M_CONFIG *m_cfg,
				      const struct soc_intel_pantherlake_config *config)
{
	m_cfg->TmeEnable = CONFIG(INTEL_TME) && is_tme_supported();
}

static void fill_fspm_uart_params(FSP_M_CONFIG *m_cfg,
				  const struct soc_intel_pantherlake_config *config)
{
	if (CONFIG(DRIVERS_UART_8250IO))
		m_cfg->PcdIsaSerialUartBase = ISA_SERIAL_BASE_ADDR_3F8;
	m_cfg->SerialIoUartDebugMode = PchSerialIoSkipInit;
	m_cfg->SerialIoUartDebugControllerNumber = CONFIG_UART_FOR_CONSOLE;
}

static void fill_fspm_ipu_params(FSP_M_CONFIG *m_cfg,
				 const struct soc_intel_pantherlake_config *config)
{
	/* IPU */
	m_cfg->SaIpuEnable = is_devfn_enabled(PCI_DEVFN_IPU);
}

static void fill_fspm_smbus_params(FSP_M_CONFIG *m_cfg,
				   const struct soc_intel_pantherlake_config *config)
{
	m_cfg->SmbusEnable = is_devfn_enabled(PCI_DEVFN_SMBUS);
}

static void fill_fspm_misc_params(FSP_M_CONFIG *m_cfg,
				  const struct soc_intel_pantherlake_config *config)
{
	/* Skip CPU replacement check */
	m_cfg->SkipCpuReplacementCheck = !config->cpu_replacement_check;

	/* Skip GPIO configuration from FSP */
	m_cfg->GpioOverride = 1;

	/* Skip MBP HOB */
	m_cfg->SkipMbpHob = !CONFIG(FSP_PUBLISH_MBP_HOB);

	m_cfg->SkipExtGfxScan = config->skip_ext_gfx_scan;

	m_cfg->DlvrRfiEnable = 1;
}

static void fill_fspm_audio_params(FSP_M_CONFIG *m_cfg,
				   const struct soc_intel_pantherlake_config *config)
{
	/* Audio: HDAUDIO_LINK_MODE I2S/SNDW */
	m_cfg->PchHdaEnable = is_devfn_enabled(PCI_DEVFN_HDA);
	m_cfg->PchHdaDspEnable = config->pch_hda_dsp_enable;
	m_cfg->PchHdaIDispLinkTmode = config->pch_hda_idisp_link_tmode;
	m_cfg->PchHdaIDispLinkFrequency = config->pch_hda_idisp_link_frequency;
	m_cfg->PchHdaIDispCodecDisconnect = !config->pch_hda_idisp_codec_enable;

	for (int i = 0; i < MAX_HD_AUDIO_SDI_LINKS; i++)
		m_cfg->PchHdaSdiEnable[i] = !!config->pch_hda_sdi_enable[i];

	/*
	 * All the PchHdaAudioLink{Hda|Dmic|Ssp|Sndw}Enable UPDs are used by FSP
	 * only to configure GPIO pads for audio. Mainboard is expected to
	 * perform all GPIO configuration in coreboot and hence these UPDs are
	 * set to 0 to skip FSP GPIO configuration for audio pads.
	 */
	m_cfg->PchHdaAudioLinkHdaEnable = 1;
	memset(m_cfg->PchHdaAudioLinkDmicEnable, 0, sizeof(m_cfg->PchHdaAudioLinkDmicEnable));
	memset(m_cfg->PchHdaAudioLinkSspEnable, 0, sizeof(m_cfg->PchHdaAudioLinkSspEnable));
	memset(m_cfg->PchHdaAudioLinkSndwEnable, 0, sizeof(m_cfg->PchHdaAudioLinkSndwEnable));
}

static void pcie_rp_init(FSP_M_CONFIG *m_cfg, uint32_t en_mask,
			 const struct pcie_rp_config *cfg, size_t cfg_count)
{
	unsigned int clk_req_mapping = 0;
	for (size_t i = 0; i < cfg_count; i++) {
		if (CONFIG(SOC_INTEL_COMPLIANCE_TEST_MODE)) {
			m_cfg->PcieClkSrcUsage[i] = FSP_CLK_FREE_RUNNING;
			continue;
		}
		if (!(en_mask & BIT(i)))
			continue;
		if (!cfg[i].flags && !cfg[i].clk_src && !cfg[i].clk_req) {
			printk(BIOS_WARNING, "Missing root port clock structure definition\n");
			continue;
		}
		if (clk_req_mapping & (1 << cfg[i].clk_req))
			printk(BIOS_WARNING, "Found overlapped clkreq assignment on clk req %d\n",
			       cfg[i].clk_req);
		if (!(cfg[i].flags & PCIE_RP_CLK_REQ_UNUSED)) {
			m_cfg->PcieClkSrcClkReq[cfg[i].clk_src] = cfg[i].clk_req;
			clk_req_mapping |= 1 << cfg[i].clk_req;
		}

		m_cfg->PcieClkSrcUsage[cfg[i].clk_src] = i;
	}
}

static void fill_fspm_pcie_rp_params(FSP_M_CONFIG *m_cfg,
				     const struct soc_intel_pantherlake_config *config)
{
	/* Disable all PCIe clock sources by default. And set RP irrelevant clock. */
	for (size_t i = 0; i < CONFIG_MAX_PCIE_CLOCK_SRC; i++) {
		if (config->pcie_clk_config_flag[i] & PCIE_CLK_FREE_RUNNING)
			m_cfg->PcieClkSrcUsage[i] = FSP_CLK_FREE_RUNNING;
		else if (config->pcie_clk_config_flag[i] & PCIE_CLK_LAN)
			m_cfg->PcieClkSrcUsage[i] = FSP_CLK_LAN;
		else
			m_cfg->PcieClkSrcUsage[i] = FSP_CLK_NOTUSED;
		m_cfg->PcieClkSrcClkReq[i] = FSP_CLK_NOTUSED;
	}

	/* PCIE ports */
	m_cfg->PcieRpEnableMask = pcie_rp_enable_mask(get_pcie_rp_table());
	pcie_rp_init(m_cfg, m_cfg->PcieRpEnableMask, config->pcie_rp,
		     CONFIG_MAX_ROOT_PORTS);
}

static void fill_fspm_ish_params(FSP_M_CONFIG *m_cfg,
				 const struct soc_intel_pantherlake_config *config)
{
	m_cfg->PchIshEnable = is_devfn_enabled(PCI_DEVFN_ISH);
}

static void fill_fspm_tcss_params(FSP_M_CONFIG *m_cfg,
				  const struct soc_intel_pantherlake_config *config)
{
	/* Tcss USB */
	m_cfg->TcssXhciEn = is_devfn_enabled(PCI_DEVFN_TCSS_XHCI);

	/* Enable TCSS port */
	m_cfg->TcssPort0 = config->tcss_cap_policy[0];
	m_cfg->TcssPort1 = config->tcss_cap_policy[1];
	m_cfg->TcssPort2 = config->tcss_cap_policy[2];
	m_cfg->TcssPort3 = config->tcss_cap_policy[3];
}

static void fill_fspm_vtd_params(FSP_M_CONFIG *m_cfg,
				 const struct soc_intel_pantherlake_config *config)
{
	m_cfg->VtdDisable = 0;
	m_cfg->VtdBaseAddress[0] = GFXVT_BASE_ADDRESS;
	m_cfg->VtdBaseAddress[1] = VTVC0_BASE_ADDRESS;
	m_cfg->VtdBaseAddress[2] = IOCVTD_BASE_ADDRESS;

	/* Change VmxEnable UPD value according to ENABLE_VMX Kconfig */
	m_cfg->VmxEnable = CONFIG(ENABLE_VMX);
}

static void fill_fspm_trace_params(FSP_M_CONFIG *m_cfg,
				   const struct soc_intel_pantherlake_config *config)
{
	m_cfg->CpuCrashLogEnable = CONFIG(SOC_INTEL_CRASHLOG);

	if (!CONFIG(SOC_INTEL_COMMON_BLOCK_TRACEHUB))
		return;

	m_cfg->PlatformDebugOption = CONFIG_SOC_INTEL_COMMON_DEBUG_CONSENT;

	switch (CONFIG_SOC_INTEL_COMMON_DEBUG_CONSENT) {
	case HW_DEBUG_TRACEHUB_ACTIVE:
	case HW_DEBUG_TRACEHUB_READY:
	case HW_DEBUG_TRACEHUB_POWEROFF:
		m_cfg->DciEn = 1;
		break;
	case HW_DEBUG_DISABLE:
		m_cfg->DciEn = 0;
		break;
	}
}

static void fill_fspm_thermal_params(FSP_M_CONFIG *m_cfg,
				     const struct soc_intel_pantherlake_config *config)
{
	m_cfg->TccActivationOffset = config->tcc_offset;
}

static void fill_fspm_vr_config_params(FSP_M_CONFIG *m_cfg,
				       const struct soc_intel_pantherlake_config *config)
{
	for (size_t i = 0; i < ARRAY_SIZE(m_cfg->EnableFastVmode); i++)
		m_cfg->EnableFastVmode[i] = 0;
}

static void soc_memory_init_params(FSP_M_CONFIG *m_cfg,
				   const struct soc_intel_pantherlake_config *config)
{
	void (*fill_fspm_params[])(FSP_M_CONFIG *m_cfg,
				   const struct soc_intel_pantherlake_config *config) = {
		fill_fspm_igd_params,
		fill_fspm_mrc_params,
		fill_fspm_cpu_params,
		fill_fspm_security_params,
		fill_fspm_uart_params,
		fill_fspm_ipu_params,
		fill_fspm_smbus_params,
		fill_fspm_misc_params,
		fill_fspm_audio_params,
		fill_fspm_pcie_rp_params,
		fill_fspm_ish_params,
		fill_fspm_tcss_params,
		fill_fspm_vtd_params,
		fill_fspm_trace_params,
		fill_fspm_thermal_params,
		fill_fspm_vr_config_params,
	};

	for (size_t i = 0; i < ARRAY_SIZE(fill_fspm_params); i++)
		fill_fspm_params[i](m_cfg, config);
}

static void fsp_set_debug_level(FSP_M_CONFIG *m_cfg,
	 enum fsp_log_level fsp_pcd_log_level, enum fsp_log_level fsp_mrc_log_level)
{
	/* Set Serial debug message level */
	m_cfg->PcdSerialDebugLevel = fsp_pcd_log_level;
	/* Set MRC debug level */
	m_cfg->SerialDebugMrcLevel = fsp_mrc_log_level;
}

static void fsp_control_log_level(FSPM_UPD *mupd, bool is_enabled)
{
	FSP_M_CONFIG *m_cfg = &mupd->FspmConfig;
	FSPM_ARCHx_UPD *arch_upd = &mupd->FspmArchUpd;

	enum fsp_log_level fsp_log_level = is_enabled ? fsp_get_pcd_debug_log_level() :
			 FSP_LOG_LEVEL_DISABLE;
	enum fsp_log_level mrc_log_level = is_enabled ? fsp_get_mrc_debug_log_level() :
			 FSP_LOG_LEVEL_DISABLE;

	fsp_set_debug_level(m_cfg, fsp_log_level, mrc_log_level);

	if ((m_cfg->PcdSerialDebugLevel > FSP_LOG_LEVEL_VERBOSE) ||
	    (m_cfg->SerialDebugMrcLevel > FSP_LOG_LEVEL_VERBOSE)) {
		printk(BIOS_ERR, "Invalid FSP log level\n");
		return;
	}

	/* Set Event Handler if log-level is non-zero */
	if (m_cfg->PcdSerialDebugLevel || m_cfg->SerialDebugMrcLevel)
		arch_upd->FspEventHandler = (uintptr_t)((FSP_EVENT_HANDLER *)fsp_debug_event_handler);
}

static void fill_fsp_event_handler(FSPM_UPD *mupd)
{
	bool fsp_debug_enable = false;

	if (CONFIG(CONSOLE_SERIAL) && CONFIG(FSP_ENABLE_SERIAL_DEBUG))
		fsp_debug_enable = true;

	fsp_control_log_level(mupd, fsp_debug_enable);
}

void platform_fsp_memory_init_params_cb(FSPM_UPD *mupd, uint32_t version)
{
	const struct soc_intel_pantherlake_config *config = config_of_soc();

	if (CONFIG(FSP_USES_CB_DEBUG_EVENT_HANDLER))
		fill_fsp_event_handler(mupd);

	soc_memory_init_params(&mupd->FspmConfig, config);
	mainboard_memory_init_params(mupd);
}

__weak void mainboard_memory_init_params(FSPM_UPD *memupd)
{
	printk(BIOS_DEBUG, "WEAK: %s/%s called\n", __FILE__, __func__);
}
