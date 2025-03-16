/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*********************************************************************************
 ***          C O N F I D E N T I A L  ---  E A  P A C I F I C                 ***
 *********************************************************************************
 *                                                                               *
 *             Project Name : G                                                  *
 *                                                                               *
 *                 $Archive:: ww3d2/streak.h                                    $*
 *                                                                               *
 *                  $Author:: Mark Lorenzen                                     $*
 *                                                                               *
 *                 $Modtime:: 8/6/02 4:09p                                      $*
 *                                                                               *
 *                $Revision:: 1                                                 $*
 *                                                                               *
 *-------------------------------------------------------------------------------*
 * Functions:                                                                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef STREAK_H
#define STREAK_H

#include "rendobj.h"
#include "shader.h"
#include "simplevec.h"
#include "seglinerenderer.h"
#include "streakrender.h"

class TextureClass;

/*
** StreakLineClass -- a render object for rendering thick segmented lines.
**										essentially it is segmentedlineclass++
**										with added characteristics, such as varying thickness, opacity
**										color and precise texture tiling...
** The reason this does not descend from segmentedlineclass, is to streamline
** the upcoming merge with the LasVegas code (to keep our changes out of their code)
*/ 

class StreakLineClass : public RenderObjClass
{
	public:

		StreakLineClass(void);
		StreakLineClass(const StreakLineClass & src);
		StreakLineClass & operator = (const StreakLineClass &that);
//		virtual ~StreakLineClass(void);

		void					Reset_Line(void);

		/*
		** StreakLineClass interface:
		*/

		// These are segment points, and include the start and end point of the
		// entire line. Therefore there must be at least two.
		int		Get_Num_Points(void);


		// Set object-space location for a given point.
		// NOTE: If given position beyond end of point list, do nothing.
		void					Set_Point_Location(unsigned int point_idx, const Vector3 &location);

		// Get object-space location for a given point.
		void					Get_Point_Location(unsigned int point_idx, Vector3 &loc);

		// Modify the line by adding and removing points
		void					Add_Point(const Vector3 & location);
		void					Delete_Point(unsigned int point_idx);

		// Get/set global properties (which affect all line segments)
		TextureClass *		Get_Texture(void);
		ShaderClass			Get_Shader(void);

		float					Get_Width(void);
		void					Get_Color(Vector3 &color);
		float					Get_Opacity(void);
		float					Get_Noise_Amplitude(void);
		float					Get_Merge_Abort_Factor(void);
		unsigned int		Get_Subdivision_Levels(void);
		SegLineRendererClass::TextureMapMode		Get_Texture_Mapping_Mode(void);
		float					Get_Texture_Tile_Factor(void);
		Vector2				Get_UV_Offset_Rate(void);
		int					Is_Merge_Intersections(void);
		int					Is_Freeze_Random(void);
		int					Is_Sorting_Disabled(void);
		int					Are_End_Caps_Enabled(void);

		void					Set_Texture(TextureClass *texture);
		void					Set_Shader(ShaderClass shader);
		void					Set_Width(float width);
		void					Set_Color(const Vector3 &color);
		void					Set_Opacity(float opacity);
		void					Set_Noise_Amplitude(float amplitude);
		void					Set_Merge_Abort_Factor(float factor);
		void					Set_Subdivision_Levels(unsigned int levels);
		void					Set_Texture_Mapping_Mode(SegLineRendererClass::TextureMapMode mode);
		// WARNING! Do NOT set the tile factor to be too high (should be less than 8) or negative
		//performance impact will result!
		void					Set_Texture_Tile_Factor(float factor);
		void					Set_UV_Offset_Rate(const Vector2 &rate);
		void					Set_Merge_Intersections(int onoff);
		void					Set_Freeze_Random(int onoff);
		void					Set_Disable_Sorting(int onoff);
		void					Set_End_Caps(int onoff);

		/////////////////////////////////////////////////////////////////////////////
		// Render Object Interface - Cloning and Identification
		/////////////////////////////////////////////////////////////////////////////
		virtual RenderObjClass *	Clone(void) const;		
		virtual int						Class_ID(void)	const { return CLASSID_SEGLINE; }
		virtual int						Get_Num_Polys(void) const;

		/////////////////////////////////////////////////////////////////////////////
		// Render Object Interface - Rendering
		/////////////////////////////////////////////////////////////////////////////
		virtual void					Render( RenderInfoClass & rinfo );

		/////////////////////////////////////////////////////////////////////////////
		// Render Object Interface - Bounding Volumes
		/////////////////////////////////////////////////////////////////////////////
		virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
		virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

		/////////////////////////////////////////////////////////////////////////////
		// Render Object Interface - Predictive LOD
		/////////////////////////////////////////////////////////////////////////////
		virtual void					Prepare_LOD(CameraClass &camera);
		virtual void					Increment_LOD(void);
		virtual void					Decrement_LOD(void);
		virtual float					Get_Cost(void) const;
		virtual float					Get_Value(void) const;
		virtual float					Get_Post_Increment_Value(void) const;
		virtual void					Set_LOD_Level(int lod);
		virtual int						Get_LOD_Level(void) const;
		virtual int						Get_LOD_Count(void) const;

		/////////////////////////////////////////////////////////////////////////////
		// Render Object Interface - Attributes, Options, Properties, etc
		/////////////////////////////////////////////////////////////////////////////
//		virtual void					Set_Texture_Reduction_Factor(float trf);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Render Object Interface - Collision Detection
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual bool					Cast_Ray(RayCollisionTestClass & raytest);

		void	Set_LocsWidthsColors( unsigned int num_points, 
																Vector3 *locs, 
																float *widths = NULL, 
																Vector4 *colors = NULL, 
																unsigned int *personalities = NULL);


	protected:

		void	Set_Locs(unsigned int num_points, Vector3 *locs);
		void	Set_Widths(unsigned int num_points, float *widths);
		void	Set_Colors(unsigned int num_points, Vector4 *colors);

		void 								Render_Seg_Line(RenderInfoClass & rinfo);
		void 								Render_Streak_Line(RenderInfoClass & rinfo);

	private:

		// Subdivision properties
		unsigned int					MaxSubdivisionLevels;


		// per-particle seeds
		unsigned int					*Personalities;

		// Normalized screen area - used for LOD purposes
		float								NormalizedScreenArea;

	// Per-point location array
	//ShareBufferClass<Vector3> *PointLocations;	
	SimpleDynVecClass<Vector3>	PointLocations;   // World/cameraspace point locs
	SimpleDynVecClass<Vector4>	PointColors;   // RGBA
	SimpleDynVecClass<float>		PointWidths;   // float line thickness

	



	// LineRenderer, contains most of the line settings.
	SegLineRendererClass		LineRenderer;
	StreakRendererClass		StreakRenderer;//special, per-point alpha/color/size
};









#endif SEGLINE_H
