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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>

#include "gp2x.h"

short gp2x_initialized = 0;
unsigned long gp2x_mem = 0;     /* mem fd */
unsigned short *gp2x_memregs = NULL;
volatile unsigned short *MEM_REG = NULL;

void gp2x_init()
{
#ifdef TARGET_GP2X
    if (!gp2x_initialized)
    {
        gp2x_mem = open("/dev/mem", O_RDWR);
        gp2x_memregs =
            (unsigned short *) mmap(0, 0x10000, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, gp2x_mem, 0xc0000000);
        MEM_REG = &gp2x_memregs[0];

        gp2x_initialized = 1;
    }
#endif
}

void gp2x_deinit()
{
#ifdef TARGET_GP2X
    if (gp2x_initialized)
    {
        gp2x_memregs[0x28DA >> 1] = 0x4AB;
        gp2x_memregs[0x290C >> 1] = 640;
        close(gp2x_mem);

        gp2x_initialized = 0;
    }
#endif
}

void gp2x_setclock(unsigned int mhz)
{
    if (mhz < 50)
        mhz = 50;
    else if (mhz > 250)
        mhz = 250;

#ifdef TARGET_GP2X
    short already_inited = gp2x_initialized;
    if (!already_inited)
        gp2x_init();

    unsigned v;
    unsigned mdiv, pdiv = 3, scale = 0;
    mhz *= 1000000;
    mdiv = (mhz * pdiv) / GP2X_CLK_FREQ;
    mdiv = ((mdiv - 8) << 8) & 0xff00;
    pdiv = ((pdiv - 2) << 2) & 0xfc;
    scale &= 3;
    v = mdiv | pdiv | scale;
    MEM_REG[0x910 >> 1] = v;

    if (!already_inited)
        gp2x_deinit();
#endif
}

void gp2x_setgamma(int gamma)
{
#ifdef TARGET_GP2X
    int i = 0;
    if (!gp2x_initialized)
        return;

    if (gamma < 1)
        gamma = 1;
    else if (gamma > 100)
        gamma = 100;

    float fgamma = gamma / 10.0;
    fgamma = 1 / fgamma;
    MEM_REG[0x2880 >> 1] &= ~(1 << 12);
    MEM_REG[0x295C >> 1] = 0;

    for (i = 0; i < 256; i++)
    {
        unsigned short s;
        unsigned char g = (unsigned char) (255.0 * pow(i / 255.0, fgamma));
        s = (g << 8) | g;
        MEM_REG[0x295E >> 1] = s;
        MEM_REG[0x295E >> 1] = g;
    }

#endif
}
