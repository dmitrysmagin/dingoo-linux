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

#include <stdlib.h>
#include <SDL.h>

#include "gfx.h"

#define PROGRESS_ANIMDELAY (50)

int progress_value = 0;
SDL_Surface *bar_surf = NULL;
SDL_Surface *loading_surf = NULL;
int progress_steps = 0;

void progress_init()
{
    bar_surf = gfx_load_image("images/icons/progressbar.png", 1);
    loading_surf = gfx_load_image("images/icons/loading_small.png", 1);
    if(loading_surf)
	    progress_steps = loading_surf->w / loading_surf->h;
}

void progress_destroy()
{

}

void progress_draw(SDL_Surface * screen, int type)
{
    if (bar_surf && screen && type == 0)
    {
        SDL_Rect src;
        SDL_Rect dest;

        src.x = 0;
        src.y = 0;
        src.w = (bar_surf->w * (progress_value * 0.01));
        src.h = bar_surf->h;

        dest.x = (screen->w - bar_surf->w) * 0.5;
        dest.y = (screen->h - bar_surf->h) * 0.5;
        dest.w = 0;
        dest.h = 0;

        SDL_BlitSurface(bar_surf, &src, screen, &dest);
    }

    if (loading_surf && screen && type == 1)
    {
        SDL_Rect src;
        SDL_Rect dest;

        src.x = ((SDL_GetTicks() % (progress_steps*PROGRESS_ANIMDELAY)) / PROGRESS_ANIMDELAY) * loading_surf->h;
        src.y = 0;
        src.w = loading_surf->h;
        src.h = loading_surf->h;

        dest.x = screen->w - loading_surf->h;
        dest.y = screen->h - loading_surf->h;
        dest.w = 0;
        dest.h = 0;

        SDL_BlitSurface(loading_surf, &src, screen, &dest);
    }
}

void progress_set(int value)
{
    progress_value = value;

    progress_value -= (progress_value % 5);     /* increments on 5 */

    if (progress_value < 0)
        progress_value = 0;
    if (progress_value > 100)
        progress_value = 100;
}

int progress_get()
{
    return progress_value;
}
