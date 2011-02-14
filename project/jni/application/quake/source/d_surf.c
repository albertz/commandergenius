/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// d_surf.c: rasterization driver surface heap manager

#include "quakedef.h"
#include "d_local.h"
#include "r_local.h"

float			surfscale;
qboolean        r_cache_thrash;         // set if surface cache is thrashing

int					sc_size;
surfcache_t			*sc_rover, *sc_base;

#ifdef USEFPM
fixedpoint_t	surfscaleFPM;
surfcache_FPM_t		*sc_roverFPM, *sc_baseFPM;
#endif

#define GUARDSIZE       4


int     D_SurfaceCacheForRes (int width, int height)
{
	int             size, pix;

	if (COM_CheckParm ("-surfcachesize"))
	{
		size = Q_atoi(com_argv[COM_CheckParm("-surfcachesize")+1]) * 1024;
		return size;
	}
	
	size = SURFCACHE_SIZE_AT_320X200;

	pix = width*height;
	if (pix > 64000)
		size += (pix-64000)*3;
		

	return size;
}

void D_CheckCacheGuard (void)
{
	byte    *s;
	int             i;

	s = (byte *)sc_base + sc_size;
	for (i=0 ; i<GUARDSIZE ; i++)
		if (s[i] != (byte)i)
			Sys_Error ("D_CheckCacheGuard: failed");
}

#ifdef USEFPM
void D_CheckCacheGuardFPM (void)
{
	byte    *s;
	int             i;

	s = (byte *)sc_baseFPM + sc_size;
	for (i=0 ; i<GUARDSIZE ; i++)
		if (s[i] != (byte)i)
			Sys_Error ("D_CheckCacheGuard: failed");
}
#endif

void D_ClearCacheGuard (void)
{
	byte    *s;
	int             i;
	
	s = (byte *)sc_base + sc_size;
	for (i=0 ; i<GUARDSIZE ; i++)
		s[i] = (byte)i;
}

#ifdef USEFPM
void D_ClearCacheGuardFPM (void)
{
	byte    *s;
	int             i;
	
	s = (byte *)sc_baseFPM + sc_size;
	for (i=0 ; i<GUARDSIZE ; i++)
		s[i] = (byte)i;
}
#endif

/*
================
D_InitCaches

================
*/
void D_InitCaches (void *buffer, int size)
{

	if (!msg_suppress_1)
		Con_Printf ("%ik surface cache\n", size/1024);

	sc_size = size - GUARDSIZE;
	sc_base = (surfcache_t *)buffer;
	sc_rover = sc_base;
	
	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;
	
	D_ClearCacheGuard ();
}

#ifdef USEFPM
void D_InitCachesFPM (void *buffer, int size)
{

	if (!msg_suppress_1)
		Con_Printf ("%ik surface cache\n", size/1024);

	sc_size = size - GUARDSIZE;
	sc_baseFPM = (surfcache_FPM_t *)buffer;
	sc_roverFPM = sc_baseFPM;
	
	sc_baseFPM->next = NULL;
	sc_baseFPM->owner = NULL;
	sc_baseFPM->size = sc_size;
	
	D_ClearCacheGuardFPM ();
}
#endif

/*
==================
D_FlushCaches
==================
*/
void D_FlushCaches (void)
{
	surfcache_t     *c;
	
	if (!sc_base)
		return;

	for (c = sc_base ; c ; c = c->next)
	{
		if (c->owner)
			*c->owner = NULL;
	}
	
	sc_rover = sc_base;
	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;
}

#ifdef USEFPM
void D_FlushCachesFPM (void)
{
	surfcache_FPM_t     *c;
	
	if (!sc_baseFPM)
		return;

	for (c = sc_baseFPM ; c ; c = c->next)
	{
		if (c->owner)
			*c->owner = NULL;
	}
	
	sc_roverFPM = sc_baseFPM;
	sc_baseFPM->next = NULL;
	sc_baseFPM->owner = NULL;
	sc_baseFPM->size = sc_size;
}
#endif

