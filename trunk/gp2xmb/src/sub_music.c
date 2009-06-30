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

#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

/* Why do I need to specify these - why does not dirent do its job? */
extern int
scandir(__const char *__restrict __dir,
        struct dirent ***__restrict __namelist,
        int (*__selector) (__const struct dirent *),
        int (*__cmp) (__const void *, __const void *)) __nonnull((1, 2));

extern int alphasort(__const void *__e1, __const void *__e2)
__THROW __attribute_pure__ __nonnull((1, 2));

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_rotozoom.h>

#include "sub_music.h"
#include "gp2x.h"
#include "joystick.h"
#include "gfx.h"
#include "sfx.h"
#include "SFont.h"
#include "gp2xmb.h"
#include "mediabar.h"
#include "itemlist.h"

#include "vorbis_player.h"
#include "mp3_player.h"

/* stealing variables from the mediabar */
extern SFont_Font *font_normal;
extern SFont_Font *font_small;
extern SFont_Font *font_monospace;

extern xmb_submenu *submenu;

/* Search path variables */
char music_path[1024];
char *music_extensions[] = { ".ogg" };

/* Pointer to self (xmb_submenu structure) */
xmb_submenu *music_self = NULL;

int music_xmb_offset_x = 0;
int music_xmb_offset_y = 0;
int music_xmb_offset_ox = 0;
int music_xmb_offset_tx = 0;
int music_levels = 0;
int music_stage = 0;

int music_animtimer_b = 0;
int music_animtimer = 0;

short sub_music_playing = 0;

/* media function pointers */

int (*mus_init) (char *path) = NULL;
int (*mus_play) (void) = NULL;
int (*mus_stop) (void) = NULL;

unsigned int (*mus_getpos) (void) = NULL;
unsigned int (*mus_getlength) (void) = NULL;
int (*mus_ready) (void) = NULL;
int (*mus_fwd) (void) = NULL;
int (*mus_bwd) (void) = NULL;
int (*mus_eof) (void) = NULL;

#define PF_MENUBAR 1

void sub_music_init(xmb_submenu * menu)
{
    music_self = menu;

    music_xmb_offset_x = 0;
    music_xmb_offset_ox = 0;
    music_xmb_offset_tx = 0;
    music_xmb_offset_y = 0;
    music_levels = 0;

    music_stage = 3;
    music_animtimer_b = music_animtimer = SDL_GetTicks();

    memset(music_path, 0, sizeof(music_path));
    sprintf(music_path, "%s", (char *) menu->extra);

    sub_music_playing = 0;

    LIST_init();
}

void sub_music_destroy()
{
    if (mus_stop)
        mus_stop();
}

int sub_music_is(char *path)    /* Yoda naming */
{
    unsigned int i = 0;
    if (!path)
        return 0;

    for (i = 0; i < sizeof(music_extensions) / sizeof(char *); i++)
    {
        int extlen = strlen(music_extensions[i]);
        if (strlen(path) - extlen > 0
            && !strncasecmp(&path[strlen(path) - extlen],
                            music_extensions[i], extlen))
        {
            return 1;
        }
    }

    return 0;
}

/* returns the file extension if it is a music file */
char *sub_music_ext(char *path)
{
    unsigned int i = 0;
    if (!path)
        return NULL;

    for (i = 0; i < (sizeof(music_extensions) / sizeof(char *)); i++)
    {
        int extlen = strlen(music_extensions[i]);
        if (strlen(path) - extlen > 0
            && !strncasecmp(&path[strlen(path) - extlen],
                            music_extensions[i], extlen))
        {
            return music_extensions[i];
        }
    }

    return NULL;
}

