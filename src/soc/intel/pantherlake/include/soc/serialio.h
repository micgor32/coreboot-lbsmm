/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _SOC_PANTHERLAKE_SERIALIO_H_
#define _SOC_PANTHERLAKE_SERIALIO_H_

enum {
	PchSerialIoDisabled,
	PchSerialIoPci,
	PchSerialIoHidden,
	PchSerialIoLegacyUart,
	PchSerialIoSkipInit
};

enum {
	PchSerialIoIndexI2C0,
	PchSerialIoIndexI2C1,
	PchSerialIoIndexI2C2,
	PchSerialIoIndexI2C3,
	PchSerialIoIndexI2C4,
	PchSerialIoIndexI2C5
};

enum {
	PchSerialIoIndexGSPI0,
	PchSerialIoIndexGSPI1,
	PchSerialIoIndexGSPI0A,
};

enum {
	PchSerialIoIndexUART0,
	PchSerialIoIndexUART1,
	PchSerialIoIndexUART2
};

#endif /* _SOC_PANTHERLAKE_SERIALIO_H_ */
