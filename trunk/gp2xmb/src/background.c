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
#include <time.h>
#include <math.h>
#include <SDL_rotozoom.h>

#include "gp2xmb.h"

#define SINMAX 3600
float sinlut_a[SINMAX + 1];

struct gradient_anim
{
    float posx;
    float posy;
    SDL_Surface *img;
};

pthread_mutex_t m_lock;
char wallpaper_path[1024];
struct gradient_anim ganim[2];

#define WALLPAPER_CACHE "images/.wallpaper.png"

SDL_Surface *background = NULL;
SDL_Surface *gradient[2] = { NULL, NULL };
SDL_Surface *bggradient = NULL;
SDL_Surface *optbg = NULL;
t_rgb bgcolor = { 10, 10, 10 };
t_rgb background_themes[10] = { {255, 128, 10},
{25, 25, 112},
{255, 105, 180},
{115, 60, 15},
{0, 160, 165},
{115, 175, 35},
{125, 100, 240},
{140, 140, 80},
{225, 0, 0},
{10, 10, 10}
};

const char *background_names[10] = { "Orange", "Midnightblue",
    "Hotpink", "Saddlebrown",
    "Turquoise", "Lawngreen",
    "Slateblue", "Darkkhaki",
    "Red", "Black"
};

int bg_usewallpaper = 0;
int offset_time = 0;

void sin_build_lut()
{
    int i;
    float angle, angleinc;

    angleinc = 3.1415926535 / (SINMAX / 2);

    for (i = 0, angle = 0.0; i <= SINMAX; ++i, angle += angleinc)
    {
        sinlut_a[i] = sin(angle);
    }
}

float sinlut(float radians)
{
    int ix;
    ix = (int) (radians * 572.957795);  /* rad2deg*10 */
    ix %= SINMAX;
    return sinlut_a[ix];
}


void bg_fix(void)
{
	SDL_Surface *stmp;
	stmp = zoomSurface(optbg,(float)SCREEN_W/optbg->w,(float)SCREEN_H/optbg->h,1);

	if(stmp)
	{
		SDL_Surface *s = SDL_DisplayFormat(stmp);
		SDL_FreeSurface(optbg);
		SDL_FreeSurface(stmp);
		optbg = s;
	}
}

int bg_init(void)
{
#ifdef GFXOPENGL

#else
    int bgtheme;

	srandom(time(NULL));

    sin_build_lut();

    pthread_mutex_init(&m_lock, NULL);

    bgtheme = config_lookup_int(&CONFIG, "bgtheme");

    if (bgtheme >= 0
        && bgtheme < (sizeof(background_themes) / sizeof(t_rgb)))
    {
        bgcolor.r = background_themes[bgtheme].r;
        bgcolor.g = background_themes[bgtheme].g;
        bgcolor.b = background_themes[bgtheme].b;
    }

    gradient[0] = gfx_load_image("images/bgeffect/gradient1.png", 0);
    if (!gradient[0])
    {
        fprintf(stderr, "Unable to load gradient1.png.\n");
        return -1;
    }

    gradient[1] = gfx_load_image("images/bgeffect/gradient4.png", 0);
    if (!gradient[1])
    {
        fprintf(stderr, "Unable to load gradient4.png.\n");
        return -1;
    }

    bggradient = gfx_load_image("images/bgeffect/bggradient.png", 0);
    if (!gradient[1])
    {
        fprintf(stderr, "Unable to load bggradient.png.\n");
        return -1;
    }

    optbg = gfx_new_surface(bggradient->w, bggradient->h, 0);
    SDL_FillRect(optbg, NULL,
                 SDL_MapRGB(optbg->format, bgcolor.r, bgcolor.g,
                            bgcolor.b));
    gfx_draw_image(bggradient, 0, 0, optbg);

	bg_fix();

    ganim[0].posx = 0;
    ganim[0].posy = 64;
    ganim[0].img = gradient[0];

    ganim[1].posx = 0;
    ganim[1].posy = 240 - gradient[1]->h - 54;
    ganim[1].img = gradient[1];

    bg_usewallpaper = config_lookup_bool(&CONFIG, "usewallpaper");;

	offset_time = random() % 20000;

#endif

    return 0;
}

