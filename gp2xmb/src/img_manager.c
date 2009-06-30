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
#include <SDL.h>
#include <SDL_image.h>

struct imglist_el
{
    struct imglist_el *next;
    char *filename;
    SDL_Surface *data;
    int links;
};
typedef struct imglist_el img_node;

img_node *imgroot = NULL;
img_node *imglast = NULL;

void img_clear_cache()
{
    img_node *curr = imgroot;

    while (curr)
    {
        img_node *tmp = curr->next;
        free(curr->filename);
        SDL_FreeSurface(curr->data);
        free(curr);
        curr = tmp;
    }
    imgroot = NULL;
    imglast = NULL;
}

int img_cache_add(char *filename, SDL_Surface * data)
{
    if (!filename || !data)
        return -1;              /* invalid input */

    if (!imgroot)
    {
        imgroot = (img_node *) malloc(sizeof(img_node));
        if (!imgroot)
            return -1;          /* malloc error */
        imglast = imgroot;
    }
    else
    {
        imglast->next = (img_node *) malloc(sizeof(img_node));
        if (!imglast->next)
            return -1;
        imglast = imglast->next;
    }

    imglast->next = NULL;
    imglast->data = data;
    imglast->links = 1;
    imglast->filename = (char *) calloc(1, strlen(filename) + 1);
    strncpy(imglast->filename, filename, strlen(filename));

    return 0;
}

SDL_Surface *img_cache_get(char *filename)
{
    img_node *curr = imgroot;
    for (; curr != NULL; curr = curr->next)
    {
        if (curr->filename && !strcmp(filename, curr->filename))
        {
            curr->links++;
            return curr->data;
        }
    }
    return NULL;
}
