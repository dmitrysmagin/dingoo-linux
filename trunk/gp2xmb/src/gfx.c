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
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <math.h>
#include <png.h>

#ifdef GFXOPENGL
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "gfx.h"
#include "joystick.h"
#include "background.h"
#include "sfx.h"
#include "img_manager.h"

#define BASELINE_OFFSET 3

SDL_Surface *gfx_new_surface(int width, int height, int alpha)
{
    Uint32 rmask, gmask, bmask;
    SDL_Surface *tmp = NULL;
    SDL_Surface *dispform = NULL;

    if (width <= 0 || height <= 0)
        return NULL;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
#endif

    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE | (alpha ? SDL_SRCALPHA : 0),
                               width, height, 16, rmask, gmask, bmask, 0);

    if (!tmp)
        return NULL;

    if (alpha)
    {
        SDL_SetAlpha(tmp, SDL_SRCALPHA, 0);
        dispform = SDL_DisplayFormatAlpha(tmp);
    }
    else
        dispform = SDL_DisplayFormat(tmp);

    SDL_FreeSurface(tmp);

    return dispform;
}

SDL_Surface *gfx_load_image(char *filename, int cache)
{
    SDL_Surface *img_tmp = NULL;
    SDL_Surface *img = NULL;

    if (cache)
        img_tmp = img_cache_get(filename);

    if (img_tmp == NULL)
    {
        img_tmp = IMG_Load(filename);

        if (!img_tmp)
        {
            return NULL;
        }

        img = SDL_DisplayFormatAlpha(img_tmp);

        if (img)
        {
            SDL_FreeSurface(img_tmp);
            img_tmp = img;
        }

        if (cache)
            img_cache_add(filename, img_tmp);
    }

    return img_tmp;
}

void gfx_unload_all()
{
    img_clear_cache();
}

void gfx_draw_image(SDL_Surface * src_surface, int x, int y,
                    SDL_Surface * dest_surface)
{
#ifdef GFXOPENGL
	glColor3ub(255,255,255);
	glBegin(GL_QUADS);
		glVertex2f(x,y+src_surface->h);
		glVertex2f(x+src_surface->w,y+src_surface->h);
		glVertex2f(x+src_surface->w,y);
		glVertex2f(x,y);
	glEnd();
#else
    SDL_Rect dest_rect;

	if(!src_surface || !dest_surface)
		return;

    if (y > dest_surface->h ||
        y + src_surface->h < 0 ||
        x > dest_surface->w || x + src_surface->w < 0)
        return;

    dest_rect.x = x;
    dest_rect.y = y;
    dest_rect.w = 0;
    dest_rect.h = 0;

    SDL_BlitSurface(src_surface, NULL, dest_surface, &dest_rect);
#endif
}

void gfx_draw_text(SDL_Surface * screen, SFont_Font * font, const int x,
                   const int y, const int useshadow, const int useglow,
                   char *string)
{
    if (y > screen->h || y + gfx_text_height(font) < 0)
        return;

    SFont_Write(screen, font, x, y, useshadow, useglow, string);
}

int gfx_text_height(SFont_Font * font)
{
    return SFont_TextHeight(font);
}

int gfx_text_width(SFont_Font * font, const char *s)
{
    return SFont_TextWidth(font, s);
}

void gfx_set_alpha(SDL_Surface * surface, unsigned char value)
{
    unsigned int i = 0;
    short offset;
    int steps;
    Uint8 *p = NULL;

    if (4 != surface->format->BytesPerPixel)
        return;                 /* must have alpha */

    steps = surface->w * surface->h * 4;

    offset = 3;                 /* little endian */
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        offset = 0;

    p = (Uint8 *) surface->pixels;

    for (i = 0; i < steps; i += 4, p += 4)
        p[offset] = p[offset] * ((float) value / 255);
}

void gfx_set_pixel(SDL_Surface * surface, Uint32 color, int x, int y)
{
    int pos;
    Uint32 *p = NULL;

    if (4 != surface->format->BytesPerPixel)
        return;                 /* must have alpha */

    pos = x + (y * surface->w);
    if (pos > surface->w * surface->h)
        return;

    p = (Uint32 *) surface->pixels;
    p[pos] = color;
}

Uint32 gfx_get_pixel(SDL_Surface * surface, int x, int y)
{
    int pos;
    Uint32 *p = NULL;

    if (4 != surface->format->BytesPerPixel)
        return -1;              /* must have alpha */

    pos = x + (y * surface->w);
    if (pos > surface->w * surface->h)
        return -1;

    p = (Uint32 *) surface->pixels;
    return p[pos];
}

