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

#include <fcntl.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#include "gfx.h"
#include "batterymon.h"

int tick_battery = 0;
short batt_loaded = 0;

SDL_Surface *battery_icon = NULL;

int battery_init()
{
    battery_icon = IMG_Load("images/icons/batt5.png");
    batt_loaded = 1;
    return 0;
}

unsigned short battery_get_level()
{
#ifdef TARGET_GP2X
    int devbatt = open("/dev/batt", O_RDONLY);
    unsigned short cbv, min = 900, max = 0;
    int v, i, battval = 0;

    if (devbatt < 0)
        return 0;

    for (i = 0; i < BATTERY_READS; i++)
    {
        if (read(devbatt, &cbv, 2) == 2)
        {
            battval += cbv;
            if (cbv > max)
                max = cbv;
            if (cbv < min)
                min = cbv;
        }
    }
    close(devbatt);

    battval -= min + max;
    battval /= BATTERY_READS - 2;

    if (battval >= 850)
        return 6;
    if (battval > 780)
        return 5;
    if (battval > 760)
        return 4;
    if (battval > 730)
        return 3;
    if (battval > 700)
        return 2;
    if (battval > 680)
        return 1;
    return 0;
#else
    return 6;                   /* AC Power */
#endif
}

int battery_draw(SDL_Surface * screen)
{
    unsigned int ticknow = SDL_GetTicks();

    if (!batt_loaded)
        return 0;

    if (!tick_battery || ticknow - tick_battery > 60000)        /* 60 seconds */
    {
        unsigned short battlevel = battery_get_level();
        tick_battery = ticknow;

        if (battery_icon)
            SDL_FreeSurface(battery_icon);

        if (battlevel > 5)
            battery_icon = IMG_Load("images/icons/batt5.png");
        else
        {
            char path[128];
            path[0] = '\0';
            /* potentianl buffer overflow */
            sprintf(path, "images/icons/batt%i.png", battlevel);
            battery_icon = IMG_Load(path);
        }
    }

    if (battery_icon)
    {
        gfx_draw_image(battery_icon, screen->w - (battery_icon->w + 10),
                       10, screen);
    }

    return 0;
}
