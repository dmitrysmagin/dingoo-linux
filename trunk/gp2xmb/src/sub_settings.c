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
#include <math.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "joystick.h"
#include "gfx.h"
#include "sfx.h"
#include "mediabar.h"
#include "gp2xmb.h"
#include "itemlist.h"
#include "sub_settings.h"
#include "settings_cb.h"
#include "popup.h"
#include "background.h"
#include "configuration.h"
#include "scripts.h"

#define XMBDEST 80

extern xmb_submenu *submenu;

int settings_xmb_offset_x = 0;
int settings_xmb_offset_y = 0;
int settings_xmb_offset_ox = 0;
int settings_xmb_offset_tx = 0;

int settings_tlist = 0;
int settings_stage = 0;

int settings_animtimer_b = 0;
int settings_animtimer = 0;
int settings_levels = 0;

void sub_settings_init(xmb_submenu * menu)
{
    if (!menu)
        return;

    settings_xmb_offset_x = 0;
    settings_xmb_offset_y = 0;
    settings_xmb_offset_ox = 0;
    settings_xmb_offset_tx = 0;
    settings_stage = 3;
    settings_tlist = (int) menu->extra;
    settings_animtimer_b = settings_animtimer = SDL_GetTicks();
    settings_levels = 0;

    LIST_init();
    POPUP_init();
}

void sub_settings_destroy()
{
    LIST_destroy();
    POPUP_destroy();
}

