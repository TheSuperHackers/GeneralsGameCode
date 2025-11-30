/*
**	Command & Conquer Generals(tm)
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
 *                     $Archive:: /Commando/Code/ww3d2/render2d.cpp                           $*
 *                                                                                             *
 *                       $Author:: Jani_p                                                                                                                                                                                  $Modtime:: 1/24/01 3:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 38                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "render2d.h"
#include "mutex.h"
#include "ww3d.h"
#include "font3d.h"
#include "rect.h"
#include "texture.h"
#include "matrix4.h"
#include "matrix3d.h"
#include "dx8wrapper.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "sortingrenderer.h"
#include "vertmaterial.h"
#include "dx8fvf.h"
#include "dx8caps.h"
#include "wwprofile.h"
#include "wwmemlog.h"
#include "assetmgr.h"

RectClass							Render2DClass::ScreenResolution( 0,0,0,0 );


/*
** Render2DClass
*/
Render2DClass::Render2DClass( TextureClass* tex ) :
	CoordinateScale( 1, 1 ),
	CoordinateOffset( 0, 0 ),
	Texture(0),
	ZValue(0),
	IsHidden( false ),
	IsGrayScale (false)
{
	Set_Texture( tex );
   Shader = Get_Default_Shader();
	return ;
}

Render2DClass::~Render2DClass()
{
	REF_PTR_RELEASE(Texture);
}

ShaderClass
Render2DClass::Get_Default_Shader( void )
{
	ShaderClass shader;

   shader.Set_Depth_Mask( ShaderClass::DEPTH_WRITE_DISABLE );
	shader.Set_Depth_Compare( ShaderClass::PASS_ALWAYS );
	shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA );
	shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_SRC_ALPHA );
	shader.Set_Fog_Func( ShaderClass::FOG_DISABLE );
	shader.Set_Primary_Gradient( ShaderClass::GRADIENT_MODULATE );
	shader.Set_Texturing( ShaderClass::TEXTURING_ENABLE );

	return shader;
}

void	Render2DClass::Reset(void)
{
	Vertices.Delete_All( false );
	UVCoordinates.Delete_All( false );
	Colors.Delete_All( false );
	Indices.Delete_All( false );

	Update_Bias(); // Keep the bias updated
}

void Render2DClass::Set_Texture(TextureClass* tex)
{
	REF_PTR_SET(Texture,tex);
}

void Render2DClass::Set_Texture( const char * filename)
{
	TextureClass * tex = WW3DAssetManager::Get_Instance()->Get_Texture( filename, MIP_LEVELS_1 );
	Set_Texture( tex );
	if ( tex != NULL ) {
		SET_REF_OWNER( tex );
		tex->Release_Ref();
	}
}

/**added for generals to draw disabled button states - MW*/
void Render2DClass::Enable_Grayscale(bool b)
{
	IsGrayScale = b;
}

void Render2DClass::Enable_Alpha(bool b)
{
	IsGrayScale = false;
	if (b) {
		Shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA );
		Shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_SRC_ALPHA );
	}
	else {
		Shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_ONE);
		Shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ZERO );
	}
}

void Render2DClass::Enable_Additive(bool b)
{
	IsGrayScale = false;

	if (b) {
		Shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ONE );
		Shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_ONE );
	}
	else {
		Shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_ONE);
		Shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ZERO );
	}
}

void Render2DClass::Enable_Texturing(bool b)
{
	if (b) {
		Shader.Set_Texturing( ShaderClass::TEXTURING_ENABLE );
	}
	else {
		Shader.Set_Texturing( ShaderClass::TEXTURING_DISABLE );
	}
}

void	Render2DClass::Set_Coordinate_Range( const RectClass & range )
{
	// default range is (-1,1)-(1,-1)
	CoordinateScale.X = 2 / range.Width();
	CoordinateScale.Y = -2 / range.Height();
	CoordinateOffset.X = -(CoordinateScale.X * range.Left) - 1;
	CoordinateOffset.Y = -(CoordinateScale.Y * range.Top) + 1;

	Update_Bias();
}

void	  Render2DClass::Update_Bias( void )
{
	BiasedCoordinateOffset = CoordinateOffset;

	if ( WW3D::Is_Screen_UV_Biased() ) {	// Global bais setting
		Vector2 bais_add( -0.5f ,-0.5f );	// offset by -0.5,-0.5 in pixels

		// Convert from pixels to (-1,1)-(1,-1) units
		bais_add.X = bais_add.X / (Get_Screen_Resolution().Width() * 0.5f);
		bais_add.Y = bais_add.Y / (Get_Screen_Resolution().Height() * -0.5f);

		BiasedCoordinateOffset += bais_add;
	}
}

