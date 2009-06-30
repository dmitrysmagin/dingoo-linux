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

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

int wftw(const char *dirpath,
         int (*fn) (const char *fpath, const struct stat * sb,
                    int typeflag), int nopenfd)
{
    int i = 0;
    struct stat statbuf;
    struct dirent **namelist;
    int files = 0;

    if (nopenfd < 0)
    {
        return -1;
    }

    files = scandir(dirpath, &namelist, 0, alphasort);
    if (files < 0)
    {
        return -1;
    }

    for (i = 0; i < files; i++)
    {
        char *tpath = (char *) calloc(1, 1024);

        if (!tpath)
            continue;

        sprintf(tpath, "%s/%s", dirpath, namelist[i]->d_name);
        stat(tpath, &statbuf);

        if (S_ISDIR(statbuf.st_mode) && strcmp(namelist[i]->d_name, "..")
            && strcmp(namelist[i]->d_name, "."))
        {
            wftw(tpath, fn, nopenfd - 1);
            fn(tpath, &statbuf, WFTW_D);
        }
        else
        {
            fn(tpath, &statbuf, WFTW_F);
        }

        free(tpath);
    }

    return 0;
}
