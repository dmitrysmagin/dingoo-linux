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

#ifndef __ITEMLIST_H__
#define __ITEMLIST_H__

#include <SDL.h>

#define LIST_ITEMHEIGHT 26
#define LIST_ITEMWIDTH 32
#define LIST_SELITEMHEIGHT 72
#define LIST_FPAD (LIST_ITEMWIDTH)+5
#define LISTYHACK 32
int LIST_OFFSETX;
extern int li_create_new;

#define ITF_NONE 0
#define ITF_SELECT 1
#define ITF_SELECTED 2
#define ITF_DESELECT 4

struct listitem
{
    SDL_Surface *icon;
    SDL_Surface *icon_semi;
    SDL_Surface *filetypeicon;
    char title[64];
    char subtitle[64];
    void *extra;
    int flags;

    struct listitem *next;
    struct listitem *prev;
    struct listitem *root;
    struct listitem *parent;
};
typedef struct listitem tlistitem;

void LIST_init(void);
void LIST_add(SDL_Surface * icon,
              SDL_Surface * filetypeicon,
              const char *title, const char *subtitle, void *extra);
void LIST_seticon(SDL_Surface *icon, tlistitem *target);

void LIST_setEmptyMsg(char *msg);
void LIST_destroy(void);
int LIST_draw(SDL_Surface * screen, int draw_offset_x, int draw_offset_y);
void LIST_in(void);
int LIST_out(void);
int LIST_up(void);
int LIST_down(void);
tlistitem *LIST_getSelected(void);
tlistitem *LIST_getelement(int index);
void LIST_empty(tlistitem ** troot);
int LIST_getSelNum(void);
int LIST_columnWidth(void);

#endif                          /* itemlist.h */
