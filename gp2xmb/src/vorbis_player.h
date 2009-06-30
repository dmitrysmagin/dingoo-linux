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

#ifndef __VORBIS_PLAYER_H__
#define __VORBIS_PLAYER_H__

int op_init(char *path);
int op_eof(void);
int op_play(void);
int op_stop(void);

void op_thread_close(void *arg);
void *op_thread_play(void *arg);
unsigned int op_getpos(void);
unsigned int op_getlength(void);
int op_ready(void);
int op_fwd(void);
int op_bwd(void);
int op_eof(void);

char *op_metainfo(char *path, int type);

#define OVMTITLE 1
#define OVMARTIST 2

#endif                          /* vorbis_player.h */
