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

#include <dirent.h>
#include <math.h>
#include <ctype.h>
#include <SDL_rotozoom.h>

#include "gp2xmb.h"
#include "sub_photo.h"
#include "mediabar.h"
#include "itemlist.h"
#include "popup.h"

/* Why do I need to specify these - why does not dirent do its job? */
extern int scandir(__const char *__restrict __dir,
                   struct dirent ***__restrict __namelist,
                   int (*__selector) (__const struct dirent *),
                   int (*__cmp) (__const void *, __const void *))
__nonnull((1, 2));

extern int alphasort(__const void *__e1, __const void *__e2)
__THROW __attribute_pure__ __nonnull((1, 2));


#define PICONSIZE 32

/* stealing variables from the mediabar */
extern SFont_Font *font_normal;
extern xmb_submenu *submenu;

/* Search path variables */
char photo_path[1024];
char *photo_extensions[] = { ".jpg", ".jpeg", ".png", ".bmp", ".tga" };

/* Pointer to self (xmb_submenu structure) */
xmb_submenu *photo_self = NULL;

int photo_xmb_offset_x = 0;
int photo_xmb_offset_y = 0;
int photo_xmb_offset_ox = 0;
int photo_xmb_offset_tx = 0;
int photo_stage = 0;
int photo_animtimer = 0;
int photo_animtimer_b = 0;
int photo_levels = 0;
int photo_stopthread = 0;

int photo_rotate = 0;
short photo_display_type = 0;   /* 0 = fit all, 1 = fit partial */

SDL_Surface *photo_fs_image = NULL;
SDL_Surface *photo_fs_cache = NULL;

SDL_Surface *photo_fs_icons[8];
short photo_fs_isel = 0;
short photo_flags = 0;

float photo_zoomvalue = 1;
int photo_move_x = 0;
int photo_move_y = 0;

pthread_t pbid;
pthread_mutex_t photo_lock;

#define PF_MENUBAR 1

void sub_photo_popupvar(int value)
{
	photo_fs_isel = value;
}

void sub_photo_init(xmb_submenu * menu)
{
    photo_self = menu;

    photo_xmb_offset_x = 0;
    photo_xmb_offset_ox = 0;
    photo_xmb_offset_tx = 0;
    photo_xmb_offset_y = 0;
    photo_levels = 0;

    photo_stage = 3;
    photo_animtimer_b = photo_animtimer = SDL_GetTicks();

    memset(photo_path, 0, sizeof(photo_path));
    sprintf(photo_path, "%s", (char *) menu->extra);

    photo_fs_image = NULL;

    photo_flags = 0;
    photo_fs_isel = 0;
    photo_zoomvalue = 1;

    photo_fs_icons[0] =
        gfx_load_image("images/icons/photo_wallpaper.png", 0);
    photo_fs_icons[1] =
    	gfx_load_image("images/icons/photo_zoom0.png", 0);
    photo_fs_icons[2] =
    	gfx_load_image("images/icons/photo_zoom1.png", 0);
    photo_fs_icons[3] =
    	gfx_load_image("images/icons/photo_zoom2.png", 0);
    photo_fs_icons[4] =
        gfx_load_image("images/icons/photo_rotate0.png", 0);
    photo_fs_icons[5] =
        gfx_load_image("images/icons/photo_rotate1.png", 0);
    photo_fs_icons[6] =
        gfx_load_image("images/icons/photo_display.png", 0);
    photo_fs_icons[7] =
    	gfx_load_image("images/icons/photo_help.png", 0);

	pthread_mutex_init(&photo_lock, NULL);

    LIST_init();
    POPUP_init();

	POPUP_setrevert(NULL, 0);

	POPUP_add(photo_fs_icons[0], "Set Wallpaper", sub_photo_popupvar, 0);
	POPUP_add(photo_fs_icons[1], "Zoom Default", sub_photo_popupvar, 1);
	POPUP_add(photo_fs_icons[2], "Zoom Out", sub_photo_popupvar, 2);
	POPUP_add(photo_fs_icons[3], "Zoom In", sub_photo_popupvar, 3);
	POPUP_add(photo_fs_icons[4], "Rotate Left", sub_photo_popupvar, 4);
	POPUP_add(photo_fs_icons[5], "Rotate Right", sub_photo_popupvar, 5);
	POPUP_add(photo_fs_icons[6], "Size Fit", sub_photo_popupvar, 6);
	POPUP_add(photo_fs_icons[7], "Help", sub_photo_popupvar, 7);

	POPUP_setselected(0);
}

