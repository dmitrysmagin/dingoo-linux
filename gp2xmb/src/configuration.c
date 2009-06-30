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
#include <unistd.h>
#include <string.h>
#include "configuration.h"

void gp2xmb_sync(void);

config_t CONFIG;

#ifdef TARGET_GP2X
#define DEFAULTVOLUME 80
#else
#define DEFAULTVOLUME 100
#endif

void cfg_init()
{
    FILE *configfile = fopen("gp2xmb.config", "r");
    config_setting_t *pcfg = NULL;

    config_init(&CONFIG);

    if (configfile)
    {
        config_read(&CONFIG, configfile);
        fclose(configfile);
    }

    /* Add all none excisting fields */

    if (!(pcfg = config_lookup(&CONFIG, "wallpaper")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG), "wallpaper",
                               CONFIG_TYPE_STRING);
        config_setting_set_string(pcfg, "");
    }

    if (!(pcfg = config_lookup(&CONFIG, "usewallpaper")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG),
                               "usewallpaper", CONFIG_TYPE_BOOL);
        config_setting_set_bool(pcfg, 0);
    }

    if (!(pcfg = config_lookup(&CONFIG, "bgtheme")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG),
                               "bgtheme", CONFIG_TYPE_INT);
        config_setting_set_int(pcfg, 0);
    }

    if (!(pcfg = config_lookup(&CONFIG, "photopreview")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG),
                               "photopreview", CONFIG_TYPE_BOOL);
        config_setting_set_bool(pcfg, 1);
    }

    if (!(pcfg = config_lookup(&CONFIG, "soundfx")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG), "soundfx",
                               CONFIG_TYPE_BOOL);
        config_setting_set_bool(pcfg, 1);
    }

    if (!(pcfg = config_lookup(&CONFIG, "volume")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG), "volume",
                               CONFIG_TYPE_INT);
        config_setting_set_int(pcfg, DEFAULTVOLUME);
    }

    if (!(pcfg = config_lookup(&CONFIG, "outro")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG), "outro",
                               CONFIG_TYPE_BOOL);
        config_setting_set_bool(pcfg, 1);
    }

    if (!(pcfg = config_lookup(&CONFIG, "defaultmenu")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG), "defaultmenu",
                               CONFIG_TYPE_INT);
        config_setting_set_int(pcfg, 0);
    }

    if (!(pcfg = config_lookup(&CONFIG, "shortcuts")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG), "shortcuts",
                               CONFIG_TYPE_LIST);
    }
    else
    {

    }

    if (!(pcfg = config_lookup(&CONFIG, "swapXB")))
    {
        pcfg =
            config_setting_add(config_root_setting(&CONFIG), "swapXB",
                               CONFIG_TYPE_BOOL);
        config_setting_set_bool(pcfg, 0);
    }
}

void cfg_destroy()
{
    config_destroy(&CONFIG);
}

void cfg_save()
{

    FILE *configfile = fopen("gp2xmb.config", "w");

    if (configfile)
    {
        config_write_file(&CONFIG, "gp2xmb.config");
        fclose(configfile);
        gp2xmb_sync();
    }
}
