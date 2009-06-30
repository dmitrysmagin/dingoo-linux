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

#include <libgen.h>
#include <math.h>

#include "gp2xmb.h"
#include "sub_shortcuts.h"
#include "popup.h"

/* A few defines */
#define SHCICONX 128
#define SHCICONY 74

/* stealing variables from the mediabar */
extern SFont_Font *font_normal;
extern xmb_submenu *submenu;

short shc_init_stage = 1;
SDL_Surface *shc_arrow = NULL;
int shc_move_xmb = 0;           /* 0=still, 1=left; 2=right */
int shc_xmb_offset_x = 0;
int shc_xmb_offset_y = 0;
int shc_timer = 0;
int shc_timer2 = 0;

/* Move requiest to handle button down while animating */
int shc_movereq = 0;
int shc_movedir = 0;

int shc_selection = 0;
int shc_menucount = 0;
int shc_showpopup = 0;

/* Pointer to shc_self (xmb_submenu structure) */
xmb_submenu *shc_self = NULL;

/* Shortcut list */
shc_node *shc_root = NULL;
shc_node *shc_last = NULL;

void shc_delete_list();
void sub_shc_popexec(int value);
int sub_game_remove_shortcut(config_setting_t * eroot, int index); /* Borrow */

void sub_shc_init(xmb_submenu * menu)
{
    shc_self = menu;
    shc_init_stage = 1;
    shc_arrow = NULL;
    shc_move_xmb = 0;
    shc_xmb_offset_x = 0;
    shc_xmb_offset_y = 0;
    shc_timer = 0;
    shc_timer2 = 0;
    shc_movedir = 0;
    shc_movereq = 0;
    shc_selection = 0;
    shc_menucount = 0;
    shc_showpopup = 0;
    POPUP_init();
}

void sub_shc_destroy()
{
    shc_delete_list();
    POPUP_destroy();
}

void add_shc(const char *filename, const char *title, SDL_Surface * icon)
{
    if (!shc_root)
    {
        shc_root = (shc_node *) calloc(1, sizeof(shc_node));
        if (!shc_root)
            return;             /* malloc error */

        shc_last = shc_root;
    }
    else
    {
        shc_last->next = (shc_node *) calloc(1, sizeof(shc_node));
        if (!shc_last->next)
            return;             /* malloc error */
        shc_last = shc_last->next;
    }
    shc_last->next = NULL;
    shc_last->icon_orig = icon;

    if (icon)
        shc_last->icon_curr = gfx_scaleperc_surface(icon, 0.5, 0.5);
    else
        shc_last->icon_curr = NULL;

    if (filename)
        strncpy(shc_last->path, filename, sizeof(shc_last->path) - 1);

    if (title)
    {
        strncpy(shc_last->title, title, sizeof(shc_last->title) - 1);
    }
    else
    {
        char *buff = NULL;
        buff = (char *) calloc(1, sizeof(shc_last->path));
        if (buff)
        {
            strncpy(buff, filename, sizeof(shc_last->path) - 1);
            strncpy(shc_last->title, basename(buff),
                    sizeof(shc_last->title) - 1);
            free(buff);
            if (strlen(shc_last->title) >= 4)
                shc_last->title[strlen(shc_last->title) - 4] = '\0';
        }
        else
            memset(shc_last->title, 0, sizeof(shc_last->title));
    }

    shc_last->pos_x = 0;
    shc_last->pos_y = 0;
    shc_last->move_x = 0;
    shc_last->move_y = 0;
    shc_menucount++;
}

void shc_assign_move(int direction)
{
    shc_node *curr = shc_root;

    while (curr)
    {
        if (direction < 0)      /* Move up */
        {
            if (curr->pos_y == 83)      /* big move */
            {
                curr->move_x = 32;
                curr->move_tx = 82;
                curr->move_y = -46;
                curr->move_ty = 37;
            }
            else if (curr->pos_y == 166)
            {
                curr->move_x = -32;
                curr->move_tx = 50;
                curr->move_y = -83;
                curr->move_ty = 83;
            }
            else
            {
                curr->move_y = -(SHCICONY / 2);
                curr->move_ty = curr->pos_y - (SHCICONY / 2);
            }
        }
        else                    /* Move down */
        {
            if (curr->pos_y == 83)      /* big move */
            {
                curr->move_x = 32;
                curr->move_tx = 82;
                curr->move_y = 83;
                curr->move_ty = 166;
            }
            else if (curr->pos_y == 37)
            {
                curr->move_x = -32;
                curr->move_tx = 50;
                curr->move_y = 46;
                curr->move_ty = 83;
            }
            else
            {
                curr->move_y = (SHCICONY / 2);
                curr->move_ty = curr->pos_y + (SHCICONY / 2);
            }
        }
        curr = curr->next;
    }
}