#if 0
Vector2 Render2DClass::Convert_Vert( const Vector2 & v )
{
	Vector2 out;

	// Convert to (-1,1)-(1,-1)
	out.X = v.X * CoordinateScale.X + CoordinateOffset.X;
	out.Y = v.Y * CoordinateScale.Y + CoordinateOffset.Y;

	// Convert to pixels
	out.X = (out.X + 1.0f) * (Get_Screen_Resolution().Width() * 0.5f);
	out.Y = (out.Y - 1.0f) * (Get_Screen_Resolution().Height() * -0.5f);

	// Round to nearest pixel
	out.X = WWMath::Floor( out.X + 0.5f );
	out.Y = WWMath::Floor( out.Y + 0.5f );

	// Bias
	if ( WW3D::Is_Screen_UV_Biased() ) {	// Global bais setting
		out.X -= 0.5f;
		out.Y -= 0.5f;
	}


	// Convert back to (-1,1)-(1,-1)
	out.X = out.X / (Get_Screen_Resolution().Width() * 0.5f) - 1.0f;
	out.Y = out.Y / (Get_Screen_Resolution().Height() * -0.5f) + 1.0f;

	return out;
}
#else
/*
** Convert Vert must convert from the convention defined by Set_Coordinate_Range
** into the convention (-1,1)-(1,-1), which is needed by the renderer.
// NOPE ** In addition, it rounds all coordinates off to the nearest pixel
** Also, it offsets the coordinates as need for Screen_UV_Bias
*/
void Render2DClass::Convert_Vert( Vector2 & vert_out, const Vector2 & vert_in )
{
	// Convert to (-1,1)-(1,-1)
	vert_out.X = vert_in.X * CoordinateScale.X + BiasedCoordinateOffset.X;
	vert_out.Y = vert_in.Y * CoordinateScale.Y + BiasedCoordinateOffset.Y;
}

void Render2DClass::Convert_Vert( Vector2 & vert_out, float x_in, float y_in )
{
	// Convert to (-1,1)-(1,-1)
	vert_out.X = x_in * CoordinateScale.X + BiasedCoordinateOffset.X;
	vert_out.Y = y_in * CoordinateScale.Y + BiasedCoordinateOffset.Y;
}

#endif

void	Render2DClass::Move( const Vector2 & move )	// Move all verts
{
	Vector2 scaled_move;
	scaled_move.X = move.X * CoordinateScale.X;
	scaled_move.Y = move.Y * CoordinateScale.Y;
	for ( int i = 0; i < Vertices.Count(); i++ ) {
		Vertices[i] += scaled_move;
	}
}



/*
** Internal Add Quad Elements
** Caller must mutex lock
*/
void	Render2DClass::Internal_Add_Quad_Vertices( const Vector2 & v0, const Vector2 & v1, const Vector2 & v2, const Vector2 & v3 )
{
	Vector2 * verts = Vertices.Add_Multiple( 4 );
	Convert_Vert( *verts++, v0 );
	Convert_Vert( *verts++, v1 );
	Convert_Vert( *verts++, v2 );
	Convert_Vert( *verts  , v3 );
}

void	Render2DClass::Internal_Add_Quad_Vertices( const RectClass & screen )
{
	Vector2 * verts = Vertices.Add_Multiple( 4 );
	Convert_Vert( *verts++, screen.Left,  screen.Top );
	Convert_Vert( *verts++, screen.Left,  screen.Bottom );
	Convert_Vert( *verts++, screen.Right, screen.Top );
	Convert_Vert( *verts,   screen.Right, screen.Bottom );
}

void	Render2DClass::Internal_Add_Quad_UVs( const RectClass & uv )
{
	Vector2 * uvs = UVCoordinates.Add_Multiple( 4 );
	uvs->X = uv.Left;		uvs->Y = uv.Top;			uvs++;
	uvs->X = uv.Left;		uvs->Y = uv.Bottom;		uvs++;
	uvs->X = uv.Right;	uvs->Y = uv.Top;			uvs++;
	uvs->X = uv.Right;	uvs->Y = uv.Bottom;
}

void	Render2DClass::Internal_Add_Quad_Colors( unsigned long color )
{
	unsigned long * colors = Colors.Add_Multiple( 4 );
	*colors++ = color;
	*colors++ = color;
	*colors++ = color;
	*colors   = color;
}

