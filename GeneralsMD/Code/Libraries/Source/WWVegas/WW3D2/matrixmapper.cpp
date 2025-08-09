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
 *                     $Archive:: /Commando/Code/ww3d2/matrixmapper.cpp                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MatrixMapperClass::MatrixMapperClass -- Constructor                                       *
 *   MatrixMapperClass::Set_Texture_Transform -- Sets the viewspace-to-texturespace transform  *
 *   MatrixMapperClass::Update_View_To_Pixel_Transform -- recomputes ViewToPixel               *
 *   MatrixMapperClass::Compute_Texture_Coordinate -- compute a single texture coord           *
 *   MatrixMapperClass::Apply -- Apply texture mapper to render states and texture stage states*
 *   MatrixMapperClass::Calculate_Texture_Matrix -- Calculate texture matrix                   *
 *   CompositeMatrixMapperClass::CompositeMatrixMapperClass -- Constructor                     *
 *   CompositeMatrixMapperClass::CompositeMatrixMapperClass -- Copy Constructor                *
 *   CompositeMatrixMapperClass::~CompositeMatrixMapperClass -- Destructor                     *
 *   CompositeMatrixMapperClass::~CompositeMatrixMapperClass -- Destructor                     *
 *   CompositeMatrixMapperClass::Apply -- Apply texture mapper to render states and ts states  *
 *   CompositeMatrixMapperClass::Calculate_Texture_Matrix -- Calculate texture matrix          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "matrixmapper.h"
#include "dx8wrapper.h"


/***********************************************************************************************
 * MatrixMapperClass::MatrixMapperClass -- Constructor                                         *
 *                                                                                             *
 *    Initializes the member variables to defaults                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/13/99   gth : created                                                                  *
 *=============================================================================================*/
MatrixMapperClass::MatrixMapperClass(int stage) :
	TextureMapperClass(stage),
	Flags(0),
	Type(ORTHO_PROJECTION),
	ViewToTexture(true),
	ViewToPixel(true),
	ViewSpaceProjectionNormal(0.0f, 0.0f, 0.0f),
	GradientUCoord(0.5f)
{
}

/***********************************************************************************************
 * MatrixMapperClass::Set_Texture_Transform -- Sets the viewspace-to-texturespace transform    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/20/2001  gth : created                                                                  *
 *=============================================================================================*/
void MatrixMapperClass::Set_Texture_Transform(const Matrix3D & view_to_texture,float texsize)
{
	ViewToTexture = Matrix4x4(view_to_texture);
	Update_View_To_Pixel_Transform(texsize);
}

/***********************************************************************************************
 * MatrixMapperClass::Set_Texture_Transform -- Sets the viewspace-to-texturespace transform    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/13/99   gth : Created.                                                                 *
 *=============================================================================================*/
void	MatrixMapperClass::Set_Texture_Transform(const Matrix4x4 & view_to_texture,float texsize)
{
	ViewToTexture=view_to_texture;

	Update_View_To_Pixel_Transform(texsize);
}

/***********************************************************************************************
 * MatrixMapperClass::Update_View_To_Pixel_Transform -- recomputes ViewToPixel                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void MatrixMapperClass::Update_View_To_Pixel_Transform(float tex_size)
{
	/*
	** Create a ViewToPixel matrix which also does all of the offseting and flipping that has
	** to take place to get the actual texture coordinates.
	**
	** Here is a description of the math:
	** If you transform eye-space points with the ViewToTexture matrix, you then need to compute:
	** s = ((x/w) + 1.0f) * 0.5f * ((texwid-2)/texwid)
	** t = (1.0f - (y/w)) * 0.5f * ((texwid-2)/texwid)
	**
	** Let K = 0.5 * ((texwid-2)/texwid), Then:
	** ((x/w) + 1.0f) * K = (K*x + K*w) / w
	** (1.0f - (y/w)) * K = (-K*y + K*w) / w
	**
	** This leads to the following matrix (The KOOK matrix!)
	** | K  0 0 K |
	** | 0 -K 0 K |
	** | 0  0 1 0 |
	** | 0  0 0 1 |
	**
	** The code below manually "optimally" pre-multiplies this matrix with the ViewToTexture matrix
	**
	** In addition, the z-coordinate is modified so that it goes from 0 to 1 rather than -1 to +1.
	** It can also be flipped if the INVERT_DEPTH_GRADIENT flag is on.
	*/
	float k = 0.5 * (tex_size - 2.0f) / tex_size;

	ViewToPixel[0][0] = k * ViewToTexture[0][0] + k * ViewToTexture[3][0];
	ViewToPixel[0][1] = k * ViewToTexture[0][1] + k * ViewToTexture[3][1];
	ViewToPixel[0][2] = k * ViewToTexture[0][2] + k * ViewToTexture[3][2];
	ViewToPixel[0][3] = k * ViewToTexture[0][3] + k * ViewToTexture[3][3];

	ViewToPixel[1][0] = -k * ViewToTexture[1][0] + k * ViewToTexture[3][0];
	ViewToPixel[1][1] = -k * ViewToTexture[1][1] + k * ViewToTexture[3][1];
	ViewToPixel[1][2] = -k * ViewToTexture[1][2] + k * ViewToTexture[3][2];
	ViewToPixel[1][3] = -k * ViewToTexture[1][3] + k * ViewToTexture[3][3];

	if (Get_Flag(INVERT_DEPTH_GRADIENT)) {
		ViewToPixel[2][0] = -0.5f * ViewToTexture[2][0] + 0.5f * ViewToTexture[3][0];
		ViewToPixel[2][1] = -0.5f * ViewToTexture[2][1] + 0.5f * ViewToTexture[3][1];
		ViewToPixel[2][2] = -0.5f * ViewToTexture[2][2] + 0.5f * ViewToTexture[3][2];
		ViewToPixel[2][3] = -0.5f * ViewToTexture[2][3] + 0.5f * ViewToTexture[3][3];
	} else {
		ViewToPixel[2][0] = 0.5f * ViewToTexture[2][0] + 0.5f * ViewToTexture[3][0];
		ViewToPixel[2][1] = 0.5f * ViewToTexture[2][1] + 0.5f * ViewToTexture[3][1];
		ViewToPixel[2][2] = 0.5f * ViewToTexture[2][2] + 0.5f * ViewToTexture[3][2];
		ViewToPixel[2][3] = 0.5f * ViewToTexture[2][3] + 0.5f * ViewToTexture[3][3];
	}

	ViewToPixel[3] = ViewToTexture[3];

	/*
	** Store the view space negative z vector for lighting effects
	*/
	ViewSpaceProjectionNormal.X = -ViewToTexture[2][0];
	ViewSpaceProjectionNormal.Y = -ViewToTexture[2][1];
	ViewSpaceProjectionNormal.Z = -ViewToTexture[2][2];
	ViewSpaceProjectionNormal.Normalize();

}