void shc_assign_initial_pos()
{
    shc_node *curr = shc_root;

    int cposy = 83;

    while (curr != NULL)
    {
        if (curr == shc_root)
        {
            curr->pos_x = 50;
            curr->pos_y = 83;

            cposy = 166;
            SDL_FreeSurface(curr->icon_curr);
            curr->icon_curr =
                gfx_scaleperc_surface(curr->icon_orig, 1.0, 1.0);
        }
        else
        {
            curr->pos_x = 82;
            curr->pos_y = cposy;
            cposy += (SHCICONY / 2);

        }
        curr = curr->next;
    }
}

void shc_delete_list()
{
    shc_node *curr = shc_root;

    while (curr)
    {
        shc_node *tmp = curr;
        tmp = curr->next;

        if (curr->icon_orig)
            SDL_FreeSurface(curr->icon_orig);
        if (curr->icon_curr)
            SDL_FreeSurface(curr->icon_curr);

        free(curr);
        curr = NULL;
        curr = tmp;
    }
    shc_root = NULL;
    shc_last = NULL;
    shc_menucount = 0;
}

int shc_load(void)
{
    config_setting_t *cfg = config_lookup(&CONFIG, "shortcuts");
    int i;

    if (!cfg)
        return -1;

    for (i = 0; i < config_setting_length(cfg); i++)
    {
        SDL_Surface *icon = NULL;
        config_setting_t *group = config_setting_get_elem(cfg, i);
        config_setting_t *celem = NULL;
        char *filename = NULL;
        char *title = NULL;
        char *pngfile = NULL;

        if (group)
        {
            const char *ctmp;
            celem = config_setting_get_member(group, "path");
            if (celem)
            {
                ctmp = config_setting_get_string(celem);
                if (ctmp)
                {
                    filename = (char *) calloc(1, 1024);
                    strncpy(filename, ctmp, 1023);
                }
            }

            celem = config_setting_get_member(group, "title");
            if (celem)
            {
                ctmp = config_setting_get_string(celem);
                if (ctmp)
                {
                    title = (char *) calloc(1, 1024);
                    strncpy(title, ctmp, 1023);
                }
            }

        }
        else
            continue;

        if (filename && strlen(filename))
        {
            pngfile = (char *) calloc(1, strlen(filename) + 1);

            strncpy(pngfile, filename, strlen(filename));
            strncpy(&pngfile[strlen(pngfile) - 3], "png", 3);
            icon = gfx_load_image(pngfile, 0);
            if (!icon)
                icon = gfx_load_image("images/icons/game_broken.png", 0);
            else if (icon->w < 128 || icon->h < 74)
            {
                SDL_Surface *tmp_icon = gfx_new_surface(128, 74, 0);
                SDL_FillRect(tmp_icon, NULL,
                             SDL_MapRGB((SDL_GetVideoSurface())->format,
                                        255, 255, 255));

                gfx_draw_image(icon, (128 - icon->w) / 2,
                               (74 - icon->h) / 2, tmp_icon);
                SDL_FreeSurface(icon);
                icon = SDL_DisplayFormatAlpha(tmp_icon);
                SDL_FreeSurface(tmp_icon);
            }

            free(pngfile);

            add_shc(filename, title, icon);

            if (title)
                free(title);

            if (filename)
                free(filename);
        }
    }

    return 0;
}

