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
#include <string.h>
#include <stdio.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_rotozoom.h>
#include <SDL_mixer.h>
#include <pthread.h>

#include "joystick.h"
#include "gp2x.h"
#include "gfx.h"
#include "background.h"
#include "messagebox.h"
#include "configuration.h"

#define MSGBUTTON_SPACING 32
#define MSG_ICONX 24
#define MSG_ICONY 24


int returnvalue = 0;
pthread_t msgptid;
pthread_mutex_t msgpt_lock;
SDL_Surface *msgb_surft = NULL;
SDL_Surface *msgb_buttonX = NULL;
SDL_Surface *msgb_buttonB = NULL;
SDL_Surface *msgb_icon = NULL;
char *msgb_message = NULL;
char *msgb_title = NULL;
enum msgbtype msgb_type = NONE;
int msgb_done = 0;

extern SFont_Font *font_normal;
extern SFont_Font *font_small;
extern Mix_Chunk *snd_menu_move;
extern Mix_Chunk *snd_menu_ok;
extern Mix_Chunk *snd_menu_back;

void *msgbox_draw(void *argv);


int msgbox(SDL_Surface * screen, SDL_Surface *icon, const char* title, const char *message, enum msgbtype type)
{
	SDL_Surface *msgb_buttons = NULL;
    msgb_surft = screen;
    msgb_type = type;
    msgb_done = 0;

    msgb_buttons = gfx_load_image("images/icons/buttons.png",0);
    if(msgb_buttons)
    {
    	SDL_Rect r;
    	r.x = msgb_buttons->h*1;
    	r.w = msgb_buttons->h;
    	r.y = 0;
    	r.h = msgb_buttons->h;
		msgb_buttonB = gfx_crop_surface(msgb_buttons, r, msgb_buttons->format, msgb_buttons->flags);
		r.x = msgb_buttons->h*2;
		msgb_buttonX = gfx_crop_surface(msgb_buttons, r, msgb_buttons->format, msgb_buttons->flags);
    }
    else
   	{
   		msgb_buttonX = NULL;
   		msgb_buttonB = NULL;
   	}

    if (message)
    {
        msgb_message = (char *) calloc(1, 1024);
        if (!msgb_message)
            return 0;

        strncpy(msgb_message, message, 1023);
    }
    else
        msgb_message = NULL;

    if(title)
    {
    	msgb_title = (char *) calloc(1, 64);
        if (!msgb_title)
            return 0;

        strncpy(msgb_title, title, 63);
    }
    else
        msgb_title = NULL;

	if(icon)
	{
		float sx = (float)MSG_ICONX/icon->w;
		float sy = (float)MSG_ICONY/icon->h;
		msgb_icon = zoomSurface(icon, sx, sy, SMOOTHING_ON);
	}

	if(msgb_buttons)
		SDL_FreeSurface(msgb_buttons);

    pthread_mutex_init(&msgpt_lock, NULL);
    pthread_create(&msgptid, NULL, msgbox_draw, NULL);
    return 1;
}