void	Render2DClass::Internal_Add_Quad_VColors( unsigned long color1, unsigned long color2 )
{
	unsigned long * colors = Colors.Add_Multiple( 4 );
	*colors++ = color1;
	*colors++ = color2;
	*colors++ = color1;
	*colors   = color2;
}

void	Render2DClass::Internal_Add_Quad_HColors( unsigned long color1, unsigned long color2 )
{
	unsigned long * colors = Colors.Add_Multiple( 4 );
	*colors++ = color1;
	*colors++ = color1;
	*colors++ = color2;
	*colors   = color2;
}


void	Render2DClass::Internal_Add_Quad_Indicies( int start_vert_index, bool backfaced )
{
	unsigned short * indices = Indices.Add_Multiple( 6 );

	if (backfaced ^ (CoordinateScale.X * CoordinateScale.Y > 0)) {
		*indices++ = start_vert_index + 1;
		*indices++ = start_vert_index + 0;
		*indices++ = start_vert_index + 2;

		*indices++ = start_vert_index + 1;
		*indices++ = start_vert_index + 2;
		*indices   = start_vert_index + 3;
	} else {
		*indices++ = start_vert_index + 0;
		*indices++ = start_vert_index + 1;
		*indices++ = start_vert_index + 2;

		*indices++ = start_vert_index + 2;
		*indices++ = start_vert_index + 1;
		*indices   = start_vert_index + 3;
	}
}


void	Render2DClass::Add_Quad( const Vector2 & v0, const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, const RectClass & uv, unsigned long color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( v0, v1, v2, v3 );
	Internal_Add_Quad_UVs( uv );
	Internal_Add_Quad_Colors( color );
}

void	Render2DClass::Add_Quad_Backfaced( const Vector2 & v0, const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, const RectClass & uv, unsigned long color )
{
	Internal_Add_Quad_Indicies( Vertices.Count(), true );
	Internal_Add_Quad_Vertices( v0, v1, v2, v3 );
	Internal_Add_Quad_UVs( uv );
	Internal_Add_Quad_Colors( color );
}

void	Render2DClass::Add_Quad_VGradient( const Vector2 & v0, const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, const RectClass & uv, unsigned long top_color, unsigned long bottom_color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( v0, v1, v2, v3 );
	Internal_Add_Quad_UVs( uv );
	Internal_Add_Quad_VColors( top_color, bottom_color );
}

void	Render2DClass::Add_Quad_HGradient( const Vector2 & v0, const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, const RectClass & uv, unsigned long left_color, unsigned long right_color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( v0, v1, v2, v3 );
	Internal_Add_Quad_UVs( uv );
	Internal_Add_Quad_HColors( left_color, right_color );
}


void	Render2DClass::Add_Quad_VGradient( const RectClass & screen, unsigned long top_color, unsigned long bottom_color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( screen );
	Internal_Add_Quad_UVs( RectClass( 0,0,1,1 ) );
	Internal_Add_Quad_VColors( top_color, bottom_color );
}

void	Render2DClass::Add_Quad_HGradient( const RectClass & screen, unsigned long left_color, unsigned long right_color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( screen );
	Internal_Add_Quad_UVs( RectClass( 0,0,1,1 ) );
	Internal_Add_Quad_HColors( left_color, right_color );
}


void	Render2DClass::Add_Quad( const RectClass & screen, const RectClass & uv, unsigned long color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( screen );
	Internal_Add_Quad_UVs( uv );
	Internal_Add_Quad_Colors( color );
}

void	Render2DClass::Add_Quad( const Vector2 & v0, const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, unsigned long color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( v0, v1, v2, v3 );
	Internal_Add_Quad_UVs( RectClass( 0,0,1,1 ) );
	Internal_Add_Quad_Colors( color );
}

void	Render2DClass::Add_Quad( const RectClass & screen, unsigned long color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( screen );
	Internal_Add_Quad_UVs( RectClass( 0,0,1,1 ) );
	Internal_Add_Quad_Colors( color );
}

