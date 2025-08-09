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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/matrixmapper.h                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef MATRIXMAPPER_H
#define MATRIXMAPPER_H

#include "always.h"
#include "bittype.h"
#include "matrix4.h"
#include "mapper.h"

// Modified to use DX 8 texture matrices
// Hector Yee 1/29/01

/**
** MatrixMapperClass.  Does the chore of computing the u-v coorinates for
** a projected texture.  Note that this VP must be "baby-sat" by something
** external to ensure that its ViewToTexture transform is up-to-date.  I
** use it in the TexProjectClass to implement projected textures.
**
** NOTE: for projected textures, the equation for computing a valid ViewToTexture
** transform is as follows (assuming my usual column vectors, etc):
**                                                        -1
** ViewToTexture = Projection * Mwrld-shadow * Mwrld-camera
*/
class MatrixMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(MatrixMapperClass)
public:

	enum {
		INVERT_DEPTH_GRADIENT	= 0x00000001,
	};

	enum MappingType {
		ORTHO_PROJECTION			= 0,
		PERSPECTIVE_PROJECTION,
		DEPTH_GRADIENT,
		NORMAL_GRADIENT
	};

	MatrixMapperClass(int stage);

	/*
	** Interface
	*/
	void						Set_Flag(uint32 flag,bool onoff);
	bool						Get_Flag(uint32 flag) const;

	void						Set_Type(MappingType type);
	MappingType				Get_Type(void);

	void						Set_Texture_Transform(const Matrix3D & view_to_texture,float texsize);
	void						Set_Texture_Transform(const Matrix4x4 & view_to_texture,float texsize);
	const Matrix4x4 &		Get_Texture_Transform(void) const;

	void						Set_Gradient_U_Coord(float coord) { GradientUCoord = coord; }
	float						Get_Gradient_U_Coord(void) { return GradientUCoord; }

	void						Compute_Texture_Coordinate(const Vector3 & point,Vector3 * set_stq);

	TextureMapperClass*	Clone(void) const { 	WWASSERT(0);	return NULL; }

	virtual void			Apply(int uv_array_index);
	virtual void			Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

protected:

	void						Update_View_To_Pixel_Transform(float texsize);

	uint32					Flags;
	MappingType				Type;
	Matrix4x4				ViewToTexture;
	Matrix4x4					ViewToPixel;
	Vector3					ViewSpaceProjectionNormal;
	float						GradientUCoord;
};

/*
** CompositeMatrixMapperClass - this is a matrix mapper which contains a pointer to another mapper
** inside it. When applied, it gets the texture matrix from the internal mapper and composites
** it with it's own matrix, then applies that. It sets the texture source to camera space
** position. The idea is to use some transformation of the camera space position (like a planar
** projection) as the 'input coordinates' to some other mapper like a linear offset mapper
** which usually uses actual texture coordinates as input. If the internal mapper is NULL, it
** simply applies it's own matrix.
*/
class CompositeMatrixMapperClass : public MatrixMapperClass
{
public:

	CompositeMatrixMapperClass(TextureMapperClass *internal_mapper, unsigned int stage);
	CompositeMatrixMapperClass(const CompositeMatrixMapperClass & src);
	virtual ~CompositeMatrixMapperClass(void);

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( CompositeMatrixMapperClass, (*this)); }

	virtual void Apply(int uv_array_index);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

protected:

	TextureMapperClass *InternalMapper;
};

inline void MatrixMapperClass::Set_Flag(uint32 flag,bool onoff)
{
	if (onoff) {
		Flags |= flag;
	} else {
		Flags &= ~flag;
	}
}

inline bool MatrixMapperClass::Get_Flag(uint32 flag) const
{
	return (Flags & flag) == flag;
}

inline void MatrixMapperClass::Set_Type(MappingType type)
{
	Type = type;
}

inline MatrixMapperClass::MappingType MatrixMapperClass::Get_Type(void)
{
	return Type;
}

inline const Matrix4x4 & MatrixMapperClass::Get_Texture_Transform(void) const
{
	return ViewToTexture;
}

#endif