void sub_photo_destroy()
{
    if (photo_fs_image)
        SDL_FreeSurface(photo_fs_image);
    photo_fs_image = NULL;

    POPUP_destroy();
}

int photoIsImage(const char *path)
{
    unsigned int i = 0;
    if (!path)
        return 0;

    for (i = 0; i < sizeof(photo_extensions) / sizeof(char *); i++)
    {
        int extlen = strlen(photo_extensions[i]);

        if (strlen(path) - extlen > 0 &&
            !strncasecmp(&path[strlen(path) - extlen], photo_extensions[i],
                         extlen))
        {
            return 1;
        }
    }

    return 0;
}

/* returns the file extension if it is an image */
char *photoGetExt(char *path)
{
    unsigned int i = 0;
    if (!path)
        return NULL;

    for (i = 0; i < (sizeof(photo_extensions) / sizeof(char *)); i++)
    {
        int extlen = strlen(photo_extensions[i]);
        if (strlen(path) - extlen > 0 &&
            !strncasecmp(&path[strlen(path) - extlen], photo_extensions[i],
                         extlen))
        {
            return photo_extensions[i];
        }
    }

    return NULL;
}

void *photo_thread_browse(void *arg)
{
    int items_added = 0;
    struct stat statbuf;
    struct dirent **namelist;
    int files = 0;
    unsigned typeloop = 0, i = 0;

	xmb_activateFlag(XMB_SYNC);

    photo_stopthread = 0;

    files = scandir(photo_path, &namelist, 0, alphasort);
    if (files < 0)
    {
        perror("scandir");
        xmb_deactivateFlag(XMB_SYNC);
        return ((void *) 0);
    }

    /* Loop twice 1 = directories, 2 = files ... should sort as well */
    for (typeloop = 0; typeloop < 2; typeloop++)
        for (i = 0; i < files; i++)
        {
            if (strcmp(namelist[i]->d_name, ".")
                && strcmp(namelist[i]->d_name, "..")
                && !photo_stopthread)
            {
                char *tpath = (char *) calloc(1, 1024);
                char subtitle[64];
                SDL_Surface *icon = NULL;
                SDL_Surface *typeicon = NULL;

                memset(&subtitle, 0, sizeof(subtitle));

                sprintf(tpath, "%s/%s", photo_path, namelist[i]->d_name);
                stat(tpath, &statbuf);

                if (S_ISDIR(statbuf.st_mode) && typeloop == 0)
                {
                    icon = gfx_load_image("images/icons/folder.png", 0);
                    pthread_mutex_lock(&photo_lock);
                    LIST_add(icon, typeicon, namelist[i]->d_name, NULL,
                             tpath);
					pthread_mutex_unlock(&photo_lock);
                    items_added++;
                }
                else if (S_ISREG(statbuf.st_mode) && typeloop == 1)
                {
                    char *ext = photoGetExt(namelist[i]->d_name);

                    strftime(subtitle, sizeof(subtitle),
                             "%Y-%m-%d   %H:%M",
                             localtime(&statbuf.st_mtime));

                    if (ext)
                    {
                    	int l = 0;
                        char extimg_path[1024];
                        memset(&extimg_path, 0, sizeof(extimg_path));

                        namelist[i]->d_name[strlen(namelist[i]->d_name) -
                                            strlen(ext)] = '\0';
                        ext =
                            &namelist[i]->
                            d_name[strlen(namelist[i]->d_name) + 1];

						for(l = 0; ext && l < strlen(ext); l++)
							ext[l] = tolower(ext[l]);

                        sprintf(extimg_path, "images/icons/%s.png", ext);
                        typeicon = gfx_load_image(extimg_path, 0);
                    }

					if (!photoIsImage(tpath))
                    {
                        free(tpath);
                        continue;
                    }

					pthread_mutex_lock(&photo_lock);
                    LIST_add(icon, typeicon, namelist[i]->d_name, subtitle,
                             tpath);
					pthread_mutex_unlock(&photo_lock);
                    items_added++;
                }
            }

            if (typeloop)
                free(namelist[i]);
        }

    free(namelist);

	pthread_mutex_lock(&photo_lock);
    if (!items_added)
    {
        LIST_setEmptyMsg("There are no images.");
    }
    else if (LIST_getSelected())
    {
        LIST_setEmptyMsg(NULL);
    }
    pthread_mutex_unlock(&photo_lock);


	if (config_lookup_bool(&CONFIG, "photopreview"))
	{
		tlistitem *pcurr = NULL;
		SDL_Surface *icon = NULL;
		int index = 0;

		while((pcurr = LIST_getelement(index++)) != NULL && !photo_stopthread)
		{
            SDL_Surface *tmpi = NULL;
            int sx = 0, sy = 0;
			icon = NULL;

			if (!(photoIsImage((char*)pcurr->extra) && (icon =
				gfx_load_image((char*)pcurr->extra, 0)) != NULL))
			{
				if (icon)
					SDL_FreeSurface(icon);
            	icon = NULL;
            	continue;
			}

			if (icon != NULL)
            {
                if (icon->w >= icon->h)
                {
                    sx = PICONSIZE;
                    sy = (int) ((float) icon->h / (float) icon->w * sx);
                }
                else
                {
                    sy = PICONSIZE;
                    sx = (int) ((float) icon->w / (float) icon->h * sy);
                }

                if (sx <= 0)
                    sx = 1;
                if (sy <= 0)
                    sy = 1;

                tmpi = gfx_scale_surface(icon, sx, sy);

                if (tmpi)
                {
                    SDL_FreeSurface(icon);
                    icon = tmpi;
                    tmpi = gfx_new_surface(32, 32, 1);
                    SDL_SetAlpha(icon, 0, 0);   /* remove alpha flag from src */
                    gfx_draw_image(icon, (32 - icon->w) / 2,
                                    (32 - icon->h) / 2, tmpi);
                }
                SDL_FreeSurface(icon);
                icon = tmpi;
            }

			pthread_mutex_lock(&photo_lock);
			LIST_seticon(icon, pcurr);
			pthread_mutex_unlock(&photo_lock);
		}


	}


	xmb_deactivateFlag(XMB_SYNC);

    return ((void *) 0);
}