/*
** Add Tri
*/
void	Render2DClass::Add_Tri( const Vector2 & v0, const Vector2 & v1, const Vector2 & v2, const Vector2 & uv0, const Vector2 & uv1, const Vector2 & uv2, unsigned long color )
{
	int old_vert_count = Vertices.Count();
	int new_vert_count = old_vert_count + 3;
	int new_index_count = Indices.Count() + 3;

	// Add the verticies (translated to new coordinates)
#if 0
	Vertices.Add( Convert_Vert( v0 ), new_vert_count );
	Vertices.Add( Convert_Vert( v1 ), new_vert_count );
	Vertices.Add( Convert_Vert( v2 ), new_vert_count );
#else
	Vector2 * verts = Vertices.Add_Multiple( 3 );
	Convert_Vert( *verts++, v0 );
	Convert_Vert( *verts++, v1 );
	Convert_Vert( *verts  , v2 );
#endif

	// Add the uv coordinates
	UVCoordinates.Add( uv0,  new_vert_count );
	UVCoordinates.Add( uv1,  new_vert_count );
	UVCoordinates.Add( uv2, new_vert_count );

	// Add the colors
	Colors.Add( color, new_vert_count );
	Colors.Add( color, new_vert_count );
	Colors.Add( color, new_vert_count );

	// Add the faces
	Indices.Add( old_vert_count + 0, new_index_count );
	Indices.Add( old_vert_count + 1, new_index_count );
	Indices.Add( old_vert_count + 2, new_index_count );

}

void	Render2DClass::Add_Line( const Vector2 & a, const Vector2 & b, float width, unsigned long color )
{
	Add_Line( a, b, width, RectClass( 0,0,1,1 ), color );
}

void	Render2DClass::Add_Line( const Vector2 & a, const Vector2 & b, float width, const RectClass & uv, unsigned long color )
{
	Vector2	corner_offset = a - b;				// get line relative to b
	float temp = corner_offset.X;					// Rotate 90
	corner_offset.X = corner_offset.Y;
	corner_offset.Y = -temp;
	corner_offset.Normalize();						// scale to length width/2
	corner_offset *= width / 2;

	Add_Quad( a - corner_offset, a + corner_offset, b - corner_offset, b + corner_offset, uv, color );
	//Add_Quad_HGradient(RectClass( a.X -corner_offset.X ,a.Y,b.X,b.Y ), color, color2);
	//Add_Quad_HGradient( const RectClass & screen, unsigned long left_color, unsigned long right_color );
}


void	Render2DClass::Add_Line( const Vector2 & a, const Vector2 & b, float width, unsigned long color, unsigned long color2 )
{
	Add_Line( a, b, width, RectClass( 0,0,1,1 ), color, color2 );
}

void Render2DClass::Add_Line( const Vector2 & a, const Vector2 & b, float width, const RectClass & uv, unsigned long color , unsigned long color2)
{
	Vector2	corner_offset = a - b;				// get line relative to b
	float temp = corner_offset.X;					// Rotate 90
	corner_offset.X = corner_offset.Y;
	corner_offset.Y = -temp;
	corner_offset.Normalize();						// scale to length width/2
	corner_offset *= width / 2;

	Add_Quad_HGradient( a - corner_offset, a + corner_offset, b - corner_offset, b + corner_offset, uv, color,color2);
	//Add_Quad_HGradient(RectClass( a.X -corner_offset.X ,a.Y,b.X,b.Y ), color, color2);
}


void	Render2DClass::Add_Rect( const RectClass & rect, float border_width, uint32 border_color, uint32 fill_color )
{
	//
	//	First add the outline
	//
	if( border_width > 0 )
	Add_Outline( rect, border_width, border_color );

	//
	//	Next, fill the contents
	//
	RectClass fill_rect = rect;
	if( border_width > 0 )
	{
		fill_rect.Left += border_width + 1;
		fill_rect.Top += border_width + 1;
		fill_rect.Right	-= border_width - 1;
		fill_rect.Bottom	-= border_width - 1;
	}
	Add_Quad (fill_rect, fill_color);
	return ;
}

void	Render2DClass::Add_Outline( const RectClass & rect, float width, unsigned long color )
{
	Add_Outline( rect, width, RectClass( 0,0,1,1 ), color );
}

void	Render2DClass::Add_Outline( const RectClass & rect, float width, const RectClass & uv, unsigned long color )
{
	//
	//	Pretty straight forward, simply add the four side of the rectangle as lines.
	//
	/** @todo colin, I had to tweak these to get precise line drawing, as we want
	the UV bias on, but it just isn't lining up */
	Add_Line (Vector2 (rect.Left + 1, rect.Bottom),	Vector2 (rect.Left + 1, rect.Top + 1),		width, color);
	Add_Line (Vector2 (rect.Left, rect.Top + 1),		Vector2 (rect.Right - 1, rect.Top + 1),			width, color);
	Add_Line (Vector2 (rect.Right, rect.Top),		Vector2 (rect.Right, rect.Bottom - 1),		width, color);
	Add_Line (Vector2 (rect.Right, rect.Bottom),	Vector2 (rect.Left + 1, rect.Bottom),	width, color);
}

