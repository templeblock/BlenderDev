/*
 * $Id: NG_NetworkObject.h 35072 2011-02-22 12:42:55Z jesterking $
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
 */

/** \file NG_NetworkObject.h
 *  \ingroup bgenet
 *  \brief generic Network Object class
 */
#ifndef NG_NETWORKOBJECT_H
#define NG_NETWORKOBJECT_H

#include "STR_String.h"

#ifdef WITH_CXX_GUARDEDALLOC
#include "MEM_guardedalloc.h"
#endif

class NG_NetworkObject
{
	STR_String m_name;
public:
	NG_NetworkObject();
	~NG_NetworkObject();
	const STR_String& GetName();


#ifdef WITH_CXX_GUARDEDALLOC
public:
	void *operator new(size_t num_bytes) { return MEM_mallocN(num_bytes, "GE:NG_NetworkObject"); }
	void operator delete( void *mem ) { MEM_freeN(mem); }
#endif
};

#endif //NG_NETWORKOBJECT_H

