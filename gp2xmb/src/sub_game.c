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
#include <libgen.h>
#include <dirent.h>

#include "gp2xmb.h"
#include "sub_photo.h"
#include "mediabar.h"
#include "itemlist.h"
#include "popup.h"

void sub_game_handle_input(unsigned int button);
void sub_game_popexec(int value);
int sub_game_remove_shortcut(config_setting_t * eroot, int index);

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
char game_path[1024];
char *game_extensions[] = { ".gpe", ".gpu" };

/* Pointer to self (xmb_submenu structure) */
xmb_submenu *game_self = NULL;

int game_xmb_offset_x = 0;
int game_xmb_offset_y = 0;
int game_xmb_offset_tx = 0;
int game_xmb_offset_ox = 0;
int game_stage = 0;
int game_animtimer = 0;
int game_animtimer_b = 0;
int game_levels = 0;

void sub_game_init(xmb_submenu * menu)
{
    game_self = menu;

    game_xmb_offset_x = 0;
    game_xmb_offset_y = 0;
    game_xmb_offset_tx = 0;
    game_xmb_offset_ox = 0;
    game_levels = 0;

    game_stage = 3;
    game_animtimer_b = game_animtimer = SDL_GetTicks();

    memset(game_path, 0, sizeof(game_path));
    sprintf(game_path, "%s", (char *) menu->extra);

    LIST_init();
    POPUP_init();
}

void sub_game_destroy()
{
    LIST_destroy();
    POPUP_destroy();
}

int game_is_exec(const char *path)
{
    unsigned int i = 0;
    if (!path)
        return 0;

    for (i = 0; i < sizeof(game_extensions) / sizeof(char *); i++)
    {
        int extlen = strlen(game_extensions[i]);

        if (strlen(path) - extlen > 0 &&
            !strncasecmp(&path[strlen(path) - extlen], game_extensions[i],
                         extlen))
        {
            return 1;
        }
    }

    return 0;
}

char *game_get_ext(char *path)
{
    unsigned int i = 0;
    if (!path)
        return NULL;

    for (i = 0; i < (sizeof(game_extensions) / sizeof(char *)); i++)
    {
        int extlen = strlen(game_extensions[i]);
        if (strlen(path) - extlen > 0 &&
            !strncasecmp(&path[strlen(path) - extlen], game_extensions[i],
                         extlen))
        {
            return game_extensions[i];
        }
    }

    return NULL;
}

void game_browse()
{
    int items_added = 0;
    struct stat statbuf;
    struct dirent **namelist;
    int files = scandir(game_path, &namelist, 0, alphasort);
    unsigned typeloop = 0, i = 0;
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
                char *tpath = (char *) calloc(1, 1024); /* Do NOT free this */
                char *tbpath = (char *) calloc(1, 1024);
                SDL_Surface *icon = NULL;
                SDL_Surface *typeicon = NULL;

                if (!tpath || !tbpath)
                {
                    if (tpath)
                        free(tpath);
                    if (tbpath)
                        free(tbpath);
                    continue;
                }

                sprintf(tpath, "%s/%s", game_path, namelist[i]->d_name);
                sprintf(tbpath, "%s/%s", game_path, namelist[i]->d_name);       /* backup */
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
                    SDL_Surface *tmpi = NULL;
                    int sx = 0, sy = 0;
                    char iconpath[1024];
                    char *ext = game_get_ext(namelist[i]->d_name);
                    memset(iconpath, 0, sizeof(iconpath));

                    if (ext)
                    {
                        namelist[i]->d_name[strlen(namelist[i]->d_name) -
                                            strlen(ext)] = '\0';
                        ext =
                            &namelist[i]->
                            d_name[strlen(namelist[i]->d_name) + 1];
                    }

                    if (!game_is_exec(tpath))
                    {
                        free(tpath);
                        free(tbpath);
                        continue;
                    }

                    sprintf(iconpath, "%s/%s.png", dirname(tbpath),
                            namelist[i]->d_name);

                    if ((icon = gfx_load_image(iconpath, 0)))
                    {
                        if (icon->w >= icon->h)
                        {
                            sx = PICONSIZE;
                            sy = (int) ((float) icon->h / (float) icon->w *
                                        sx);
                        }
                        else
                        {
                            sy = PICONSIZE;
                            sx = (int) ((float) icon->w / (float) icon->h *
                                        sy);
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
                            SDL_SetAlpha(icon, 0, 0);
                            gfx_draw_image(icon, (32 - icon->w) / 2,
                                           (32 - icon->h) / 2, tmpi);
                        }
                        SDL_FreeSurface(icon);
                        icon = tmpi;
                    }

                    LIST_add(icon, NULL, namelist[i]->d_name, NULL, tpath);
                    items_added++;

                    if (tbpath)
                        free(tbpath);
                }
            }

            if (typeloop)
                free(namelist[i]);
        }

    free(namelist);

    if (!items_added)
    {
        LIST_setEmptyMsg("There are no executables.");
    }
    else if (LIST_getSelected())
    {
        LIST_setEmptyMsg(NULL);
    }
    return;
}

