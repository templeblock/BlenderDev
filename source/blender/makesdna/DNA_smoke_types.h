/*
* $Id: DNA_smoke_types.h 34941 2011-02-17 20:48:12Z jesterking $
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
* The Original Code is Copyright (C) 2006 by NaN Holding BV.
* All rights reserved.
*
* The Original Code is: all of this file.
*
* Contributor(s): Daniel Genrich (Genscher)
*
* ***** END GPL LICENSE BLOCK *****
*/
#ifndef DNA_SMOKE_TYPES_H
#define DNA_SMOKE_TYPES_H

/** \file DNA_smoke_types.h
 *  \ingroup DNA
 */

/* flags */
#define MOD_SMOKE_HIGHRES (1<<1) /* enable high resolution */
#define MOD_SMOKE_DISSOLVE (1<<2) /* let smoke dissolve */
#define MOD_SMOKE_DISSOLVE_LOG (1<<3) /* using 1/x for dissolve */

#define MOD_SMOKE_HIGH_SMOOTH (1<<5) /* smoothens high res emission*/
#define MOD_SMOKE_FILE_LOAD (1<<6) /* flag for file load */

/* noise */
#define MOD_SMOKE_NOISEWAVE (1<<0)
#define MOD_SMOKE_NOISEFFT (1<<1)
#define MOD_SMOKE_NOISECURL (1<<2)
/* viewsettings */
#define MOD_SMOKE_VIEW_SHOWBIG (1<<0)

/* cache compression */
#define SM_CACHE_LIGHT		0
#define SM_CACHE_HEAVY		1

/* domain border collision */
#define SM_BORDER_OPEN		0
#define SM_BORDER_VERTICAL	1
#define SM_BORDER_CLOSED	2

typedef struct SmokeDomainSettings {
	struct SmokeModifierData *smd; /* for fast RNA access */
	struct FLUID_3D *fluid;
	struct Group *fluid_group;
	struct Group *eff_group; // UNUSED
	struct Group *coll_group; // collision objects group
	struct WTURBULENCE *wt; // WTURBULENCE object, if active
	struct GPUTexture *tex;
	struct GPUTexture *tex_wt;
	struct GPUTexture *tex_shadow;
	float *shadow;
	float p0[3]; /* start point of BB */
	float p1[3]; /* end point of BB */
	float dx; /* edge length of one cell */
	float omega; /* smoke color - from 0 to 1 */
	float temp; /* fluid temperature */
	float tempAmb; /* ambient temperature */
	float alpha;
	float beta;
	int res[3]; /* domain resolution */
	int amplify; /* wavelet amplification */
	int maxres; /* longest axis on the BB gets this resolution assigned */
	int flags; /* show up-res or low res, etc */
	int pad; 
	int viewsettings;
	short noise; /* noise type: wave, curl, anisotropic */
	short diss_percent; 
	int diss_speed;/* in frames */
	float strength;
	int res_wt[3];
	float dx_wt;
	int v3dnum;
	int cache_comp;
	int cache_high_comp;

	/* Smoke uses only one cache from now on (index [0]), but keeping the array for now for reading old files. */
	struct PointCache *point_cache[2];	/* definition is in DNA_object_force.h */
	struct ListBase ptcaches[2];
	struct EffectorWeights *effector_weights;
	int border_collisions;	/* How domain border collisions are handled */
	float time_scale;
	float vorticity;
	int pad2;
} SmokeDomainSettings;


/* inflow / outflow */

/* type */
#define MOD_SMOKE_FLOW_TYPE_OUTFLOW (1<<1)

/* flags */
#define MOD_SMOKE_FLOW_ABSOLUTE (1<<1) /*old style emission*/
#define MOD_SMOKE_FLOW_INITVELOCITY (1<<2) /* passes particles speed to
										 the smoke*/


typedef struct SmokeFlowSettings {
	struct SmokeModifierData *smd; /* for fast RNA access */
	struct ParticleSystem *psys;
	float density;
	float temp; /* delta temperature (temp - ambient temp) */
	float velocity[2]; /* UNUSED, velocity taken from particles */
	float vel_multi; // Multiplier for particle velocity
	float vgrp_heat_scale[2]; /* min and max scaling for vgroup_heat */
	short vgroup_flow; /* where inflow/outflow happens - red=1=action */
	short vgroup_density;
	short vgroup_heat;
	short type; /* inflow =0 or outflow = 1 */
	int flags; /* absolute emission etc*/
} SmokeFlowSettings;

/*
	struct BVHTreeFromMesh *bvh;
	float mat[4][4];
	float mat_old[4][4];
	*/

/* collision objects (filled with smoke) */
typedef struct SmokeCollSettings {
	struct SmokeModifierData *smd; /* for fast RNA access */
	struct BVHTree *bvhtree; /* bounding volume hierarchy for this cloth object */
	struct DerivedMesh *dm;
	float *points;
	float *points_old;
	float *vel;
	float mat[4][4];
	float mat_old[4][4];
	int numpoints;
	int numverts; // check if mesh changed
	short type; // static = 0, rigid = 1, dynamic = 2
	short pad;
	int pad2;
} SmokeCollSettings;

#endif
