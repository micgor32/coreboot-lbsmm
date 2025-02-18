/* SPDX-License-Identifier: GPL-2.0-only */

#include <device/azalia_device.h>

const u32 cim_verb_data[] = {
	0x111d76e7,	/* Codec Vendor / Device ID: IDT */
	0x1028049b,	/* Subsystem ID */
	11,		/* Number of 4 dword sets */
	AZALIA_SUBVENDOR(0, 0x1028049b),
	AZALIA_PIN_CFG(0, 0xa, AZALIA_PIN_DESC(
		AZALIA_JACK,
		AZALIA_EXTERNAL_PRIMARY_CHASSIS | AZALIA_RIGHT,
		AZALIA_MIC_IN,
		AZALIA_STEREO_MONO_1_8,
		AZALIA_BLACK,
		AZALIA_JACK_PRESENCE_DETECT,
		2, 0
	)),
	AZALIA_PIN_CFG(0, 0xb, AZALIA_PIN_DESC(
		AZALIA_JACK,
		AZALIA_EXTERNAL_PRIMARY_CHASSIS | AZALIA_RIGHT,
		AZALIA_HP_OUT,
		AZALIA_STEREO_MONO_1_8,
		AZALIA_BLACK,
		AZALIA_JACK_PRESENCE_DETECT,
		1, 15
	)),
	AZALIA_PIN_CFG(0, 0xc, 0x400000f0),
	AZALIA_PIN_CFG(0, 0xd, AZALIA_PIN_DESC(
		AZALIA_INTEGRATED,
		AZALIA_INTERNAL,
		AZALIA_SPEAKER,
		AZALIA_OTHER_ANALOG,
		AZALIA_COLOR_UNKNOWN,
		AZALIA_NO_JACK_PRESENCE_DETECT,
		1, 0
	)),
	AZALIA_PIN_CFG(0, 0xe, AZALIA_PIN_DESC(
		AZALIA_JACK,
		AZALIA_SEPARATE_CHASSIS | AZALIA_LEFT,
		AZALIA_LINE_OUT,
		AZALIA_STEREO_MONO_1_8,
		AZALIA_BLACK,
		AZALIA_JACK_PRESENCE_DETECT,
		5, 0
	)),
	AZALIA_PIN_CFG(0, 0xf, AZALIA_PIN_DESC(
		AZALIA_JACK,
		AZALIA_SEPARATE_CHASSIS | AZALIA_LEFT,
		AZALIA_MIC_IN,
		AZALIA_STEREO_MONO_1_8,
		AZALIA_BLACK,
		AZALIA_JACK_PRESENCE_DETECT,
		2, 14
	)),
	AZALIA_PIN_CFG(0, 0x10, 0x400000f3),
	AZALIA_PIN_CFG(0, 0x11, 0x400000f0),
	AZALIA_PIN_CFG(0, 0x1f, 0x400000f0),
	AZALIA_PIN_CFG(0, 0x20, AZALIA_PIN_DESC(
		AZALIA_JACK_AND_INTEGRATED,
		AZALIA_INTERNAL | AZALIA_TOP,
		AZALIA_MIC_IN,
		AZALIA_ATAPI_INTERNAL,
		AZALIA_COLOR_UNKNOWN,
		AZALIA_NO_JACK_PRESENCE_DETECT,
		3, 0
	)),

	0x80862805,	/* Codec Vendor / Device ID: Intel */
	0x80860101,	/* Subsystem ID */
	4,		/* Number of 4 dword sets */
	AZALIA_SUBVENDOR(3, 0x80860101),
	AZALIA_PIN_CFG(3, 0x5, AZALIA_PIN_DESC(
		AZALIA_JACK,
		AZALIA_DIGITAL_DISPLAY,
		AZALIA_DIGITAL_OTHER_OUT,
		AZALIA_OTHER_DIGITAL,
		AZALIA_COLOR_UNKNOWN,
		AZALIA_JACK_PRESENCE_DETECT,
		1, 0
	)),
	AZALIA_PIN_CFG(3, 0x6, AZALIA_PIN_DESC(
		AZALIA_JACK,
		AZALIA_DIGITAL_DISPLAY,
		AZALIA_DIGITAL_OTHER_OUT,
		AZALIA_OTHER_DIGITAL,
		AZALIA_COLOR_UNKNOWN,
		AZALIA_JACK_PRESENCE_DETECT,
		2, 0
	)),
	AZALIA_PIN_CFG(3, 0x7, AZALIA_PIN_DESC(
		AZALIA_JACK,
		AZALIA_DIGITAL_DISPLAY,
		AZALIA_DIGITAL_OTHER_OUT,
		AZALIA_OTHER_DIGITAL,
		AZALIA_COLOR_UNKNOWN,
		AZALIA_JACK_PRESENCE_DETECT,
		3, 0
	)),
};

const u32 pc_beep_verbs[0] = {};

AZALIA_ARRAY_SIZES;