void sub_game_draw(SDL_Surface * screen)
{
    if (game_stage == 1)        /* move left */
    {
        if (!game_xmb_offset_tx)
        {
            game_xmb_offset_ox = game_xmb_offset_x;
            game_xmb_offset_tx = -LIST_columnWidth();
        }
        game_xmb_offset_x -=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - game_animtimer));
    }
    else if (game_stage == 2)   /* move right */
    {
        if (!game_xmb_offset_tx)
        {
            game_xmb_offset_ox = game_xmb_offset_x;
            game_xmb_offset_tx = LIST_columnWidth();
        }
        game_xmb_offset_x +=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - game_animtimer));
    }
    else if (game_stage == 3)   /* move left (inital) */
    {
        if (!game_xmb_offset_tx)
        {
            game_xmb_offset_ox = game_xmb_offset_x;
            game_xmb_offset_tx = -LIST_columnWidth();
        }
        game_xmb_offset_x -=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - game_animtimer));
    }
    else if (game_stage == 4)   /* move right (final) */
    {
        if (!game_xmb_offset_tx)
        {
            game_xmb_offset_ox = game_xmb_offset_x;
            game_xmb_offset_tx = LIST_columnWidth();
        }
        game_xmb_offset_x +=
            rint(LIST_columnWidth() / ((float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - game_animtimer));
    }
    /* Stupid solution to allow the XMB in the right place */
    else if (game_stage == 5)
    {
        game_stage++;
    }
    else if (game_stage == 6)
    {
        game_browse();
        game_stage = 0;
    }

    if (SDL_GetTicks() - game_animtimer_b > XMBMOVEDELAY && game_stage)
    {
        if (game_stage == 1 || game_stage == 3)
        {
            LIST_setEmptyMsg("Please Wait...");
            game_stage = 5;
        }

        if (game_stage == 4 && game_levels == -1)
        {
            game_xmb_offset_x = 0;
            LIST_destroy();
            sub_game_destroy();
            submenu = NULL;
        }

        if (game_stage <= 5)
        {
            game_xmb_offset_x = game_xmb_offset_ox + game_xmb_offset_tx;
            game_xmb_offset_tx = 0;
            game_xmb_offset_ox = 0;
        }

        if (game_stage > 0 && game_stage < 5)
            game_stage = 0;
    }
    game_animtimer = SDL_GetTicks();

    if (game_stage != 3 && game_stage != 4)
    {
        LIST_draw(screen, (int) game_xmb_offset_x + ICONSCALE_X - 5, 0);
    }

    if (game_stage == 10)
        POPUP_draw(screen);
}

