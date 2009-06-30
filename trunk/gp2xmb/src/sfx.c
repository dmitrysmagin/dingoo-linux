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

#include <SDL.h>
#include <SDL_mixer.h>
#include "sfx.h"
#include "progressbar.h"
#include "configuration.h"

Mix_Chunk *sfx_move = NULL;
Mix_Chunk *sfx_ok = NULL;
Mix_Chunk *sfx_back = NULL;
Mix_Chunk *sfx_run = NULL;
Mix_Chunk *sfx_intro = NULL;

void sfx_init(void)
{
	int soundfx = 0;

    config_lookup_bool(&CONFIG, "soundfx", &soundfx);

	sfx_intro = Mix_LoadWAV("sound/xmb_load.wav");
    if (!sfx_intro)
    {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
    }
    else if(soundfx)
    	Mix_PlayChannel(-1, sfx_intro, 0);

    /* Load sound effects */
    sfx_move = Mix_LoadWAV("sound/xmb_move.wav");
    if (!sfx_move)
    {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
    }

    progress_set(35);

    sfx_ok = Mix_LoadWAV("sound/xmb_ok.wav");
    if (!sfx_ok)
    {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
    }
    progress_set(40);

    sfx_back = Mix_LoadWAV("sound/xmb_back.wav");
    if (!sfx_back)
    {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
    }

    sfx_run = Mix_LoadWAV("sound/xmb_run.wav");
    if (!sfx_run)
    {
        printf("Mix_LoadWAV: %s\n", Mix_GetError());
    }
}

void sfx_play(enum sfxfile file)
{
	int soundfx = 0;

    config_lookup_bool(&CONFIG, "soundfx", &soundfx);

    if (!soundfx)
        return;

    switch (file)
    {
    case SFXMOVE:
        Mix_PlayChannel(-1, sfx_move, 0);
        break;

    case SFXOK:
        Mix_PlayChannel(-1, sfx_ok, 0);
        break;

    case SFXBACK:
        Mix_PlayChannel(-1, sfx_back, 0);
        break;

    case SFXRUN:
        Mix_PlayChannel(-1, sfx_run, 0);
        break;

    default:
        break;
    }
}

void sfx_destroy(void)
{
    if (sfx_move)
        Mix_FreeChunk(sfx_move);
    if (sfx_ok)
        Mix_FreeChunk(sfx_ok);
    if (sfx_back)
        Mix_FreeChunk(sfx_back);
    if (sfx_run)
        Mix_FreeChunk(sfx_run);
    if (sfx_intro)
        Mix_FreeChunk(sfx_intro);
}
