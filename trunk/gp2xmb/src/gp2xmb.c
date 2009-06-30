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

#include <signal.h>
#include <SDL_mixer.h>
#include <SDL_joystick.h>

#include "gp2xmb.h"
#include "mediabar.h"
#include "batterymon.h"
#include "volumecontrol.h"
#include "splash.h"
#include "scripts.h"

int killonce = 0;
SDL_Surface *screen = NULL;
char initpath[1024];
pthread_t loaderid;
pthread_t syncid = 0;
int relaunchsystem = 1;
int coldboot = 1;

keyobj keys[] = { {xmb_up, 250, 75, 0, 0},
{xmb_upleft, 250, 75, 0, 0},
{xmb_left, 250, 75, 0, 0},
{xmb_downleft, 250, 75, 0, 0},
{xmb_down, 250, 75, 0, 0},
{xmb_downright, 250, 75, 0, 0},
{xmb_right, 250, 75, 0, 0},
{xmb_upright, 250, 75, 0, 0},
{xmb_start, 0, 125, 0, 0},
{xmb_select, 0, 125, 0, 0},
{xmb_R, 0, 125, 0, 0},
{xmb_L, 0, 125, 0, 0},
{xmb_A, 0, 125, 0, 0},
{xmb_B, 0, 125, 0, 0},
{xmb_Y, 0, 125, 0, 0},
{xmb_X, 0, 125, 0, 0},
{xmb_volup, 250, 25, 0, 0},
{xmb_voldown, 250, 25, 0, 0},
{xmb_click, 0, 125, 0, 0}
};

extern SFont_Font *font_small;
extern SFont_Font *font_normal;

void quit(int code)
{
    if (killonce == 1)
        exit(0);
    killonce++;

    printf("Signal Caught: terminating...\n");

    gp2xmb_syncwait();
    cfg_save();
    gp2xmb_syncwait();

    xmb_destroy();

    Mix_CloseAudio();
    SDL_Quit();

#ifdef TARGET_GP2X
    fflush(stdout);
    fflush(stderr);
    sync();
    gp2x_setgamma(10);

    chdir("/usr/gp2x");
    if (relaunchsystem)
        execl("/usr/gp2x/gp2xmenu", "gp2xmenu", (char *) 0);
    else
        execl("/usr/gp2x/gp2xmenu", "gp2xmenu", "--disable-autorun",
              (char *) 0);
#endif

    exit(0);
}

void quit_handle(void)
{
    quit(1);
}


