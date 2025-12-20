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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/light.h                                $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/05/01 6:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "rendobj.h"
#include "w3derr.h"

class ChunkLoadClass;
class ChunkSaveClass;


/**
** LightClass
** This "render object" is a light source.
*/
class LightClass : public RenderObjClass
{
public:

	enum LightType
	{
		POINT = 0,
		DIRECTIONAL,
		SPOT
	};

	enum FlagsType
	{
		NEAR_ATTENUATION				= 0,
		FAR_ATTENUATION,
	};

	LightClass(LightType type = POINT);
	LightClass(const LightClass & src);
	LightClass & operator = (const LightClass &);
	virtual ~LightClass(void);
	RenderObjClass *		Clone(void) const;
	virtual int				Class_ID(void) const											{ return CLASSID_LIGHT; }

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	// Lights do not "Render" but they are vertex processors.
	/////////////////////////////////////////////////////////////////////////////
	virtual void			Render(RenderInfoClass & rinfo)							{ }
	virtual bool			Is_Vertex_Processor(void)									{ return true; }

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	// Lights register themselves with the scene as VertexProcessors.
	/////////////////////////////////////////////////////////////////////////////
	virtual void			Notify_Added(SceneClass * scene);
	virtual void			Notify_Removed(SceneClass * scene);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	// Bounding volume of a light extends to its attenuation radius
	/////////////////////////////////////////////////////////////////////////////
	virtual void			Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
   virtual void			Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

	/////////////////////////////////////////////////////////////////////////////
	// LightClass Interface
	/////////////////////////////////////////////////////////////////////////////
	LightType				Get_Type() {return (Type);}

	void						Set_Intensity(float inten) { Intensity = inten; }
	float						Get_Intensity(void) const { return Intensity; }

	void						Set_Ambient(const Vector3 & color) { Ambient = color; }
	void						Get_Ambient(Vector3 * set_c) const { if (set_c) { *set_c = Ambient; } }

	void						Set_Diffuse(const Vector3 & color) { Diffuse = color; }
	void						Get_Diffuse(Vector3 * set_c) const { if (set_c) { *set_c = Diffuse; } }

	void						Set_Specular(const Vector3 & color) { Specular = color; }
	void						Get_Specular(Vector3 * set_c) const { if (set_c) { *set_c = Specular; } }

	void						Set_Far_Attenuation_Range(double fStart, double fEnd)				{ FarAttenStart = fStart; FarAttenEnd = fEnd; }
	void						Get_Far_Attenuation_Range(double& fStart, double& fEnd) const	{ fStart = FarAttenStart; fEnd = FarAttenEnd; }
	void						Get_Far_Attenuation_Range(float & fStart, float & fEnd) const	{ fStart = FarAttenStart; fEnd = FarAttenEnd; }
	void						Set_Near_Attenuation_Range(double nStart, double nEnd)			{ NearAttenStart = nStart; NearAttenEnd = nEnd; }
	void						Get_Near_Attenuation_Range(double& nStart, double& nEnd)	const	{ nStart = NearAttenStart; nEnd = NearAttenEnd; }
	float						Get_Attenuation_Range(void) const										{ return FarAttenEnd; }

	/////////////////////////////////////////////////////////////////////////////
	// Control over the light flags
	/////////////////////////////////////////////////////////////////////////////
	void						Set_Flag(FlagsType flag,bool onoff) { if (onoff) { Flags |= flag; } else { Flags &= ~flag; } }
	int						Get_Flag(FlagsType flag) const		{ return ((Flags & flag) != 0); }
	void						Enable_Shadows(bool onoff)				{ CastShadows = onoff; }
	bool						Are_Shadows_Enabled(void) const		{ return CastShadows; }
	LightType				Get_Type (void) const					{ return Type; }

	/////////////////////////////////////////////////////////////////////////////
	// Spotlight controls:
	/////////////////////////////////////////////////////////////////////////////
	void						Set_Spot_Angle(float a)							{ SpotAngle = a; SpotAngleCos = WWMath::Fast_Cos(a); }
	float						Get_Spot_Angle(void)	const						{ return SpotAngle; }
	float						Get_Spot_Angle_Cos(void) const				{ return SpotAngleCos; }
	void						Set_Spot_Direction(const Vector3 & dir)	{ SpotDirection = dir; }
	void						Get_Spot_Direction(Vector3 & dir) const	{ dir = SpotDirection; }
	void						Set_Spot_Exponent(float k)						{ SpotExponent = k; }
	float						Get_Spot_Exponent(void) const					{ return SpotExponent; }

	/////////////////////////////////////////////////////////////////////////////
	// Save/Load
	/////////////////////////////////////////////////////////////////////////////
	WW3DErrorType			Load_W3D(ChunkLoadClass & cload);
	WW3DErrorType			Save_W3D(ChunkSaveClass & csave);

	/////////////////////////////////////////////////////////////////////////////
	// Persistant object save-load interface
	/////////////////////////////////////////////////////////////////////////////
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool								Save (ChunkSaveClass &csave);
	virtual bool								Load (ChunkLoadClass &cload);
	//bool isDonut(void) {return Donut; };
	//void setDonut(bool donut) { Donut = donut; };

protected:

	LightType				Type;
	unsigned int			Flags;
	bool						CastShadows;

	float						Intensity;
	Vector3					Ambient;
	Vector3					Diffuse;
	Vector3					Specular;

	float						NearAttenStart;
	float						NearAttenEnd;
	float						FarAttenStart;
	float						FarAttenEnd;

	float						SpotAngle;
	float						SpotAngleCos;
	float						SpotExponent;
	Vector3					SpotDirection;
	//bool						Donut; ///does this light only apply at edges
};
