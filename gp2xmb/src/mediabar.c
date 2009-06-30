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

#include <SDL_mixer.h>
#include <SDL_rotozoom.h>
#include <libgen.h>

#include "gp2xmb.h"
#include "mediabar.h"
#include "volumecontrol.h"
#include "sub_game.h"
#include "sub_photo.h"
#include "sub_music.h"
#include "sub_settings.h"
#include "sub_shortcuts.h"

/* XMB List */
xmb_node *root = NULL;
xmb_node *last = NULL;

/* Fonts - TODO: should be handled by a font.c file */
SFont_Font *font_small = NULL;
SFont_Font *font_small_semi = NULL;
SFont_Font *font_normal = NULL;
SFont_Font *font_normal_semi = NULL;
SFont_Font *font_monospace = NULL;

/* Pictures */
SDL_Surface *pic_line = NULL;

long selected = 0;              /* current selected item in X dir */
int menumove_offset_x;          /* used for level 1 */
int menumove_offset_y;          /* used for level 2 */
short menumovedir;              /* 0=still,1=right,-1=left,2=down,-2=up */
int menumove_timer;             /* timer for xmb animations */
int xmbflags;                   /* various activation flags */

xmb_submenu *submenu;           /* Magic */

int vgamma = 10;


void firmware_execute(struct submenu *self)
{
    char ctmp[1024];
    char *dir_name = NULL;
	int outro = 0;

    memset(ctmp, 0, sizeof(ctmp));
    strncpy(ctmp, self->extra, sizeof(ctmp) - 1);

    config_lookup_bool(&CONFIG, "outro", &outro);

    if (outro)
        gfx_draw_outro(SDL_GetVideoSurface());

    gp2xmb_deinit();

    gp2x_setclock(200);

    dir_name = (char *) calloc(1, 1024);
    strncpy(dir_name, ctmp, 1023);
    chdir((char *) dirname(dir_name));
    free(dir_name);

    system(ctmp);

    gp2xmb_init();
}

/*
 * Launch the original frontend.
 *
 */
void xmb_launch_frontend(struct submenu *menu)
{
    relaunchsystem = 0;
    exit(0);
}

/**
 * Media bar init function, run this to create the XMB structure
 * @return non-zero on failure
 */