void sub_music_browse()
{
    int items_added = 0;
    struct stat statbuf;
    struct dirent **namelist;
    int files = scandir(music_path, &namelist, 0, alphasort);
    unsigned int typeloop = 0, i = 0;
    if (files < 0)
    {
        perror("scandir");
        return;
    }

    /* Loop twice 1 = directories, 2 = files ... should sort as well */
    for (typeloop = 0; typeloop < 2; typeloop++)
        for (i = 0; i < files; i++)
        {
            if (strcmp(namelist[i]->d_name, ".")
                && strcmp(namelist[i]->d_name, ".."))
            {
                char *tpath = (char *) calloc(1, 1024);
                char subtitle[64];
                SDL_Surface *icon = NULL, *typeicon = NULL;

                memset(&subtitle, 0, sizeof(subtitle));

                sprintf(tpath, "%s/%s", music_path, namelist[i]->d_name);
                stat(tpath, &statbuf);

                if (S_ISDIR(statbuf.st_mode) && typeloop == 0)
                {
                    icon = gfx_load_image("images/icons/folder.png", 0);
                    LIST_add(icon, typeicon, namelist[i]->d_name, NULL,
                             tpath);
                    items_added++;
                }
                else if (S_ISREG(statbuf.st_mode) && typeloop == 1)
                {
                    char *ext = sub_music_ext(namelist[i]->d_name);
                    if (ext)
                    {
                        char extimg_path[1024];
                        char *stmp = NULL;
                        char martist[1024], mtitle[1024];
                        memset(martist, 0, sizeof(martist));
                        memset(mtitle, 0, sizeof(mtitle));


                        stmp = op_metainfo(tpath, OVMARTIST);
                        if (stmp)
                            strncpy(martist, stmp, sizeof(martist) - 1);
                        stmp = op_metainfo(tpath, OVMTITLE);
                        if (stmp)
                            strncpy(mtitle, stmp, sizeof(mtitle) - 1);

                        memset(&extimg_path, 0, sizeof(extimg_path));

                        namelist[i]->d_name[strlen(namelist[i]->d_name) -
                                            strlen(ext)] = '\0';
                        ext =
                            &(namelist[i]->
                              d_name[strlen(namelist[i]->d_name) + 1]);

                        sprintf(extimg_path, "images/icons/%s.png", ext);
                        typeicon = gfx_load_image(extimg_path, 0);

                        icon =
                            gfx_load_image("images/icons/media_num.png",
                                           1);
                        if (!martist[0])
                            martist[0] = '-';

                        if (!mtitle[0])
                            strncpy(mtitle, namelist[i]->d_name,
                                    sizeof(mtitle) - 1);

                        LIST_add(gfx_clone_surface(icon), typeicon, mtitle, martist, tpath);
                        items_added++;
                    }
                }
            }

            if (typeloop)
                free(namelist[i]);
        }

    free(namelist);

    if (!items_added)
    {
        LIST_setEmptyMsg("There are no tracks.");
    }
    else if (LIST_getSelected())
    {
        LIST_setEmptyMsg(NULL);
    }

    return;
}