/*
=================
D_SCAlloc
=================
*/
surfcache_t     *D_SCAlloc (int width, int size)
{
	surfcache_t             *new;
	qboolean                wrapped_this_time;

	if ((width < 0) || (width > 256))
		Sys_Error ("D_SCAlloc: bad cache width %d\n", width);

	if ((size <= 0) || (size > 0x10000))
		Sys_Error ("D_SCAlloc: bad cache size %d\n", size);
	
	size = (int)&((surfcache_t *)0)->data[size];
	size = (size + 3) & ~3;
	if (size > sc_size)
		Sys_Error ("D_SCAlloc: %i > cache size",size);

// if there is not size bytes after the rover, reset to the start
	wrapped_this_time = false;

	if ( !sc_rover || (byte *)sc_rover - (byte *)sc_base > sc_size - size)
	{
		if (sc_rover)
		{
			wrapped_this_time = true;
		}
		sc_rover = sc_base;
	}
		
// colect and free surfcache_t blocks until the rover block is large enough
	new = sc_rover;
	if (sc_rover->owner)
		*sc_rover->owner = NULL;
	
	while (new->size < size)
	{
	// free another
		sc_rover = sc_rover->next;
		if (!sc_rover)
			Sys_Error ("D_SCAlloc: hit the end of memory");
		if (sc_rover->owner)
			*sc_rover->owner = NULL;
			
		new->size += sc_rover->size;
		new->next = sc_rover->next;
	}

// create a fragment out of any leftovers
	if (new->size - size > 256)
	{
		sc_rover = (surfcache_t *)( (byte *)new + size);
		sc_rover->size = new->size - size;
		sc_rover->next = new->next;
		sc_rover->width = 0;
		sc_rover->owner = NULL;
		new->next = sc_rover;
		new->size = size;
	}
	else
		sc_rover = new->next;
	
	new->width = width;
// DEBUG
	if (width > 0)
		new->height = (size - sizeof(*new) + sizeof(new->data)) / width;

	new->owner = NULL;              // should be set properly after return

	if (d_roverwrapped)
	{
		if (wrapped_this_time || (sc_rover >= d_initial_rover))
			r_cache_thrash = true;
	}
	else if (wrapped_this_time)
	{       
		d_roverwrapped = true;
	}

D_CheckCacheGuard ();   // DEBUG
	return new;
}

#ifdef USEFPM
surfcache_FPM_t     *D_SCAllocFPM (int width, int size)
{
	surfcache_FPM_t             *new;
	qboolean                wrapped_this_time;

	if ((width < 0) || (width > 256))
		Sys_Error ("D_SCAlloc: bad cache width %d\n", width);

	if ((size <= 0) || (size > 0x10000))
		Sys_Error ("D_SCAlloc: bad cache size %d\n", size);
	
	size = (int)&((surfcache_FPM_t *)0)->data[size];
	size = (size + 3) & ~3;
	if (size > sc_size)
		Sys_Error ("D_SCAlloc: %i > cache size",size);

// if there is not size bytes after the rover, reset to the start
	wrapped_this_time = false;

	if ( !sc_roverFPM || (byte *)sc_roverFPM - (byte *)sc_baseFPM > sc_size - size)
	{
		if (sc_roverFPM)
		{
			wrapped_this_time = true;
		}
		sc_roverFPM = sc_baseFPM;
	}
		
// colect and free surfcache_t blocks until the rover block is large enough
	new = sc_roverFPM;
	if (sc_roverFPM->owner)
		*sc_roverFPM->owner = NULL;
	
	while (new->size < size)
	{
	// free another
		sc_roverFPM = sc_roverFPM->next;
		if (!sc_roverFPM)
			Sys_Error ("D_SCAlloc: hit the end of memory");
		if (sc_roverFPM->owner)
			*sc_roverFPM->owner = NULL;
			
		new->size += sc_roverFPM->size;
		new->next = sc_roverFPM->next;
	}

// create a fragment out of any leftovers
	if (new->size - size > 256)
	{
		sc_roverFPM = (surfcache_FPM_t *)( (byte *)new + size);
		sc_roverFPM->size = new->size - size;
		sc_roverFPM->next = new->next;
		sc_roverFPM->width = 0;
		sc_roverFPM->owner = NULL;
		new->next = sc_roverFPM;
		new->size = size;
	}
	else
		sc_roverFPM = new->next;
	
	new->width = width;
// DEBUG
	if (width > 0)
		new->height = (size - sizeof(*new) + sizeof(new->data)) / width;

	new->owner = NULL;              // should be set properly after return

	if (d_roverwrapped)
	{
		if (wrapped_this_time || (sc_roverFPM >= d_initial_roverFPM))
			r_cache_thrash = true;
	}
	else if (wrapped_this_time)
	{       
		d_roverwrapped = true;
	}

	D_CheckCacheGuardFPM ();   // DEBUG
	return new;
}
#endif