void Render2DClass::Render(void)
{
	if ( !Indices.Count() || IsHidden) {
		return;
	}

	// save the view and projection matrices since we're nuking them
	Matrix4x4 view,proj;
	Matrix4x4 identity(true);

	DX8Wrapper::Get_Transform(D3DTS_VIEW,view);
	DX8Wrapper::Get_Transform(D3DTS_PROJECTION,proj);

	//
	//	Configure the viewport for entire screen
	//
	D3DVIEWPORT8 vp = { 0 };
	vp.X			= (DWORD)ScreenResolution.Left;
	vp.Y			= (DWORD)ScreenResolution.Top;
	vp.Width		= (DWORD)ScreenResolution.Width ();
	vp.Height	= (DWORD)ScreenResolution.Height ();
	vp.MinZ		= 0;
	vp.MaxZ		= 1;
	DX8Wrapper::Set_Viewport(&vp);


	DX8Wrapper::Set_Texture(0,Texture);

	VertexMaterialClass *vm=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(vm);
	REF_PTR_RELEASE(vm);

	DX8Wrapper::Set_World_Identity();
	DX8Wrapper::Set_View_Identity();
	DX8Wrapper::Set_Transform(D3DTS_PROJECTION,identity);

	DynamicVBAccessClass vb(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,Vertices.Count());
	{
		DynamicVBAccessClass::WriteLockClass Lock(&vb);
		const FVFInfoClass &fi=vb.FVF_Info();
		unsigned char *vb=(unsigned char*)Lock.Get_Formatted_Vertex_Array();
		int i;

		for (i=0; i<Vertices.Count(); i++)
		{
			Vector3 temp(Vertices[i].X,Vertices[i].Y,ZValue);
			*(Vector3*)(vb+fi.Get_Location_Offset())=temp;
			*(unsigned int*)(vb+fi.Get_Diffuse_Offset())=Colors[i];
			*(Vector2*)(vb+fi.Get_Tex_Offset(0))=UVCoordinates[i];
			vb+=fi.Get_FVF_Size();
		}
	}

	DynamicIBAccessClass ib(BUFFER_TYPE_DYNAMIC_DX8,Indices.Count());
	{
		DynamicIBAccessClass::WriteLockClass Lock(&ib);
		unsigned short *mem=Lock.Get_Index_Array();
		for (int i=0; i<Indices.Count(); i++)
			mem[i]=Indices[i];
	}

	DX8Wrapper::Set_Vertex_Buffer(vb);
	DX8Wrapper::Set_Index_Buffer(ib,0);

	if (IsGrayScale)
	{	//special case added to draw grayscale non-alpha blended images.
		DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
		DX8Wrapper::Apply_Render_State_Changes();	//force update of all regular W3D states.
		if (DX8Wrapper::Get_Current_Caps()->Support_Dot3())
		{	//Override W3D states with customizations for grayscale
			DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, 0x80A5CA8E);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_COLORARG0, D3DTA_TFACTOR | D3DTA_ALPHAREPLICATE);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR | D3DTA_ALPHAREPLICATE);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);

			DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3);
		}
		else
		{	//doesn't have DOT3 blend mode so fake it another way.
			DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, 0x60606060);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		}
	}
	else
		DX8Wrapper::Set_Shader(Shader);

	DX8Wrapper::Draw_Triangles(0,Indices.Count()/3,0,Vertices.Count());
//	SphereClass sphere(Vector3(0.0f,0.0f,0.0f),0.0f);
//	SortingRendererClass::Insert_Triangles(sphere,0,Indices.Count()/3,0,Vertices.Count());

	DX8Wrapper::Set_Transform(D3DTS_VIEW,view);
	DX8Wrapper::Set_Transform(D3DTS_PROJECTION,proj);
	if (IsGrayScale)
		ShaderClass::Invalidate();	//force both stages to be reset.

}


/*
** Render2DTextClass
*/
Render2DTextClass::Render2DTextClass(Font3DInstanceClass *font) :
	Location(0.0f,0.0f),
	Cursor(0.0f,0.0f),
	Font(NULL),
	WrapWidth(0),
	ClipRect(0, 0, 0, 0),
	IsClippedEnabled(false)
{
	Set_Coordinate_Range( RectClass( -320, -240, 320, 240 ) );
	Set_Font( font );

	Reset();
}

