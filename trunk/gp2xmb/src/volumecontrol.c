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

#include <SDL.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/* For the soundcard */
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "gfx.h"
#include "volumecontrol.h"
#include "configuration.h"

#ifndef _PATH_MIXER
#define _PATH_MIXER "/dev/mixer"
#endif

#ifndef MAX
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))
#endif

/* to handle the mixer */
const char *devices[] = SOUND_DEVICE_NAMES;
int fd;
int devmask;
int stereodevs;

int volflags = 0;
int time_switchoff = 0;
long volume = 0;                /* default volume */
SDL_Surface *volume_meter = NULL;

int volume_init(void)
{
	config_lookup_int(&CONFIG,"volume", &volume);
    volume_set(volume);
    volume_meter = gfx_load_image("images/icons/volume_control.png", 1);
    return 0;
}

void volume_set(int vol)
{
    if ((fd = open(_PATH_MIXER, O_RDONLY)) == -1)
    {
        perror("open: " _PATH_MIXER);
        return;
    }

    if (ioctl(fd, SOUND_MIXER_READ_DEVMASK, &devmask) == -1)
    {
        perror("ioctl: SOUND_MIXER_READ_DEVMASK");
        return;
    }

    if (ioctl(fd, SOUND_MIXER_READ_STEREODEVS, &stereodevs) == -1)
    {
        perror("ioctl: SOUND_MIXER_READ_STEREODEVS");
        return;
    }
    level_set("pcm", vol);
    close(fd);
}

void volume_draw(SDL_Surface * screen)
{
    if (volflags & VOLCHANGE)
    {
        SDL_Rect src;
        SDL_Rect dst;
        src.x = 0;
        src.y = 24 * (volume / 5);
        src.w = volume_meter->w;
        src.h = 24;
        dst.x = 20;
        dst.y = 200;
        SDL_BlitSurface(volume_meter, &src, screen, &dst);

        if (time_switchoff < SDL_GetTicks())
        {
            volflags ^= VOLCHANGE;
        }
    }
}

void volume_up(void)
{
	config_setting_t *cfgd = config_lookup(&CONFIG, "volume");

    if (volume < 100)
    {
        volume_set(++volume);
        time_switchoff = SDL_GetTicks() + STAYTIME;
        volflags |= VOLCHANGE;
        if (cfgd)
		{
			config_setting_set_int(cfgd, volume);
    	}
    }
}

void volume_down(void)
{
	config_setting_t *cfgd = config_lookup(&CONFIG, "volume");

    if (volume > 0)
    {
        volume_set(--volume);
        time_switchoff = SDL_GetTicks() + STAYTIME;
        volflags |= VOLCHANGE;
       	if (cfgd)
		{
			config_setting_set_int(cfgd, volume);
    	}
    }
}

/* Functions to properly handle the mixer */
int device_get(const char *dev)
{
    int i;

    if (!dev || !dev[0])
        return (-1);

    for (i = 0; i < SOUND_MIXER_NRDEVICES; i++)
        if ((1 << i) & devmask)
            if (!strcmp(dev, devices[i]))
                return (i);

    return (-1);
}

int level_get(const char *dev, int *level)
{
    int device, left, right;

    if (!dev || !level)
        return (0);

    if ((device = device_get(dev)) < 0 || device > SOUND_MIXER_NRDEVICES)
        return (0);

    if (ioctl(fd, MIXER_READ(device), level) == -1)
    {
        perror("ioctl: MIXER_READ");
        return (0);
    }

    left = *level & 0xff;
    right = ((1 << device) & stereodevs) ? (*level & 0xff00) >> 8 : -1;

    if (level)
        *level = MAX(left, right);

    return (1);
}

int level_set(const char *dev, int level)
{
    int device, l;

    if (!dev)
        return (0);

    if ((device = device_get(dev)) < 0 || device > SOUND_MIXER_NRDEVICES)
        return (0);

    if (level < 0)
        level = 0;
    if (level > 100)
        level = 100;

    l = (level << 8) + level;

    if (ioctl(fd, MIXER_WRITE(device), &l) == -1)
    {
        perror("ioctl: MIXER_WRITE");
        return (0);
    }

    return (1);
}