void bg_load_wallpaper()
{
    const char *wallpaper = config_lookup_string(&CONFIG, "wallpaper");

    if (background)
    {
        SDL_FreeSurface(background);
        background = NULL;
    }

    if (wallpaper)
    {
        pthread_mutex_lock(&m_lock);
        bg_set((char *) wallpaper);
        pthread_mutex_unlock(&m_lock);
    }
}

void bg_destroy(void)
{
    int i = 0;

    if (background)
    {
        SDL_FreeSurface(background);
    }

    for (; i < 2; i++)
    {
        if (gradient[i])
            SDL_FreeSurface(gradient[i]);

        gradient[i] = NULL;
    }

    if (bggradient)
        SDL_FreeSurface(bggradient);

    background = NULL;
}

void bg_showbg(int boolean)
{
    bg_usewallpaper = boolean;
}

void bg_colorset(int index)
{
    if (index >= 0 && index < (sizeof(background_themes) / sizeof(t_rgb)))
    {
        bgcolor.r = background_themes[index].r;
        bgcolor.g = background_themes[index].g;
        bgcolor.b = background_themes[index].b;
    }

    if (optbg)
        SDL_FreeSurface(optbg);

    optbg = gfx_new_surface(bggradient->w, bggradient->h, 0);
    SDL_FillRect(optbg, NULL,
                 SDL_MapRGB(optbg->format, bgcolor.r, bgcolor.g,
                            bgcolor.b));
    gfx_draw_image(bggradient, 0, 0, optbg);
	bg_fix();
}

void bg_set(char *path)
{
    SDL_Surface *screen = SDL_GetVideoSurface();
    SDL_Surface *btmp = NULL;
    config_setting_t *cfgd;

    if (!path)
        return;

    if (background)
    {
    	pthread_mutex_lock(&m_lock);
        SDL_FreeSurface(background);
        background = NULL;
        pthread_mutex_unlock(&m_lock);
    }

    btmp = IMG_Load(path);
    if (!btmp)
    {
        fprintf(stderr, "Unable to load background.\n");
        return;
    }

    if (screen->h != btmp->h || screen->w != btmp->w)
    {
        float sx, sy;
        sy = screen->h / (float) btmp->h;
        sx = screen->w / (float) btmp->w;
        screen = zoomSurface(btmp, sx, sy, 1);
        if (screen)
        {
            SDL_FreeSurface(background);
            gfx_write_png(WALLPAPER_CACHE,screen);
            SDL_FreeSurface(screen);
            btmp = gfx_load_image(WALLPAPER_CACHE,0);
        }
    }

	background = btmp;

    cfgd = config_lookup(&CONFIG, "wallpaper");
    if (cfgd)
    {
        config_setting_set_string(cfgd, WALLPAPER_CACHE);
        cfg_save();
    }
}

void bg_draw(SDL_Surface * screen)
{
#ifdef GFXOPENGL
	glBegin(GL_QUADS);
		glColor3ub(70,100,50);
		glVertex2f(0,screen->h);
		glVertex2f(screen->w,screen->h);
		glColor3ub(0,0,0);
		glVertex2f(screen->w,0);
		glVertex2f(0,0);
	glEnd();
#else
    int i;
    SDL_Rect dst_rect, src_rect;

    if (background && bg_usewallpaper)
    {
        pthread_mutex_lock(&m_lock);
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        gfx_draw_image(background, 0, 0, screen);
        pthread_mutex_unlock(&m_lock);
    }
    else
    {
        gfx_draw_image(optbg, 0, 0, screen);
        for (i = 0; i < 2; i++)
        {
            float speed = 0;
            switch (i)
            {
            case 0:
                speed = 1.29;
                break;
            case 1:
                speed = 2.63;
                break;
            }

            ganim[i].posx =
                (sinlut(((SDL_GetTicks()+offset_time) / (float) (4096 * (speed)))) *
                 ((ganim[i].img->w - 320) * 0.5)) - ganim[i].img->w * 0.25;

            if (ganim[i].img)
            {
                dst_rect.x = 0;
                dst_rect.y = ganim[i].posy;
                src_rect.x = -ganim[i].posx;
                src_rect.w = screen->w;
                src_rect.y = 0;
                src_rect.h = ganim[i].img->h;

                SDL_BlitSurface(ganim[i].img, &src_rect, screen,
                                &dst_rect);
            }
        }
    }
#endif
}
