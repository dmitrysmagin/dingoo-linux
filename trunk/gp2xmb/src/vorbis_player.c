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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#ifdef TREMOR
# include <tremor/ivorbiscodec.h>
# include <tremor/ivorbisfile.h>
#else
# include <vorbis/codec.h>
# include <vorbis/vorbisfile.h>
#endif

#include "vorbis_player.h"


char pcmout[4096];              /* take 4k out of the data segment, not the stack */

OggVorbis_File ovfd;
int oveof = 0;
int ovcurrent_section;
char **ovptr = NULL;
vorbis_info *ovvi;

int ovaudio_fd = 0;
FILE *ovmfd;
const char *ovstrDeviceName = "/dev/dsp";
int ovformat = AFMT_S16_LE;
int ovchannels = 2;
int ovspeed = 11025;

int ov_seek = 0;

pthread_t ovid;
pthread_mutex_t ov_lock;

char st_metainfo[1024];         /* meta info query storage */

#define SNDCTL_DSP_POLICY 45

char *op_metainfo(char *path, int type)
{
    FILE *fd = fopen(path, "r");        /* ov_open will release this */
    OggVorbis_File ofd;
    char **ptr;

    memset(st_metainfo, 0, sizeof(st_metainfo));

    if (ov_open(fd, &ofd, NULL, 0) < 0)
    {
        fclose(fd);
        return NULL;
    }

    ptr = ov_comment(&ofd, -1)->user_comments;
    while (*ptr)
    {
        char key[32], value[1024];
        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));

        sscanf(*ptr, "%31[^=]%*c%1024c", key, value);

        if (type == OVMARTIST)
        {
            if (!strcasecmp(key, "ARTIST") ||
                !strcasecmp(key, "PERFORMER"))
            {
                strncpy(st_metainfo, value, sizeof(st_metainfo) - 1);
                return st_metainfo;
            }
        }
        else if (type == OVMTITLE)
        {
            if (!strcasecmp(key, "TITLE"))
            {
                strncpy(st_metainfo, value, sizeof(st_metainfo) - 1);
                return st_metainfo;
            }
        }

        ++ptr;
    }

    ov_clear(&ofd);

    return NULL;
}

int op_init(char *path)
{
    int frag = 0;

    oveof = 0;

    ov_clear(&ovfd);

    pthread_mutex_init(&ov_lock, NULL);

    ovmfd = fopen(path, "r");

    /* open ogg file */
    if (ov_open(ovmfd, &ovfd, NULL, 0) < 0)
    {
        fprintf(stderr, "Input does not appear to be an Ogg bitstream.\n");
        return 1;
    }

    ovvi = ov_info(&ovfd, -1);

    if ((ovaudio_fd = open(ovstrDeviceName, O_WRONLY, 0)) == -1)
    {
        perror(ovstrDeviceName);
        return 2;
    }

    /* Need to lower the soundcard buffer otherwise we will have sync problems */
    /* see http://manuals.opensound.com/developer/SNDCTL_DSP_SETFRAGMENT.html */
    frag = (12 << 16) | (13);

    if (ioctl(ovaudio_fd, SNDCTL_DSP_SETFRAGMENT, &frag) == -1)
    {
        perror("SNDCTL_DSP_SETFRAGMENT");
        close(ovaudio_fd);
        return 3;
    }

    if (ioctl(ovaudio_fd, SNDCTL_DSP_SETFMT, &ovformat) == -1)
    {
        /* Fatal error */
        perror("SNDCTL_DSP_SETFMT");
        close(ovaudio_fd);
        return 3;
    }
    if (ovformat != AFMT_S16_LE)
    {
        /* The device doesn't support the requested audio format. The program
           should use another format (for example the one returned in "format")
           or alternatively it must display an error message and to abort. */
        perror("AFMT_S16_LE unsupported");
        close(ovaudio_fd);
        return 4;
    }
    /* set oss channel format */
    ovchannels = ovvi->channels;
    if (ioctl(ovaudio_fd, SNDCTL_DSP_CHANNELS, &ovchannels) == -1)
    {
        /* Fatal error */
        perror("SNDCTL_DSP_CHANNELS");
        close(ovaudio_fd);
        return 5;
    }
    if (ovchannels != ovvi->channels)
    {
        perror("The device doesn't support the requested channels");
        close(ovaudio_fd);
        return 6;
    }

    /* set oss sample speed */
    ovspeed = ovvi->rate;
    if (ioctl(ovaudio_fd, SNDCTL_DSP_SPEED, &ovspeed) == -1)
    {
        /* Fatal error */
        perror("SNDCTL_DSP_SPEED");
        close(ovaudio_fd);
        return 7;
    }
    if (ovspeed != ovvi->rate)
    {
        perror("The device doesn't support the requested speed.");
        close(ovaudio_fd);
        return 8;
    }

    return 0;
}

