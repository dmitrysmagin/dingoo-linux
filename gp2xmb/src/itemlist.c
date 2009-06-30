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
#include <math.h>
#include <SDL_rotozoom.h>

#include "itemlist.h"
#include "gfx.h"
#include "mediabar.h"

tlistitem *li_root = NULL;
tlistitem *li_curr_root = NULL;

int li_selected = 0;
int li_stage = 0;
int li_create_new = 0;
SDL_Surface *li_arrow = NULL;
SDL_Surface *li_line = NULL;
int li_animtimer = 0;
char *li_emptymsg = NULL;

int LIST_OFFSETX = LIST_ITEMWIDTH;

extern SFont_Font *font_small;
extern SFont_Font *font_small_semi;
extern SFont_Font *font_normal;
extern SFont_Font *font_normal_semi;

void LIST_init(void)
{
    li_root = NULL;
    li_curr_root = NULL;
    li_selected = 0;
    li_stage = 0;
    li_create_new = 0;
    li_arrow = gfx_load_image("images/icons/arrow-left.png", 1);
    li_line = gfx_load_image("images/icons/line.png", 1);
    LIST_OFFSETX = LIST_ITEMWIDTH;
}

void LIST_add(SDL_Surface * icon,
              SDL_Surface * filetypeicon,
              const char *title, const char *subtitle, void *extra)
{
    tlistitem *curr = NULL;

    if (li_root == NULL)
    {
        li_root = (tlistitem *) malloc(sizeof(tlistitem));
        if (!li_root)
            return;
        li_root->prev = NULL;
        li_root->next = NULL;
        li_curr_root = li_root;
        curr = li_curr_root;
        curr->parent = NULL;
    }
    else if (li_create_new && li_curr_root)
    {

        li_curr_root->root = (tlistitem *) malloc(sizeof(tlistitem));
        if (!li_curr_root->root)
            return;
        li_curr_root->root->prev = NULL;
        li_curr_root->root->next = NULL;
        li_curr_root->root->parent = LIST_getSelected();
        li_curr_root = li_curr_root->root;
        li_create_new = 0;
        curr = li_curr_root;

        li_selected = 0;        /* Important */
    }
    else if (li_curr_root)
    {
        tlistitem *walker = li_curr_root;
        while (walker && walker->next)
            walker = walker->next;
        walker->next = (tlistitem *) malloc(sizeof(tlistitem));
        walker->next->prev = walker;
        walker->next->next = NULL;
        curr = walker->next;
        curr->parent = li_curr_root->parent;
    }

	LIST_seticon(icon, curr);

    curr->filetypeicon = filetypeicon;
    curr->flags = ITF_NONE;

    memset(curr->title, 0, sizeof(curr->title));
    if (title)
    {
        strncpy(curr->title, title, sizeof(curr->title) - 1);
        /* Elipsis
           if(SFont_TextWidth(font_normal,curr->title) > li_line->w)
           {
           char *cptr = curr->title;
           int swidth = 0;
           char sc[2];
           memset(sc,0,sizeof(sc));
           for( sc[0] = cptr[0];
           *cptr && swidth < li_line->w - 10;
           cptr++,sc[0] = cptr[0])
           {
           swidth+=SFont_TextWidth(font_normal,sc);
           }
           cptr-=2;
           sprintf(cptr,"...");
           } */
    }
    memset(curr->subtitle, 0, sizeof(curr->subtitle));
    if (subtitle)
        strncpy(curr->subtitle, subtitle, sizeof(curr->subtitle) - 1);

    curr->extra = extra;
    curr->root = NULL;
}

void LIST_seticon(SDL_Surface *icon, tlistitem *target)
{
    target->icon = icon;
    if (icon)
    {
        target->icon_semi =
            SDL_ConvertSurface(icon, icon->format, icon->flags);
        gfx_set_alpha(target->icon_semi, 192);
    }
    else
        target->icon_semi = NULL;
}

/* Not recursive - yet */
void LIST_destroy(void)
{
    if (li_root)
        LIST_empty(&li_root);

    li_root = NULL;
    li_curr_root = NULL;
    li_selected = 0;
    li_stage = 0;
    li_create_new = 0;
}