SDL_Surface *gfx_clone_surface(SDL_Surface * surf)
{
    return SDL_ConvertSurface(surf, surf->format, surf->flags);
}

SDL_Surface *gfx_scale_surface(SDL_Surface * surf, int w, int h)
{
    SDL_Surface *fsurf = gfx_new_surface(w, h, 1);
    Uint32 color;
    int x, y;
    float tx, ty;

    if (!fsurf)
        return NULL;

    tx = surf->w / (float) w;
    ty = surf->h / (float) h;

    for (x = 0; x < w; x++)
    {
        for (y = 0; y < h; y++)
        {
            color = gfx_get_pixel(surf, rint(x * tx), rint(y * ty));
            gfx_set_pixel(fsurf, color, x, y);
        }
    }

    return fsurf;
}

SDL_Surface *gfx_scaleperc_surface(SDL_Surface * surf, float w, float h)
{
    return zoomSurface(surf, w, h, 0);
}

void gfx_draw_about(SDL_Surface *screen)
{
	SDL_Surface *about = NULL;
	int stime = 0;
	float offset = -screen->h;
	const static float speed = 50.0/1000.0;
	SDL_Event event;
	int done = 0;

	about = gfx_load_image("images/icons/about_text.png",0);

	if(!about)
		return;

	stime = SDL_GetTicks();

	while(offset < about->h && !done)
	{
		int ttime = SDL_GetTicks()-stime;
		stime = SDL_GetTicks();

		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

		gfx_draw_image(about,0,-(int)offset,screen);

		SDL_Flip(screen);

		offset += speed*ttime;

		while (SDL_PollEvent(&event))
        {
			switch (event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                case SDLK_BACKSPACE:
                case SDLK_RETURN:
                    done = 1;
                    break;
                default:
                    break;
                }
                break;
#ifdef TARGET_GP2X
            case SDL_JOYBUTTONDOWN:
                switch (event.jbutton.button)
                {
                case GP2X_BUTTON_B:
                case GP2X_BUTTON_X:
                    done = 1;
					break;
                default:
                    break;
                }
                break;
#endif
            }
        }

	}

	SDL_FreeSurface(about);
}

void gfx_draw_outro(SDL_Surface * screen)
{
    SDL_Surface *logo = NULL;

    int stime = 0, alpha = 0, time_var = 0;
    SDL_Surface *black = gfx_new_surface(320, 240, 1);

    if (!black)
    {
        if (black)
            SDL_FreeSurface(black);

        return;
    }

    logo = gfx_load_image("images/icons/game_run.png", 0);

    stime = SDL_GetTicks();

    time_var = SDL_GetTicks();

    while (stime + 200 > SDL_GetTicks())
    {
        SDL_FillRect(black, NULL,
                     SDL_MapRGBA(black->format, 0, 0, 0, alpha));
        bg_draw(screen);
        gfx_draw_image(black, 0, 0, screen);
        SDL_Flip(screen);

        alpha += (255 / (float) 200) * (SDL_GetTicks() - time_var);
        if (alpha > 255)
            alpha = 255;
        time_var = SDL_GetTicks();
    }

    sfx_play(SFXRUN);

    alpha = 255;
    while (stime + 3000 > SDL_GetTicks())
    {
        SDL_FillRect(black, NULL,
                     SDL_MapRGBA(black->format, 0, 0, 0, alpha));
        gfx_draw_image(black, 0, 0, screen);
        if (logo)
            gfx_draw_image(logo, (screen->w - logo->w) / 2,
                           (screen->h - logo->h) / 2, screen);
        SDL_Flip(screen);
    }
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Flip(screen);

    SDL_FreeSurface(logo);
}

