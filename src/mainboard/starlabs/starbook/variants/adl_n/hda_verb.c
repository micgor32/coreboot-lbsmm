/* SPDX-License-Identifier: GPL-2.0-only */

#include <device/azalia_device.h>
#include <stdint.h>

#define DMIC		0x12
#define SPEAKERS	0x14
#define HEADPHONE	0x15
#define MONO		0x17
#define MIC1		0x18
#define MIC2		0x19
#define LINE1		0x1a
#define LINE2		0x1b
#define PC_BEEP		0x1d
#define S_PDIF		0x1e

const u32 cim_verb_data[] = {
	/* coreboot specific header */
	0x10ec0269,	/* Codec Vendor / Device ID: Realtek ALC269 */
	0x1e507038,	/* Subsystem ID */
	18,		/* Number of verb entries */

	/* Reset Codec First */
	AZALIA_RESET(0x1),

	/* HDA Codec Subsystem ID */
	AZALIA_SUBVENDOR(0, 0x1e507038),

	AZALIA_PIN_CFG(0, 0x01,		0x00000000),
	AZALIA_PIN_CFG(0, DMIC,		AZALIA_PIN_DESC(					\
						AZALIA_INTEGRATED,				\
						AZALIA_INTERNAL | AZALIA_TOP,			\
						AZALIA_MIC_IN,					\
						AZALIA_TYPE_UNKNOWN,				\
						AZALIA_BLACK,					\
						AZALIA_JACK_PRESENCE_DETECT,			\
						3,						\
						0						\
					)),

	AZALIA_PIN_CFG(0, SPEAKERS,	AZALIA_PIN_DESC(					\
						AZALIA_INTEGRATED,				\
						AZALIA_INTERNAL | AZALIA_FRONT,			\
						AZALIA_SPEAKER,					\
						AZALIA_TYPE_UNKNOWN,				\
						AZALIA_BLACK,					\
						AZALIA_JACK_PRESENCE_DETECT,			\
						1,						\
						0						\
					)),
	AZALIA_PIN_CFG(0, HEADPHONE,	AZALIA_PIN_DESC(					\
						AZALIA_JACK,					\
						AZALIA_EXTERNAL_PRIMARY_CHASSIS | AZALIA_RIGHT,	\
						AZALIA_HP_OUT,					\
						AZALIA_STEREO_MONO_1_8,				\
						AZALIA_BLACK,					\
						AZALIA_JACK_PRESENCE_DETECT,			\
						2,						\
						0						\
					)),
	AZALIA_PIN_CFG(0, MONO,		AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, MIC1,		AZALIA_PIN_DESC(					\
						AZALIA_JACK,					\
						AZALIA_EXTERNAL_PRIMARY_CHASSIS | AZALIA_RIGHT,	\
						AZALIA_MIC_IN,					\
						AZALIA_STEREO_MONO_1_8,				\
						AZALIA_BLACK,					\
						AZALIA_JACK_PRESENCE_DETECT,			\
						4,						\
						0						\
					)),
	AZALIA_PIN_CFG(0, MIC2,		AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, LINE1,	AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, LINE2,	AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, PC_BEEP,	AZALIA_PIN_CFG_NC(0)),
	AZALIA_PIN_CFG(0, S_PDIF,	AZALIA_PIN_CFG_NC(0)),

	0x02050018,
	0x02040184,

	0x0205001C,
	0x02044b00,

	0x02050024,
	0x02040000,

	0x02050004,
	0x02040080,

	0x02050008,
	0x02040000,

	0x0205000C,
	0x02043F00,

	0x02050015,
	0x02048002,

	0x02050015,
	0x02048002,

	0x00C37080,
	0x00270610,
	0x00D37080,
	0x00370610,

	0x8086280d,	/* Codec Vendor / Device ID: Intel */
	0x80860101,	/* Subsystem ID */
	4,		/* Number of 4 dword sets */

	AZALIA_SUBVENDOR(2, 0x80860101),

	AZALIA_PIN_CFG(2, 0x05, 0x18560010),
	AZALIA_PIN_CFG(2, 0x06, 0x18560020),
	AZALIA_PIN_CFG(2, 0x07, 0x18560030),
};

const u32 pc_beep_verbs[] = {
};

AZALIA_ARRAY_SIZES;