void sub_music_draw(SDL_Surface * screen)
{
    if (music_stage == 1)       /* move left */
    {
        if (!music_xmb_offset_tx)
        {
            music_xmb_offset_ox = music_xmb_offset_x;
            music_xmb_offset_tx = -LIST_columnWidth();
        }
        music_xmb_offset_x -=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - music_animtimer));
    }
    else if (music_stage == 2)  /* move right */
    {
        if (!music_xmb_offset_tx)
        {
            music_xmb_offset_ox = music_xmb_offset_x;
            music_xmb_offset_tx = LIST_columnWidth();
        }
        music_xmb_offset_x +=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - music_animtimer));
    }
    else if (music_stage == 3)  /* move left (inital) */
    {
        if (!music_xmb_offset_tx)
        {
            music_xmb_offset_ox = music_xmb_offset_x;
            music_xmb_offset_tx = -LIST_columnWidth();
        }
        music_xmb_offset_x -=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - music_animtimer));
    }
    else if (music_stage == 4)  /* move right (final) */
    {
        if (!music_xmb_offset_tx)
        {
            music_xmb_offset_ox = music_xmb_offset_x;
            music_xmb_offset_tx = LIST_columnWidth();
        }
        music_xmb_offset_x +=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - music_animtimer));
    }
    else if (music_stage == 5)
    {
        music_stage++;
    }
    else if (music_stage == 6)
    {
        sub_music_browse();
        music_stage = 0;
    }

    if (SDL_GetTicks() - music_animtimer_b > XMBMOVEDELAY && music_stage)
    {
        if (music_stage == 1 || music_stage == 3)
        {
            LIST_setEmptyMsg("Please Wait...");
            music_stage = 5;
        }

        if (music_stage == 4 && music_levels == -1)
        {
            music_xmb_offset_x = 0;
            LIST_destroy();
            sub_music_destroy();
            submenu = NULL;
            sub_music_playing = 0;
            return;
        }

        if (music_stage <= 5)
        {
            music_xmb_offset_x = music_xmb_offset_ox + music_xmb_offset_tx;
            music_xmb_offset_tx = 0;
            music_xmb_offset_ox = 0;
        }

        if (music_stage > 0 && music_stage < 5)
            music_stage = 0;
    }
    music_animtimer = SDL_GetTicks();

    if (music_stage != MUS_FULLSCREEN)
    {
        if (music_stage != 3 && music_stage != 4)
        {
            LIST_draw(screen, (int) music_xmb_offset_x + ICONSCALE_X - 5,
                      0);
        }
    }
    else if (sub_music_playing)
    {
        tlistitem *sel;
        SDL_Rect wrect;
        char textstr[16];
        int minutes = 0, seconds = 0, tmin = 0, tsec = 0;
        textstr[0] = '\0';
        wrect.x = screen->w - 210;
        wrect.y = screen->h - 14;
        wrect.w = 200;
        wrect.h = 4;
        SDL_FillRect(screen, &wrect,
                     SDL_MapRGB(screen->format, 255, 255, 255));
        wrect.w *= mus_getpos() / (float) mus_getlength();
        wrect.w = ceil(wrect.w);
        SDL_FillRect(screen, &wrect,
                     SDL_MapRGB(screen->format, 0, 130, 180));

#ifdef TREMOR
        minutes = ((mus_getpos() / 1000) / 60);
        seconds = ((mus_getpos() / 1000) % 60);
        tmin = ((mus_getlength() / 1000) / 60);
        tsec = ((mus_getlength() / 1000) % 60);
#else
        minutes = ((mus_getpos()) / 60);
        seconds = ((mus_getpos()) % 60);
        tmin = ((mus_getlength()) / 60);
        tsec = ((mus_getlength()) % 60);
#endif

        sprintf(textstr, "%02i:%02i / %02i:%02i", minutes, seconds, tmin,
                tsec);
        gfx_draw_text(screen, font_monospace, 215, 210, USESHADOW, NOGLOW,
                      textstr);

        sel = LIST_getSelected();

        gfx_draw_image(sel->icon, 25, 102, screen);
        gfx_draw_text(screen, font_normal, 71, 100, USESHADOW, NOGLOW,
                      sel->title);
        gfx_draw_text(screen, font_small, 71, 126, USESHADOW, NOGLOW,
                      sel->subtitle);


        if (mus_eof())
        {
            if (LIST_down())
            {
                mus_stop();
                while (!mus_ready());
                sub_music_playsel();
            }
            else
            {
                sub_music_handle_input(GP2X_BUTTON_B);
            }
        }
    }
}

