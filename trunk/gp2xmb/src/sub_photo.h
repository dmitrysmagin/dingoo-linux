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

#ifndef __SUB_PHOTO_H__
#define __SUB_PHOTO_H__

#include "mediabar.h"

void sub_photo_init(xmb_submenu * menu);
void sub_photo_destroy();
void sub_photo_draw(SDL_Surface * screen);
void sub_photo_handle_input(unsigned int button);
int sub_photo_get_offset_x();
int sub_photo_get_offset_y();

#define PS_FULLSCREEN (10)
#define PS_ASPECTRATIO (4/(float)3)


#endif                          /* sub_photo.h */
