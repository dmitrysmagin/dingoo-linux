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

#include <SDL_rotozoom.h>

#include "gp2xmb.h"
#include "popup.h"

#define MAX(a,b) (a > b ? a: b)

#define POPUP_ITEMH (20)
#define POPUP_NUMITEMS (SCREEN_H / POPUP_ITEMH)
#define POPUP_H (SCREEN_H)
#define POPUP_W (120)

SDL_Surface *bg_popup = NULL;
extern SFont_Font *font_small;
extern SFont_Font *font_small_semi;

short visible = 0;

int popselected = 0;

struct POPUP_element *poproot;
struct POPUP_element *poplast;

int popup_numel = 0;

int popup_origvalue = 0;
void (*popup_origfptr) (int) = NULL;

int POPUP_init()
{
    t_rgb color;
    int colormax = 0;
	long bgtheme = 0;

	config_lookup_int(&CONFIG, "bgtheme", &bgtheme);

    color = background_themes[bgtheme];

    colormax = MAX(MAX(color.r, color.g), color.b);

    if (colormax < 192)
    {
        int r = (((color.r / (float) colormax) * 0.7) * 255);
        int g = (((color.g / (float) colormax) * 0.7) * 255);
        int b = (((color.b / (float) colormax) * 0.7) * 255);
        if (r > 255)
            r = 255;
        if (g > 255)
            g = 255;
        if (b > 255)
            b = 255;
        color.r = r;
        color.g = g;
        color.b = b;
    }
    else
    {
        int r = (((color.r / (float) colormax) * 0.5) * 255);
        int g = (((color.g / (float) colormax) * 0.5) * 255);
        int b = (((color.b / (float) colormax) * 0.5) * 255);
        if (r < 0)
            r = 0;
        if (g < 0)
            g = 0;
        if (b < 0)
            b = 0;
        color.r = r;
        color.g = g;
        color.b = b;
    }

    bg_popup = gfx_new_surface(POPUP_W, POPUP_H, 1);
    SDL_FillRect(bg_popup, NULL,
                 SDL_MapRGBA(bg_popup->format, color.r, color.g, color.b, 127));

    visible = 1;

    poproot = NULL;
    poplast = NULL;
    popup_numel = 0;

    popup_origvalue = 0;
    popup_origfptr = NULL;

    return 0;
}

void POPUP_destroy()
{
    visible = 0;
    POPUP_empty();
    if (bg_popup)
    {
        SDL_FreeSurface(bg_popup);
        bg_popup = NULL;
    }
}

void POPUP_setrevert(void (*fptr) (int), int value)
{
    popup_origvalue = value;
    popup_origfptr = fptr;
}

void POPUP_setselected(int index)
{
    if (index >= 0 && index < popup_numel)
        popselected = index;
}

struct POPUP_element *POPUP_getelement(int index)
{
	int i;
	struct POPUP_element *pcurr = poproot;

	for(i = 0; pcurr && i < popup_numel; i++,pcurr = pcurr->next)
	{
		if(i == index)
			return pcurr;
	}

	return NULL;
}

void POPUP_revert(void)
{
    if (popup_origfptr)
        popup_origfptr(popup_origvalue);
}

void
POPUP_add(SDL_Surface * icon, const char *title, void (*fptr) (int),
          int value)
{
    if (!poproot)
    {
        poproot =
            (struct POPUP_element *) malloc(sizeof(struct POPUP_element));
        if (!poproot)
            return;             /* error */

        poproot->next = NULL;
        poproot->icon = icon;
        poproot->value = value;
        poproot->fptr = fptr;
        memset(poproot->title, 0, sizeof(poproot->title));
        if (title)
            strncpy(poproot->title, title, sizeof(poproot->title) - 1);
        poplast = poproot;
    }
    else if (poplast)
    {
        poplast->next =
            (struct POPUP_element *) malloc(sizeof(struct POPUP_element));
        if (!poplast->next)
            return;             /* error */

        poplast = poplast->next;
        poplast->next = NULL;
        poplast->icon = icon;
        poplast->value = value;
        poplast->fptr = fptr;
        memset(poplast->title, 0, sizeof(poplast->title));
        if (title)
            strncpy(poplast->title, title, sizeof(poplast->title) - 1);
    }
    else
        return;                 /* error */

    popup_numel++;
}

void POPUP_empty()
{
    struct POPUP_element *pcurr = poproot;
    for (; pcurr != NULL;)
    {
        struct POPUP_element *tmp = pcurr->next;
        if (pcurr->icon)
            SDL_FreeSurface(pcurr->icon);
        free(pcurr);
        pcurr = tmp;
    }
    poproot = NULL;
    poplast = NULL;
    popup_numel = 0;
    popselected = 0;

    popup_origvalue = 0;
    popup_origfptr = NULL;
}

int POPUP_up()
{
    if (popselected > 0)
        popselected--;
    else
        return 0;
    return 1;
}

int POPUP_down()
{
    if (popselected < (popup_numel - 1))
        popselected++;
    else
        return 0;
    return 1;
}

int POPUP_execsel(void)
{
    int i = 0;
    struct POPUP_element *pcurr = poproot;
    for (; pcurr != NULL && i < popselected; i++, pcurr = pcurr->next);

    if (pcurr == NULL)
        return 0;

    if (pcurr->fptr == NULL)
        return 0;
    else
    {
        pcurr->fptr(pcurr->value);
    }

    return 1;
}

void POPUP_draw(SDL_Surface * screen)
{
    struct POPUP_element *pcurr;
    int offy;
    int loop;

    if (bg_popup)
    {
        gfx_draw_image(bg_popup, screen->w - bg_popup->w, 0, screen);
    }

    pcurr = poproot;
    offy = -1;                  /* start one further up */

    for (loop = 0; pcurr != NULL; offy++, pcurr = pcurr->next, loop++)
    {
        int offx = 0;
        int ty = POPUP_ITEMH + offy * POPUP_ITEMH;
        int selected = (popselected == loop);

        if(popup_numel < POPUP_NUMITEMS)
        {
        	ty += ((POPUP_NUMITEMS - popup_numel)/2) * POPUP_ITEMH;
        }

        if (pcurr->icon)
        {
            offx = pcurr->icon->w + 3;
            gfx_draw_image(pcurr->icon,
                           screen->w - bg_popup->w + 5,
                           ty + (POPUP_ITEMH-pcurr->icon->h)/2,
                           screen);
        }

        gfx_draw_text(screen,
                      (selected ? font_small : font_small_semi),
                      screen->w - bg_popup->w + 5 + offx, ty + (POPUP_ITEMH-gfx_text_height(font_small))/2,
                      USESHADOW, (selected ? USEGLOW : NOGLOW),
                      pcurr->title);
    }
}
