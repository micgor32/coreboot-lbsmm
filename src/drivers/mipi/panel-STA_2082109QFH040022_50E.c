/* SPDX-License-Identifier: GPL-2.0-only */

#include <mipi/panel.h>

struct panel_serializable_data STA_QFH040022_50E = {
	.edid = {
		.ascii_string = "QFH040022_50E",
		.manufacturer_name = "STA",
		.panel_bits_per_color = 8,
		.panel_bits_per_pixel = 24,
		.mode = {
			.pixel_clock = 187097,
			.lvds_dual_channel = 0,
			.refresh = 60,
			.ha = 1200, .hbl = 346, .hso = 160, .hspw = 120,
			.va = 1920, .vbl = 151, .vso = 115, .vspw = 28,
			.phsync = '-', .pvsync = '-',
			.x_mm = 147, .y_mm = 235,
		},
	},

	.init = {
		PANEL_DCS(0xB9, 0x83, 0x10, 0x21, 0x55, 0x00),
		PANEL_DCS(0xE9, 0xC4),
		PANEL_DCS(0xD9, 0xD1),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xB1, 0x2C, 0xB5, 0xB5, 0x31, 0xF1, 0x33, 0xC3, 0x57, 0x36,
			0x36, 0x36, 0x36, 0x1A, 0x8B, 0x11, 0x65, 0x00, 0x88, 0xFA,
			0xFF, 0xFF, 0x8F, 0xFF, 0x08, 0x3C, 0x33),
		PANEL_DCS(0xB2, 0x00, 0x47, 0xB0, 0x80, 0x00, 0x22, 0x70, 0x3C, 0xA1,
			0x22, 0x00, 0x00, 0x00, 0x88, 0xF4),
		PANEL_DCS(0xB4, 0x14, 0x16, 0x14, 0x50, 0x14, 0x50, 0x0D, 0x6A, 0x0D,
			0x6A, 0x01, 0x9E),
		PANEL_DCS(0xB6, 0x34, 0x34, 0x03),
		PANEL_DCS(0xB8, 0x40),
		PANEL_DCS(0xE9, 0xCD),
		PANEL_DCS(0xBA, 0x84),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xBC, 0x1B, 0x04),
		PANEL_DCS(0xBE, 0x20),
		PANEL_DCS(0xBF, 0xFC, 0xC4),
		PANEL_DCS(0xC0, 0x38, 0x38, 0x22, 0x11, 0x33, 0xA0, 0x61, 0x08, 0xF5,
			0x03),
		PANEL_DCS(0xE9, 0xCC),
		PANEL_DCS(0xC7, 0x80),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xE9, 0xC6),
		PANEL_DCS(0xC8, 0x97),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xC9, 0x00, 0x1E, 0x30, 0xD4, 0x01),
		PANEL_DCS(0xCB, 0x08, 0x13, 0x07, 0x00, 0x0F, 0x16),
		PANEL_DCS(0xCC, 0x02, 0x03, 0x44),
		PANEL_DCS(0xE9, 0xC4),
		PANEL_DCS(0xD0, 0x03),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xD1, 0x37, 0x06, 0x00, 0x02, 0x04, 0x2C, 0xFF),
		PANEL_DCS(0xD3, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x3B, 0x03, 0x73, 0x3B, 0x21, 0x21, 0x03, 0x03, 0x98, 0x10,
			0x1D, 0x00, 0x1D, 0x32, 0x17, 0xA1, 0x07, 0xA1, 0x43, 0x17,
			0xA6, 0x07, 0xA6, 0x00, 0x00),
		PANEL_DCS(0xD5, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x40, 0x40, 0x18,
			0x18, 0x18, 0x18, 0x2A, 0x2B, 0x1F, 0x1F, 0x1E, 0x1E, 0x24,
			0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x00, 0x01, 0x02,
			0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x20,
			0x21, 0x18, 0x18, 0x18, 0x18),
		PANEL_DCS(0xD8, 0x02, 0xAA, 0xEA, 0xAA, 0xAA, 0x00, 0x02, 0xAA, 0xEA,
			0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00),
		PANEL_DCS(0xE7, 0x07, 0x10, 0x10, 0x2A, 0x32, 0x9F, 0x01, 0x5A, 0x91,
			0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x12, 0x05, 0x02, 0x02,
			0x10, 0x33, 0x02, 0x04, 0x18, 0x01),
		PANEL_DCS(0xBD, 0x01),
		PANEL_DCS(0xB1, 0x01, 0x7F, 0x11, 0xFD),
		PANEL_DCS(0xCB, 0x86),
		PANEL_DCS(0xD2, 0x3D),
		PANEL_DCS(0xE9, 0xC5),
		PANEL_DCS(0xD3, 0x00, 0x00, 0x00, 0x80, 0x80, 0x0C, 0xA1),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xD8, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x03, 0xFF, 0xFF,
			0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00),
		PANEL_DCS(0xE7, 0x02, 0x00, 0x2D, 0x01, 0x7F, 0x0F, 0x7C, 0x10, 0xA0,
			0x00, 0x00, 0x77, 0x00, 0x00, 0x00),
		PANEL_DCS(0xBD, 0x02),
		PANEL_DCS(0xBF, 0xF2),
		PANEL_DCS(0xCB, 0x02, 0x00, 0x00, 0x10, 0x58),
		PANEL_DCS(0xD2, 0x0A, 0x0A, 0x05, 0x03, 0x0A, 0x0A, 0x01, 0x03, 0x01,
			0x01, 0x05, 0x0E),
		PANEL_DCS(0xE9, 0xCC),
		PANEL_DCS(0xD3, 0x03, 0x1F, 0xE0, 0x11, 0x70),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xD8, 0xAB, 0xFF, 0xFF, 0xFF, 0xFF, 0xA0, 0xAB, 0xFF, 0xFF,
			0xFF, 0xFF, 0xA0),
		PANEL_DCS(0xE7, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0x00, 0x00, 0x00,
			0x03, 0x00, 0x03, 0x81, 0x02, 0x40, 0x00, 0x20, 0x9E, 0x02,
			0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00),
		PANEL_DCS(0xBD, 0x03),
		PANEL_DCS(0xE9, 0xC6),
		PANEL_DCS(0xB4, 0x03, 0xFF, 0xF8),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xD8, 0xAA, 0xAB, 0xEA, 0xAA, 0xAA, 0xA0, 0xAA, 0xAB, 0xEA,
			0xAA, 0xAA, 0xA0, 0xAA, 0xBF, 0xFF, 0xFF, 0xFE, 0xA0, 0xAA,
			0xBF, 0xFF, 0xFF, 0xFE, 0xA0, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
			0xA0, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xA0),
		PANEL_DCS(0xE1, 0x00),
		PANEL_DCS(0xBD, 0x00),
		PANEL_DCS(0xE9, 0xC4),
		PANEL_DCS(0xBA, 0x96),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xBD, 0x01),
		PANEL_DCS(0xE9, 0xC5),
		PANEL_DCS(0xBA, 0x4F),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xBD, 0x02),
		PANEL_DCS(0xE9, 0xCC),
		PANEL_DCS(0xBA, 0x84),
		PANEL_DCS(0xE9, 0x3F),
		PANEL_DCS(0xBD, 0x00),
		PANEL_DCS(0xB9, 0x00, 0x00, 0x00, 0x00),

		PANEL_DCS(0x11),
		PANEL_DELAY(120),
		PANEL_DCS(0x29),
		PANEL_DELAY(5),
		PANEL_END,
	},
};
