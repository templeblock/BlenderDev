/*
 * $Id: GHOST_SystemPathsX11.cpp 39160 2011-08-07 17:38:36Z campbellbarton $
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
 * The Original Code is Copyright (C) 2010 Blender Foundation.
 * All rights reserved.
 *
 * 
 * Contributor(s): Blender Foundation
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file ghost/intern/GHOST_SystemPathsX11.cpp
 *  \ingroup GHOST
 */


#include "GHOST_SystemPathsX11.h"

#include "GHOST_Debug.h"

// For timing

#include <sys/time.h>
#include <unistd.h>

#include <stdio.h> // for fprintf only
#include <cstdlib> // for exit

#ifdef PREFIX
static const char *static_path= PREFIX "/share" ;
#else
static const char *static_path= NULL;
#endif

GHOST_SystemPathsX11::GHOST_SystemPathsX11()
{
}

GHOST_SystemPathsX11::~GHOST_SystemPathsX11()
{
}

const GHOST_TUns8* GHOST_SystemPathsX11::getSystemDir() const
{
	/* no prefix assumes a portable build which only uses bundled scripts */
	return (const GHOST_TUns8 *)static_path;
}

const GHOST_TUns8* GHOST_SystemPathsX11::getUserDir() const
{
	return (const GHOST_TUns8 *)getenv("HOME");
}

const GHOST_TUns8* GHOST_SystemPathsX11::getBinaryDir() const
{
	return NULL;
}

void GHOST_SystemPathsX11::addToSystemRecentFiles(const char* filename) const
{
	/* XXXXX TODO: Implementation for X11 if possible */

}
