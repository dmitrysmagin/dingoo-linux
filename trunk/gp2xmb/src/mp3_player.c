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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <sys/soundcard.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "gp2xmb.h"
#include "mp3_player.h"

#include <mad.h>


int mp3eof = 0;
char *mp3_file = NULL;
pthread_t mp3id;
pthread_mutex_t mp3_lock;
Mix_Music *mp3music;
unsigned int mp3_start_time = 0;

const char *mp3strDeviceName = "/dev/dsp";
int mp3format = AFMT_S16_LE;
int mp3channels = 2;
int mp3speed = 11025;
int mp3audio_fd = 0;

struct mp3_buffer {
  unsigned char const *start;
  unsigned long length;
};
struct mp3_thread_argv {
	unsigned char const *start;
	unsigned long length;
};
struct stat mp3_stat;
void * mp3_thread_play(void * argv);
static int mp3_decode(unsigned char const *, unsigned long);



static enum mad_flow mp3_input(void *data, struct mad_stream *stream)
{
  struct mp3_buffer *buffer = data;

  if (!buffer->length)
    return MAD_FLOW_STOP;

  mad_stream_buffer(stream, buffer->start, buffer->length);

  buffer->length = 0;

  return MAD_FLOW_CONTINUE;
}

static inline signed int mp3_scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow mp3_output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  while (nsamples--)
  {
    signed int sample;
    signed int buff;

    sample = mp3_scale(*left_ch++);
    buff = (sample >> 0) & 0xff;
    write(mp3audio_fd,&buff,sizeof(buff));
    buff = (sample >> 8) & 0xff;
    write(mp3audio_fd,&buff,sizeof(buff));

    if (nchannels == 2)
    {
		sample = mp3_scale(*right_ch++);
        buff = (sample >> 0) & 0xff;
	    write(mp3audio_fd,&buff,sizeof(buff));
	    buff = (sample >> 8) & 0xff;
	    write(mp3audio_fd,&buff,sizeof(buff));
    }
  }

  return MAD_FLOW_CONTINUE;
}

static enum mad_flow mp3_error(void *data, struct mad_stream *stream,
		    struct mad_frame *frame)
{
  struct mp3_buffer *buffer = data;

  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
	  stream->error, mad_stream_errorstr(stream),
	  stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

static int mp3_decode(unsigned char const *start, unsigned long length)
{
  struct mp3_buffer buffer;
  struct mad_decoder decoder;
  int result;

  /* initialize our private message structure */

  buffer.start  = start;
  buffer.length = length;

  /* configure input, output, and error functions */

  mad_decoder_init(&decoder, &buffer,
		   mp3_input, 0 /* header */, 0 /* filter */, mp3_output,
		   mp3_error, 0 /* message */);

  /* start decoding */

  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */

  mad_decoder_finish(&decoder);

  return result;
}

void mp3_thread_close(void *arg)
{
    /* oss device */
    close(mp3audio_fd);

    mp3audio_fd = 0;
}

void * mp3_thread_play(void * argv)
{
	struct stat mp3_stat;
	void *fdm;
	int fd = 0;

	pthread_cleanup_push(mp3_thread_close, NULL);

	if (mp3_file == NULL)
		return ((void *) 1);

	fd = open(mp3_file,O_RDONLY);

	if (fstat(fd, &mp3_stat) == -1 || mp3_stat.st_size == 0)
		return ((void *) 2);

	fdm = mmap(0, mp3_stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (fdm == MAP_FAILED)
		return ((void *) 3);

	mp3_decode(fdm, mp3_stat.st_size);

	if (munmap(fdm, mp3_stat.st_size) == -1)
		return ((void *) 4);

	if(fd)
		close(fd);

	pthread_cleanup_pop(1);

	return ((void *) 0);
}


char *mp3_metainfo(char *path, int type)
{
    return NULL;
}

int mp3_init(char *path)
{
	int frag = 0;

    mp3eof = 0;
	mp3_file = path;


	if ((mp3audio_fd = open(mp3strDeviceName, O_WRONLY, 0)) == -1)
    {
        perror(mp3strDeviceName);
        return 2;
    }
    frag = (12 << 16) | (13);

    if (ioctl(mp3audio_fd, SNDCTL_DSP_SETFRAGMENT, &frag) == -1)
    {
        perror("SNDCTL_DSP_SETFRAGMENT");
        close(mp3audio_fd);
        return 3;
    }
    if (ioctl(mp3audio_fd, SNDCTL_DSP_SETFMT, &mp3format) == -1)
    {
        /* Fatal error */
        perror("SNDCTL_DSP_SETFMT");
        close(mp3audio_fd);
        return 3;
    }
    if (mp3format != AFMT_S16_LE)
    {
        /* The device doesn't support the requested audio format. The program
           should use another format (for example the one returned in "format")
           or alternatively it must display an error message and to abort. */
        perror("AFMT_S16_LE unsupported");
        close(mp3audio_fd);
        return 4;
    }

    if (ioctl(mp3audio_fd, SNDCTL_DSP_CHANNELS, &mp3channels) == -1)
    {
        /* Fatal error */
        perror("SNDCTL_DSP_CHANNELS");
        close(mp3audio_fd);
        return 5;
    }
    if (!mp3channels)
    {
        perror("The device doesn't support the requested channels");
        close(mp3audio_fd);
        return 6;
    }

    /* set oss sample speed */
    /* mp3speed = ovvi->rate; */
    if (ioctl(mp3audio_fd, SNDCTL_DSP_SPEED, &mp3speed) == -1)
    {
        /* Fatal error */
        perror("SNDCTL_DSP_SPEED");
        close(mp3audio_fd);
        return 7;
    }
    if (!mp3speed)
    {
        perror("The device doesn't support the requested speed.");
        close(mp3audio_fd);
        return 8;
    }

    return 0;
}

int mp3_eof(void)
{
    return mp3eof;
}

int mp3_ready(void)
{
    return 1;
}

void mp3music_finished()
{
    mp3eof = 1;
}

int mp3_play(void)
{
	pthread_create(&mp3id, NULL, mp3_thread_play, NULL);
    return 0;
}

unsigned int mp3_getpos(void)
{
    return 0;
}

unsigned int mp3_getlength(void)
{
    return 1;
}

int mp3_fwd(void)
{
    return 0;
}

int mp3_bwd(void)
{
    return 0;
}

int mp3_stop(void)
{
    return 0;
}