void sub_music_playsel(void)
{
    struct stat statbuf;
    char *path;
    tlistitem *item = NULL;

    item = LIST_getSelected();
    if (!item)
        return;

    path = (char *) item->extra;
    if (!path)
        return;

    stat(path, &statbuf);

    if (S_ISDIR(statbuf.st_mode))
    {
        LIST_in();
        sprintf(music_path, "%s", path);
        music_levels++;
        music_stage = 1;
        music_animtimer_b = music_animtimer = SDL_GetTicks();
        sfx_play(SFXOK);
    }
    else
    {
        Mix_CloseAudio();
        music_stage = MUS_FULLSCREEN;
        xmb_activateFlag(XMB_APPFULLSCREEN);
        if (!strncasecmp(sub_music_ext(path), ".ogg", 4))
        {
            mus_init = op_init;
            mus_play = op_play;
            mus_stop = op_stop;
            mus_getpos = op_getpos;
            mus_getlength = op_getlength;
            mus_ready = op_ready;
            mus_fwd = op_fwd;
            mus_bwd = op_bwd;
            mus_eof = op_eof;
        }
        else
        {
            mus_init = mp3_init;
            mus_play = mp3_play;
            mus_stop = mp3_stop;
            mus_getpos = mp3_getpos;
            mus_getlength = mp3_getlength;
            mus_ready = mp3_ready;
            mus_fwd = mp3_fwd;
            mus_bwd = mp3_bwd;
            mus_eof = mp3_eof;
        }

        mus_init(path);
        mus_play();

        sub_music_playing = 1;
    }
}

void sub_music_handle_input(unsigned int button)
{
    switch (button)
    {
    case GP2X_BUTTON_L:
        if (music_stage == MUS_FULLSCREEN)
        {
            if (LIST_up())
            {
                mus_stop();
                while (!mus_ready());
                sub_music_playsel();
            }
        }
        break;

    case GP2X_BUTTON_R:
        if (music_stage == MUS_FULLSCREEN)
        {
            if (LIST_down())
            {
                mus_stop();
                while (!mus_ready());
                sub_music_playsel();
            }
        }
        break;

    case GP2X_BUTTON_B:
        if (music_stage == MUS_FULLSCREEN && sub_music_playing)
        {
            sub_music_playing = 0;
            mus_stop();
            music_stage = 0;
            xmb_deactivateFlag(XMB_APPFULLSCREEN);

            while (!mus_ready());

            if (Mix_OpenAudio
                (AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS))
            {
                printf("Unable to open audio!\n");
            }
            break;
        }
    case GP2X_BUTTON_LEFT:
        {
            if (music_stage == MUS_FULLSCREEN && sub_music_playing)
            {
                mus_bwd();
                break;
            }

            if (!music_stage && music_stage < 5)
            {
                sfx_play(SFXBACK);
                LIST_setEmptyMsg(NULL);
                if (LIST_out())
                    music_stage = 4;
                else
                    music_stage = 2;
                music_levels--;
                music_animtimer_b = music_animtimer = SDL_GetTicks();
            }
            break;
        }
    case GP2X_BUTTON_RIGHT:
        if (music_stage == MUS_FULLSCREEN && sub_music_playing)
        {
            mus_fwd();
            break;
        }
        break;
    case GP2X_BUTTON_UP:
        {
            if (music_stage == MUS_FULLSCREEN && sub_music_playing)
            {
                break;
            }

            if (LIST_up())
                sfx_play(SFXMOVE);
            break;
        }
    case GP2X_BUTTON_DOWN:
        {
            if (music_stage == MUS_FULLSCREEN && sub_music_playing)
            {
                break;
            }

            if (LIST_down())
                sfx_play(SFXMOVE);
            break;
        }
    case GP2X_BUTTON_Y:
        {
            break;
        }
    case GP2X_BUTTON_X:
        {
            if (!music_stage && LIST_getSelected())
            {
                if (music_stage == MUS_FULLSCREEN && sub_music_playing)
                {
                    break;
                }

                if (li_create_new)
                    break;

                LIST_setEmptyMsg(NULL);

                sub_music_playsel();
            }
            break;
        }
    default:
        break;
    }
}

int sub_music_get_offset_x()
{
    return music_xmb_offset_x;
}

int sub_music_get_offset_y()
{
    return 0;
}
