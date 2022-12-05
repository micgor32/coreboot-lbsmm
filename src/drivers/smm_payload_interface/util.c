/* SPDX-License-Identifier: GPL-2.0-only */

#include <commonlib/region.h>
#include <console/console.h>
#include <fmap.h>
#include <smm_payload_interface.h>
#include <smmstore.h>
#include <string.h>
#include <types.h>

#define SMMSTORE_REGION "SMMSTORE"
#define HOB_HEADER_LEN 8

/* FIXME: Considering the MM case: Create compile-time conflict with SMMSTORE? */
__weak int smmstore_lookup_region(struct region_device *rstore)
{
	static int done;
	static struct region_device rdev;
	static int ret;

	if (!done) {
		done = 1;

		if (fmap_locate_area_as_rdev_rw(SMMSTORE_REGION, &rdev)) {
			printk(BIOS_WARNING,
			       "smm store: Unable to find SMM store FMAP region '%s'\n",
				SMMSTORE_REGION);
			ret = -1;
		} else {
			ret = 0;
		}
	}

	*rstore = rdev;
	return ret;
}

/* Utilities for walking HOBs (fsp2_0 and HobLib.h) */
bool pld_guid_compare(const uint8_t guid1[16], const uint8_t guid2[16])
{
	return !memcmp(guid1, guid2, 16);
}

const void *pld_hob_header_to_struct(const struct pld_hob_header *hob)
{
	union {
		const struct pld_hob_header *hob_hdr;
		const void *hob_descr;
		uintptr_t addr;
	} hob_walker;

	hob_walker.hob_hdr = hob;
	hob_walker.addr += HOB_HEADER_LEN;
	return hob_walker.hob_descr;
}

const void *pld_hob_header_to_extension_hob(const struct pld_hob_header *hob)
{
	union {
		const struct pld_hob_header *hob_hdr;
		const void *hob_descr;
		uintptr_t addr;
	} hob_walker;

	hob_walker.hob_hdr = hob;
	hob_walker.addr += HOB_HEADER_LEN + 16; /* header and 16-byte GUID */
	return hob_walker.hob_descr;
}