void sub_shc_stages(SDL_Surface * screen)
{
    if (!shc_arrow)
        shc_arrow = gfx_load_image("images/icons/arrow-left.png", 1);

    if (shc_init_stage == 1)
    {
        if (shc_timer && shc_xmb_offset_x > -80)
        {
            shc_xmb_offset_x -=
                (int) rint(((80) / (float) XMBMOVEDELAY) *
                           (SDL_GetTicks() - shc_timer));
        }
        else if (shc_xmb_offset_x == -80)
            shc_init_stage = 2;

        shc_timer = SDL_GetTicks();

        if (shc_xmb_offset_x < -80)
            shc_xmb_offset_x = -80;
    }
    else if (shc_init_stage == 2)
    {
        gfx_draw_text(screen, font_normal, 50, 115, USESHADOW, NOGLOW,
                      "Please Wait...");
        shc_init_stage++;
    }
    else if (shc_init_stage == 3)
    {
        /* Load everything */
        shc_load();

        shc_assign_initial_pos();
        shc_init_stage++;
    }
    else if (shc_init_stage == 4)
        shc_init_stage = 0;

    if (shc_init_stage == 10)
    {
        if (shc_timer && shc_xmb_offset_x < 0)
        {
            shc_xmb_offset_x +=
                (int) rint(((80) / (float) XMBMOVEDELAY) *
                           (SDL_GetTicks() - shc_timer));
        }
        else if (shc_xmb_offset_x == 0)
            shc_init_stage = 11;

        shc_timer = SDL_GetTicks();

        if (shc_xmb_offset_x > 0)
            shc_xmb_offset_x = 0;
    }
    else if (shc_init_stage == 11)
    {
        shc_xmb_offset_x = 0;
        shc_timer = 0;
        submenu = NULL;
        shc_init_stage = 1;
        shc_selection = 0;
        shc_delete_list();
    }
}

void sub_shc_draw(SDL_Surface * screen)
{
    int td, animating = 0;
    shc_node *curr = NULL;

    if (shc_init_stage)
    {
        sub_shc_stages(screen);
    }

    if (shc_movereq && !shc_movedir)
    {
        if (shc_movereq < 0)
        {
            sfx_play(SFXMOVE);
            shc_movedir = -1;
            shc_movereq++;
        }
        else if (shc_movereq > 0)
        {
            sfx_play(SFXMOVE);
            shc_movedir = 1;
            shc_movereq--;
        }
        shc_assign_move(shc_movedir);
        shc_timer2 = SDL_GetTicks();
    }

    if (!shc_init_stage)
        gfx_draw_image(shc_arrow, 25, 113, screen);

    curr = shc_root;

    td = (SDL_GetTicks() - shc_timer2);

    shc_timer2 = SDL_GetTicks();

    if (!curr && !shc_init_stage)       /* Did not find anything */
    {
        gfx_draw_text(screen, font_normal, 50, 115, USESHADOW, NOGLOW,
                      "There are no applications.");
    }

    while (curr && !shc_init_stage)
    {
        char t_name[32];
        t_name[0] = '\0';
        strncat(t_name, curr->title, 31);
        if (shc_movedir)
        {
            int move_td_x =
                (int) rint((curr->move_x) / (float) XMBMOVEDELAY * td);
            int move_td_y =
                (int) rint((curr->move_y) / (float) XMBMOVEDELAY * td);

            curr->pos_x += move_td_x;
            curr->pos_y += move_td_y;

            if ((move_td_x > 0 && curr->pos_x > curr->move_tx) ||
                (move_td_x < 0 && curr->pos_x < curr->move_tx))
            {
                curr->pos_x = curr->move_tx;
                curr->move_x = 0;
                curr->move_tx = 0;

                if (curr->pos_x == 50)
                {
                    SDL_FreeSurface(curr->icon_curr);
                    curr->icon_curr =
                        gfx_scaleperc_surface(curr->icon_orig, 1.0, 1.0);
                }
                else
                {
                    SDL_FreeSurface(curr->icon_curr);
                    curr->icon_curr =
                        gfx_scaleperc_surface(curr->icon_orig, 0.5, 0.5);
                }

            }

            if ((move_td_y > 0 && curr->pos_y > curr->move_ty) ||
                (move_td_y < 0 && curr->pos_y < curr->move_ty))
            {
                curr->pos_y = curr->move_ty;
                curr->move_y = 0;
                curr->move_ty = 0;
            }

            if (curr->move_x)
            {
                float bval =
                    ((1.0 / (SHCICONX / 4)) *
                     (curr->pos_x + (curr->move_x - curr->move_tx)) / 2.0);

                if (bval < 0)
                    bval = 0.5 + fabs(bval);
                else if (bval > 0)
                    bval = 0.5 + (0.5 - fabs(bval));

                if (bval)
                {
                    SDL_FreeSurface(curr->icon_curr);
                    curr->icon_curr =
                        gfx_scaleperc_surface(curr->icon_orig, bval, bval);
                }
            }

            if (curr->move_x || curr->move_y)
                animating = 1;
        }
        else if (curr->pos_y == 83)     /* selected ... ugly but it works, for now */
        {
            gfx_draw_text(screen,
                          font_normal,
                          curr->pos_x + SHCICONX + 10,
                          curr->pos_y +
                          ((SHCICONY - SFont_TextHeight(font_normal)) / 2),
                          USESHADOW, NOGLOW, curr->title);
        }

        if ((curr->pos_y + curr->icon_curr->h) > 0 && curr->pos_y < 240)
            gfx_draw_image(curr->icon_curr, curr->pos_x, curr->pos_y,
                           screen);

        curr = curr->next;
    }
    if (!animating)
        shc_movedir = 0;

	if(shc_showpopup)
		POPUP_draw(screen);
}

