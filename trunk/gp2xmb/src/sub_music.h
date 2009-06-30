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

#ifndef __SUB_MUSIC_H__
#define __SUB_MUSIC_H__

#include "mediabar.h"

void sub_music_init(xmb_submenu * menu);
void sub_music_destroy();
void sub_music_draw(SDL_Surface * screen);
void sub_music_handle_input(unsigned int button);
int sub_music_get_offset_x();
int sub_music_get_offset_y();
void *sub_music_player_thread(void *arg);
void sub_music_playsel(void);

#define MUS_FULLSCREEN 10

#endif                          /* sub_music.h */