Render2DTextClass::~Render2DTextClass()
{
	REF_PTR_RELEASE(Font);
}

void	Render2DTextClass::Reset(void)
{
	Render2DClass::Reset();
	Cursor = Location;
	WrapWidth = 0;
	DrawExtents = RectClass( 0,0,0,0 );
	TotalExtents = RectClass( 0,0,0,0 );
	ClipRect.Set (0, 0, 0, 0);
	IsClippedEnabled = false;
}

void	Render2DTextClass::Set_Font( Font3DInstanceClass *font )
{
	REF_PTR_SET(Font,font);

	if ( Font != NULL ) {
		Set_Texture( Font->Peek_Texture() );

	#define	BLOCK_CHAR	0
		BlockUV = Font->Char_UV( BLOCK_CHAR );
		// Inset it a bit to be sure we have no edge problems
		BlockUV.Inflate( Vector2(-BlockUV.Width()/4, -BlockUV.Height()/4) );
	}
}


/*
**
*/
void	Render2DTextClass::Draw_Char( WCHAR ch, unsigned long color )
{
	float char_spacing	= Font->Char_Spacing( ch );
	float char_height		= Font->Char_Height();

	//
	//	Check to see if this character is clipped
	//
	bool is_clipped = false;
	if (	IsClippedEnabled &&
			(Cursor.X < ClipRect.Left ||
			 Cursor.X + char_spacing > ClipRect.Right ||
			 Cursor.Y < ClipRect.Top ||
			 Cursor.Y + char_height > ClipRect.Bottom))
	{
		is_clipped = true;
	}

	if ( ch != (WCHAR)' ' && !is_clipped ) {
		RectClass screen( Cursor.X, Cursor.Y, Cursor.X + Font->Char_Width(ch), Cursor.Y + char_height );

		Internal_Add_Quad_Indicies( Vertices.Count() );
		Internal_Add_Quad_Vertices( screen );
		Internal_Add_Quad_UVs( Font->Char_UV( ch ) );
		Internal_Add_Quad_Colors( color );

		DrawExtents += screen;
		TotalExtents += screen;
	}
	Cursor.X += char_spacing;
}

void	Render2DTextClass::Draw_Text( const char * text, unsigned long color )
{
	WWMEMLOG(MEM_GEOMETRY);
	WideStringClass wide(0,true);
	wide.Convert_From( text );
	Draw_Text( wide, color );
}

void	Render2DTextClass::Draw_Text( const WCHAR * text, unsigned long color )
{
	WWMEMLOG(MEM_GEOMETRY);

	// Reset the Extents
	DrawExtents = RectClass( Location, Location );
	if ( TotalExtents.Width() == 0 ) {
		TotalExtents = RectClass( Location, Location );
	}

	while (*text) {
		WCHAR ch = *text++;

		//	Check to see if we need to move to a newline or not
		bool wrap = ( ch == (WCHAR)'\n' );

		// if the current char is a space, and the next word length puts us past our Width, wrap
		if ( ch == (WCHAR)' ' && WrapWidth > 0 ) {
			const WCHAR * word = text;
			float word_width = Font->Char_Spacing(ch);
			while ( (*word != (WCHAR)0) && (*word > (WCHAR)' ') ) {
				word_width += Font->Char_Spacing(*word++);
			}
			wrap = ( (Cursor.X + word_width) >= (Location.X + WrapWidth) );
		}

		if ( wrap ) {
			Cursor.Y += Font->Char_Height();
			Cursor.X = Location.X;
		} else {

			// Draw char at cursor, update cursor and extents
			Draw_Char( ch, color );
		}
	}
}

void	Render2DTextClass::Draw_Block( const RectClass & screen, unsigned long color )
{
	Internal_Add_Quad_Indicies( Vertices.Count() );
	Internal_Add_Quad_Vertices( screen );
	Internal_Add_Quad_UVs( BlockUV );
	Internal_Add_Quad_Colors( color );

	TotalExtents += screen;
}

Vector2	Render2DTextClass::Get_Text_Extents( const WCHAR * text )
{
	Vector2 extent (0, Font->Char_Height());

	while (*text) {
		WCHAR ch = *text++;

		if ( ch != (WCHAR)'\n' ) {
			extent.X += Font->Char_Spacing( ch );
		}
	}

	return extent;
}

