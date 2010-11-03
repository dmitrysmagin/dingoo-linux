/*
 * LZW decode
 *
 *  Copyright (c) 2009 Ignacio Garcia Perez <iggarpe@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "lzw.h"

struct lzw_node {
	unsigned char symbol;
	int parent, son;
};

int lzw_decode (void *temp, const void *src, void *dst) {

	struct lzw_node *node = (struct lzw_node *)temp;
	unsigned long *s = (unsigned long *)src;
	unsigned char *d = (unsigned char *)dst;
	unsigned long c;
	int i, j, k, fix = -1, count = 257, bits = 9, bitpos = 0;

	for (i = 0; i < count; i++) {
		node[i].symbol = i;
		node[i].parent = -1;
	}

	for (;;) {

		c = *s >> bitpos;

		if ((bitpos += bits) >= 32) {
			bitpos -= 32;
			c |= *++s << (bits - bitpos);
		}

		c &= (1 << bits) - 1;

		/* Check for decode error and special end marker */
		if (c >= count) return -1;
		if (c == 256) return d - (unsigned char *)dst;

		/* Walk the code chain backwards up to the head while forward linking it */
		for (i = -1, j = c;; i = j, j = k) {
			k = node[j].parent;
			node[j].parent = i;
			if (k < 0) break;
		}

		/* If there is a node to be fixed, use the head symbol */
		if (fix >= 0) node[fix].symbol = node[j].symbol;

		/* Walk the code chain forward outputting symbols and relinking it */
		for (;; k = j, j = i) {
			*d++ = node[j].symbol;
			i = node[j].parent;
			node[j].parent = k;
			if (i < 0) break;
		}

		/* Add a new node (to be fixed, since we don't know its symbol yet) */
		fix = count;
		node[count].parent = c;
		if (++count > (1 << bits)) bits++;
	}
}