/***********************************************************************************************
 * MatrixMapperClass::Compute_Texture_Coordinate -- compute a single texture coord             *
 *                                                                                             *
 * INPUT:                                                                                      *
 * point - 3D point to calculate the texture coordinate for                                    *
 * set_stq - pointer to texture coordinate, s,t will be stored in X,Y.  q will be stored in Z  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    gth : Created.                                                                 *
 *=============================================================================================*/
void MatrixMapperClass::Compute_Texture_Coordinate(const Vector3 & point,Vector3 * set_stq)
{
	set_stq->X = ViewToPixel[0][0]*point.X + ViewToPixel[0][1]*point.Y + ViewToPixel[0][2]*point.Z + ViewToPixel[0][3];
	set_stq->Y = ViewToPixel[1][0]*point.X + ViewToPixel[1][1]*point.Y + ViewToPixel[1][2]*point.Z + ViewToPixel[1][3];
	set_stq->Z = ViewToPixel[3][0]*point.X + ViewToPixel[3][1]*point.Y + ViewToPixel[3][2]*point.Z + ViewToPixel[3][3];
}

/***********************************************************************************************
 * MatrixMapperClass::Apply -- Apply texture mapper to render states and texture stage states  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/05/01     NH : Created comment block.                                                  *
 *=============================================================================================*/
void MatrixMapperClass::Apply(int uv_array_index)
{
	Matrix4x4 m;

	switch (Type)
	{
	case ORTHO_PROJECTION:
		/*
		** Orthographic projection
		*/
		DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + Stage),ViewToPixel);
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
		break;
	case PERSPECTIVE_PROJECTION:
		/*
		** Perspective projection
		*/
		m[0]=ViewToPixel[0];
		m[1]=ViewToPixel[1];
		m[2]=ViewToPixel[3];
		DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + Stage),m);
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_PROJECTED|D3DTTFF_COUNT3);
		break;
	case DEPTH_GRADIENT:
		/*
		** Depth gradient, Set up second stage texture coordinates to
		** apply a depth gradient to the projection.  Note that the
		** depth values have been set up to vary from 0 to 1 in the
		** Update_View_To_Pixel_Transform function.
		*/
		m[0].Set(0,0,0,GradientUCoord);
		m[1]=ViewToPixel[2];
		DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + Stage),m);
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
		break;
	case NORMAL_GRADIENT:
		/*
		** Normal Gradient, Set up the second stage texture coordinates to
		** apply a gradient based on the dot product of the vertex normal
		** and the projection direction.  (NOTE: this is basically texture-
		** based diffuse lighting!)
		*/
		m[0].Set(0,0,0,GradientUCoord);
		m[1].Set(ViewSpaceProjectionNormal.X,ViewSpaceProjectionNormal.Y,ViewSpaceProjectionNormal.Z, 0);
		DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + Stage),m);
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
		break;
	}


}

/***********************************************************************************************
 * MatrixMapperClass::Calculate_Texture_Matrix -- Calculate texture matrix                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/06/01     NH : Created comment block.                                                  *
 *=============================================================================================*/
void MatrixMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	// We return ViewToPixel. This is not, strictly speaking, always correct, but it is close
	// and it is OK not to be 100% correct since this function is not called by the Apply()
	// function.
	tex_matrix = ViewToPixel;
}