void sub_settings_draw(SDL_Surface * screen)
{
	int photopreview = 0;
	long bgtheme = 0;
	int usewallpaper = 0;
	int swapXB = 0;
	long defaultmenu = 0;
	int outro = 0;
	int soundfx = 0;

    config_lookup_bool(&CONFIG, "photopreview", &photopreview);
    config_lookup_int(&CONFIG, "bgtheme", &bgtheme);
    config_lookup_bool(&CONFIG, "usewallpaper", &usewallpaper);
    config_lookup_bool(&CONFIG, "swapXB", &swapXB);
    config_lookup_int(&CONFIG, "defaultmenu", &defaultmenu);
    config_lookup_bool(&CONFIG, "outro", &outro);
    config_lookup_bool(&CONFIG, "soundfx", &soundfx);

    switch (settings_stage)
    {
    case 1:
        if (!settings_xmb_offset_tx)
        {
            settings_xmb_offset_ox = settings_xmb_offset_x;
            settings_xmb_offset_tx = -LIST_columnWidth();
        }
        settings_xmb_offset_x -=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - settings_animtimer));
        break;

    case 2:
        if (!settings_xmb_offset_tx)
        {
            settings_xmb_offset_ox = settings_xmb_offset_x;
            settings_xmb_offset_tx = LIST_columnWidth();
        }
        settings_xmb_offset_x +=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - settings_animtimer));
        break;

    case 3:
        if (!settings_xmb_offset_tx)
        {
            settings_xmb_offset_ox = settings_xmb_offset_x;
            settings_xmb_offset_tx = -(LIST_columnWidth() * 1.7);
        }
        settings_xmb_offset_x -=
            rint((LIST_columnWidth() * 1.7) / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - settings_animtimer));
        break;

    case 4:
        if (!settings_xmb_offset_tx)
        {
            settings_xmb_offset_ox = settings_xmb_offset_x;
            settings_xmb_offset_tx = (LIST_columnWidth() * 1.7);
        }
        settings_xmb_offset_x +=
            rint((LIST_columnWidth() * 1.7) / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - settings_animtimer));
        break;

    case 5:
        settings_stage++;
        break;

    case 6:                    /* populate list */
        {
            SDL_Surface *icon =
                gfx_load_image("images/icons/config.png", 1);
            switch (settings_tlist)
            {
            case SETUSB:
                LIST_add(gfx_load_image("images/icons/sd.png", 0), NULL,
                         "SD Memory Card", NULL, NULL);
                LIST_add(gfx_load_image("images/icons/nand.png", 0), NULL,
                         "Built-in Memory", NULL, NULL);
                break;

            case SETPHOTO:
                LIST_add(gfx_clone_surface(icon), NULL, "Preview Icon",
                         NULL, NULL);
                break;

            case SETTHEME:
                LIST_add(gfx_clone_surface(icon), NULL, "Background Theme",
                         NULL, NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "Icon Theme", NULL,
                         NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "Sound Theme",
                         NULL, NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "Wallpaper", NULL,
                         NULL);
                break;

            case SETSYSTEM:
            	LIST_add(gfx_clone_surface(icon), NULL, "System Name",
                         NULL, NULL);
            	LIST_add(gfx_clone_surface(icon), NULL, "System Language",
                         NULL, NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "Button Layout",
                         NULL, NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "Default Menu",
                         NULL, NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "Launcher Outro",
                         NULL, NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "My Shortcuts",
                         NULL, NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "Format Utility",
                         NULL, NULL);
				LIST_add(gfx_clone_surface(icon), NULL, "Backup Utility",
                         NULL, NULL);
				LIST_add(gfx_clone_surface(icon), NULL, "Restore Default Settings",
                         NULL, NULL);
				LIST_add(gfx_clone_surface(icon), NULL, "Default System",
                         NULL, NULL);
				LIST_add(gfx_clone_surface(icon), NULL, "System Information",
                         NULL, NULL);
                LIST_add(gfx_clone_surface(icon), NULL, "About GP2XMB",
                         NULL, NULL);
                break;

            case SETSOUND:
                LIST_add(gfx_clone_surface(icon), NULL, "Sound Effects",
                         NULL, NULL);
                break;

            default:
                break;

            }
            LIST_setEmptyMsg(NULL);
            settings_stage = 0;
        }
        break;

    case 7:                    /* populate popup */
        {
            switch (settings_tlist)
            {
            case SETUSB:
                switch (LIST_getSelNum())
                {
                case 0:
                	{

/*						char pathbuf[PATH_MAX];
                		memset(pathbuf,0,sizeof(pathbuf));
                		getcwd(pathbuf,sizeof(pathbuf));
                		chdir("/");
*/
                	    scripts_exec(USB_ENABLE, "sd");
                	    msgbox(SDL_GetVideoSurface(),
                	    	   gfx_load_image("images/icons/usb.png",1),
                	    	   "USB Connection",
                	           "USB file transfer enabled.", DISCONNECT);
                	    msgbox_retval();
                	    scripts_exec(USB_DISABLE, "sd");

/*                	    chdir(pathbuf);*/
                	    break;
                	 }

                case 1:
                    {

/*                		char pathbuf[PATH_MAX];
                		memset(pathbuf,0,sizeof(pathbuf));
                		getcwd(pathbuf,sizeof(pathbuf));
                		chdir("/");
*/
                	    scripts_exec(USB_ENABLE, "nand");
                	    msgbox(SDL_GetVideoSurface(),
                	    	   gfx_load_image("images/icons/usb.png",1),
                	    	   "USB Connection",
                	           "USB file transfer enabled.", DISCONNECT);
                	    msgbox_retval();
                	    scripts_exec(USB_DISABLE, "nand");

/*                	    chdir(pathbuf);*/
                	    break;
                	 }
                }
                settings_stage = 0;
                xmb_deactivateFlag(XMB_NOBATT | XMB_FOCUS);
                POPUP_empty();
                break;

            case SETPHOTO:
                switch (LIST_getSelNum())
                {
                case 0:
                    {
                        POPUP_setrevert(callback_photopreview, !photopreview);

                        POPUP_add(NULL, "Enabled", callback_photopreview,
                                  0);
                        POPUP_add(NULL, "Disabled", callback_photopreview,
                                  1);

                        POPUP_setselected(!photopreview);
                    }
                    break;
                }
                break;

            case SETTHEME:
                switch (LIST_getSelNum())
                {
                case 0:        /* Background Theme */
                    {
                        int i = 0;

                        POPUP_setrevert(callback_bgtheme, bgtheme);

                        for (;
                             i <
                             (sizeof(background_themes) / sizeof(t_rgb));
                             i++)
                        {
                            t_rgb color = background_themes[i];
                            SDL_Surface *tmp = gfx_new_surface(16, 10, 0);
                            SDL_FillRect(tmp, NULL,
                                         SDL_MapRGB(screen->format,
                                                    color.r, color.g,
                                                    color.b));
                            POPUP_add(tmp, background_names[i],
                                      callback_bgtheme, i);
                        }

                        POPUP_setselected(bgtheme);
                    }
                    break;
                case 1:        /* Icon Theme */
                    {
                        POPUP_setrevert(NULL, 0);
                        POPUP_add(NULL, "Default", NULL, 0);;
                    }
                    break;
                case 2:        /* Sound Theme */
                    {
                        POPUP_setrevert(NULL, 0);
                        POPUP_add(NULL, "Default", NULL, 0);
                    }
                    break;
                case 3:        /* Wallpaper */
                    {
                        POPUP_setrevert(callback_wallpaper, !usewallpaper);

                        POPUP_add(NULL, "Use", callback_wallpaper, 0);
                        POPUP_add(NULL, "Do Not Use", callback_wallpaper,
                                  1);

                        POPUP_setselected(!usewallpaper);
                    }
                    break;
                }
                break;

            case SETSYSTEM:
                switch (LIST_getSelNum())
                {
                case 0:
                	{

                	}
                	break;

                case 1:
                	{

                	}
                	break;

                case 2:
                    {
                        POPUP_setrevert(callback_swapXB, swapXB);

                        POPUP_add(NULL, "Default", callback_swapXB, 0);
                        POPUP_add(NULL, "GP2X Style", callback_swapXB, 1);

                        POPUP_setselected(swapXB);
                    }
                    break;

                case 3:
                    {
                        POPUP_setrevert(callback_defaultmenu, defaultmenu);

                        POPUP_add(NULL, "Settings", callback_defaultmenu,
                                  0);
                        POPUP_add(NULL, "Photo", callback_defaultmenu, 1);
                        POPUP_add(NULL, "Music", callback_defaultmenu, 2);
                        POPUP_add(NULL, "Video", callback_defaultmenu, 3);
                        POPUP_add(NULL, "Game", callback_defaultmenu, 4);

                        POPUP_setselected(defaultmenu);
                    }
                    break;

                case 4:
                    {
                        POPUP_setrevert(callback_outro, !outro);

                        POPUP_add(NULL, "Enabled", callback_outro, 0);
                        POPUP_add(NULL, "Disabled", callback_outro, 1);

                        POPUP_setselected(!outro);
                    }
                    break;

                case 5:
                    {
                        POPUP_setrevert(NULL, 0);

                        POPUP_add(NULL, "Fancy List", NULL, 0);
                        POPUP_add(NULL, "Basic Grid", NULL, 1);

                        POPUP_setselected(0);
                    }
                    break;
                case 11:
                	{
                		gfx_draw_about(screen);
                		settings_stage = 0;
						xmb_deactivateFlag(XMB_NOBATT | XMB_FOCUS);
						POPUP_empty();
					}
					break;
                }
                break;

            case SETSOUND:
                switch (LIST_getSelNum())
                {
                case 0:
                    {
                        POPUP_setrevert(callback_soundfx, !soundfx);

                        POPUP_add(NULL, "Enabled", callback_soundfx, 0);
                        POPUP_add(NULL, "Disabled", callback_soundfx, 1);

                        POPUP_setselected(!soundfx);
                    }
                    break;
                }
                break;

            default:
                break;
            }

            if (settings_stage == 7)
                settings_stage = 10;
        }
        break;
    }

    if (settings_levels <= 0 && !settings_stage)
    {
        settings_stage = 4;
    }

    if (SDL_GetTicks() - settings_animtimer_b > XMBMOVEDELAY
        && settings_stage)
    {
        if (settings_stage == 1 || settings_stage == 3)
        {
            LIST_setEmptyMsg(NULL);
            settings_stage = 5;
        }

        if (settings_stage == 4 && settings_levels == -1)
        {
            settings_xmb_offset_x = 0;
            LIST_destroy();
            sub_settings_destroy();
            submenu = NULL;
            return;
        }

        if (settings_stage <= 5 && settings_xmb_offset_tx)
        {
            settings_xmb_offset_x =
                settings_xmb_offset_ox + settings_xmb_offset_tx;
            settings_xmb_offset_tx = 0;
            settings_xmb_offset_ox = 0;
        }

        if (settings_stage > 0 && settings_stage < 5)
            settings_stage = 0;
    }
    settings_animtimer = SDL_GetTicks();

    if (settings_stage != 3 && settings_stage != 4)
    {
        LIST_draw(screen, (int) settings_xmb_offset_x + ICONSCALE_X - 5,
                  0);
    }

    if (settings_stage == 10)
    {
        POPUP_draw(screen);
    }
}

