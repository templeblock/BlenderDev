/*
 * $Id: BKE_endian.h 34962 2011-02-18 13:05:18Z jesterking $
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 * Are we little or big endian?  From Harbison&Steele.
 */
#ifndef BKE_ENDIAN_H
#define BKE_ENDIAN_H

/** \file BKE_endian.h
 *  \ingroup bke
 */

/**
 * BKE_ENDIANNESS(a) returns 1 if big endian and returns 0 if little endian
 */
#define BKE_ENDIANNESS(a) {  \
	union {  \
		intptr_t l;  \
		char c[sizeof (intptr_t)];  \
	} u;  \
	u.l = 1;  \
	a = (u.c[sizeof (intptr_t) - 1] == 1) ? 1 : 0;  \
}

#endif

