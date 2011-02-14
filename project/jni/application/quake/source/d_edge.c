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
// d_edge.c

#include "quakedef.h"
#include "d_local.h"
#include "LogFloat.h"

static int	miplevel;

float		scale_for_mip;
int			screenwidth;
int			ubasestep, errorterm, erroradjustup, erroradjustdown;
int			vstartscan;

#ifdef USEFPM
fixedpoint_t	scale_for_mipFPM;
#endif

// FIXME: should go away
extern void			R_RotateBmodel (void);
extern void			R_TransformFrustum (void);

vec3_t		transformed_modelorg;

#ifdef USEFPM
extern void			R_RotateBmodelFPM (void);
extern void			R_TransformFrustumFPM (void);
vec3_FPM_t	transformed_modelorgFPM;
#endif
/*
==============
D_DrawPoly

==============
*/
void D_DrawPoly (void)
{
// this driver takes spans, not polygons
}

#ifdef USEFPM
void D_DrawPolyFPM (void)
{
// this driver takes spans, not polygons
}
#endif

/*
=============
D_MipLevelForScale
=============
*/
int D_MipLevelForScale (float scale)
{
	int		lmiplevel;

	if (scale >= d_scalemip[0] )
		lmiplevel = 0;
	else if (scale >= d_scalemip[1] )
		lmiplevel = 1;
	else if (scale >= d_scalemip[2] )
		lmiplevel = 2;
	else
		lmiplevel = 3;

	if (lmiplevel < d_minmip)
		lmiplevel = d_minmip;

	return lmiplevel;
}

#ifdef USEFPM
int D_MipLevelForScaleFPM (fixedpoint_t scale)
{
	int		lmiplevel;

	if (scale >= d_scalemipFPM[0] )
		lmiplevel = 0;
	else if (scale >= d_scalemipFPM[1] )
		lmiplevel = 1;
	else if (scale >= d_scalemipFPM[2] )
		lmiplevel = 2;
	else
		lmiplevel = 3;

	if (lmiplevel < d_minmip)
		lmiplevel = d_minmip;

	return lmiplevel;
}
#endif

/*
==============
D_DrawSolidSurface
==============
*/

// FIXME: clean this up

void D_DrawSolidSurface (surf_t *surf, int color)
{
	espan_t	*span;
	byte	*pdest;
	int		u, u2, pix;
	
	pix = (color<<24) | (color<<16) | (color<<8) | color;
	for (span=surf->spans ; span ; span=span->pnext)
	{
		pdest = (byte *)d_viewbuffer + screenwidth*span->v;
		u = span->u;
		u2 = span->u + span->count - 1;
		((byte *)pdest)[u] = pix;

		if (u2 - u < 8)
		{
			for (u++ ; u <= u2 ; u++)
				((byte *)pdest)[u] = pix;
		}
		else
		{
			for (u++ ; u & 3 ; u++)
				((byte *)pdest)[u] = pix;

			u2 -= 4;
			for ( ; u <= u2 ; u+=4)
				*(int *)((byte *)pdest + u) = pix;
			u2 += 4;
			for ( ; u <= u2 ; u++)
				((byte *)pdest)[u] = pix;
		}
	}
}