void sub_settings_handle_input(unsigned int button)
{
    switch (button)
    {
    case GP2X_BUTTON_B:
        if (settings_stage == 10)
        {
            sfx_play(SFXBACK);
            settings_stage = 0;
            xmb_deactivateFlag(XMB_NOBATT | XMB_FOCUS);
            POPUP_revert();     /* must run this before POPUP_empty */
            POPUP_empty();
            break;
        }
    case GP2X_BUTTON_LEFT:
        if (!settings_stage)
        {
            sfx_play(SFXBACK);
            LIST_setEmptyMsg(NULL);
            if (LIST_out())
                settings_stage = 4;
            else
                settings_stage = 2;
            settings_levels--;
            settings_animtimer_b = settings_animtimer = SDL_GetTicks();
        }
        break;

    case GP2X_BUTTON_UP:
        if (settings_stage != 10 && LIST_up())
            sfx_play(SFXMOVE);
        else if (settings_stage == 10 && POPUP_up())
        {
            sfx_play(SFXMOVE);
            POPUP_execsel();
        }
        break;
    case GP2X_BUTTON_DOWN:
        if (settings_stage != 10 && LIST_down())
            sfx_play(SFXMOVE);
        else if (settings_stage == 10 && POPUP_down())
        {
            sfx_play(SFXMOVE);
            POPUP_execsel();
        }
        break;

    case GP2X_BUTTON_X:
        if (settings_stage != 10)
        {
            sfx_play(SFXOK);
            settings_stage = 7;
            xmb_activateFlag(XMB_NOBATT | XMB_FOCUS);
        }
        else
        {
            sfx_play(SFXBACK);
            settings_stage = 0;
            xmb_deactivateFlag(XMB_NOBATT | XMB_FOCUS);
            POPUP_empty();
            cfg_save();
        }
        break;

    default:
        break;
    }
}

int sub_settings_get_offset_x()
{
    return settings_xmb_offset_x;
}

int sub_settings_get_offset_y()
{
    return 0;
}