int xmb_init(void)
{
    config_setting_t *cfgd = NULL;

    /* Load various fonts */
    font_small =
        SFont_InitFont(gfx_load_image("images/font/small.png", 0), NULL);
    if (!font_small)
    {
        fprintf(stderr, "An error occured while loading the font '%s'.\n",
                "");
        return -1;
    }
    font_small_semi =
        SFont_InitFont(gfx_load_image("images/font/small_semi.png", 0),
                       NULL);
    if (!font_small_semi)
    {
        fprintf(stderr, "An error occured while loading the font '%s'.\n",
                "font_small_semi");
        return -1;
    }
    progress_set(5);

    font_normal =
        SFont_InitFont(gfx_load_image("images/font/normal.png", 0), NULL);
    if (!font_normal)
    {
        fprintf(stderr, "An error occured while loading the font '%s'.\n",
                "");
        return -1;
    }
    font_normal =
        SFont_InitFont(gfx_load_image("images/font/normal_e.png", 0),
                       font_normal);
    if (!font_normal)
    {
        fprintf(stderr, "An error occured while loading the font '%s'.\n",
                "");
        return -1;
    }
    progress_set(10);

    font_normal_semi =
        SFont_InitFont(gfx_load_image("images/font/normal_semi.png", 0),
                       NULL);
    if (!font_normal_semi)
    {
        fprintf(stderr, "An error occured while loading the font '%s'.\n",
                "");
        return -1;
    }
    progress_set(15);

    font_monospace =
        SFont_InitFont(gfx_load_image("images/font/monospace.png", 0),
                       NULL);
    if (!font_monospace)
    {
        fprintf(stderr, "An error occured while loading the font '%s'.\n",
                "");
        return -1;
    }
    progress_set(20);

    pic_line = gfx_load_image("images/icons/line.png", 1);
    if (!pic_line)
    {
        fprintf(stderr,
                "An error occured while loading a required image '%s'.\n",
                "images/icons/line.png");
        return -1;
    }

    progress_set(45);

    /* Populate the XMB */
    xmb_add(NULL, "settings", "Settings", NULL);

    xmb_add(&last, "exit", "Original Frontend", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = NULL;
    last->childlast->pmenu->handle_input = NULL;
    last->childlast->pmenu->xmb_offset_x = NULL;
    last->childlast->pmenu->xmb_offset_y = NULL;
    last->childlast->pmenu->init = xmb_launch_frontend;
    last->childlast->pmenu->destroy = NULL;
    last->childlast->pmenu->extra = NULL;

    xmb_add(&last, "update", "System Update", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = NULL;
    last->childlast->pmenu->handle_input = NULL;
    last->childlast->pmenu->xmb_offset_x = NULL;
    last->childlast->pmenu->xmb_offset_y = NULL;
    last->childlast->pmenu->init = callback_systemupdate;
    last->childlast->pmenu->destroy = NULL;
    last->childlast->pmenu->extra = NULL;

    xmb_add(&last, "usb", "USB Connection", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_settings_draw;
    last->childlast->pmenu->handle_input = sub_settings_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_settings_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_settings_get_offset_y;
    last->childlast->pmenu->init = sub_settings_init;
    last->childlast->pmenu->destroy = sub_settings_destroy;
    last->childlast->pmenu->extra = (void *) SETUSB;

    xmb_add(&last, "conf_video", "Video Settings", NULL);
    xmb_add(&last, "conf_photo", "Photo Settings", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_settings_draw;
    last->childlast->pmenu->handle_input = sub_settings_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_settings_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_settings_get_offset_y;
    last->childlast->pmenu->init = sub_settings_init;
    last->childlast->pmenu->destroy = sub_settings_destroy;
    last->childlast->pmenu->extra = (void *) SETPHOTO;

    xmb_add(&last, "conf_system", "System Settings", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_settings_draw;
    last->childlast->pmenu->handle_input = sub_settings_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_settings_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_settings_get_offset_y;
    last->childlast->pmenu->init = sub_settings_init;
    last->childlast->pmenu->destroy = sub_settings_destroy;
    last->childlast->pmenu->extra = (void *) SETSYSTEM;

    /* Sound */
    xmb_add(&last, "conf_sound", "Sound Settings", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_settings_draw;
    last->childlast->pmenu->handle_input = sub_settings_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_settings_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_settings_get_offset_y;
    last->childlast->pmenu->init = sub_settings_init;
    last->childlast->pmenu->destroy = sub_settings_destroy;
    last->childlast->pmenu->extra = (void *) SETSOUND;

    /* Theme */
    xmb_add(&last, "conf_theme", "Theme Settings", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_settings_draw;
    last->childlast->pmenu->handle_input = sub_settings_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_settings_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_settings_get_offset_y;
    last->childlast->pmenu->init = sub_settings_init;
    last->childlast->pmenu->destroy = sub_settings_destroy;
    last->childlast->pmenu->extra = (void *) SETTHEME;


    /* Power save */
    xmb_add(&last, "conf_power", "Power Save Settings", NULL);

    /* Settings default select nr 2 */
    last->selected = 1;

    progress_set(50);

    xmb_add(NULL, "photo", "Photo", NULL);

    xmb_add(&last, "nand", "Built-in Memory", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_photo_draw;
    last->childlast->pmenu->handle_input = sub_photo_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_photo_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_photo_get_offset_y;
    last->childlast->pmenu->init = sub_photo_init;
    last->childlast->pmenu->destroy = sub_photo_destroy;
#ifdef TARGET_GP2X
    last->childlast->pmenu->extra = "/mnt/nand";
#else
    last->childlast->pmenu->extra = "/tmp/mnt/nand";
#endif

    xmb_add(&last, "sd", "SD Memory Card", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_photo_draw;
    last->childlast->pmenu->handle_input = sub_photo_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_photo_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_photo_get_offset_y;
    last->childlast->pmenu->init = sub_photo_init;
    last->childlast->pmenu->destroy = sub_photo_destroy;
#ifdef TARGET_GP2X
    last->childlast->pmenu->extra = "/mnt/sd";
#else
    last->childlast->pmenu->extra = "/tmp/mnt/sd";
#endif
	last->selected = 1;

    progress_set(55);

	xmb_add(NULL, "music", "Music", NULL);
    xmb_add(&last, "nand", "Built-in Memory", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_music_draw;
    last->childlast->pmenu->handle_input = sub_music_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_music_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_music_get_offset_y;
    last->childlast->pmenu->init = sub_music_init;
    last->childlast->pmenu->destroy = sub_music_destroy;
#ifdef TARGET_GP2X
    last->childlast->pmenu->extra = "/mnt/nand";
#else
    last->childlast->pmenu->extra = "/tmp/mnt/nand";
#endif

    xmb_add(&last, "sd", "SD Memory Card", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_music_draw;
    last->childlast->pmenu->handle_input = sub_music_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_music_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_music_get_offset_y;
    last->childlast->pmenu->init = sub_music_init;
    last->childlast->pmenu->destroy = sub_music_destroy;
#ifdef TARGET_GP2X
    last->childlast->pmenu->extra = "/mnt/sd";
#else
    last->childlast->pmenu->extra = "/tmp/mnt/sd";
#endif

	last->selected = 1;



    /*xmb_add(&last,"nand","Built-in Memory",NULL); */

    progress_set(60);

    xmb_add(NULL, "video", "Video", NULL);
    xmb_add(&last, "firmware", "Firmware Player", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = NULL;
    last->childlast->pmenu->handle_input = NULL;
    last->childlast->pmenu->xmb_offset_x = NULL;
    last->childlast->pmenu->xmb_offset_y = NULL;
    last->childlast->pmenu->init = firmware_execute;
    last->childlast->pmenu->destroy = NULL;
#ifdef TARGET_GP2X
    last->childlast->pmenu->extra = "/usr/gp2x/mplayer";
#else
    last->childlast->pmenu->extra = "/usr/bin/xterm";
#endif

    /*xmb_add(&last,"nand","Built-in Memory",NULL); */

    progress_set(65);

    xmb_add(NULL, "game", "Game", NULL);
    xmb_add(&last, "nand", "Built-in Memory", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_game_draw;
    last->childlast->pmenu->handle_input = sub_game_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_game_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_game_get_offset_y;
    last->childlast->pmenu->init = sub_game_init;
    last->childlast->pmenu->destroy = sub_game_destroy;
#ifdef TARGET_GP2X
    last->childlast->pmenu->extra = "/mnt/nand";
#else
    last->childlast->pmenu->extra = "/tmp/mnt/nand";
#endif

    xmb_add(&last, "myshortcuts", "My Shortcuts", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_shc_draw;
    last->childlast->pmenu->handle_input = sub_shc_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_shc_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_shc_get_offset_y;
    last->childlast->pmenu->init = sub_shc_init;
    last->childlast->pmenu->destroy = sub_shc_destroy;
#ifdef TARGET_GP2X
    last->childlast->pmenu->extra = "/mnt/sd";
#else
    last->childlast->pmenu->extra = "/tmp/mnt/sd";
#endif

    xmb_add(&last, "sd", "SD Memory Card", NULL);
    last->childlast->pmenu = (xmb_submenu *) malloc(sizeof(xmb_submenu));
    last->childlast->pmenu->draw = sub_game_draw;
    last->childlast->pmenu->handle_input = sub_game_handle_input;
    last->childlast->pmenu->xmb_offset_x = sub_game_get_offset_x;
    last->childlast->pmenu->xmb_offset_y = sub_game_get_offset_y;
    last->childlast->pmenu->init = sub_game_init;
    last->childlast->pmenu->destroy = sub_game_destroy;
#ifdef TARGET_GP2X
    last->childlast->pmenu->extra = "/mnt/sd";
#else
    last->childlast->pmenu->extra = "/tmp/mnt/sd";
#endif



    last->selected = 1;

    progress_set(70);

    /* xmb_add(NULL,"network","Network",NULL); */

    config_lookup_int(&CONFIG, "defaultmenu", &selected);

    if (selected < 0)
        selected = 0;
    else if (selected > xmb_count(root) - 1)
        selected = xmb_count(root) - 1;

    cfgd = config_lookup(&CONFIG, "defaultmenu");
    if (cfgd)
    {
        config_setting_set_int(cfgd, selected);
    }

    menumove_offset_x = 0;
    menumove_offset_y = 0;
    menumovedir = 0;
    menumove_timer = 0;
    xmbflags = 0;
    submenu = NULL;

    progress_set(75);

    return 0;
}

/**
 * Destroyes the media bar
 * @return non-zero on failure
 */
int xmb_destroy(void)
{
    if (font_small)
        SFont_FreeFont(font_small);
    if (font_normal)
        SFont_FreeFont(font_normal);
    if (font_normal_semi)
        SFont_FreeFont(font_normal_semi);
    if (font_monospace)
        SFont_FreeFont(font_monospace);
    if (font_small_semi)
        SFont_FreeFont(font_small_semi);
    sfx_destroy();

    xmb_rdel(root);
    root = NULL;
    last = NULL;

    return 0;
}

int xmb_rdel(xmb_node * begin)
{
    while (begin != NULL)
    {
        xmb_node *this = begin;
        begin = begin->next;
        xmb_rdel(this->childroot);
        if (this && this->pmenu)
        {
            if (this->pmenu->destroy)
                this->pmenu->destroy();
            free(this->pmenu);
        }
        if (this)
            free(this);
        this = NULL;
    }
    return 0;
}


int xmb_add(xmb_node ** node, char *name, char *title, char *subtitle)
{
    SDL_Surface *scale = NULL;
    xmb_node *curr = NULL;
    char filename[512];
    filename[0] = '\0';

    if (!subtitle)
        subtitle = "";

    if (node == NULL)
    {
        if (!root)
        {
            root = (xmb_node *) malloc(sizeof(xmb_node));

            if (!root)
                return -1;      /* malloc error */

            last = root;
            last->next = NULL;
            last->prev = NULL;
        }
        else
        {
            last->next = (xmb_node *) malloc(sizeof(xmb_node));
            if (!last->next)
                return -1;      /* malloc error */

            last->next->prev = last;
            last = last->next;
            last->next = NULL;
        }
        curr = last;
    }
    else
    {
        if (!(*node)->childroot)
        {
            (*node)->childroot = (xmb_node *) malloc(sizeof(xmb_node));

            if (!(*node)->childroot)
                return -1;      /* malloc error */
            (*node)->childlast = (*node)->childroot;
            (*node)->childlast->next = NULL;
            (*node)->childlast->prev = NULL;
            curr = (*node)->childlast;
        }
        else
        {
            (*node)->childlast->next =
                (xmb_node *) malloc(sizeof(xmb_node));
            if (!(*node)->childlast->next)
                return -1;      /* malloc error */

            (*node)->childlast->next->prev = (*node)->childlast;
            (*node)->childlast = (*node)->childlast->next;
            (*node)->childlast->next = NULL;
            curr = (*node)->childlast;
        }
    }

    curr->childroot = NULL;
    curr->childlast = NULL;
    curr->fp_subtitle = NULL;
    curr->pmenu = NULL;
    curr->selected = 0;

    /* potential buffer overflow */
    sprintf(filename, "images/icons/%s.png", name);
    curr->tex_base[0] = gfx_load_image(filename, 1);

    /* make semi transparent */
    curr->tex_base[1] = gfx_load_image(filename, 0);
    if (node == NULL)
    {
        scale = zoomSurface(curr->tex_base[1],
                            ((float) ICONSIZE_X / ICONSCALE_X),
                            ((float) ICONSIZE_Y / ICONSCALE_Y), 1);
        SDL_FreeSurface(curr->tex_base[1]);
        curr->tex_base[1] = scale;
    }
    gfx_set_alpha(curr->tex_base[1], 192);

    /* make more semi transparent */
    curr->tex_base[2] = gfx_load_image(filename, 0);
    if (node == NULL)
    {
        scale = zoomSurface(curr->tex_base[2],
                            ((float) ICONSIZE_X / ICONSCALE_X),
                            ((float) ICONSIZE_Y / ICONSCALE_Y), 1);
        SDL_FreeSurface(curr->tex_base[2]);
        curr->tex_base[2] = scale;
    }
    gfx_set_alpha(curr->tex_base[2], 64);

    strncpy(curr->title, title, sizeof(curr->title) - 1);
    strncpy(curr->subtitle, subtitle, sizeof(curr->subtitle) - 1);


    return 0;
}

int xmb_up(void)
{
    unsigned int count = 0;
    xmb_node *d = root;

    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_UP);
        return 0;
    }

    for (count = 0; count < selected; count++, d = d->next)
        if (d == NULL)
            return -1;

    if (d->selected > 0 && menumovedir != 1 && menumovedir != -1)
    {
        d->selected--;

        menumove_offset_y = -(ICONSIZE_Y + ICONSPACEINGY);
        menumovedir = -2;
        menumove_timer = SDL_GetTicks();
        sfx_play(SFXMOVE);
    }
    return 0;
}


int xmb_upleft(void)
{
    return 0;
}

int xmb_left(void)
{
    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_LEFT);
        return 0;
    }

    if (selected > 0 && menumovedir != 2 && menumovedir != -2)
    {
        selected--;

        menumove_offset_x = -(ICONSIZE_X + ICONSPACEING);
        menumovedir = -1;
        menumove_timer = SDL_GetTicks();
        sfx_play(SFXMOVE);
    }
    return 0;
}

int xmb_downleft(void)
{
    return 0;
}

int xmb_down(void)
{
    unsigned int count = 0;
    xmb_node *current = root;
    int length = 0;

    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_DOWN);
        return 0;
    }

    /* find selected 1st level node */
    for (count = 0; count < selected; count++, current = current->next)
        if (current == NULL)
            return -1;

    length = xmb_count(current->childroot);     /* length of level 2 */

    if (current->selected < length - 1 && menumovedir != 1
        && menumovedir != -1)
    {
        current->selected++;

        menumove_offset_y = (ICONSIZE_Y + ICONSPACEINGY);
        menumovedir = 2;
        menumove_timer = SDL_GetTicks();
        sfx_play(SFXMOVE);
    }

    return 0;
}

int xmb_downright(void)
{
    return 0;
}

int xmb_right(void)
{
    xmb_node *d = root;
    int count = 0;

    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_RIGHT);
        return 0;
    }

    while (d != NULL)
    {
        count++;
        d = d->next;
    }

    if (selected < count - 1 && menumovedir != 2 && menumovedir != -2)
    {
        selected++;

        menumove_offset_x = (ICONSIZE_X + ICONSPACEING);
        menumovedir = 1;
        menumove_timer = SDL_GetTicks();
        sfx_play(SFXMOVE);
    }

    return 0;
}

int xmb_upright(void)
{
    return 0;
}

int xmb_start(void)
{
    return 0;
}

int xmb_select(void)
{
    return 0;
}

int xmb_R(void)
{
    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_R);
        return 0;
    }

    return 0;
}

int xmb_L(void)
{
    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_L);
        return 0;
    }

    return 0;
}

int xmb_A(void)
{
    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_A);
        return 0;
    }

    return 0;
}

