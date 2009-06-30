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

#ifndef __GFX_H__
#define __GFX_H__

#include "SFont.h"

#define USESHADOW 1
#define NOSHADOW 0
#define USEGLOW 1
#define NOGLOW 0

struct sRGB
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};
typedef struct sRGB t_rgb;

SDL_Surface *gfx_new_surface(int width, int height, int alpha);
SDL_Surface *gfx_load_image(char *filename, int cache);

void gfx_set_alpha(SDL_Surface * surface, unsigned char value);
void gfx_unload_all();

void gfx_draw_image(SDL_Surface * src_surface, int x, int y,
                    SDL_Surface * dest_surface);
void gfx_draw_text(SDL_Surface * screen, SFont_Font * font, const int x,
                   const int y, const int useshadow, const int useglow,
                   char *string);
SDL_Surface *gfx_clone_surface(SDL_Surface * surf);
SDL_Surface *gfx_scale_surface(SDL_Surface * surf, int w, int h);
SDL_Surface *gfx_scaleperc_surface(SDL_Surface * surf, float w, float h);
int gfx_text_height(SFont_Font * font);
int gfx_text_width(SFont_Font * font, const char *s);
Uint32 gfx_get_pixel(SDL_Surface * surface, int x, int y);
void gfx_set_pixel(SDL_Surface * surface, Uint32 color, int x, int y);
void gfx_draw_outro(SDL_Surface * screen);
void gfx_draw_about(SDL_Surface *screen);
SDL_Surface *gfx_crop_surface(SDL_Surface * surface, SDL_Rect rect,
                              SDL_PixelFormat * format, Uint32 flags);
int gfx_write_png(char *filename, SDL_Surface *surf);

#ifdef GFXOPENGL
int gfx_initgl(void);
int gfx_resizewindow(int width, int height);
#endif

#endif