/* Will not perform or test for recursive delete.
   Will not free icons */
void LIST_empty(tlistitem ** troot)
{
    tlistitem *c = (*troot);
    while (c != NULL)
    {
        tlistitem *tmp = c->next;
        if (c->icon)
            SDL_FreeSurface(c->icon);
        if (c->icon_semi)
            SDL_FreeSurface(c->icon_semi);
        if (c->filetypeicon)
            SDL_FreeSurface(c->filetypeicon);
        if (c->extra)
            free(c->extra);
        free(c);
        c = tmp;
    }
    (*troot) = NULL;
}

void LIST_setEmptyMsg(char *msg)
{
    li_emptymsg = msg;
}

int LIST_draw(SDL_Surface * screen, int draw_offset_x, int draw_offset_y)
{
    tlistitem *curr = li_curr_root;
    tlistitem *curr_level = li_curr_root;
    tlistitem *c = NULL;
    int num_levels = 0;

    int offset_x = 0, i = 0, centery = 0, backup_sel = 0, level_back =
        0, levels = 0;
    int offset_y = draw_offset_y;

    for (c = li_curr_root; c != NULL; c = c->parent, levels++);

    offset_x =
        ((levels) * ((LIST_ITEMWIDTH + li_arrow->w))) + LIST_OFFSETX +
        draw_offset_x + LIST_ITEMWIDTH + li_arrow->w;

    switch (li_stage)
    {
    case 1:                    /* Move up */
        {
            offset_y =
                ((LIST_ITEMHEIGHT / (float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - li_animtimer)) - LIST_ITEMHEIGHT;
            if (offset_y > LIST_ITEMHEIGHT)
                offset_y = LIST_ITEMHEIGHT;
        }
        break;

    case 2:                    /* Move down */
        {
            offset_y =
                LIST_ITEMHEIGHT -
                ((LIST_ITEMHEIGHT / (float) (XMBMOVEDELAY)) *
                 (SDL_GetTicks() - li_animtimer));
            if (offset_y < 0)
                offset_y = 0;
        }
        break;

    default:
        break;
    }

    if (SDL_GetTicks() - li_animtimer > XMBMOVEDELAY)
    {
        li_stage = 0;
        offset_y = 0;
    }

    backup_sel = li_selected;

    while (curr_level != NULL)
    {
    	num_levels++;
        curr = curr_level;

        if (curr != li_curr_root)
        {
            li_selected = level_back;
            offset_y = 0;
        }
        else
            li_selected = backup_sel;

        offset_x -= LIST_ITEMWIDTH + li_arrow->w;

        gfx_draw_image(li_arrow, offset_x - li_arrow->w, 113, screen);

        for (i = 0; curr != NULL && i < li_selected;
             i++, curr = curr->next)
        {
            int seloffset = 0;

            if (xmb_getFlagState(XMB_FOCUS))
                continue;

            if (offset_y && li_stage == 2 && i + 1 == li_selected)
                seloffset =
                    (offset_y / (float) LIST_ITEMHEIGHT) *
                    (LIST_SELITEMHEIGHT -
                     ((LIST_SELITEMHEIGHT - LIST_ITEMHEIGHT) / 2));
            else
                seloffset = offset_y;

            centery = -((li_selected - 2) * LIST_ITEMHEIGHT) +
                i * LIST_ITEMHEIGHT + seloffset + (LIST_ITEMHEIGHT / 2.0) +
                LISTYHACK;

            if (curr->icon)
            {
                SDL_Surface *icon = NULL;
                float zoom = 0.5;

                if (li_stage == 2 && i + 1 == li_selected)
                {
                    zoom +=
                        ((float) offset_y / (LIST_SELITEMHEIGHT / 2)) /
                        2.0;
                }

                icon =
                    zoomSurface(curr->icon_semi, zoom, zoom,
                                SMOOTHING_OFF);

                gfx_draw_image(icon,
                               offset_x + (curr->icon_semi->w -
                                           icon->w) / 2,
                               centery - (LIST_ITEMHEIGHT / 2.0) +
                               (LIST_ITEMHEIGHT - icon->h) / 2, screen);

                SDL_FreeSurface(icon);
            }
            if (curr_level == li_curr_root && !li_create_new)
            {
                gfx_draw_text(screen,
                                font_normal_semi,
                                LIST_FPAD + offset_x,
                                centery -
                                (gfx_text_height(font_normal) / 2),
                                NOSHADOW, NOGLOW, curr->title);

				if (curr->filetypeicon)
                {
                    gfx_draw_image(curr->filetypeicon,
                                    screen->w -
                                    curr->filetypeicon->w - 10,
                                    centery +
                                    ((LIST_ITEMHEIGHT / 2) -
                                    curr->filetypeicon->h),
                                    screen);
                }
            }
        }

        for (i = 0; curr != NULL; i++, curr = curr->next)
        {
            centery = 0;

            if (i != 0)
            {
                int seloffset = 0;

                if (xmb_getFlagState(XMB_FOCUS))
                    continue;

                if (offset_y && li_stage == 1 && i == 1)
                {
                    seloffset =
                        (offset_y / (float) LIST_ITEMHEIGHT) *
                        (LIST_SELITEMHEIGHT -
                         ((LIST_SELITEMHEIGHT - LIST_ITEMHEIGHT) / 2));
                    if (seloffset > 0)
                        seloffset = 0;
                }
                else
                    seloffset = offset_y;

                centery =
                    ((LIST_ITEMHEIGHT * 2) +
                     (LIST_SELITEMHEIGHT - LIST_ITEMHEIGHT)) +
                    i * LIST_ITEMHEIGHT + seloffset +
                    (LIST_ITEMHEIGHT / 2.0) + LISTYHACK;

                if (curr->icon)
                {
                    SDL_Surface *icon = NULL;
                    float zoom = 0.5;

                    if (li_stage == 1 && i == 1)
                    {
                        zoom -=
                            ((float) offset_y / (LIST_SELITEMHEIGHT / 2)) /
                            2.0;
                    }

                    icon =
                        zoomSurface(curr->icon_semi, zoom, zoom,
                                    SMOOTHING_OFF);

                    gfx_draw_image(icon,
                                   offset_x + (curr->icon->w -
                                               icon->w) / 2,
                                   centery - (LIST_ITEMHEIGHT / 2.0) +
                                   (LIST_ITEMHEIGHT - icon->h) / 2,
                                   screen);

                    SDL_FreeSurface(icon);

                }

                if (curr_level == li_curr_root && !li_create_new)
                {
                    gfx_draw_text(screen,
                                    font_normal_semi,
                                    LIST_FPAD + offset_x,
                                    centery -
                                    (gfx_text_height(font_normal) / 2),
                                    NOSHADOW, NOGLOW, curr->title);

					if (curr->filetypeicon)
                    {
                        gfx_draw_image(curr->filetypeicon,
                                        screen->w -
                                        curr->filetypeicon->w - 10,
                                        centery +
                                        ((LIST_ITEMHEIGHT / 2) -
                                        curr->filetypeicon->h),
                                        screen);
                    }
                }
            }
            else
            {
                int seloffset = 0;
                int useglow, useshadow;
                SFont_Font *afont =
                    (!xmb_getFlagState(XMB_FOCUS) ? font_normal :
                     font_normal_semi);

                useglow = useshadow = !xmb_getFlagState(XMB_FOCUS);


                if (offset_y && li_stage)
                    seloffset =
                        (offset_y / (float) LIST_ITEMHEIGHT) *
                        (LIST_SELITEMHEIGHT -
                         ((LIST_SELITEMHEIGHT - LIST_ITEMHEIGHT) / 2));
                else
                    seloffset = 0;

                centery =
                    LIST_ITEMHEIGHT * 2 + seloffset +
                    (LIST_SELITEMHEIGHT / 2.0) + LISTYHACK;

                if (curr->icon)
                {
                    gfx_draw_image(curr->icon,
                                   offset_x,
                                   centery - (LIST_ITEMHEIGHT / 2.0) +
                                   (LIST_ITEMHEIGHT - curr->icon->h) / 2,
                                   screen);

                    gfx_set_alpha(curr->icon, 255);
                }

                if (curr_level == li_curr_root && !li_create_new)
                {
                    if (curr->subtitle[0] != '\0')
                    {
                        gfx_draw_text(screen,
                                      afont,
                                      LIST_FPAD + offset_x,
                                      centery - (LIST_ITEMHEIGHT / 2),
                                      useshadow, useglow, curr->title);

                        gfx_draw_text(screen,
                                      font_small,
                                      LIST_FPAD + offset_x,
                                      centery + ((LIST_ITEMHEIGHT / 2) -
                                                 gfx_text_height
                                                 (font_small)), USESHADOW,
                                      NOGLOW, curr->subtitle);

                        if (curr->filetypeicon)
                        {
                            gfx_draw_image(curr->filetypeicon,
                                           screen->w -
                                           curr->filetypeicon->w - 10,
                                           centery +
                                           ((LIST_ITEMHEIGHT / 2) -
                                            curr->filetypeicon->h),
                                           screen);
                        }
                    }
                    else
                    {
                        gfx_draw_text(screen,
                                      afont,
                                      LIST_FPAD + offset_x,
                                      centery -
                                      (gfx_text_height(font_normal) / 2),
                                      useshadow, useglow, curr->title);
                    }
                }
            }
        }

        for (level_back = 0, curr_level = curr_level->parent;
             curr_level != NULL && curr_level->prev != NULL;
             curr_level = curr_level->prev, level_back++);
    }

    if ((li_create_new || curr_level == NULL) && li_emptymsg)
    {
        gfx_draw_image(li_arrow, offset_x - li_arrow->w, 113, screen);
        gfx_draw_text(screen, font_normal,
                      offset_x + (LIST_columnWidth()*num_levels),
                      115, USESHADOW, NOGLOW, li_emptymsg);
    }

    li_selected = backup_sel;

    return 0;
}

void LIST_in(void)
{
    li_create_new = 1;
}

int LIST_out(void)
{
    tlistitem *tmp_selected = NULL, *tmp_root = NULL;
    int count = 0;

    if (li_create_new)
    {
        li_create_new = 0;
        return 0;
    }

    if (!li_curr_root || li_curr_root == li_root)
        return 1;

    /* find selected item */
    tmp_selected = LIST_getSelected();

    if (!tmp_selected)
        return 1;

    /* find the root of the parent list of tmp_selected */
    tmp_root = tmp_selected->parent;
    for (count = 0;
         tmp_root != NULL && tmp_root->prev != NULL;
         tmp_root = tmp_root->prev, count++);

    /* count should now be the amount of steps down to the selected item in
       the "new" list */

    li_selected = count;

    /* empty old and set to new root */
    LIST_empty(&li_curr_root);
    li_curr_root = tmp_root;


    return 0;
}

int LIST_up(void)
{
    if (li_curr_root && LIST_getSelected()->prev != NULL && !li_create_new)
    {
        li_stage = 1;
        li_selected--;
        li_animtimer = SDL_GetTicks();
        return 1;
    }
    return 0;
}

int LIST_down(void)
{
    if (li_curr_root && LIST_getSelected()->next != NULL && !li_create_new)
    {
        li_stage = 2;
        li_selected++;
        li_animtimer = SDL_GetTicks();
        return 1;
    }
    return 0;
}

tlistitem *LIST_getSelected(void)
{
    tlistitem *tmp_selected = li_curr_root;
    int count;
    for (count = 0;
         tmp_selected != NULL && count < li_selected;
         tmp_selected = tmp_selected->next, count++);

    return tmp_selected;
}

tlistitem *LIST_getelement(int index)
{
    int count;
    tlistitem *element = LIST_getSelected();

    for(; element != NULL && element->prev != NULL; element = element->prev);

    for (count = 0; element != NULL; count++, element = element->next)
    {
		if(count == index)
			return element;
    }

    return NULL;
}

int LIST_getSelNum(void)
{
    tlistitem *tmp_selected = li_curr_root;
    int count;
    for (count = 0;
         tmp_selected != NULL && count < li_selected;
         tmp_selected = tmp_selected->next, count++);

    return count;
}

int LIST_columnWidth(void)
{
    return LIST_ITEMWIDTH + li_arrow->w;
}