void sub_game_handle_input(unsigned int button)
{
    switch (button)
    {
    case GP2X_BUTTON_L:
        {

        }
        break;

    case GP2X_BUTTON_R:
        {

        }
        break;

    case GP2X_BUTTON_B:
        if (game_stage == 10)
        {
            game_stage = 0;
            POPUP_empty();
            xmb_deactivateFlag(XMB_NOBATT | XMB_FOCUS);
            break;
        }
    case GP2X_BUTTON_LEFT:
        {
            if (!game_stage)
            {
                sfx_play(SFXBACK);
                LIST_setEmptyMsg(NULL);
                if (LIST_out())
                {
                    game_stage = 4;
                }
                else
                    game_stage = 2;
                game_levels--;
                game_animtimer_b = game_animtimer = SDL_GetTicks();
            }
            break;
        }
    case GP2X_BUTTON_UP:
        {
            if (game_stage == 10)
            {
                if (POPUP_up())
                    sfx_play(SFXMOVE);
            }
            else if (LIST_up())
                sfx_play(SFXMOVE);
            break;
        }
    case GP2X_BUTTON_DOWN:
        {
            if (game_stage == 10)
            {
                if (POPUP_down())
                    sfx_play(SFXMOVE);
            }
            else if (LIST_down())
                sfx_play(SFXMOVE);
            break;
        }
    case GP2X_BUTTON_Y:
        {
            if (!game_stage && LIST_getSelected())
            {
                struct stat statbuf;
                tlistitem *item = LIST_getSelected();

                stat((char *) item->extra, &statbuf);

                if (!item)
                    break;

                if (S_ISREG(statbuf.st_mode))
                {
                    xmb_activateFlag(XMB_NOBATT | XMB_FOCUS);

                    POPUP_setrevert(NULL, 0);

                    POPUP_add(NULL, "Add Shortcut", sub_game_popexec, 0);
                    POPUP_add(NULL, "Delete Shortcut", sub_game_popexec,
                              1);

                    POPUP_setselected(0);
                    game_stage = 10;
                }
            }
            break;
        }
    case GP2X_BUTTON_X:
        {
            if (game_stage == 10)
            {
                POPUP_execsel();
                xmb_deactivateFlag(XMB_NOBATT | XMB_FOCUS);
                POPUP_empty();
                game_stage = 0;
                break;
            }

            if (!game_stage && LIST_getSelected())
            {
                tlistitem *item = NULL;
                char *path = NULL;
                struct stat statbuf;

                if (li_create_new)
                    break;

                LIST_setEmptyMsg(NULL);

                item = LIST_getSelected();
                if (!item)
                    break;

                path = (char *) item->extra;
                if (!path)
                    break;

                stat(path, &statbuf);

                if (S_ISDIR(statbuf.st_mode))
                {
                    LIST_in();
                    sprintf(game_path, "%s", path);
                    game_levels++;
                    game_stage = 1;
                    game_animtimer_b = game_animtimer = SDL_GetTicks();
                    sfx_play(SFXOK);
                }
                else
                {
                    if (item)
                    {
                        char *dir_name = (char *) calloc(1, 1024);

                        if (config_lookup_bool(&CONFIG, "outro"))
                            gfx_draw_outro(SDL_GetVideoSurface());

						gp2xmb_deinit();

                        if (dir_name)
                        {
                            strncpy(dir_name, path, 1023);
                            chdir(dirname(dir_name));
                            free(dir_name);
                        }

                        gp2x_setclock(200);

                        if (execl(path, path, NULL))
                        {
                            gp2xmb_init();      /* start everything up again */
                            while (!xmb_getFlagState(XMB_LOADED))
                            {
                                bg_draw(SDL_GetVideoSurface());
                                progress_draw(SDL_GetVideoSurface(), 1);
                                SDL_Flip(SDL_GetVideoSurface());
                            }
                            msgbox(SDL_GetVideoSurface(),NULL,"System Error",
                                   "Failed to execute program.", OK);
                            msgbox_retval();
                            return;
                        }
                    }

                }
            }
            break;
        }
    default:
        break;
    }
}

int sub_game_get_offset_x()
{
    return game_xmb_offset_x;
}

int sub_game_get_offset_y()
{
    return 0;
}

