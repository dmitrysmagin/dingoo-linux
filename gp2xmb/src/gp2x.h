/***************************************************************************
 *   Copyright (C) 2006 by Mikael Ganehag Brorsson                         *
 *   brorsson@bsnet.se                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __GP2X_H__
#define __GP2X_H__

#include "joystick.h"

#define GP2X_CLK_FREQ 7372800

void gp2x_init();
void gp2x_deinit();
void gp2x_setclock(unsigned int mhz);
void gp2x_setgamma(int gamma);

extern short gp2x_initialized;
extern unsigned long gp2x_mem;
extern unsigned short *gp2x_memregs;
extern volatile unsigned short *MEM_REG;

#endif                          /* gp2x.h */
