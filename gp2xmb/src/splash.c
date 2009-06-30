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

#include "gp2xmb.h"
#include "splash.h"
#include "gfx.h"

SDL_Surface *authorlogo = NULL;
SDL_Surface *systemlogo = NULL;

int splash_init_time = 0;

void splash_init(void)
{
/*	authorlogo = gfx_load_image("images/icons/authorlogo.png",1); */
	systemlogo = gfx_load_image("images/icons/systemlogo.png",1);
	splash_init_time = SDL_GetTicks();
}

void splash_draw(SDL_Surface *screen)
{
/*	if(authorlogo)
		gfx_draw_image(authorlogo,10
								 ,(screen->h-authorlogo->h)-10
								 ,screen);*/
	int alpha = 0;

	if(SDL_GetTicks() - splash_init_time < 1000)
	{
		alpha = 0;
	}
	else if(SDL_GetTicks() - splash_init_time < 2000)
	{
		alpha = (SDL_GetTicks() - splash_init_time - 1000) / 4;
	}
	else if(SDL_GetTicks() - splash_init_time < SPLASH_DELAY-2000)
	{
		alpha = 255;
	}
	else if(SDL_GetTicks() - splash_init_time < SPLASH_DELAY-1000)
	{
		alpha = 255 - (SDL_GetTicks() - splash_init_time - (SPLASH_DELAY-2000)) / 4;
	}

	LIMIT(alpha,0,255);

	if(alpha && systemlogo)
	{
		SDL_Surface *bsurf = NULL;

		if(alpha < 255)
		{
			bsurf = gfx_clone_surface(systemlogo);
			gfx_set_alpha(bsurf,alpha);
		}
		else
		{
			bsurf = systemlogo;
		}

		gfx_draw_image(bsurf,(screen->w-systemlogo->w)-5
								 ,(screen->h-systemlogo->h)/2
								 ,screen);
		if(alpha < 255)
			SDL_FreeSurface(bsurf);
	}
}