void photoBrowse()
{
	pthread_create(&pbid, NULL, photo_thread_browse, NULL);
}

void sub_photo_draw(SDL_Surface * screen)
{
    if (photo_stage == 1)       /* move left */
    {
    	int colwidth = 0;
    	pthread_mutex_lock(&photo_lock);
    	colwidth = LIST_columnWidth();
    	pthread_mutex_unlock(&photo_lock);

        if (!photo_xmb_offset_tx)
        {
            photo_xmb_offset_ox = photo_xmb_offset_x;
            pthread_mutex_lock(&photo_lock);
            photo_xmb_offset_tx = -colwidth;
            pthread_mutex_unlock(&photo_lock);
        }
        photo_xmb_offset_x -=
            rint(colwidth / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - photo_animtimer));
    }
    else if (photo_stage == 2)  /* move right */
    {
    	int colwidth = 0;
    	pthread_mutex_lock(&photo_lock);
    	colwidth = LIST_columnWidth();
    	pthread_mutex_unlock(&photo_lock);

        if (!photo_xmb_offset_tx)
        {
            photo_xmb_offset_ox = photo_xmb_offset_x;
            photo_xmb_offset_tx = colwidth;
        }
        photo_xmb_offset_x +=
            rint(colwidth / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - photo_animtimer));
    }
    else if (photo_stage == 3)  /* move left (inital) */
    {
    	int colwidth = 0;
    	pthread_mutex_lock(&photo_lock);
    	colwidth = LIST_columnWidth();
    	pthread_mutex_unlock(&photo_lock);

        if (!photo_xmb_offset_tx)
        {
            photo_xmb_offset_ox = photo_xmb_offset_x;
            photo_xmb_offset_tx = -colwidth;
        }
        photo_xmb_offset_x -=
            rint(colwidth / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - photo_animtimer));
    }
    else if (photo_stage == 4)  /* move right (final) */
    {
    	int colwidth = 0;
    	pthread_mutex_lock(&photo_lock);
    	colwidth = LIST_columnWidth();
    	pthread_mutex_unlock(&photo_lock);

        if (!photo_xmb_offset_tx)
        {
            photo_xmb_offset_ox = photo_xmb_offset_x;
            photo_xmb_offset_tx = colwidth;
        }
        photo_xmb_offset_x +=
            rint(colwidth / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - photo_animtimer));
    }
    /* Stupid solution to put the XMB in the right place */
    else if (photo_stage == 5)
    {
        photo_stage++;
    }
    else if (photo_stage == 6)
    {
        photoBrowse();
        photo_stage = 0;
    }

    if (SDL_GetTicks() - photo_animtimer_b > XMBMOVEDELAY && photo_stage)
    {
        if (photo_stage == 1 || photo_stage == 3)
        {
        	pthread_mutex_lock(&photo_lock);
            LIST_setEmptyMsg(NULL);
            pthread_mutex_unlock(&photo_lock);
            photo_stage = 5;
        }

        if (photo_stage == 4 && photo_levels == -1)
        {
            photo_xmb_offset_x = 0;
            pthread_mutex_lock(&photo_lock);
            LIST_destroy();
            pthread_mutex_unlock(&photo_lock);
            sub_photo_destroy();
            submenu = NULL;
        }

        if (photo_stage <= 5)
        {
            photo_xmb_offset_x = photo_xmb_offset_ox + photo_xmb_offset_tx;
            photo_xmb_offset_tx = 0;
            photo_xmb_offset_ox = 0;
        }
        if (photo_stage > 0 && photo_stage < 5)
            photo_stage = 0;
    }
    photo_animtimer = SDL_GetTicks();

    if (photo_stage != PS_FULLSCREEN)
    {
        if (photo_stage != 3 && photo_stage != 4)
        {
        	pthread_mutex_lock(&photo_lock);
            LIST_draw(screen, (int) photo_xmb_offset_x + ICONSCALE_X - 5,
                      0);
			pthread_mutex_unlock(&photo_lock);
        }
    }
    else                        /* Full screen viewier */
    {
        tlistitem *item = NULL;

        pthread_mutex_lock(&photo_lock);
        item = LIST_getSelected();
        pthread_mutex_unlock(&photo_lock);

        if (item && item->extra)
        {
            SDL_FillRect(screen, NULL,
                         SDL_MapRGB(screen->format, 200, 200, 200));
            if (photo_fs_image)
            {
                gfx_draw_image(photo_fs_image,
                               ((screen->w - photo_fs_image->w) / 2.0) +
                               photo_move_x,
                               ((screen->h - photo_fs_image->h) / 2.0) +
                               photo_move_y, screen);

                if (photo_flags & PF_MENUBAR)
                {
                	POPUP_draw(screen);
                	/*
                    for (i = 0;
                         i <
                         sizeof(photo_fs_icons) / sizeof(SDL_Surface *);
                         i += 2)
                    {
                        gfx_draw_image(photo_fs_icons
                                       [i +
                                        (photo_fs_isel * 2 == i ? 0 : 1)],
                                       screen->w -
                                       24 *
                                       ((sizeof(photo_fs_icons) /
                                         sizeof(SDL_Surface *)) / 2) +
                                       (i / 2 * 24) - 20, 30, screen);
                    }*/
                }
            }
        }
    }
}

