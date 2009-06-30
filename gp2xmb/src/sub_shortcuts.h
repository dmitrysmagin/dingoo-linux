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

#ifndef __SUB_SHORTCUTS_H__
#define __SUB_SHORTCUTS_H__

#include "mediabar.h"

struct shc_el
{
    /* graphics */
    SDL_Surface *icon_orig;
    SDL_Surface *icon_curr;     /* to allow for scaled/animated images */

    /* text */
    char title[32];             /* static title */
    char path[1024];            /* execution path */

    /* list structure */
    struct shc_el *next;

    int pos_x;
    int pos_y;

    int move_x;
    int move_y;

    int move_tx;
    int move_ty;

};
typedef struct shc_el shc_node;

void sub_shc_draw(SDL_Surface * screen);
void sub_shc_handle_input();
int sub_shc_get_offset_x();
int sub_shc_get_offset_y();

void sub_shc_init(xmb_submenu * menu);
void sub_shc_destroy();

void sub_shc_draw_run(SDL_Surface * screen);

#endif                          /* sub_shortcuts.h */
