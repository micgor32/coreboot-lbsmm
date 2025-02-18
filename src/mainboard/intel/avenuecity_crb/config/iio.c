/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <soc/iio.h>

static const struct iio_pe_config iio_config_table[] = {
	/*
	 * CB_IIO_BIFURCATE_x8x2x2x2x2 is first set to indicate how the IIO is bifurcated
	 * then port settings are listed accordingly. The minimal port elements are x2.
	 * If an x8 port is enabled, the neighboring 3 x2 port elements needs to be
	 * disabled.
	 */
	{_IIO_PE_CFG_STRUCT(0x0, PE0, CB_IIO_BIFURCATE_x8x2x2x2x2, PE_TYPE_PCIE) {
		/* _IIO_PORT_CFG_STRUCT_BASIC(sltpls, sltplv, psn) */
		_IIO_PORT_CFG_STRUCT_BASIC_X8(0x0, 0x4B, 0x1),
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_BASIC_X2(0x0, 0x4B, 0x2),
		_IIO_PORT_CFG_STRUCT_BASIC_X2(0x0, 0x4B, 0x3),
		_IIO_PORT_CFG_STRUCT_BASIC_X2(0x0, 0x4B, 0x4),
		_IIO_PORT_CFG_STRUCT_BASIC_X2(0x0, 0x4B, 0x5),
	}},

	{_IIO_PE_CFG_STRUCT(0x0, PE1, CB_IIO_BIFURCATE_x16xxxxx, PE_TYPE_PCIE) {
		/* _IIO_PORT_CFG_STRUCT_BASIC(sltpls, sltplv, psn) */
		_IIO_PORT_CFG_STRUCT_BASIC_X16(0x0, 0x4B, 0x6),
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_DISABLED,
		_IIO_PORT_CFG_STRUCT_DISABLED,
	}},

	_IIO_PE_CFG_DISABLED(0x0, PE2),
	_IIO_PE_CFG_DISABLED(0x0, PE3),
	_IIO_PE_CFG_DISABLED(0x0, PE4),
	_IIO_PE_CFG_DISABLED(0x0, PE5),
	_IIO_PE_CFG_DISABLED(0x0, PEa),
	_IIO_PE_CFG_DISABLED(0x0, PEb),
	_IIO_PE_CFG_DISABLED(0x0, PEc),
	_IIO_PE_CFG_DISABLED(0x0, PEd),

	_IIO_PE_CFG_DISABLED(0x1, PE0),
	_IIO_PE_CFG_DISABLED(0x1, PE1),
	_IIO_PE_CFG_DISABLED(0x1, PE2),
	_IIO_PE_CFG_DISABLED(0x1, PE3),
	_IIO_PE_CFG_DISABLED(0x1, PE4),
	_IIO_PE_CFG_DISABLED(0x1, PE5),
	_IIO_PE_CFG_DISABLED(0x1, PEa),
	_IIO_PE_CFG_DISABLED(0x1, PEb),
	_IIO_PE_CFG_DISABLED(0x1, PEc),
	_IIO_PE_CFG_DISABLED(0x1, PEd),
};

const struct iio_pe_config *get_iio_config_table(int *size)
{
	*size = ARRAY_SIZE(iio_config_table);
	return iio_config_table;
}