int photo_newFullscreenImage(void)
{
    SDL_Surface *screen = SDL_GetVideoSurface();
    struct stat statbuf;
    tlistitem *item = LIST_getSelected();
    SDL_Surface *tmp = NULL;

    if (!item || (item && !item->extra))        /* Weird error */
    {
        return 1;
    }

    stat((char *) item->extra, &statbuf);
    if (S_ISREG(statbuf.st_mode))       /* this IS an image */
    {
        float scale_size = 0;
        if (photo_fs_image)
        {
            SDL_FreeSurface(photo_fs_image);
            photo_fs_image = NULL;
        }

        if (photo_fs_cache == NULL)
        {
            photo_fs_cache = gfx_load_image((char *) item->extra, 0);
            if (!photo_fs_cache)        /* if errors */
                return 1;
        }

        photo_fs_image = SDL_ConvertSurface(photo_fs_cache,
                                            photo_fs_cache->format,
                                            photo_fs_cache->flags);

        tmp = rotozoomSurface(photo_fs_image, photo_rotate, 1, 0);
        SDL_FreeSurface(photo_fs_image);
        photo_fs_image = tmp;

        if (photo_display_type ^
            (photo_fs_image->w / (float) photo_fs_image->h >
             PS_ASPECTRATIO))
            scale_size = screen->w / (float) photo_fs_image->w;
        else
            scale_size = screen->h / (float) photo_fs_image->h;

        tmp =
            rotozoomSurface(photo_fs_image, 0,
                            scale_size * photo_zoomvalue, 0);
        SDL_FreeSurface(photo_fs_image);
        photo_fs_image = tmp;
    }
    else
        return -1;

    return 0;
}