SDL_Surface *gfx_crop_surface(SDL_Surface * surface, SDL_Rect rect,
                              SDL_PixelFormat * format, Uint32 flags)
{
    SDL_Surface *convert;
    Uint32 colorkey = 0;
    Uint8 alpha = 0;
    Uint32 surface_flags;
    SDL_Rect bounds;

    /* Check for empty destination palette! (results in empty image) */
    if (format->palette != NULL)
    {
        int i;
        for (i = 0; i < format->palette->ncolors; ++i)
        {
            if ((format->palette->colors[i].r != 0) ||
                (format->palette->colors[i].g != 0) ||
                (format->palette->colors[i].b != 0))
                break;
        }
        if (i == format->palette->ncolors)
        {
            SDL_SetError("Empty destination palette");
            return (NULL);
        }
    }

    /* Create a new surface with the desired format */
    convert = SDL_CreateRGBSurface(flags,
                                   rect.w, rect.h, format->BitsPerPixel,
                                   format->Rmask, format->Gmask,
                                   format->Bmask, format->Amask);
    if (convert == NULL)
    {
        return (NULL);
    }

    /* Copy the palette if any */
    if (format->palette && convert->format->palette)
    {
        memcpy(convert->format->palette->colors,
               format->palette->colors,
               format->palette->ncolors * sizeof(SDL_Color));
        convert->format->palette->ncolors = format->palette->ncolors;
    }

    /* Save the original surface color key and alpha */
    surface_flags = surface->flags;
    if ((surface_flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY)
    {
        /* Convert colourkeyed surfaces to RGBA if requested */
        if ((flags & SDL_SRCCOLORKEY) != SDL_SRCCOLORKEY && format->Amask)
        {
            surface_flags &= ~SDL_SRCCOLORKEY;
        }
        else
        {
            colorkey = surface->format->colorkey;
            SDL_SetColorKey(surface, 0, 0);
        }
    }
    if ((surface_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
    {
        alpha = surface->format->alpha;
        SDL_SetAlpha(surface, 0, 0);
    }

    /* Copy over the image data */
    bounds.x = 0;
    bounds.y = 0;
    bounds.w = rect.w;
    bounds.h = rect.h;
    SDL_LowerBlit(surface, &rect, convert, &bounds);

    if ((surface_flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY)
    {
        Uint32 cflags = surface_flags & (SDL_SRCCOLORKEY | SDL_RLEACCELOK);
        if (convert != NULL)
        {
            Uint8 keyR, keyG, keyB;

            SDL_GetRGB(colorkey, surface->format, &keyR, &keyG, &keyB);
            SDL_SetColorKey(convert, cflags | (flags & SDL_RLEACCELOK),
                            SDL_MapRGB(convert->format, keyR, keyG, keyB));
        }
        SDL_SetColorKey(surface, cflags, colorkey);
    }
    if ((surface_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
    {
        Uint32 aflags = surface_flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
        if (convert != NULL)
        {
            SDL_SetAlpha(convert, aflags | (flags & SDL_RLEACCELOK),
                         alpha);
        }
        SDL_SetAlpha(surface, aflags, alpha);
    }

    /* We're ready to go! */
    return (convert);
}

static int png_colortype_from_surface(SDL_Surface *surface)
{
	int colortype = PNG_COLOR_MASK_COLOR; /* grayscale not supported */

	if (surface->format->palette)
		colortype |= PNG_COLOR_MASK_PALETTE;
	else if (surface->format->Amask)
		colortype |= PNG_COLOR_MASK_ALPHA;

	return colortype;
}

void png_user_warn(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: warning: %s\n", str);
}


void png_user_error(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: error: %s\n", str);
}

int gfx_write_png(char *filename, SDL_Surface *surf)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	int i, colortype;
	png_bytep *row_pointers;

	/* Opening output file */
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		perror("fopen error");
		return -1;
	}

	/* Initializing png structures and callbacks */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, png_user_error, png_user_warn);
	if (png_ptr == NULL) {
		printf("png_create_write_struct error!\n");
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		printf("png_create_info_struct error!\n");
		exit(-1);
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		exit(-1);
	}

	png_init_io(png_ptr, fp);

	colortype = png_colortype_from_surface(surf);
	png_set_IHDR(png_ptr, info_ptr, surf->w, surf->h, 8, colortype,	PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Writing the image */
	png_write_info(png_ptr, info_ptr);
	png_set_packing(png_ptr);

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*surf->h);
	for (i = 0; i < surf->h; i++)
		row_pointers[i] = (png_bytep)(Uint8 *)surf->pixels + i*surf->pitch;
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);

	/* Cleaning out... */
	free(row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	return 0;
}

#ifdef GFXOPENGL

int gfx_initgl(void)
{
	glShadeModel( GL_SMOOTH );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClearDepth( 1.0f );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

    return 1;
}

int gfx_resizewindow(int width, int height)
{
	float ratio;

    if (height == 0)
        height = 1;

    ratio = (float)width / (float)height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* gluPerspective( 45.0f, ratio, 0.1f, 100.0f ); */

    glOrtho(0.0f,width,height,0.0f,-1.0f,1.0f);

    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );

    return 1;
}

#endif /* GFXOPENGL */