int op_eof(void)
{
    return oveof;
}

int op_ready(void)
{
    return !ovaudio_fd;
}

/*
 * Plays the music loaded.
 */
int op_play(void)
{
    pthread_create(&ovid, NULL, op_thread_play, NULL);
    return 0;
}

unsigned int op_getpos(void)
{
    unsigned int pos = 0;
    pthread_mutex_lock(&ov_lock);
    pos = ov_time_tell(&ovfd);
    pthread_mutex_unlock(&ov_lock);
    return pos;
}

unsigned int op_getlength(void)
{
    unsigned int length = 0;
    pthread_mutex_lock(&ov_lock);
    length = ov_time_total(&ovfd, -1);
    pthread_mutex_unlock(&ov_lock);
    return length;
}

int op_stop(void)
{
    pthread_mutex_lock(&ov_lock);
    oveof = 1;
    pthread_mutex_unlock(&ov_lock);
    return 0;
}

int op_fwd(void)
{
    pthread_mutex_lock(&ov_lock);
#ifdef TREMOR
    ov_seek += 5000;
#else
    ov_seek += 5;
#endif
    pthread_mutex_unlock(&ov_lock);

    return 0;
}

int op_bwd(void)
{
    pthread_mutex_lock(&ov_lock);
#ifdef TREMOR
    ov_seek -= 5000;
#else
    ov_seel -= 5000;
#endif
    pthread_mutex_unlock(&ov_lock);

    return 0;
}

void op_thread_close(void *arg)
{
    /* oss device */
    close(ovaudio_fd);

    ovaudio_fd = 0;
}

void *op_thread_play(void *arg)
{
    pthread_cleanup_push(op_thread_close, NULL);

    if (ioctl(ovaudio_fd, SNDCTL_DSP_RESET, 0) == -1)
    {
        perror(":ioctl(SNDCTL_DSP_RESET)");
    }

    while (!op_eof())
    {
        long ret = 0;

        if (ov_seek)
        {
            pthread_mutex_lock(&ov_lock);
            ov_time_seek(&ovfd, ov_time_tell(&ovfd) + ov_seek);
            ov_seek = 0;
            pthread_mutex_unlock(&ov_lock);
        }

#ifdef TREMOR
        ret = ov_read(&ovfd, pcmout, sizeof(pcmout), &ovcurrent_section);
#else
        ret =
            ov_read(&ovfd, pcmout, sizeof(pcmout), 0, 2, 1,
                    &ovcurrent_section);
#endif

        if (ret == 0)
        {
            pthread_mutex_lock(&ov_lock);
            oveof = 1;
            pthread_mutex_unlock(&ov_lock);
        }
        else if (ret < 0)
        {
            fprintf(stderr, "error in the stream.\n");
        }
        else
        {
            /* write oss device */
            int pos = 0, len = 0;
            do
            {
                pos += len;
                if ((len =
                     write(ovaudio_fd, pcmout + pos, ret - pos)) == -1)
                {
                    perror("audio write");
                    close(ovaudio_fd);
                    pthread_mutex_unlock(&ov_lock);
                    oveof = 1;
                    break;
                }
            }
            while (pos + len < ret);
        }
    }

    if (ioctl(ovaudio_fd, SNDCTL_DSP_RESET, 0) == -1)
    {
        perror(":ioctl(SNDCTL_DSP_RESET)");
    }

    pthread_cleanup_pop(1);

    return ((void *) 0);
}