void sub_photo_checkBounds()
{
    SDL_Surface *screen = SDL_GetVideoSurface();
    int imgw = 0, scrw = 0;

    if (photo_fs_image->w > screen->w)
    {
        imgw = rint(photo_fs_image->w / (float) 2);
        scrw = (rint(screen->w / (float) 2) + photo_move_x);
        if (scrw > imgw)
            photo_move_x += imgw - scrw;

        imgw = -rint(photo_fs_image->w / (float) 2);
        scrw = -(rint(screen->w / (float) 2) - photo_move_x);
        if (scrw < imgw)
            photo_move_x += imgw - scrw;
    }
    else
        photo_move_x = 0;

    if (photo_fs_image->h > screen->h)
    {
        imgw = rint(photo_fs_image->h / (float) 2);
        scrw = (rint(screen->h / (float) 2) + photo_move_y);
        if (scrw > imgw)
            photo_move_y += imgw - scrw;

        imgw = -rint(photo_fs_image->h / (float) 2);
        scrw = -(rint(screen->h / (float) 2) - photo_move_y);
        if (scrw < imgw)
            photo_move_y += imgw - scrw;
    }
    else
        photo_move_y = 0;
}

void sub_photo_handle_input(unsigned int button)
{
    switch (button)
    {
    case GP2X_BUTTON_L:
        {
            if (photo_stage == PS_FULLSCREEN)
            {
                SDL_Surface *surf_tmp = photo_fs_cache;
                float tmpzoom = photo_zoomvalue;
                photo_zoomvalue = 1;
                photo_fs_cache = NULL;

                if (LIST_up() && photo_newFullscreenImage())    /* failed */
                {
                    LIST_down();
                    photo_zoomvalue = tmpzoom;
                    photo_fs_cache = surf_tmp;
                }
                else
                {
                    SDL_FreeSurface(surf_tmp);
                }
            }
        }
        break;

    case GP2X_BUTTON_R:
        {
            if (photo_stage == PS_FULLSCREEN)
            {
                SDL_Surface *surf_tmp = photo_fs_cache;
                float tmpzoom = photo_zoomvalue;
                photo_zoomvalue = 1;
                photo_fs_cache = NULL;

                if (LIST_down() && photo_newFullscreenImage())  /* failed */
                {
                    LIST_up();
                    photo_zoomvalue = tmpzoom;
                    photo_fs_cache = surf_tmp;
                }
                else
                {
                    SDL_FreeSurface(surf_tmp);
                }
            }
        }
        break;

    case GP2X_BUTTON_B:
        if (photo_stage == PS_FULLSCREEN)
        {
            photo_stage = 0;
            xmb_deactivateFlag(XMB_APPFULLSCREEN | XMB_NOBATT);
            photo_rotate = 0;
            photo_display_type = 0;
            photo_flags = 0;
            photo_zoomvalue = 1;
            if (photo_fs_cache)
                SDL_FreeSurface(photo_fs_cache);
            photo_fs_cache = NULL;
            break;
        }

    case GP2X_BUTTON_LEFT:
        {
        	void *tret;
        	photo_stopthread = 1;
        	if (pbid)
        		pthread_join(pbid, &tret);

            if (!photo_stage && photo_stage < 5)
            {
                sfx_play(SFXBACK);
                LIST_setEmptyMsg(NULL);
                if (LIST_out())
                {
                    photo_stage = 4;
                }
                else
                    photo_stage = 2;
                photo_levels--;
                photo_animtimer_b = photo_animtimer = SDL_GetTicks();
            }
            else if (photo_stage == PS_FULLSCREEN)
            {
                photo_move_x += 10;
                sub_photo_checkBounds();
            }
            break;
        }
    case GP2X_BUTTON_RIGHT:
        {
            if (photo_stage == PS_FULLSCREEN)
            {
                photo_move_x -= 10;
                sub_photo_checkBounds();
                break;
            }

            break;
        }
    case GP2X_BUTTON_UP:
        {
        	if (photo_flags & PF_MENUBAR)
            {
                if(POPUP_up())
                {
                	sfx_play(SFXMOVE);
	                POPUP_execsel();
	            }
                break;
            }

            if (photo_stage == PS_FULLSCREEN)
            {
                photo_move_y += 10;
                sub_photo_checkBounds();
                break;
            }

			pthread_mutex_lock(&photo_lock);
            if (LIST_up())
                sfx_play(SFXMOVE);
            pthread_mutex_unlock(&photo_lock);

            break;
        }
    case GP2X_BUTTON_DOWN:
        {
        	if (photo_flags & PF_MENUBAR)
            {
                if(POPUP_down())
                {
                	sfx_play(SFXMOVE);
	                POPUP_execsel();
	            }
                break;
            }

            if (photo_stage == PS_FULLSCREEN)
            {
                photo_move_y -= 10;
                sub_photo_checkBounds();
                break;
            }
			pthread_mutex_lock(&photo_lock);
            if (LIST_down())
                sfx_play(SFXMOVE);
            pthread_mutex_unlock(&photo_lock);

            break;
        }
    case GP2X_BUTTON_Y:
        {
            if (photo_stage == PS_FULLSCREEN)
            {
                if (photo_flags & PF_MENUBAR)
                    photo_flags ^= PF_MENUBAR;
                else
                    photo_flags |= PF_MENUBAR;
            }
            break;
        }
    case GP2X_BUTTON_X:
        {
            if (photo_flags & PF_MENUBAR)
            {
                switch (photo_fs_isel)
                {
                case 0:
                    {
                        tlistitem *item = LIST_getSelected();
                        config_setting_t *cfgd;

                        msgbox(SDL_GetVideoSurface(), NULL, "Wallpaper", "Please Wait...",
                               NONE);
                        bg_set((char *) item->extra);
                        msgbox_retval();
                        bg_showbg(1);

                        cfgd = config_lookup(&CONFIG, "usewallpaper");
                        if (cfgd)
                        {
                            config_setting_set_bool(cfgd, 1);
                            cfg_save();
                        }
                        msgbox(SDL_GetVideoSurface(), NULL, "Wallpaper",
                               "Your new wallpaper has now been set.", OK);
                        msgbox_retval();

                    }
                    break;

                case 1:
                    photo_zoomvalue = 1.0;
                    photo_newFullscreenImage();
                    sub_photo_checkBounds();
                    break;

                case 2:
                    photo_zoomvalue -= 0.1;
                    if (photo_zoomvalue < 0.1)
                        photo_zoomvalue = 0.1;
                    photo_newFullscreenImage();
                    sub_photo_checkBounds();
                    break;

                case 3:
                    photo_zoomvalue += 0.1;
                    if (photo_zoomvalue > 2.0)
                        photo_zoomvalue = 2.0;
                    photo_newFullscreenImage();
                    sub_photo_checkBounds();
                    break;

                case 4:
                    photo_rotate += 90;
                    if (photo_rotate > 360)
                        photo_rotate = 90;

                    photo_newFullscreenImage();
                    sub_photo_checkBounds();
                    break;

                case 5:
                    photo_rotate -= 90;
                    if (photo_rotate < 0)
                        photo_rotate = 270;
                    photo_newFullscreenImage();
                    sub_photo_checkBounds();
                    break;

                case 6:
                    photo_display_type = !photo_display_type;
                    photo_newFullscreenImage();
                    sub_photo_checkBounds();
                    break;

                case 7:
                    printf("Help!\n");
                    break;

                default:
                    break;
                }
                break;
            }

            if (!photo_stage && LIST_getSelected())
            {
                tlistitem *item = NULL;
                char *path = NULL;
                struct stat statbuf;

                if (li_create_new)
                    break;

				pthread_mutex_lock(&photo_lock);
                LIST_setEmptyMsg(NULL);
                item = LIST_getSelected();
                pthread_mutex_unlock(&photo_lock);

                if (!item)
                    break;

                path = (char *) item->extra;
                if (!path)
                    break;

                stat(path, &statbuf);

                if (S_ISDIR(statbuf.st_mode))
                {
                	pthread_mutex_lock(&photo_lock);
                    LIST_in();
                    pthread_mutex_unlock(&photo_lock);

                    sprintf(photo_path, "%s", path);
                    photo_levels++;
                    photo_stage = 1;
                    photo_animtimer_b = photo_animtimer = SDL_GetTicks();
                    sfx_play(SFXOK);
                }
                else
                {
                    photo_stage = PS_FULLSCREEN;
                    xmb_activateFlag(XMB_APPFULLSCREEN | XMB_NOBATT);
                    photo_newFullscreenImage();
                }
            }
            break;
        }
    default:
        break;
    }
}

int sub_photo_get_offset_x()
{
    return photo_xmb_offset_x;
}

int sub_photo_get_offset_y()
{
    return 0;
}
