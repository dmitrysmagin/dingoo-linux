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

#ifndef __POPUP_H__
#define __POPUP_H__

struct POPUP_element
{
    SDL_Surface *icon;
    char title[64];
    struct POPUP_element *next;
    int value;
    void (*fptr) (int);
};

int POPUP_init();
void POPUP_destroy();
void POPUP_draw(SDL_Surface * screen);
void POPUP_add(SDL_Surface * icon, const char *title, void (*fptr) (int),
               int value);
void POPUP_empty(void);
int POPUP_up();
int POPUP_down();
int POPUP_execsel(void);
void POPUP_setselected(int index);
struct POPUP_element *POPUP_getelement(int index);

void POPUP_setrevert(void (*fptr) (int), int value);
void POPUP_revert(void);

#endif