void sub_game_popexec(int value)
{
    tlistitem *item = LIST_getSelected();
    config_setting_t *root = config_lookup(&CONFIG, "shortcuts");
    config_setting_t *path = NULL;
    config_setting_t *title = NULL;
    char bname[1024];
    int i = 0;
    memset(bname, 0, sizeof(bname));

    if (!item || !root)
        return;

    switch (value)
    {
    case 0:
        {
            for (i = config_setting_length(root); i--;)
            {
                config_setting_t *elem = config_setting_get_elem(root, i);
                config_setting_t *member =
                    config_setting_get_member(elem, "path");

                if (member)
                {
                    if (!strcmp
                        (config_setting_get_string(member),
                         (char *) item->extra))
                    {
                        msgbox(SDL_GetVideoSurface(),NULL,"System Error",
                               "This Shortcut already exists.", OK);
                        msgbox_retval();
                        return;
                    }
                }
            }

            strncpy(bname, (char *) item->extra, sizeof(bname) - 1);

            root = config_setting_add(root, "sak", CONFIG_TYPE_GROUP);
            path = config_setting_add(root, "path", CONFIG_TYPE_STRING);
            title = config_setting_add(root, "title", CONFIG_TYPE_STRING);


            if (path)
                config_setting_set_string(path, bname);

            if (title)
                config_setting_set_string(title, basename(bname));

            msgbox(SDL_GetVideoSurface(),NULL, "My Shortcuts",
                   "A new Shortcut has been created.", OK);
            cfg_save();
            msgbox_retval();
        }
        break;

    case 1:
        {
            for (i = config_setting_length(root); i--;)
            {
                config_setting_t *elem = config_setting_get_elem(root, i);
                config_setting_t *member =
                    config_setting_get_member(elem, "path");

                if (member)
                {
                    if (!strcmp
                        (config_setting_get_string(member),
                         (char *) item->extra))
                    {
                        sub_game_remove_shortcut(root, i);
                        msgbox(SDL_GetVideoSurface(),NULL, "My Shortcuts",
                               "Shortcut has been deleted.", OK);
                        cfg_save();
                        msgbox_retval();
                        return;
                    }
                }
            }
            msgbox(SDL_GetVideoSurface(),NULL,"System Error","Unable to delete Shortcut.",
                   OK);
            msgbox_retval();
        }
        break;

    default:
        break;
    }
}

int sub_game_remove_shortcut(config_setting_t * eroot, int index)
{
    int i;
    char sname[1024];
    config_setting_t *troot = NULL;
    config_setting_t *vroot = NULL;

    memset(sname, 0, sizeof(sname));
    strncpy(sname, config_setting_name(eroot), sizeof(sname) - 1);

    troot =
        config_setting_add(config_root_setting(&CONFIG), "temp",
                           CONFIG_TYPE_LIST);

    for (i = 0; i < config_setting_length(eroot); i++)
    {
        config_setting_t *elem = config_setting_get_elem(eroot, i);
        config_setting_t *path = config_setting_get_member(elem, "path");
        config_setting_t *title = config_setting_get_member(elem, "title");

        if (i != index)
        {
            config_setting_t *group =
                config_setting_add(troot, "", CONFIG_TYPE_GROUP);
            config_setting_set_string(config_setting_add
                                      (group, "path", CONFIG_TYPE_STRING),
                                      config_setting_get_string(path));

            config_setting_set_string(config_setting_add(group,
                                                         "title",
                                                         CONFIG_TYPE_STRING),
                                      config_setting_get_string(title));
        }
    }

    config_setting_remove(config_root_setting(&CONFIG), sname);

    vroot =
        config_setting_add(config_root_setting(&CONFIG), sname,
                           CONFIG_TYPE_LIST);

    for (i = 0; i < config_setting_length(troot); i++)
    {
        config_setting_t *elem = config_setting_get_elem(troot, i);
        config_setting_t *path = config_setting_get_member(elem, "path");
        config_setting_t *title = config_setting_get_member(elem, "title");

        config_setting_t *group =
            config_setting_add(vroot, "", CONFIG_TYPE_GROUP);
        config_setting_set_string(config_setting_add
                                  (group, "path", CONFIG_TYPE_STRING),
                                  config_setting_get_string(path));

        config_setting_set_string(config_setting_add(group,
                                                     "title",
                                                     CONFIG_TYPE_STRING),
                                  config_setting_get_string(title));
    }

    config_setting_remove(config_root_setting(&CONFIG), "temp");

    return CONFIG_TRUE;
}
