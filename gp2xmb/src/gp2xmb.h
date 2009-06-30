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

#ifndef __GP2XMB_H__
#define __GP2XMB_H__

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <SDL.h>
#include <SDL_image.h>
#include <string.h>

#ifdef GFXOPENGL
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "gp2x.h"
#include "messagebox.h"
#include "gfx.h"
#include "sfx.h"
#include "background.h"
#include "configuration.h"
#include "progressbar.h"
#include "utils.h"

#ifndef PATH_MAX
#define PATH_MAX 4095
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

struct s_keyobj
{
    int (*func) (void);
    int timedelay;
    int repeatdelay;
    int lastcall;
    short pressed;
};
typedef struct s_keyobj keyobj;

void gp2xmb_init();
void gp2xmb_deinit();
void gp2xmb_sync(void);
void gp2xmb_syncwait(void);

extern keyobj keys[];
extern int relaunchsystem;

#define SCREEN_W 320
#define SCREEN_H 240

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 128

#endif                          /* gp2xmb.h */