void sub_shc_handle_input(unsigned int button)
{
    switch (button)
    {
    case GP2X_BUTTON_B:
        {
            sfx_play(SFXBACK);
            if(shc_showpopup)
            {
            	POPUP_empty();
            	xmb_deactivateFlag(XMB_NOBATT | XMB_FOCUS);
				shc_showpopup = 0;
            }
            else
            {
            	shc_init_stage = 10;
            	shc_timer = 0;
           	}
            break;
        }
    case GP2X_BUTTON_LEFT:
        {
        	if(shc_showpopup)
        		break;

            sfx_play(SFXBACK);
            shc_init_stage = 10;
            shc_timer = 0;
            break;
        }
    case GP2X_BUTTON_UP:
        {
        	if(shc_showpopup)
        	{
        		if(POPUP_up())
        			sfx_play(SFXMOVE);

        		break;
        	}

            if (shc_selection > 0)
            {
                shc_movereq += 1;
                shc_selection--;
            }
            break;
        }
    case GP2X_BUTTON_DOWN:
        {
        	if(shc_showpopup)
        	{
				if(POPUP_down())
					sfx_play(SFXMOVE);

				break;
        	}

            if (shc_selection + 1 < shc_menucount)
            {
                shc_movereq -= 1;
                shc_selection++;
            }
            break;
        }
	case GP2X_BUTTON_Y:
        {
        	shc_node *curr = shc_root;
            unsigned int i = 0;

            for (i = 0; curr != NULL && i < shc_selection;
                 curr = curr->next, i++);

            if (!shc_init_stage && curr && !shc_showpopup)
            {
                struct stat statbuf;

                stat(curr->path, &statbuf);

                if (S_ISREG(statbuf.st_mode))
                {
                    xmb_activateFlag(XMB_NOBATT | XMB_FOCUS);

                    POPUP_setrevert(NULL, 0);

                    POPUP_add(NULL, "Delete Shortcut", sub_shc_popexec, 0);

                    POPUP_setselected(0);

		            shc_showpopup = 1;
                }
            }
            break;
        }
    case GP2X_BUTTON_X:
        {
            shc_node *curr = shc_root;
            unsigned int i = 0;

            if (shc_showpopup)
            {
                POPUP_execsel();
                sub_shc_destroy();         /* Empty list */
                sub_shc_init(shc_self);    /* Reload */
                xmb_deactivateFlag(XMB_NOBATT | XMB_FOCUS);
                POPUP_empty();
                shc_showpopup = 0;
                break;
            }

            for (i = 0; curr != NULL && i < shc_selection;
                 curr = curr->next, i++);

            if (curr)
            {
                char *dir_name = (char *) calloc(1, 1024);

                if (config_lookup_bool(&CONFIG, "outro"))
                    gfx_draw_outro(SDL_GetVideoSurface());

                gp2xmb_deinit();

                if (dir_name)
                {
                    strncpy(dir_name, curr->path, 1023);
                    chdir(dirname(dir_name));
                    free(dir_name);
                }

                gp2x_setclock(200);

                if (execl(curr->path, curr->path, NULL))
                {
                    gp2xmb_init();      /* start everything up again */
                    while (!xmb_getFlagState(XMB_LOADED))
                    {
                        bg_draw(SDL_GetVideoSurface());
                        progress_draw(SDL_GetVideoSurface(), 1);
                        SDL_Flip(SDL_GetVideoSurface());
                    }
                    msgbox(SDL_GetVideoSurface(),NULL, "System Error",
                           "Failed to execute program.", OK);
                    msgbox_retval();
                    return;
                }
            }

            break;
        }
    default:
        break;
    }
}

void sub_shc_popexec(int value)
{
	shc_node *item = shc_root;
    config_setting_t *root = config_lookup(&CONFIG, "shortcuts");
    char bname[1024];
    int i = 0;
    memset(bname, 0, sizeof(bname));

	for (i = 0; item != NULL && i < shc_selection; item = item->next, i++);

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
                    if (!strcmp(config_setting_get_string(member),item->path))
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

int sub_shc_get_offset_x()
{
    return shc_xmb_offset_x;
}

int sub_shc_get_offset_y()
{
    return 0;
}
