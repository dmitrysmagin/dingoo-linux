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

#ifndef __MEDIABAR_H__
#define __MEDIABAR_H__

/* Defines */
#define ICONSIZE_X (32)
#define ICONSIZE_Y (32)
#define ICONSPACEING (32)
#define ICONSPACEINGY (0)
#define ICONSCALE_X (40)
#define ICONSCALE_Y (40)


#define XMBOFFSET_X (64)
#define XMBOFFSET_Y (48)
#define XMBBASEBOTTOM  (XMBOFFSET_Y + ICONSIZE_Y + 25)

#define XMBMOVEDELAY (125)

/* Define flags */
#define XMB_SIDEBAR 1
#define XMB_MOVEPX 2
#define XMB_MOVENX 4
#define XMB_MOVEPY 8
#define XMB_MOVENY 16
#define XMB_APPFULLSCREEN 32
#define XMB_NOBATT 64
#define XMB_LOADED 128
#define XMB_SYNC 256
#define XMB_FOCUS 512

/* Structures */
struct submenu
{
    void (*draw) (SDL_Surface *);
    void (*handle_input) (unsigned int);
    int (*xmb_offset_x) (void);
    int (*xmb_offset_y) (void);
    void (*init) (struct submenu *);
    void (*destroy) (void);
    void *extra;
};
typedef struct submenu xmb_submenu;

struct list_el
{
    /* graphics */
    SDL_Surface *tex_base[3];
    SDL_Surface *tex_shadow;
    SDL_Surface *tex_highlight;

    /* text */
    char title[32];             /* static title */
    char subtitle[32];          /* static subtitle */
    char (*fp_subtitle) (void); /* function pointer to dynamic subtitle */

    /* list structure */
    struct list_el *next;
    struct list_el *prev;
    struct list_el *childroot;
    struct list_el *childlast;

    int selected;               /* counter */
    xmb_submenu *pmenu;
};
typedef struct list_el xmb_node;

/* Functions */
int xmb_add(xmb_node ** node, char *name, char *title, char *subtitle);
int xmb_draw(SDL_Surface * screen);

int xmb_up(void);
int xmb_upleft(void);
int xmb_left(void);
int xmb_downleft(void);
int xmb_down(void);
int xmb_downright(void);
int xmb_right(void);
int xmb_upright(void);

int xmb_start(void);
int xmb_select(void);

int xmb_R(void);
int xmb_L(void);
int xmb_A(void);
int xmb_B(void);
int xmb_Y(void);
int xmb_X(void);

int xmb_volup(void);
int xmb_voldown(void);
int xmb_click(void);

int xmb_count(xmb_node * d);

int xmb_init(void);
int xmb_rdel(xmb_node * begin);
int xmb_destroy(void);

void xmb_activateFlag(int flag);
void xmb_deactivateFlag(int flag);
int xmb_getFlagState(int flag);

void firmware_execute(struct submenu *self);

#endif                          /* mediabar.h */
