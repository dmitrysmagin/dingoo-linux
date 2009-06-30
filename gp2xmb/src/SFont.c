/*  SFont: a simple font-library that uses special .pngs as fonts
    Copyright (C) 2003 Karl Bartel

	Modified by: Mikael Ganehag Brorsson

    License: GPL or LGPL (at your choice)
    WWW: http://www.linux-games.com/sfont/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Karl Bartel
    Cecilienstr. 14
    12307 Berlin
    GERMANY
    karlb@gmx.net
*/

#include <SDL.h>
#include <SDL_gfxBlitFunc.h>

#include <assert.h>
#include <stdlib.h>
#include "SFont.h"
#include "gfx.h"

static Uint32 GetPixel(SDL_Surface * Surface, Sint32 X, Sint32 Y)
{
    Uint8 *bits;
    Uint32 Bpp;

    assert(X >= 0);
    assert(X < Surface->w);

    Bpp = Surface->format->BytesPerPixel;
    bits = ((Uint8 *) Surface->pixels) + Y * Surface->pitch + X * Bpp;

    /* Get the pixel */
    switch (Bpp)
    {
    case 1:
        return *((Uint8 *) Surface->pixels + Y * Surface->pitch + X);
        break;
    case 2:
        return *((Uint16 *) Surface->pixels + Y * Surface->pitch / 2 + X);
        break;
    case 3:
        {                       /* Format/endian independent */
            Uint8 r, g, b;
            r = *((bits) + Surface->format->Rshift / 8);
            g = *((bits) + Surface->format->Gshift / 8);
            b = *((bits) + Surface->format->Bshift / 8);
            return SDL_MapRGB(Surface->format, r, g, b);
        }
        break;
    case 4:
        return *((Uint32 *) Surface->pixels + Y * Surface->pitch / 4 + X);
        break;
    }

    return -1;
}

SFont_Font *SFont_InitFont(SDL_Surface * Surface, SFont_Font * fsuite)
{
    int x = 0, i = 0, y = 0;
    Uint32 pixel;
    SFont_Font *Font;
    Uint32 pink;

    if (Surface == NULL)
        return NULL;

    if (fsuite == NULL)
    {
        Font = (SFont_Font *) malloc(sizeof(SFont_Font));
        Font->Surface = Surface;
        Font->Surface2 = NULL;
    }
    else
    {
        Font = fsuite;
        Font->Surface2 = Surface;
        i = 190;                /* can I trust this? */
    }
    Font->MaxPos = 0;

    SDL_LockSurface(Surface);

    pink = SDL_MapRGB(Surface->format, 255, 0, 255);

    if (!Font->Surface2)
    {
        Font->fheight = 0;
        Font->fbaseline = 0;
        Font->ftop = 0;
        while (y < Surface->h)
        {
            if (GetPixel(Surface, 0, y) == pink && !Font->fheight)
            {
                Font->ftop = y;
                Font->fheight = y;
            }
            else if (GetPixel(Surface, 0, y) == pink && Font->fheight
                     && !Font->fbaseline)
            {
                Font->fbaseline = y;
            }
            else if (GetPixel(Surface, 0, y) == pink && Font->fbaseline)
            {
                Font->fheight = y - Font->fheight;
                break;
            }
            y++;
        }
    }

    while (x < Surface->w)
    {
        if (GetPixel(Surface, x, 0) == pink)
        {
            Font->CharPos[i++] = x;
            while ((x < Surface->w) && (GetPixel(Surface, x, 0) == pink))
                x++;

            Font->CharPos[i++] = x;
        }
        x++;
    }
    Font->MaxPos += x - 1;

    pixel = GetPixel(Surface, 0, Surface->h - 1);
    SDL_UnlockSurface(Surface);
    SDL_SetColorKey(Surface, SDL_SRCCOLORKEY, pixel);

    return Font;
}

void SFont_FreeFont(SFont_Font * FontInfo)
{
    SDL_FreeSurface(FontInfo->Surface);
    if (FontInfo->Surface2)
        SDL_FreeSurface(FontInfo->Surface2);
    free(FontInfo);
}