/*
=================
D_SCDump
=================
*/
void D_SCDump (void)
{
	surfcache_t             *test;

	for (test = sc_base ; test ; test = test->next)
	{
		if (test == sc_rover)
			Sys_Printf ("ROVER:\n");
		printf ("%p : %i bytes     %i width\n",test, test->size, test->width);
	}
}

#ifdef USEFPM
void D_SCDumpFPM (void)
{
	surfcache_FPM_t	*test;

	for (test = sc_baseFPM ; test ; test = test->next)
	{
		if (test == sc_roverFPM)
			Sys_Printf ("ROVER:\n");
		printf ("%p : %i bytes     %i width\n",test, test->size, test->width);
	}
}
#endif

//=============================================================================

// if the num is not a power of 2, assume it will not repeat

int     MaskForNum (int num)
{
	if (num==128)
		return 127;
	if (num==64)
		return 63;
	if (num==32)
		return 31;
	if (num==16)
		return 15;
	return 255;
}

int D_log2 (int num)
{
	int     c;
	
	c = 0;
	
	while (num>>=1)
		c++;
	return c;
}

//=============================================================================

/*
================
D_CacheSurface
================
*/
surfcache_t *D_CacheSurface (msurface_t *surface, int miplevel)
{
	surfcache_t     *cache;

//
// if the surface is animating or flashing, flush the cache
//
	r_drawsurf.texture = R_TextureAnimation (surface->texinfo->texture);
	r_drawsurf.lightadj[0] = d_lightstylevalue[surface->styles[0]];
	r_drawsurf.lightadj[1] = d_lightstylevalue[surface->styles[1]];
	r_drawsurf.lightadj[2] = d_lightstylevalue[surface->styles[2]];
	r_drawsurf.lightadj[3] = d_lightstylevalue[surface->styles[3]];
	
//
// see if the cache holds apropriate data
//
	cache = surface->cachespots[miplevel];

	if (cache && !cache->dlight && surface->dlightframe != r_framecount
			&& cache->texture == r_drawsurf.texture
			&& cache->lightadj[0] == r_drawsurf.lightadj[0]
			&& cache->lightadj[1] == r_drawsurf.lightadj[1]
			&& cache->lightadj[2] == r_drawsurf.lightadj[2]
			&& cache->lightadj[3] == r_drawsurf.lightadj[3] )
		return cache;

//
// determine shape of surface
//
	surfscale = (float) 1.0 / (1<<miplevel);
	r_drawsurf.surfmip = miplevel;
	r_drawsurf.surfwidth = surface->extents[0] >> miplevel;
	r_drawsurf.rowbytes = r_drawsurf.surfwidth;
	r_drawsurf.surfheight = surface->extents[1] >> miplevel;
	
//
// allocate memory if needed
//
	if (!cache)     // if a texture just animated, don't reallocate it
	{
		cache = D_SCAlloc (r_drawsurf.surfwidth,
						   r_drawsurf.surfwidth * r_drawsurf.surfheight);
		surface->cachespots[miplevel] = cache;
		cache->owner = &surface->cachespots[miplevel];
		cache->mipscale = surfscale;
	}
	
	if (surface->dlightframe == r_framecount)
		cache->dlight = 1;
	else
		cache->dlight = 0;

	r_drawsurf.surfdat = (pixel_t *)cache->data;
	
	cache->texture = r_drawsurf.texture;
	cache->lightadj[0] = r_drawsurf.lightadj[0];
	cache->lightadj[1] = r_drawsurf.lightadj[1];
	cache->lightadj[2] = r_drawsurf.lightadj[2];
	cache->lightadj[3] = r_drawsurf.lightadj[3];

//
// draw and light the surface texture
//
	r_drawsurf.surf = surface;

	c_surf++;
	R_DrawSurface ();

	return surface->cachespots[miplevel];
}

