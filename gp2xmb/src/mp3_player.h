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

#ifndef __MP3_PLAYER_H__
#define __MP3_PLAYER_H__

int mp3_init(char *path);
int mp3_eof(void);
int mp3_play(void);
int mp3_stop(void);

unsigned int mp3_getpos(void);
unsigned int mp3_getlength(void);
int mp3_ready(void);
int mp3_fwd(void);
int mp3_bwd(void);

char *mp3_metainfo(char *path, int type);

void *mp3_thread_play(void *arg);
void mp3_thread_close(void *arg);

#endif                          /* mp3_player.h */