int main(int argc, char *argv[])
{
    SDL_Event event;
    int mainloop = 1;
    unsigned int i = 0;
#ifdef FPSCOUNTER
    int fpscount = 0;
    int fpsclock = 0;
    char fpsstr[10];
    memset(fpsstr, 0, sizeof(fpsstr));
#endif

    getcwd(initpath, 1023);

    signal(SIGINT, &quit);
    atexit(quit_handle);

    gp2x_init();
#ifdef TARGET_GP2X
    gp2x_setgamma(7);
#endif
    gp2xmb_init();

    while (mainloop)
    {
#ifdef FPSCOUNTER
        if (fpsclock + 1000 < SDL_GetTicks())
        {
            fpsclock = SDL_GetTicks();
            memset(fpsstr, 0, sizeof(fpsstr));
            sprintf(fpsstr, "FPS: %i", fpscount);
            fpscount = 0;
        }
        fpscount++;
#endif

        while (SDL_PollEvent(&event) && xmb_getFlagState(XMB_LOADED))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    keys[GP2X_BUTTON_LEFT].pressed = 1;
                    break;
                case SDLK_RIGHT:
                    keys[GP2X_BUTTON_RIGHT].pressed = 1;
                    break;
                case SDLK_UP:
                    keys[GP2X_BUTTON_UP].pressed = 1;
                    break;
                case SDLK_DOWN:
                    keys[GP2X_BUTTON_DOWN].pressed = 1;
                    break;
                case SDLK_HOME:
                    keys[GP2X_BUTTON_Y].pressed = 1;
                    break;
                case SDLK_BACKSPACE:
                case SDLK_ESCAPE:
                    keys[GP2X_BUTTON_B].pressed = 1;
                    break;
                case SDLK_RETURN:
                    keys[GP2X_BUTTON_X].pressed = 1;
                    break;
                case SDLK_PAGEUP:
                    keys[GP2X_BUTTON_R].pressed = 1;
                    break;
                case SDLK_INSERT:
                    keys[GP2X_BUTTON_L].pressed = 1;
                    break;
                case SDLK_a:
                    keys[GP2X_BUTTON_VOLDOWN].pressed = 1;
                    break;
                case SDLK_s:
                    keys[GP2X_BUTTON_VOLUP].pressed = 1;
                    break;
                case SDLK_SPACE:
                    mainloop = 0;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    keys[GP2X_BUTTON_LEFT].pressed = 0;
                    break;
                case SDLK_RIGHT:
                    keys[GP2X_BUTTON_RIGHT].pressed = 0;
                    break;
                case SDLK_UP:
                    keys[GP2X_BUTTON_UP].pressed = 0;
                    break;
                case SDLK_DOWN:
                    keys[GP2X_BUTTON_DOWN].pressed = 0;
                    break;
                case SDLK_HOME:
                    keys[GP2X_BUTTON_Y].pressed = 0;
                    break;
                case SDLK_BACKSPACE:
                case SDLK_ESCAPE:
                    keys[GP2X_BUTTON_B].pressed = 0;
                    break;
                case SDLK_RETURN:
                    keys[GP2X_BUTTON_X].pressed = 0;
                    break;
                case SDLK_PAGEUP:
                    keys[GP2X_BUTTON_R].pressed = 0;
                    break;
                case SDLK_INSERT:
                    keys[GP2X_BUTTON_L].pressed = 0;
                    break;
                case SDLK_a:
                    keys[GP2X_BUTTON_VOLDOWN].pressed = 0;
                    break;
                case SDLK_s:
                    keys[GP2X_BUTTON_VOLUP].pressed = 0;
                    break;
                case SDLK_SPACE:
                    mainloop = 0;
                    break;
                default:
                    break;
                }
                break;
            case SDL_JOYBUTTONDOWN:
                keys[event.jbutton.button].pressed = 1;
                break;

            case SDL_JOYBUTTONUP:
                keys[event.jbutton.button].pressed = 0;
                break;

            default:
                break;
            }
        }
        /* handle keys */
        for (i = 0; i < sizeof(keys) / sizeof(keyobj); i++)
        {
            if (i > 18)
                printf("%i\n", i);
            if (keys[i].pressed)
            {
                if (keys[i].timedelay > 0)
                {
                    keys[i].func();
                    keys[i].lastcall = keys[i].timedelay + SDL_GetTicks();
                    keys[i].timedelay *= -1;
                }
                else if (SDL_GetTicks() > keys[i].lastcall)
                {
                    if (keys[i].func)
                    {
						int swapXB = 0;

                        config_lookup_bool(&CONFIG, "swapXB", &swapXB);

						if (swapXB && (GP2X_BUTTON_X == i || GP2X_BUTTON_B == i))
                        {
                            if (GP2X_BUTTON_X == i)
                                keys[GP2X_BUTTON_B].func();
                            else if (GP2X_BUTTON_B == i)
                                keys[GP2X_BUTTON_X].func();
                        }
                        else
                            keys[i].func();
                    }
                    keys[i].lastcall =
                        keys[i].repeatdelay + SDL_GetTicks();
                }

                if (!keys[i].timedelay) /* no time delay */
                    keys[i].pressed = 0;
            }
            else if (keys[i].timedelay < 0)
            {
                keys[i].timedelay *= -1;
                keys[i].lastcall = 0;
            }
        }

#ifdef GFXOPENGL
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		bg_draw(screen);

		if (xmb_getFlagState(XMB_LOADED))
        {
            xmb_draw(screen);
        }

		SDL_GL_SwapBuffers( );
#else
        bg_draw(screen);

        if (xmb_getFlagState(XMB_LOADED))
        {
            xmb_draw(screen);
            if (!xmb_getFlagState(XMB_NOBATT))
                battery_draw(screen);

            if (xmb_getFlagState(XMB_SYNC))
                progress_draw(screen, 1);

            volume_draw(screen);

			#ifdef FPSCOUNTER
            gfx_draw_text(screen, font_small, 0, 0, 0, 0, fpsstr);
			#endif
        }
        else
        {
			splash_draw(screen);
        }
        SDL_Flip(screen);