#ifdef USEFPM
surfcache_FPM_t *D_CacheSurfaceFPM (msurface_FPM_t *surface, int miplevel)
{
	surfcache_FPM_t     *cache;

//
// if the surface is animating or flashing, flush the cache
//
	r_drawsurfFPM.texture = R_TextureAnimationFPM (surface->texinfo->texture);
	r_drawsurfFPM.lightadj[0] = d_lightstylevalue[surface->styles[0]];
	r_drawsurfFPM.lightadj[1] = d_lightstylevalue[surface->styles[1]];
	r_drawsurfFPM.lightadj[2] = d_lightstylevalue[surface->styles[2]];
	r_drawsurfFPM.lightadj[3] = d_lightstylevalue[surface->styles[3]];
	
//
// see if the cache holds apropriate data
//
	cache = surface->cachespots[miplevel];

	if (cache && !cache->dlight && surface->dlightframe != r_framecount
			&& cache->texture == r_drawsurfFPM.texture
			&& cache->lightadj[0] == r_drawsurfFPM.lightadj[0]
			&& cache->lightadj[1] == r_drawsurfFPM.lightadj[1]
			&& cache->lightadj[2] == r_drawsurfFPM.lightadj[2]
			&& cache->lightadj[3] == r_drawsurfFPM.lightadj[3] )
		return cache;

//
// determine shape of surface
//
	surfscaleFPM = FPM_INV(FPM_FROMLONG(1<<miplevel));
	r_drawsurfFPM.surfmip = miplevel;
	r_drawsurfFPM.surfwidth = surface->extents[0] >> miplevel;
	r_drawsurfFPM.rowbytes = r_drawsurfFPM.surfwidth;
	r_drawsurfFPM.surfheight = surface->extents[1] >> miplevel;
	
//
// allocate memory if needed
//
	if (!cache)     // if a texture just animated, don't reallocate it
	{
		cache = (surfcache_FPM_t *)D_SCAllocFPM (r_drawsurfFPM.surfwidth,
						   r_drawsurfFPM.surfwidth * r_drawsurfFPM.surfheight);
		surface->cachespots[miplevel] = cache;
		cache->owner = &surface->cachespots[miplevel];
		cache->mipscale = surfscaleFPM;
	}
	
	if (surface->dlightframe == r_framecount)
		cache->dlight = 1;
	else
		cache->dlight = 0;

	r_drawsurfFPM.surfdat = (pixel_t *)cache->data;
	
	cache->texture = r_drawsurfFPM.texture;
	cache->lightadj[0] = r_drawsurfFPM.lightadj[0];
	cache->lightadj[1] = r_drawsurfFPM.lightadj[1];
	cache->lightadj[2] = r_drawsurfFPM.lightadj[2];
	cache->lightadj[3] = r_drawsurfFPM.lightadj[3];

//
// draw and light the surface texture
//
	r_drawsurfFPM.surf = surface;

	c_surf++;
	R_DrawSurfaceFPM ();

	return surface->cachespots[miplevel];
}
#endif