int xmb_B(void)
{
    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_B);
        return 0;
    }

    return 0;
}

int xmb_Y(void)
{
    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_Y);
        return 0;
    }

    return 0;
}

int xmb_X(void)
{
    unsigned int count = 0;
    xmb_node *curr = root;
    int max = selected;

    if (submenu && submenu->handle_input)
    {
        submenu->handle_input(GP2X_BUTTON_X);
        return 0;
    }

    /* find selected node in level 1 */
    for (count = 0; curr != NULL && count < max;
         count++, curr = curr->next);

    if (!curr)
        return 1;

    max = curr->selected;
    curr = curr->childroot;

    /* find selected node in level 2 */
    for (count = 0; curr != NULL && count < max;
         count++, curr = curr->next);

    if (!curr)
        return 1;

    submenu = curr->pmenu;

    if (submenu)
    {
        if (submenu->init)
            submenu->init(curr->pmenu);

        sfx_play(SFXOK);
    }

    return 0;
}

int xmb_volup(void)
{
    volume_up();                /* from volumecontrol.h */
    return 0;
}

int xmb_voldown(void)
{
    volume_down();              /* from volumecontrol.h */
    return 0;
}

int xmb_click(void)
{
    xmb_X();
    return 0;
}

int xmb_count(xmb_node * p_root)
{
    unsigned int length = 0;
    for (length = 0; p_root != NULL; p_root = p_root->next, length++);
    return length;
}