#endif

       	/* let operating system breath */
        SDL_Delay(1);
    }

    SDL_FreeSurface(screen);

    quit(0);                    /* clean quit */

    return 0;
}

void *gp2xmb_tload(void *arg)
{
	int delay = SDL_GetTicks() + SPLASH_DELAY;

    /* start drawing the background and progress bar bere */
    progress_set(0);

    if (xmb_init() < 0)
    {
        fprintf(stderr, "Couldn't initialize XMB: %s\n", SDL_GetError());
        quit(1);
    }

    progress_set(80);
    /* Init battery monitor */
    if (battery_init())         /* non-zero on failure */
    {
        fprintf(stderr,
                "An error occured while initalising battery monitor.\n");
        quit(1);
    }

    progress_set(85);
    bg_load_wallpaper();

    progress_set(95);
    /* set volume */
    volume_init();

    progress_set(100);

	while(SDL_GetTicks() < delay)
	{
		SDL_Delay(100);
	}

	xmb_activateFlag(XMB_LOADED);       /* this will start the drawing of the XMB */

    progress_set(0);


    return ((void *) 0);
}

void gp2xmb_init()
{
#ifdef GFXOPENGL
	const SDL_VideoInfo *videoInfo;
	int videoflags = 0;
#endif

    syncid = 0;

    relaunchsystem = 1;

    /* Set CPU speed */
    gp2x_setclock(200);

    chdir(initpath);
    cfg_init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
    {
        printf
            ("\033[0;34mGP2XMB:\033[0;31m Could not initialize SDL:\033[0m %s\n",
             SDL_GetError());
        quit(1);
    }

    if (Mix_OpenAudio
        (AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS))
    {
        printf("Unable to open audio!\n");
        quit(1);
    }

#ifdef GFXOPENGL
	videoInfo = SDL_GetVideoInfo();

	if (!videoInfo)
	{
		fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
		quit(1);
	}

	videoflags  = SDL_OPENGL;
	videoflags |= SDL_GL_DOUBLEBUFFER;
	videoflags |= SDL_HWPALETTE;
	/* videoflags |= SDL_FULLSCREEN; */

    if (videoInfo->hw_available)
        videoflags |= SDL_HWSURFACE;
    else
        videoflags |= SDL_SWSURFACE;

    if (videoInfo->blit_hw)
        videoflags |= SDL_HWACCEL;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 24, videoflags);

	if (!screen)
	{
		fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
		quit( 1 );
	}

    gfx_initgl();
    gfx_resizewindow(screen->w, screen->h);

#else
    screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 16, SDL_SWSURFACE);
#endif

    SDL_ShowCursor(0);
    SDL_JoystickOpen(0);

    sfx_init();
    bg_init();
    progress_init();
    splash_init();
	scripts_init();

    pthread_create(&loaderid, NULL, gp2xmb_tload, NULL);
}

void gp2xmb_deinit()
{
    unsigned int i = 0;

    /* reset keys */
    for (i = 0; i < sizeof(keys) / sizeof(keyobj); i++)
    {
        if (keys[i].timedelay < 0)
            keys[i].timedelay *= -1;
        keys[i].lastcall = 0;
        keys[i].pressed = 0;
    }

    bg_destroy();
    xmb_destroy();
    scripts_destroy();
    gfx_unload_all();
    Mix_CloseAudio();
    gp2x_deinit();
    SDL_Quit();
    gp2xmb_syncwait();
    cfg_save();
    gp2xmb_syncwait();
    cfg_destroy();
    xmb_deactivateFlag(XMB_LOADED);
}

void *gp2xmb_ptreadsync(void *arg)
{
    /* xmb_activateFlag(XMB_SYNC); */
    sync();
    /* xmb_deactivateFlag(XMB_SYNC); */
    return ((void *) 0);
}

void gp2xmb_sync(void)
{
    pthread_create(&syncid, NULL, gp2xmb_ptreadsync, NULL);
}

void gp2xmb_syncwait(void)
{
    void *tret;

    if (syncid)
        pthread_join(syncid, &tret);
}