/***********************************************************************************************
 * CompositeMatrixMapperClass::CompositeMatrixMapperClass -- Constructor                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/05/01     NH : Created.                                                                *
 *=============================================================================================*/
CompositeMatrixMapperClass::CompositeMatrixMapperClass(TextureMapperClass *internal_mapper, unsigned int stage) :
	MatrixMapperClass(stage),
	InternalMapper(internal_mapper)
{
	if (InternalMapper) {
		InternalMapper->Add_Ref();
	}
}

/***********************************************************************************************
 * CompositeMatrixMapperClass::CompositeMatrixMapperClass -- Copy Constructor                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/05/01     NH : Created.                                                                *
 *=============================================================================================*/
CompositeMatrixMapperClass::CompositeMatrixMapperClass(const CompositeMatrixMapperClass & src) :
	MatrixMapperClass(src),
	InternalMapper(src.InternalMapper ? src.InternalMapper->Clone() : NULL)
{
	if (InternalMapper) {
		InternalMapper->Add_Ref();
	}
}

/***********************************************************************************************
 * CompositeMatrixMapperClass::~CompositeMatrixMapperClass -- Destructor                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/05/01     NH : Created.                                                                *
 *=============================================================================================*/
CompositeMatrixMapperClass::~CompositeMatrixMapperClass(void)
{
	if (InternalMapper) {
		InternalMapper->Release_Ref();
		InternalMapper = NULL;
	}
}

/***********************************************************************************************
 * CompositeMatrixMapperClass::Apply -- Apply texture mapper to render states and ts states    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/05/01     NH : Created.                                                                *
 *=============================================================================================*/
void CompositeMatrixMapperClass::Apply(int uv_array_index)
{
	if (InternalMapper) {
		// Get the texture matrix from the internal mapper, composite it into ViewToPixel (save off
		// the previous value of ViewToPixel first), call the base class Apply() function (which will
		// use the modifiedViewToPixel) and then restore ViewToPixel to its previous state.
		Matrix4x4 int_mat;
		InternalMapper->Calculate_Texture_Matrix(int_mat);
		Matrix4x4 view_to_pixel_copy(ViewToPixel);

		// We need to modify the view-to-pixel matrix to produce q (third texture coordinate values)
		// equal to one. This is the input which the internal mappers' matrix was designed for (it
		// is what you get when you use 2D vertex coordinates from the vertex buffer).
		// For this we need to multiply the matrix by the following matrix:
		// [1 0 0 0]
		// [0 1 0 0]  This is equivalent to overwriting the third row with the fourth one.
		// [0 0 0 1]
		// [0 0 0 1]
		Matrix4x4 tmp;
		tmp[0] = ViewToPixel[0];
		tmp[1] = ViewToPixel[1];
		tmp[2] = ViewToPixel[3];
		tmp[3] = ViewToPixel[3];

		// We multiply the matrices in this order so the camera position, transformed by ViewToPixel
		// is used as the 'input texture coordinates' to be affected by the internal mapper matrix.
		Matrix4x4::Multiply(int_mat, tmp, &ViewToPixel);

		MatrixMapperClass::Apply(uv_array_index);
		ViewToPixel = view_to_pixel_copy;
	} else {
		MatrixMapperClass::Apply(uv_array_index);
	}
}

/***********************************************************************************************
 * MatrixMapperClass::Calculate_Texture_Matrix -- Calculate texture matrix                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/06/01     NH : Created comment block.                                                  *
 *=============================================================================================*/
void CompositeMatrixMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	if (InternalMapper) {
		// We return the internal mapper's matrix times ViewToPixel, adjusted to produce the correct
		// z-value of 1. This is only correct if the mapper is in ORTHO_PROJECTION or
		// PERSPECTIVE_PROJECTION mode, which should be the case for most uses of this mapper. It is
		// OK for this function not be be 100% correct in all cases, since it is not used in the
		// Apply() function.
		Matrix4x4 int_mat;
		InternalMapper->Calculate_Texture_Matrix(int_mat);

			// We need to modify the view-to-pixel matrix to produce q (third texture coordinate values)
			// equal to one. This is the input which the internal mappers' matrix was designed for (it
			// is what you get when you use 2D vertex coordinates from the vertex buffer).
			// For this we need to multiply the matrix by the following matrix:
			// [1 0 0 0]
			// [0 1 0 0]  This is equivalent to overwriting the third row with the fourth one.
			// [0 0 0 1]
			// [0 0 0 1]
			Matrix4x4 tmp;
			tmp[0] = ViewToPixel[0];
			tmp[1] = ViewToPixel[1];
			tmp[2] = ViewToPixel[3];
			tmp[3] = ViewToPixel[3];

		// We multiply the matrices in this order so the camera position, transformed by ViewToPixel
		// is used as the 'input texture coordinates' to be affected by the internal mapper matrix.
		Matrix4x4::Multiply(int_mat, tmp, &tex_matrix);
	} else {
		MatrixMapperClass::Calculate_Texture_Matrix(tex_matrix);
	}
}