void xmb_activateFlag(int flag)
{
    xmbflags |= flag;
}

void xmb_deactivateFlag(int flag)
{
    xmbflags ^= flag;
}

int xmb_getFlagState(int flag)
{
    return (xmbflags & flag);
}

int xmb_draw(SDL_Surface * screen)
{
    int offset_x = -((ICONSIZE_X + ICONSPACEING) * selected);
    int loop = 0;
    int base_unsel;
    int base_sel;
    xmb_node *d = NULL;

    if (menumovedir == -1 || menumovedir == 1)
        menumove_offset_x -=
            rint(((ICONSIZE_X +
                   ICONSPACEING) / (float) XMBMOVEDELAY) *
                 (SDL_GetTicks() - menumove_timer)) * menumovedir;

    if (menumovedir == -2 || menumovedir == 2)
        menumove_offset_y -=
            rint(((ICONSIZE_Y +
                   ICONSPACEINGY) / (float) XMBMOVEDELAY) *
                 (SDL_GetTicks() - menumove_timer)) * (menumovedir >
                                                       0 ? 1 : -1);

    if (menumovedir)
        menumove_timer = SDL_GetTicks();

    if ((menumovedir == 1 && menumove_offset_x < 0) ||
        (menumovedir == -1 && menumove_offset_x > 0))
    {
        menumovedir = 0;
        menumove_offset_x = 0;
        menumove_timer = 0;
    }

    if ((menumovedir == 2 && menumove_offset_y < 0) ||
        (menumovedir == -2 && menumove_offset_y > 0))
    {
        menumovedir = 0;
        menumove_offset_y = 0;
        menumove_timer = 0;
    }

    if (xmbflags & XMB_SIDEBAR)
        offset_x -= (ICONSIZE_X);

    offset_x += menumove_offset_x;

    if (submenu && submenu->xmb_offset_x)
    {
        offset_x += submenu->xmb_offset_x();
    }

    if (xmbflags & XMB_SIDEBAR)
    {
        base_unsel = 2;
        base_sel = 0;
    }
    else
    {
        base_unsel = 1;
        base_sel = 0;
    }

    /* draw the xmb tree */
    for (d = root;
         d != NULL && !xmb_getFlagState(XMB_APPFULLSCREEN);
         d = d->next, loop++)
    {
        unsigned int l = 0;
        if (loop == selected && menumovedir != 1 && menumovedir != -1)
        {
            /* Level 2 menu items */
            int center_offset = 0;
            xmb_node *level2 = NULL;
            int offset_y =
                XMBOFFSET_Y - ((ICONSIZE_Y + ICONSPACEINGY) * d->selected)-10;

            offset_y += menumove_offset_y;

            level2 = d->childroot;

			/* Render everything above the level one menu */
            for (l = 0; level2 != NULL && l < d->selected;
                 level2 = level2->next, offset_y +=
                 ICONSIZE_Y + ICONSPACEINGY, l++)
            {
                int far_move = 0;
                SDL_Surface *icon = NULL;
                float zoom = 0.5;

                if (offset_y + 5 + ICONSIZE_Y < 0)
                    continue;

                far_move += (l + 1 == d->selected && (menumovedir == 2)) ?
                    (int)rint((menumove_offset_y/(float)ICONSIZE_Y)*(ICONSIZE_Y*2)) : 0;

				if(far_move)
				{
					zoom += (float)far_move / (ICONSIZE_Y*4);
				}

                icon =
                    zoomSurface(level2->tex_base[base_unsel], zoom, zoom,
                                SMOOTHING_OFF);

                gfx_draw_image(icon, offset_x + XMBOFFSET_X +((ICONSIZE_X-icon->w)/2),
                               far_move + offset_y + ((ICONSIZE_Y-icon->h)/2), screen);

                if (!submenu)
                    gfx_draw_text(screen,
                                  font_normal_semi,
                                  offset_x + XMBOFFSET_X + ICONSIZE_X + 5,
                                  far_move + offset_y +
                                  ((ICONSIZE_Y -
                                    gfx_text_height(font_normal_semi)) /
                                   2), NOSHADOW, NOGLOW, level2->title);

                SDL_FreeSurface(icon);
            }

            offset_y = menumove_offset_y;

			/* render everything below the level 1 menu */
            for (l = 0; level2 != NULL;
                 level2 = level2->next, offset_y +=
                 ICONSIZE_Y + ICONSPACEINGY, l++)
            {
            	float zoom = 0.5;
            	SDL_Surface *icon = NULL;
                char *subtitle = level2->subtitle;
                int far_move = XMBBASEBOTTOM;

                if (submenu)
                    subtitle = "";

                far_move += (l == 0
                             && (menumovedir ==
                                 -2)) ? (int)rint((menumove_offset_y/(float)ICONSIZE_Y)*(ICONSIZE_Y*2)) : 0;

				if(far_move != XMBBASEBOTTOM)
				{
					zoom = 1.0;
					zoom += (float)(far_move-XMBBASEBOTTOM) / (ICONSIZE_Y*4);
				}
				else if(l == 1 && menumovedir == -2)
				{
					int offv = (int)rint((menumove_offset_y/(float)ICONSIZE_Y)*(ICONSIZE_Y*2));
					zoom -= (float)(offv) / (ICONSIZE_Y*4);
				}
				else if(l == 0 && menumovedir == 2)
				{
					int offv = (int)rint((menumove_offset_y/(float)ICONSIZE_Y)*(ICONSIZE_Y*2));
					zoom = 1.0;
					zoom -= (float)(offv) / (ICONSIZE_Y*4);
				}

				if(l == 0 && menumovedir != 2 && menumovedir != -2)
				{
					zoom = 1.0;
					icon =
                    	zoomSurface(level2->tex_base[base_sel], zoom, zoom,
                                SMOOTHING_OFF);
				}
				else
				{
                	icon =
                    	zoomSurface(level2->tex_base[base_unsel], zoom, zoom,
                                SMOOTHING_OFF);
                }

                gfx_draw_image(icon, offset_x + XMBOFFSET_X +((ICONSIZE_X-icon->w)/2), far_move + offset_y+((ICONSIZE_Y-icon->h)/2), screen);

                if (!submenu)
                {
                    SFont_Font *tfont = NULL;
                    int glow, shadow;
                    if (l == 0 && menumovedir != 2 && menumovedir != -2)
                    {
                        tfont = font_normal;
                        glow = USEGLOW;
                        shadow = USESHADOW;
                    }
                    else
                    {
                        tfont = font_normal_semi;
                        glow = NOGLOW;
                        shadow = NOSHADOW;
                    }

                    gfx_draw_text(screen, tfont,
                                  offset_x + XMBOFFSET_X + ICONSIZE_X + 5,
                                  far_move + offset_y +
                                  ((ICONSIZE_Y -
                                    gfx_text_height(tfont)) / 2), shadow,
                                  glow, level2->title);
                }

                SDL_FreeSurface(icon);
            }

            /* Level 1 */
            gfx_draw_image(d->tex_base[0],
                           offset_x + XMBOFFSET_X -
                           ((ICONSCALE_X - ICONSIZE_X) / 2),
                           XMBOFFSET_Y - ((ICONSCALE_Y - ICONSIZE_Y)),
                           screen);

            center_offset =
                ((ICONSIZE_X - SFont_TextWidth(font_small, d->title)) / 2);

            if (!submenu)
                gfx_draw_text(screen,
                              font_small,
                              offset_x + XMBOFFSET_X + center_offset,
                              XMBOFFSET_Y + ICONSIZE_Y, USESHADOW, NOGLOW,
                              d->title);
        }
        else
        {
            if (!submenu)
                gfx_draw_image(d->tex_base[base_unsel],
                               offset_x + XMBOFFSET_X, XMBOFFSET_Y,
                               screen);
        }

        offset_x += ICONSIZE_X + ICONSPACEING;
    }

    if (submenu && submenu->draw)
        submenu->draw(screen);

    return 0;
}