void SFont_Write(SDL_Surface * Surface, const SFont_Font * Font,
                 int x, int y, const int useshadow, const int useglow,
                 const char *text)
{
    const char *c;
    int charoffset, surfpart;
    SDL_Rect srcrect, dstrect, shadowrect, glowrect;
    int glowalpha = 255;
    int glowpadding = 3;

    if (text == NULL)
        return;
    /* these values won't change in the loop */
    surfpart = (Font->Surface->h / 3);
    srcrect.y = (Font->ftop ? Font->ftop : 1);
    shadowrect.y = srcrect.y + surfpart;
    glowrect.y = srcrect.y + surfpart + surfpart - glowpadding;
    dstrect.y = y;
    srcrect.h = dstrect.h = shadowrect.h = glowrect.h =
        Font->fbaseline ? Font->fheight : Font->Surface->h - 1;

    glowrect.h += glowpadding;

    glowalpha = (SDL_GetTicks() % 1536) / 4;

    if (glowalpha > 192)
        glowalpha = 192 - (glowalpha - 192);

    if (glowalpha < 0)
        glowalpha = 0;

    glowalpha += 63;

    for (c = text; *c != '\0' && x <= Surface->w; c++)
    {
        charoffset =
            ((int) ((unsigned char) *c - 33)) * 2 + ((unsigned char) *c >
                                                     127 ? -1 : 1);

        /* skip spaces and nonprintable characters */
        if (*c == ' ' || charoffset < 0 || charoffset > Font->MaxPos)
        {
            x += Font->CharPos[2] - Font->CharPos[1];
            continue;
        }

        shadowrect.w = glowrect.w = srcrect.w = dstrect.w =
            (Font->CharPos[charoffset + 2] +
             Font->CharPos[charoffset + 1]) / 2 -
            (Font->CharPos[charoffset] +
             Font->CharPos[charoffset - 1]) / 2;
        shadowrect.x = glowrect.x = srcrect.x =
            (Font->CharPos[charoffset] +
             Font->CharPos[charoffset - 1]) / 2;
        dstrect.x =
            x - (float) (Font->CharPos[charoffset] -
                         Font->CharPos[charoffset - 1]) / 2;

        if ((unsigned char) *c > 127 && Font->Surface2)
            SDL_BlitSurface(Font->Surface2, &srcrect, Surface, &dstrect);
        else
        {
            if (useshadow)
                SDL_BlitSurface(Font->Surface, &shadowrect, Surface,
                                &dstrect);

            SDL_BlitSurface(Font->Surface, &srcrect, Surface, &dstrect);

            if (useglow)
            {
                SDL_Surface *tmp =
                    gfx_crop_surface(Font->Surface, glowrect,
                                     Font->Surface->format, SDL_SRCALPHA);
                dstrect.y += -glowpadding;
                gfx_set_alpha(tmp, glowalpha);
                SDL_BlitSurface(tmp, NULL, Surface, &dstrect);
                SDL_FreeSurface(tmp);
                dstrect.y += glowpadding;
            }
        }


        x += Font->CharPos[charoffset + 1] - Font->CharPos[charoffset];

        dstrect.y = y;          /* needed this to write outside y = 0 */
    }

}

int SFont_TextWidth(const SFont_Font * Font, const char *text)
{
    const char *c;
    int charoffset = 0;
    int width = 0;

    int loop = 0;

    if (text == NULL)
        return 0;

    for (c = text; *c != '\0'; c++)
    {
        loop++;

        charoffset = ((int) *c - 33) * 2 + 1;
        /* skip spaces and nonprintable characters */
        if (*c == ' ' || charoffset < 0 || charoffset > Font->MaxPos)
        {
            width += Font->CharPos[2] - Font->CharPos[1];
            continue;
        }
        width += Font->CharPos[charoffset + 1] - Font->CharPos[charoffset];
    }

    return width;
}

int SFont_TextHeight(const SFont_Font * Font)
{
    if (Font->fbaseline)
    {
        return Font->fbaseline - Font->ftop;
    }
    else
        return Font->Surface->h - 1;
}

void SFont_WriteCenter(SDL_Surface * Surface, const SFont_Font * Font,
                       int y, const char *text)
{
    /*
       SFont_Write(Surface, Font,
       Surface->w / 2 - SFont_TextWidth(Font, text) / 2, y, text);
     */
}
