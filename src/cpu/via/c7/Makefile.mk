## SPDX-License-Identifier: GPL-2.0-only

bootblock-y	+= ../car/cache_as_ram.S
bootblock-y	+= ../../intel/car/bootblock.c

romstage-y	+= ../../intel/car/romstage.c

postcar-y	+= ../../intel/car/p4-netburst/exit_car.S
