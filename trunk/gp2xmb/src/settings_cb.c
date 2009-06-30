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
#include <string.h>
#include <errno.h>

#include "background.h"
#include "configuration.h"
#include "mediabar.h"
#include "messagebox.h"

extern xmb_submenu *submenu;

void callback_wallpaper(int value)
{
    config_setting_t *cfgd = config_lookup(&CONFIG, "usewallpaper");

    if (cfgd)
    {
        config_setting_set_bool(cfgd, !value);
    }

    bg_showbg(!value);
}

void callback_bgtheme(int value)
{
    config_setting_t *cfgd = config_lookup(&CONFIG, "bgtheme");

    bg_colorset(value);

    if (cfgd)
    {
        config_setting_set_int(cfgd, value);
    }
}

void callback_soundfx(int value)
{
    config_setting_t *cfgd = config_lookup(&CONFIG, "soundfx");

    if (cfgd)
    {
        config_setting_set_bool(cfgd, !value);
    }
}

void callback_outro(int value)
{
    config_setting_t *cfgd = config_lookup(&CONFIG, "outro");

    if (cfgd)
    {
        config_setting_set_bool(cfgd, !value);
    }
}

void callback_defaultmenu(int value)
{
    config_setting_t *cfgd = config_lookup(&CONFIG, "defaultmenu");

    if (cfgd)
    {
        config_setting_set_int(cfgd, value);
    }
}

void callback_swapXB(int value)
{
    config_setting_t *cfgd = config_lookup(&CONFIG, "swapXB");

    if (cfgd)
    {
        config_setting_set_bool(cfgd, value);
    }
}

void callback_photopreview(int value)
{
    config_setting_t *cfgd = config_lookup(&CONFIG, "photopreview");

    if (cfgd)
    {
        config_setting_set_bool(cfgd, !value);
    }
}

void callback_systemupdate(struct submenu *menu)
{
    FILE *fp = NULL;
    char buf[1024];
    int bread = 0;
    int error;

    const char *failed = "Unable to update system.";
    const char *nofile = "No update file found.";

    memset(buf, 0, sizeof(buf));

#ifdef TARGET_GP2X
    fp = popen("/mnt/sd/gp2xmb.update", "r");
#else
    fp = popen("/tmp/mnt/sd/gp2xmb.update", "r");
#endif
    error = errno;              /* TODO: popen does not set errno */

    if (!fp)
    {
        msgbox(SDL_GetVideoSurface(),NULL, "System Update", error == ENOENT ? nofile : failed,
               OK);
        msgbox_retval();
        submenu = NULL;
        return;
    }

    /* Initialise */
    msgbox(SDL_GetVideoSurface(),NULL, "System Update", "Please Wait...", NONE);

    while (!feof(fp))
    {
        bread += fread(&buf[bread], 1, 3, fp);
        if (bread == 3)
            break;
        SDL_Delay(100);
    }
    msgbox_retval();

    if (bread != 3)
    {
        submenu = NULL;
        pclose(fp);
        msgbox(SDL_GetVideoSurface(),NULL, "System Update", error == ENOENT ? nofile : failed,
               OK);
        msgbox_retval();
        return;
    }

    bread = 0;
    memset(buf, 0, sizeof(buf));

    /* Install */
    msgbox(SDL_GetVideoSurface(),NULL, "System Update", "Installing", NONE);

    while (!feof(fp))
    {
        bread += fread(&buf[bread], 1, 3, fp);
        if (bread == 3)
            break;
        SDL_Delay(100);
    }
    msgbox_retval();

    if (bread != 3)
    {
        submenu = NULL;
        pclose(fp);
        msgbox(SDL_GetVideoSurface(),NULL, "System Update", error == ENOENT ? nofile : failed,
               OK);
        msgbox_retval();
        return;
    }

    /* Done */
    msgbox(SDL_GetVideoSurface(), NULL, "System Update", "The installation was successful.", OK);
    msgbox_retval();
    msgbox(SDL_GetVideoSurface(), NULL, "System Update", "The system will now restart.", OK);
    msgbox_retval();

    pclose(fp);

    submenu = NULL;

    exit(0);
}
