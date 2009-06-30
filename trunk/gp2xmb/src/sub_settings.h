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

#ifndef __SUBSETTINGS_H__
#define __SUBSETTINGS_H__

#define SETNONE 0
#define SETVIDEO 1
#define SETSYSTEM 2
#define SETMUSIC 3
#define SETSOUND 4
#define SETTHEME 5
#define SETPHOTO 6
#define SETUSB 7

#include "settings_cb.h"

void sub_settings_init(xmb_submenu * menu);
void sub_settings_destroy(void);
void sub_settings_draw(SDL_Surface * screen);
void sub_settings_handle_input(unsigned int button);
int sub_settings_get_offset_x(void);
int sub_settings_get_offset_y(void);

#endif                          /* sub_settings.h */