void *msgbox_draw(void *argv)
{
    SDL_Surface *bg = NULL;
    SDL_Event event;

    char *sX = NULL;
    char *sB = NULL;

    bg = gfx_new_surface(msgb_surft->w, msgb_surft->h, 1);

    while (!msgb_done)
    {
        bg_draw(msgb_surft);
        SDL_FillRect(bg, NULL, SDL_MapRGBA(bg->format, 0, 0, 0, 64));
        gfx_draw_image(bg, 0, 0, msgb_surft);
        lineRGBA(msgb_surft, 0, 30, msgb_surft->w, 30, 255,255,255,255);
        lineRGBA(msgb_surft, 0, msgb_surft->h-30, msgb_surft->w, msgb_surft->h-30, 255,255,255,255);

		if (font_normal)
        {
			gfx_draw_text(msgb_surft,
                              font_normal,
                              (msgb_surft->w -
                               gfx_text_width(font_normal,
                                              msgb_message)) / 2,
                              (msgb_surft->h -
                               gfx_text_height(font_normal)) / 2,
                              USESHADOW, NOGLOW, msgb_message);
        }

        switch (msgb_type)
        {
        case NONE:
            break;

        case HELP:
            break;

        case DONE:
        	sB = "Done";
        	break;

        case OK:
        	sX = "OK";
            break;

        case CLOSE:
        	sB = "Close";
            break;

        case CANCEL:
           	sX = "OK";
        	sB = "Cancel";
            break;

        case DISCONNECT:
	        sB = "Disconnect";
        	break;
        }

		if(msgb_icon)
		{
			gfx_draw_image(msgb_icon,5,(30-msgb_icon->h)/2,msgb_surft);
		}

        if(msgb_title && font_normal)
        {
			gfx_draw_text(msgb_surft, font_normal,
						  MSG_ICONX+10, (30-gfx_text_height(font_normal))/2, USESHADOW, NOGLOW, msgb_title);
        }

		if(sX && msgb_buttonX && font_small)
		{
			int pX = (msgb_surft->w/2)-((MSGBUTTON_SPACING/2)+ msgb_buttonX->w + 5 + gfx_text_width(font_small,sX));
			int pY = ((30-msgb_buttonX->h)/2) + msgb_surft->h - 30;
			gfx_draw_image(config_lookup_bool(&CONFIG, "swapXB") ?
						   msgb_buttonB : msgb_buttonX,pX, pY, msgb_surft);

			pX += msgb_buttonX->w + 5;
			pY = ((30-gfx_text_height(font_small))/2) + msgb_surft->h - 30;
			gfx_draw_text(msgb_surft, font_small, pX, pY, USESHADOW, NOGLOW, sX);
		}

		if(sB && msgb_buttonB && font_small)
		{
			int pX = (msgb_surft->w/2)+(MSGBUTTON_SPACING/2);
			int pY = ((30-msgb_buttonB->h)/2) + msgb_surft->h - 30;
			gfx_draw_image(config_lookup_bool(&CONFIG, "swapXB") ?
						   msgb_buttonX : msgb_buttonB, pX, pY,
						   msgb_surft);

			pX += msgb_buttonB->w + 5;
			pY = ((30-gfx_text_height(font_small))/2) + msgb_surft->h - 30;
			gfx_draw_text(msgb_surft, font_small, pX, pY, USESHADOW, NOGLOW, sB);
		}

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    break;
                case SDLK_RIGHT:
                    break;

				case SDLK_BACKSPACE:
				case SDLK_ESCAPE:
					if(sB || (config_lookup_bool(&CONFIG, "swapXB") && sX))
					{
                    	msgb_done = 1;
                    }
					break;

                case SDLK_RETURN:
                    if(sX || (config_lookup_bool(&CONFIG, "swapXB") && sB))
					{
                    	msgb_done = 1;
                    }
					break;

                default:
                    break;
                }
                break;
#ifdef TARGET_GP2X
            case SDL_JOYBUTTONDOWN:
                switch (event.jbutton.button)
                {
                case GP2X_BUTTON_LEFT:

                    break;
                case GP2X_BUTTON_RIGHT:
                    break;

				case GP2X_BUTTON_B:
					if(sB || (config_lookup_bool(&CONFIG, "swapXB") && sX))
					{
                    	msgb_done = 1;
                    }
					break;

                case GP2X_BUTTON_X:
					if(sX || (config_lookup_bool(&CONFIG, "swapXB") && sB))
					{
                    	msgb_done = 1;
                    }
					break;
                default:
                    break;
                }
                break;
#endif
            }
        }


        SDL_Flip(msgb_surft);
        SDL_Delay(1);           /* let operating system breath */
    }

    SDL_FreeSurface(bg);
    if(msgb_buttonB)
    	SDL_FreeSurface(msgb_buttonB);
    if(msgb_buttonX)
    	SDL_FreeSurface(msgb_buttonX);
    if(msgb_icon)
    	SDL_FreeSurface(msgb_icon);


    if (msgb_message)
        free(msgb_message);

    if (msgb_title)
        free(msgb_title);

    return ((void *) 0);
}

int msgbox_retval(void)
{
    if (msgb_type == NONE)
    {
        msgb_done = 1;
    }

    pthread_join(msgptid, NULL);

    return returnvalue;
}
