/* SPDX-License-Identifier: GPL-2.0-only OR MIT */

#include <fw_config.h>
#include <gpio.h>
#include <soc/ddp.h>
#include <soc/dsi.h>

#include "gpio.h"
#include "panel.h"

/* Set up backlight control pins as output pin and power-off by default */
static void configure_backlight(void)
{
	gpio_output(GPIO_AP_EDP_BKLTEN, 0);
	gpio_output(GPIO_BL_PWM_1V8, 0);
}

static void power_on_panel(void)
{
	gpio_set_mode(GPIO_EDP_HPD_1V8, GPIO_FUNC(EINT13, EDP_TX_HPD));
}

static struct panel_description panel = {
	.configure_backlight = configure_backlight,
	.power_on = power_on_panel,
	.disp_path = DISP_PATH_EDP,
	.orientation = LB_FB_ORIENTATION_NORMAL,
};

static void power_on_oled_panel(void)
{
	gpio_output(GPIO_EN_PP3300_EDP_X, 1);
	gpio_set_mode(GPIO_EDP_HPD_1V8, GPIO_FUNC(EINT13, EDP_TX_HPD));
}

static struct panel_description oled_panel = {
	.configure_backlight = configure_backlight,
	.power_on = power_on_oled_panel,
	.disp_path = DISP_PATH_EDP,
	.orientation = LB_FB_ORIENTATION_NORMAL,
};

struct panel_description *get_active_panel(void)
{
	if (fw_config_probe(FW_CONFIG(PANEL, OLED))) {
		return &oled_panel;
	}
	return &panel;
}