/*
==============
D_CalcGradients
==============
*/
#ifndef USE_PQ_OPT3
void D_CalcGradients (msurface_t *pface)
{
	mplane_t	*pplane;
	float		mipscale;
	vec3_t		p_temp1;
	vec3_t		p_saxis, p_taxis;
	float		t;

	pplane = pface->plane;

	mipscale = (float)(1.0 / (float)(1 << miplevel));

	//LogFloat((int)miplevel, "miplevel", -1, -1);
	//LogFloat(mipscale, "mipscale", -1, -1);

	TransformVector (pface->texinfo->vecs[0], p_saxis);
	TransformVector (pface->texinfo->vecs[1], p_taxis);

	//LogFloat(p_saxis[0], "p_saxis", 0, -1);
	//LogFloat(p_saxis[1], "p_saxis", 1, -1);
	//LogFloat(p_saxis[2], "p_saxis", 2, -1);
	//LogFloat(p_taxis[0], "p_taxis", 0, -1);
	//LogFloat(p_taxis[1], "p_taxis", 1, -1);
	//LogFloat(p_taxis[2], "p_taxis", 2, -1);

	t = xscaleinv * mipscale;
	d_sdivzstepu = p_saxis[0] * t;
	d_tdivzstepu = p_taxis[0] * t;

	//LogFloat(xscaleinv, "xscaleinv", -1, -1);
	//LogFloat(d_sdivzstepu, "d_sdivzstepu", -1, -1);
	//LogFloat(d_tdivzstepu, "d_tdivzstepu", -1, -1);

	t = yscaleinv * mipscale;
	d_sdivzstepv = -p_saxis[1] * t;
	d_tdivzstepv = -p_taxis[1] * t;

	//LogFloat(yscaleinv, "yscaleinv", -1, -1);
	//LogFloat(d_sdivzstepv, "d_sdivzstepv", -1, -1);
	//LogFloat(d_tdivzstepv, "d_tdivzstepv", -1, -1);

	d_sdivzorigin = p_saxis[2] * mipscale - xcenter * d_sdivzstepu -
			ycenter * d_sdivzstepv;
	d_tdivzorigin = p_taxis[2] * mipscale - xcenter * d_tdivzstepu -
			ycenter * d_tdivzstepv;

	//LogFloat(d_sdivzorigin, "d_sdivzorigin", -1, -1);
	//LogFloat(d_tdivzorigin, "d_tdivzorigin", -1, -1);

	VectorScale (transformed_modelorg, mipscale, p_temp1);

	//LogFloat(p_temp1[0], "p_temp1", 0, -1);
	//LogFloat(p_temp1[1], "p_temp1", 1, -1);
	//LogFloat(p_temp1[2], "p_temp1", 2, -1);
	
	t = 0x10000*mipscale;

	//LogFloat(t, "t", -1, -1);

	sadjust = (int)(((fixed16_t)(DotProduct (p_temp1, p_saxis) * 0x10000 + 0.5)) -
			((pface->texturemins[0] << 16) >> miplevel)
			+ pface->texinfo->vecs[0][3]*t);
	tadjust = (int)(((fixed16_t)(DotProduct (p_temp1, p_taxis) * 0x10000 + 0.5)) -
			((pface->texturemins[1] << 16) >> miplevel)
			+ pface->texinfo->vecs[1][3]*t);

	//LogFloat(sadjust, "sadjust", -1, -1);
	//LogFloat(tadjust, "tadjust", -1, -1);
//
// -1 (-epsilon) so we never wander off the edge of the texture
//
	bbextents = ((pface->extents[0] << 16) >> miplevel) - 1;
	bbextentt = ((pface->extents[1] << 16) >> miplevel) - 1;

	//LogFloat(bbextents, "bbextents", -1, -1);
	//LogFloat(bbextentt, "bbextentt", -1, -1);
}
#else
void D_CalcGradients (msurface_t *pface)
{
	mplane_t	*pplane;
	int			mipscale_fxp;
	int			p_temp1_fxp[3];
	int			p_saxis_fxp[3], p_taxis_fxp[3];
	int			t_fxp;

	pplane = pface->plane;

	//Dan: TODO: get miplevel ranges
	mipscale_fxp = (1 << miplevel);

	//(26.6->11.21) / 24.8 = 19.13
	p_saxis_fxp[0]=	(pface->texinfo->vecs_fxp[0][0]<<15)/vright_fxp[0]+
				(pface->texinfo->vecs_fxp[0][1]<<15)/vright_fxp[1]+
				(pface->texinfo->vecs_fxp[0][2]<<15)/vright_fxp[2];
	p_saxis_fxp[1]=	(pface->texinfo->vecs_fxp[0][0]<<15)/vup_fxp[0]+
				(pface->texinfo->vecs_fxp[0][1]<<15)/vup_fxp[1]+
				(pface->texinfo->vecs_fxp[0][2]<<15)/vup_fxp[2];
	p_saxis_fxp[2]=	(pface->texinfo->vecs_fxp[0][0]<<15)/vpn_fxp[0]+
				(pface->texinfo->vecs_fxp[0][1]<<15)/vpn_fxp[1]+
				(pface->texinfo->vecs_fxp[0][2]<<15)/vpn_fxp[2];

	p_taxis_fxp[0]=	(pface->texinfo->vecs_fxp[1][0]<<15)/vright_fxp[0]+
				(pface->texinfo->vecs_fxp[1][1]<<15)/vright_fxp[1]+
				(pface->texinfo->vecs_fxp[1][2]<<15)/vright_fxp[2];
	p_taxis_fxp[1]=	(pface->texinfo->vecs_fxp[1][0]<<15)/vup_fxp[0]+
				(pface->texinfo->vecs_fxp[1][1]<<15)/vup_fxp[1]+
				(pface->texinfo->vecs_fxp[1][2]<<15)/vup_fxp[2];
	p_taxis_fxp[2]=	(pface->texinfo->vecs_fxp[1][0]<<15)/vpn_fxp[0]+
				(pface->texinfo->vecs_fxp[1][1]<<15)/vpn_fxp[1]+
				(pface->texinfo->vecs_fxp[1][2]<<15)/vpn_fxp[2];

	//TransformVector (pface->texinfo->vecs[0], p_saxis);
	//TransformVector (pface->texinfo->vecs[1], p_taxis);

	//Dan: Maybe not enough precision here
	//19.13 -> 3.29 / 9.23 = 26.6
	d_sdivzstepu_fxp=(p_saxis_fxp[0]<<16)/xscale_fxp;
	//26.6 / 32.0 = 26.6
	d_sdivzstepu_fxp/=mipscale_fxp;

	//19.13 -> 3.29 / 9.23 = 26.6
	d_tdivzstepu_fxp=(p_taxis_fxp[0]<<16)/xscale_fxp;
	//26.6 / 32.0 = 26.6
	d_sdivzstepu_fxp/=mipscale_fxp;

	//19.13 -> 3.29 / 9.23 = 26.6
	d_sdivzstepv_fxp=(p_saxis_fxp[1]<<16)/yscale_fxp;
	//26.6 / 32.0 = 26.6
	d_sdivzstepv_fxp/=mipscale_fxp;

	//19.13 -> 3.29 / 9.23 = 26.6
	d_tdivzstepv_fxp=(p_taxis_fxp[1]<<16)/yscale_fxp;
	//26.6 / 32.0 = 26.6
	d_tdivzstepv_fxp/=mipscale_fxp;

	//Dan: left off here
	d_sdivzorigin = p_saxis_fxp[2] * mipscale_fxp - xcenter_fxp * d_sdivzstepu_fxp -
			ycenter_fxp * d_sdivzstepv_fxp;
	d_tdivzorigin = p_taxis_fxp[2] * mipscale_fxp - xcenter_fxp * d_tdivzstepu_fxp -
			ycenter_fxp * d_tdivzstepv_fxp;

	VectorScale (transformed_modelorg, mipscale_fxp, p_temp1_fxp);

	t_fxp = 0x10000*mipscale_fxp;
	sadjust = (int)(((fixed16_t)(DotProduct (p_temp1_fxp, p_saxis_fxp) * 0x10000 + 0.5)) -
			((pface->texturemins[0] << 16) >> miplevel)
			+ pface->texinfo->vecs_fxp[0][3]*t_fxp);
	tadjust = (int)(((fixed16_t)(DotProduct (p_temp1_fxp, p_taxis_fxp) * 0x10000 + 0.5)) -
			((pface->texturemins[1] << 16) >> miplevel)
			+ pface->texinfo->vecs_fxp[1][3]*t_fxp);

//
// -1 (-epsilon) so we never wander off the edge of the texture
//
	bbextents = ((pface->extents[0] << 16) >> miplevel) - 1;
	bbextentt = ((pface->extents[1] << 16) >> miplevel) - 1;
}
#endif
/*
#else
//JB: Optimization
extern int	fpxscale, fpyscale;
extern int fpxcenter, fpycenter;
static fpvec3 mo;
static float tmo;
void D_CalcGradients (msurface_t *pface)
{
	fpvec3		v0, v1, p_saxis, p_taxis, p_temp1;
	int			dps, dpt;

	v0[0] = (int)(16384.0f * pface->texinfo->vecs[0][0]); // 18.14
	v0[1] = (int)(16384.0f * pface->texinfo->vecs[0][1]);
	v0[2] = (int)(16384.0f * pface->texinfo->vecs[0][2]);
	v1[0] = (int)(16384.0f * pface->texinfo->vecs[1][0]);
	v1[1] = (int)(16384.0f * pface->texinfo->vecs[1][1]);
	v1[2] = (int)(16384.0f * pface->texinfo->vecs[1][2]);
	
	FPTransformVector (v0, p_saxis);
	FPTransformVector (v1, p_taxis);

	sdivzstepu = ((p_saxis[0] >> miplevel) / fpxscale) << 8;
	tdivzstepu = ((p_taxis[0] >> miplevel) / fpxscale) << 8;
	sdivzstepv = ((-p_saxis[1] >> miplevel) / fpyscale) << 8;
	tdivzstepv = ((-p_taxis[1] >> miplevel) / fpyscale) << 8;
	sdivzorigin = ((p_saxis[2] >> 2) >> miplevel) - fpxcenter * (sdivzstepu >> 6) - fpycenter * (sdivzstepv >> 6);
	tdivzorigin = ((p_taxis[2] >> 2) >> miplevel) - fpxcenter * (tdivzstepu >> 6) - fpycenter * (tdivzstepv >> 6);

	if (tmo != transformed_modelorg[0])
	{
		tmo = transformed_modelorg[0];
		mo[0] = (int)(256.0f * transformed_modelorg[0]);
		mo[1] = (int)(256.0f * transformed_modelorg[1]);
		mo[2] = (int)(256.0f * transformed_modelorg[2]);
	}
	p_temp1[0] = mo[0] >> miplevel;
	p_temp1[1] = mo[1] >> miplevel;
	p_temp1[2] = mo[2] >> miplevel;
	
	dps = ((p_temp1[0] * (p_saxis[0] >> 14)) >> 2 ) + 
		  ((p_temp1[1] * (p_saxis[1] >> 14)) >> 2 ) + 
		  ((p_temp1[2] * (p_saxis[2] >> 14)) >> 2 );
	dpt = ((p_temp1[0] * (p_taxis[0] >> 14)) >> 2 ) + 
		  ((p_temp1[1] * (p_taxis[1] >> 14)) >> 2 ) + 
		  ((p_temp1[2] * (p_taxis[2] >> 14)) >> 2 );
	
	sadjust = dps - (pface->texturemins[0] << (16 - miplevel)) + ((int)(pface->texinfo->vecs[0][3]) << (16 - miplevel));
	tadjust = dpt - (pface->texturemins[1] << (16 - miplevel)) + ((int)(pface->texinfo->vecs[1][3]) << (16 - miplevel));
	bbextents = ((pface->extents[0] << 16) >> miplevel) - 1;
	bbextentt = ((pface->extents[1] << 16) >> miplevel) - 1;
}
#endif
*/
/*
==============
D_DrawSurfaces
==============
*/
#ifndef USE_PQ_OPT

 
void D_DrawSurfaces (void)
{
	surf_t			*s;
	msurface_t		*pface;
	surfcache_t		*pcurrentcache;
	vec3_t			world_transformed_modelorg;
	vec3_t			local_modelorg;
			    

	currententity = &cl_entities[0];
	TransformVector (modelorg, transformed_modelorg);
	VectorCopy (transformed_modelorg, world_transformed_modelorg);

// TODO: could preset a lot of this at mode set time
	if (r_drawflat.value)
	{
		for (s = &surfaces[1] ; s<surface_p ; s++)
		{
			if (!s->spans)
				continue;

			//			r_drawnpolycount++;
			/*
#ifndef		USE_PQ_OPT3
			d_zistepu = s->d_zistepu;
			d_zistepv = s->d_zistepv;
			d_ziorigin = s->d_ziorigin;
#else
			d_zistepu_fxp = s->d_zistepu_fxp;
			d_zistepv_fxp = s->d_zistepv_fxp;
			d_ziorigin_fxp = s->d_ziorigin_fxp;
#endif
			*/
			    pface = s->data;
			    char *surfcol;

			    surfcol=(char *)((byte*) pface->texinfo->texture + sizeof(texture_t));
		//			    GpError(va("%d",(int)*((byte*) tx + tx->offsets[0]) & 0xFF),666);
			    D_DrawSolidSurface (s, surfcol[0] & 0xFF);
			    //D_DrawSolidSurface (s, (int)s->data & 0xFF);
			D_DrawZSpans (s->spans);
		}
	}
	else
	{
		for (s = &surfaces[1] ; s<surface_p ; s++)
		{
			if (!s->spans)
				continue;

			r_drawnpolycount++;

#ifndef		USE_PQ_OPT3
			d_zistepu = s->d_zistepu;
			d_zistepv = s->d_zistepv;
			d_ziorigin = s->d_ziorigin;
#else
			d_zistepu_fxp = s->d_zistepu_fxp;
			d_zistepv_fxp = s->d_zistepv_fxp;
			d_ziorigin_fxp = s->d_ziorigin_fxp;

			//d_zistepu = s->d_zistepu_fxp/4194304.0f;
			//d_zistepv = s->d_zistepv_fxp/4194304.0f;
			//d_ziorigin = s->d_ziorigin_fxp/4194304.0f;
#endif

			if (s->flags & SURF_DRAWSKY)
			{
			  extern cvar_t r_fastsky;
			  extern cvar_t r_skycolor;
			  
			  if (r_fastsky.value)
			    D_DrawSolidSurface (s, (int)r_skycolor.value & 0xFF);
			  else {
			    if (!r_skymade)
			      {
				R_MakeSky ();
			      }
			    
			    D_DrawSkyScans8 (s->spans);
			  }
			  D_DrawZSpans (s->spans);
			}
			else if (s->flags & SURF_DRAWBACKGROUND)
			{
			// set up a gradient for the background surface that places it
			// effectively at infinity distance from the viewpoint
				d_zistepu = 0;
				d_zistepv = 0;
				d_ziorigin = (float)-0.9;

				D_DrawSolidSurface (s, (int)r_clearcolor.value & 0xFF);
				D_DrawZSpans (s->spans);
			}
			else if (s->flags & SURF_DRAWTURB)
			{
			  extern cvar_t r_fastturb;
			  if (r_fastturb.value) {
			    texture_t *tx;
			    
			    pface = s->data;
			    tx = pface->texinfo->texture;
			    D_DrawSolidSurface (s, *((byte*) tx + tx->offsets[0] + ((tx->width * tx->height) >> 1)));
			    D_DrawZSpans(s->spans);
			    continue;
			  }
				pface = s->data;
				miplevel = 0;
				cacheblock = (pixel_t *)
						((byte *)pface->texinfo->texture +
						pface->texinfo->texture->offsets[0]);
				cachewidth = 64;

				if (s->insubmodel)
				{
				// FIXME: we don't want to do all this for every polygon!
				// TODO: store once at start of frame
					currententity = s->entity;	//FIXME: make this passed in to
												// R_RotateBmodel ()
					VectorSubtract (r_origin, currententity->origin,
							local_modelorg);
					TransformVector (local_modelorg, transformed_modelorg);

					R_RotateBmodel ();	// FIXME: don't mess with the frustum,
										// make entity passed in
				}

				D_CalcGradients (pface);
				Turbulent8 (s->spans);
				D_DrawZSpans (s->spans);

				if (s->insubmodel)
				{
				//
				// restore the old drawing state
				// FIXME: we don't want to do this every time!
				// TODO: speed up
				//
					currententity = &cl_entities[0];
					VectorCopy (world_transformed_modelorg,
								transformed_modelorg);
					VectorCopy (base_vpn, vpn);
					VectorCopy (base_vup, vup);
					VectorCopy (base_vright, vright);
					VectorCopy (base_modelorg, modelorg);
					R_TransformFrustum ();
				}
			}
			else
			{
				if (s->insubmodel)
				{
				// FIXME: we don't want to do all this for every polygon!
				// TODO: store once at start of frame
					currententity = s->entity;	//FIXME: make this passed in to
												// R_RotateBmodel ()
					VectorSubtract (r_origin, currententity->origin, local_modelorg);
					TransformVector (local_modelorg, transformed_modelorg);

					R_RotateBmodel ();	// FIXME: don't mess with the frustum,
										// make entity passed in
				}

				pface = s->data;
				miplevel = D_MipLevelForScale (s->nearzi * scale_for_mip
				* pface->texinfo->mipadjust);

			// FIXME: make this passed in to D_CacheSurface
				pcurrentcache = D_CacheSurface (pface, miplevel);

				cacheblock = (pixel_t *)pcurrentcache->data;
				cachewidth = pcurrentcache->width;

				D_CalcGradients (pface);

				(*d_drawspans) (s->spans);

				D_DrawZSpans (s->spans);

				if (s->insubmodel)
				{
				//
				// restore the old drawing state
				// FIXME: we don't want to do this every time!
				// TODO: speed up
				//
					currententity = &cl_entities[0];
					VectorCopy (world_transformed_modelorg,
								transformed_modelorg);
					VectorCopy (base_vpn, vpn);
					VectorCopy (base_vup, vup);
					VectorCopy (base_vright, vright);
					VectorCopy (base_modelorg, modelorg);
					R_TransformFrustum ();
				}
			}
		}
	}
}
#else
//JB: Optimization
void D_DrawSurfaces (void)
{
	surf_t			*s;
	msurface_t		*pface;
	surfcache_t		*pcurrentcache;
	vec3_t			world_transformed_modelorg;
	vec3_t			local_modelorg;

	currententity = &cl_entities[0];
	TransformVector (modelorg, transformed_modelorg);
	VectorCopy (transformed_modelorg, world_transformed_modelorg);

// TODO: could preset a lot of this at mode set time
	if (r_drawflat.value)
	{
		for (s = &surfaces[1] ; s<surface_p ; s++)
		{
			if (!s->spans)
				continue;

			zistepu = (int)(4194304.0f * s->d_zistepu);
			zistepv = (int)(4194304.0f * s->d_zistepv);
			ziorigin = (int)(4194304.0f * s->d_ziorigin);

			D_DrawSolidSurface (s, (int)s->data & 0xFF);
			D_DrawZSpans (s->spans);
		}
	}
	else
	{
		for (s = &surfaces[1] ; s<surface_p ; s++)
		{
			if (!s->spans)
				continue;

			r_drawnpolycount++;

			zistepu = (int)(4194304.0f * s->d_zistepu);
			zistepv = (int)(4194304.0f * s->d_zistepv);
			ziorigin = (int)(4194304.0f * s->d_ziorigin);

			if (s->flags & SURF_DRAWSKY)
			{
				if (!r_skymade)
				{
					R_MakeSky ();
				}

				D_DrawSkyScans8 (s->spans);
				D_DrawZSpans (s->spans);
			}
			else if (s->flags & SURF_DRAWBACKGROUND)
			{
			// set up a gradient for the background surface that places it
			// effectively at infinity distance from the viewpoint
				zistepu = 0;
				zistepv = 0;
				ziorigin = 3774873;

				D_DrawSolidSurface (s, (int)r_clearcolor.value & 0xFF);
				D_DrawZSpans (s->spans);
			}
			else if (s->flags & SURF_DRAWTURB)
			{
				pface = s->data;
				miplevel = 0;
				cacheblock = (pixel_t *)
						((byte *)pface->texinfo->texture +
						pface->texinfo->texture->offsets[0]);
				cachewidth = 64;

				if (s->insubmodel)
				{
				// FIXME: we don't want to do all this for every polygon!
				// TODO: store once at start of frame
					currententity = s->entity;	//FIXME: make this passed in to
												// R_RotateBmodel ()
					VectorSubtract (r_origin, currententity->origin,
							local_modelorg);
					TransformVector (local_modelorg, transformed_modelorg);

					R_RotateBmodel ();	// FIXME: don't mess with the frustum,
										// make entity passed in
				}

				D_CalcGradients (pface);
				Turbulent8 (s->spans);
				D_DrawZSpans (s->spans);

				if (s->insubmodel)
				{
				//
				// restore the old drawing state
				// FIXME: we don't want to do this every time!
				// TODO: speed up
				//
					currententity = &cl_entities[0];
					VectorCopy (world_transformed_modelorg,
								transformed_modelorg);
					VectorCopy (base_vpn, vpn);
					VectorCopy (base_vup, vup);
					VectorCopy (base_vright, vright);
					VectorCopy (base_modelorg, modelorg);
					R_TransformFrustum ();
				}
			}
			else
			{
				if (s->insubmodel)
				{
				// FIXME: we don't want to do all this for every polygon!
				// TODO: store once at start of frame
					currententity = s->entity;	//FIXME: make this passed in to
												// R_RotateBmodel ()
					VectorSubtract (r_origin, currententity->origin, local_modelorg);
					TransformVector (local_modelorg, transformed_modelorg);

					R_RotateBmodel ();	// FIXME: don't mess with the frustum,
										// make entity passed in
				}

				pface = s->data;
				miplevel = D_MipLevelForScale (s->nearzi * scale_for_mip
				* pface->texinfo->mipadjust);

			// FIXME: make this passed in to D_CacheSurface
				pcurrentcache = D_CacheSurface (pface, miplevel);

				cacheblock = (pixel_t *)pcurrentcache->data;
				cachewidth = pcurrentcache->width;

				D_CalcGradients (pface);

				D_DrawSpans8WithZ(s->spans);

				// D_DrawZSpans (s->spans);

				if (s->insubmodel)
				{
				//
				// restore the old drawing state
				// FIXME: we don't want to do this every time!
				// TODO: speed up
				//
					currententity = &cl_entities[0];
					VectorCopy (world_transformed_modelorg,
								transformed_modelorg);
					VectorCopy (base_vpn, vpn);
					VectorCopy (base_vup, vup);
					VectorCopy (base_vright, vright);
					VectorCopy (base_modelorg, modelorg);
					R_TransformFrustum ();
				}
			}
		}
	}
}
#endif
