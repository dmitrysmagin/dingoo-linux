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

#include "gp2xmb.h"
#include "scripts.h"

char *script_buf_usbE = NULL;
char *script_buf_usbD = NULL;

void scripts_init(void)
{
	FILE *fd = NULL;
	long flength = 0;

	fd = fopen("scripts/enableusb.sh","r");
	if(fd)
	{
		fseek(fd,0,SEEK_END);
		flength = ftell(fd);
		fseek(fd,0,SEEK_SET);
		script_buf_usbE = (char*)calloc(1,flength+1);
		if(fread(script_buf_usbE, flength, sizeof(char), fd) != flength)
		{
			/* read error */
		}
		fclose(fd);
	}

	fd = fopen("scripts/disableusb.sh","r");
	if(fd)
	{
		fseek(fd,0,SEEK_END);
		flength = ftell(fd);
		fseek(fd,0,SEEK_SET);
		script_buf_usbD = (char*)calloc(1,flength+1);
		if(fread(script_buf_usbD, flength, sizeof(char), fd) != flength)
		{
			/* read error */
		}
		fclose(fd);
	}
}

void scripts_destroy(void)
{
	if(script_buf_usbE)
		free(script_buf_usbE);
	if(script_buf_usbD)
		free(script_buf_usbD);
}

void scripts_exec(enum scriptcache sc, const char *arguments, ...)
{
	char text[1024];
	char path[PATH_MAX];
	va_list	ap;

	memset(text,0,sizeof(text));
	memset(path,0,sizeof(path));

	if (arguments != NULL)
	{
		va_start(ap, arguments);
		vsprintf(text, arguments, ap);
		va_end(ap);
	}

	switch(sc)
	{
		case USB_ENABLE:
			sprintf(path,"%s %s","enableusb.sh", text);
			setenv("ARGV", path, 1);
			system(script_buf_usbE);
			unsetenv("ARGV");
		break;

		case USB_DISABLE:
			sprintf(path,"%s %s","disableusb.sh", text);
			setenv("ARGV", path, 1);
			system(script_buf_usbD);
			unsetenv("ARGV");
		break;
	}
}
