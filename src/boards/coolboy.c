/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2015 CaH4e3
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * CoolBoy 400-in-1 FK23C-mimic mapper 32Mb PROM + 128K CHR RAM, no wram, no CROM
 * only MMC3 mode
 *
 */

#include "mapinc.h"
#include "mmc3.h"

static void COOLBOYCW(uint32 A, uint8 V) {
	if(EXPREGS[3] & 0x10)
		setchr8(EXPREGS[2] & 0xF);
	else
		setchr1(A, V);
}

static void COOLBOYPW(uint32 A, uint8 V) {
	uint32 mask, shift;
	uint32 base = ((EXPREGS[0] & 0x07) >> 0) | ((EXPREGS[1] & 0x10) >> 1) | ((EXPREGS[1] & 0x0C) << 2) | ((EXPREGS[0] & 0x30) << 2);
	switch(EXPREGS[0]&0xC0) {
	case 0x00:
      base >>= 2;
		mask = 0x3F;
		shift = 6;
      break;
   case 0x80:
      base >>= 1;
      mask = 0x1F;
      shift = 5;
		break;
	case 0xC0:
		shift = 4;
		if(EXPREGS[3] & 0x10) {
         mask = 0x01 | (EXPREGS[1] & 2);
		} else {
			mask = 0x0F;
		}
		break;
	}
	if(EXPREGS[3] & 0x10)
      setprg8(A, (base << shift) | (V & mask) | ((EXPREGS[3] & (0x0E ^ (EXPREGS[1] & 2))) ));
	else
		setprg8(A, (base << shift) | (V & mask));
}

static DECLFW(COOLBOYWrite) {
	uint32_t base;

	if((EXPREGS[3] & 0x80) == 0)
   {
		EXPREGS[A & 3] = V;
		FixMMC3PRG(MMC3_cmd);
		FixMMC3CHR(MMC3_cmd);
		base = ((EXPREGS[0] & 0x07) >> 0) | ((EXPREGS[1] & 0x10) >> 1) | ((EXPREGS[1] & 0x0C) << 2) | ((EXPREGS[0] & 0x30) << 2);
		FCEU_printf("exp %02x %02x (base %03d)\n",A,V,base);
	}
}

static void COOLBOYReset(void) {
	MMC3RegReset();
	EXPREGS[0] = EXPREGS[1] = EXPREGS[2] = EXPREGS[3] = 0;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
}

static void COOLBOYPower(void) {
	GenMMC3Power();
	EXPREGS[0] = EXPREGS[1] = EXPREGS[2] = EXPREGS[3] = 0;
	FixMMC3PRG(MMC3_cmd);
	FixMMC3CHR(MMC3_cmd);
	SetWriteHandler(0x5000, 0x5fff, CartBW);            // some games access random unmapped areas and crashes because of KT-008 PCB hack in MMC3 source lol
	SetWriteHandler(0x6000, 0x6fff, COOLBOYWrite);
}

void COOLBOY_Init(CartInfo *info) {
	GenMMC3_Init(info, 512, 128, 0, 0);
	pwrap = COOLBOYPW;
	cwrap = COOLBOYCW;
	info->Power = COOLBOYPower;
	info->Reset = COOLBOYReset;
	AddExState(EXPREGS, 4, 0, "EXPR");
}

