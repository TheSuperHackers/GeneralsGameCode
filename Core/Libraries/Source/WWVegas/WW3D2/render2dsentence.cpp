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
 *                     $Archive:: /Commando/Code/ww3d2/render2dsentence.cpp                   $*
 *                                                                                             *
 *                       $Author:: Patrick                  $*
 *                                                                                             *
 *								$Modtime:: 8/29/01 11:16a                                             $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "render2dsentence.h"
#include "surfaceclass.h"
#include "texture.h"
#include "WWDebug/wwprofile.h"
#include "WWDebug/wwmemlog.h"
#include "dx8wrapper.h"


////////////////////////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////////////////////////
#define no_TEST_PLACEMENT 1	 // Shows alignment markers for text.

#define TEXTURE_OFFSET 2
////////////////////////////////////////////////////////////////////////////////////
//
//	Render2DSentenceClass
//
////////////////////////////////////////////////////////////////////////////////////
Render2DSentenceClass::Render2DSentenceClass () :
	Font (nullptr),
	Location (0.0F,0.0F),
	Cursor (0.0F,0.0F),
	TextureOffset (0, 0),
	TextureStartX (0),
	CurSurface (nullptr),
	CurrTextureSize (0),
	MonoSpaced (false),
	IsClippedEnabled (false),
	ClipRect (0, 0, 0, 0),
	BaseLocation (0, 0),
	LockedPtr (nullptr),
	LockedStride (0),
	TextureSizeHint (0),
	WrapWidth (0),
	Centered (false),
	DrawExtents (0, 0, 0, 0),
	ParseHotKey( false ),
	useHardWordWrap( false)
{
	Shader = Render2DClass::Get_Default_Shader ();
}


////////////////////////////////////////////////////////////////////////////////////
//
//	~Render2DSentenceClass
//
////////////////////////////////////////////////////////////////////////////////////
Render2DSentenceClass::~Render2DSentenceClass ()
{
	REF_PTR_RELEASE (Font);
	Reset ();
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Font
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Set_Font (FontCharsClass *font)
{
	Reset ();
	REF_PTR_SET (Font, font);
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Polys
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Reset_Polys ()
{
	for (int index = 0; index < Renderers.Count (); index ++) {
		Renderers[index].Renderer->Reset ();
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Reset
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Reset ()
{
	//
	//	Make sure we unlock the current surface (if necessary)
	//
	if (LockedPtr != nullptr) {
		CurSurface->Unlock ();
		LockedPtr = nullptr;
	}

	//
	//	Release our hold on the current surface
	//
	REF_PTR_RELEASE (CurSurface);

	//
	//	Free each renderer
	//
	while (Renderers.Count () > 0) {
		delete Renderers[0].Renderer;
		Renderers.Delete(0);
	}

	Cursor.Set (0, 0);
	MonoSpaced = false;
	ParseHotKey = false;

	Release_Pending_Surfaces ();
	Reset_Sentence_Data ();
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Make_Additive
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Make_Additive ()
{
	Shader.Set_Dst_Blend_Func (ShaderClass::DSTBLEND_ONE);
	Shader.Set_Src_Blend_Func (ShaderClass::SRCBLEND_ONE);
	Shader.Set_Primary_Gradient (ShaderClass::GRADIENT_MODULATE);
	Shader.Set_Secondary_Gradient (ShaderClass::SECONDARY_GRADIENT_DISABLE);

	Set_Shader (Shader);
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Make_Additive
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Set_Shader (ShaderClass shader)
{
	Shader = shader;

	//
	//	Change each renderer's shader
	//
	for (int i = 0; i < Renderers.Count (); i ++) {
		ShaderClass *curr_shader = Renderers[i].Renderer->Get_Shader ();
		(*curr_shader) = Shader;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Render ()
{
	//
	//	Build any textures that are pending
	//
	Build_Textures ();

	//
	//	Ask each renderer to draw its contents
	//
	for (int i = 0; i < Renderers.Count (); i ++) {
		Renderers[i].Renderer->Render ();
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Base_Location
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Set_Base_Location (const Vector2 &loc)
{
	Vector2 dif		= loc - BaseLocation;
	BaseLocation	= loc;
	for (int i = 0; i < Renderers.Count (); i ++) {
		Renderers[i].Renderer->Move (dif);
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Location
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Set_Location (const Vector2 &loc)
{
	Location	= loc;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Text_Extents
//
////////////////////////////////////////////////////////////////////////////////////
Vector2
Render2DSentenceClass::Get_Text_Extents (const WCHAR *text)
{
	Vector2 extent (0, Font->Get_Char_Height());

	while (*text) {
		WCHAR ch = *text++;

		if ( ch != (WCHAR)'\n' ) {
			extent.X += Font->Get_Char_Spacing( ch );
		}
	}

	return extent;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Formatted_Text_Extents
//
////////////////////////////////////////////////////////////////////////////////////
Vector2
Render2DSentenceClass::Get_Formatted_Text_Extents (const WCHAR *text)
{
	return Build_Sentence_Not_Centered(text, nullptr, nullptr, true);
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Sentence_Data
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Reset_Sentence_Data ()
{
	//
	//	Release our hold on each texture used in the sentence
	//
	for (int index = 0; index < SentenceData.Count (); index ++) {
		REF_PTR_RELEASE (SentenceData[index].Surface);
	}

	if (SentenceData.Count()>0) {
		SentenceData.Delete_All ();
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Release_Pending_Surfaces
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Release_Pending_Surfaces ()
{
	//
	//	Release our hold on each pending surface
	//
	for (int index = 0; index < PendingSurfaces.Count (); index ++) {
		SurfaceClass *curr_surface = PendingSurfaces[index].Surface;
		REF_PTR_RELEASE (curr_surface);
	}

	if (PendingSurfaces.Count()>0) PendingSurfaces.Delete_All ();
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Build_Textures
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Build_Textures ()
{
	WWMEMLOG(MEM_TEXTURE);

	//
	//	Make sure we unlock the current surface
	//
	if (LockedPtr != nullptr) {
		CurSurface->Unlock ();
		LockedPtr = nullptr;
	}

	//
	//	Release our hold on the current surface
	//
	REF_PTR_RELEASE (CurSurface);
	TextureOffset.Set (0, 0);
	TextureStartX = 0;

	//
	//	Convert all pending surfaces to textures
	//
	for (int index = 0; index < PendingSurfaces.Count (); index ++) {
		PendingSurfaceStruct &surface_info = PendingSurfaces[index];
		SurfaceClass *curr_surface = surface_info.Surface;

		//
		//	Get the dimensions of the surface
		//
		SurfaceClass::SurfaceDescription desc;
		curr_surface->Get_Description (desc);

		//
		//	Create the new texture
		//
		TextureClass *new_texture = W3DNEW TextureClass (desc.Width, desc.Width, WW3D_FORMAT_A4R4G4B4, MIP_LEVELS_1);
		SurfaceClass *texture_surface = new_texture->Get_Surface_Level ();

		new_texture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		new_texture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		new_texture->Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_NONE);
		new_texture->Get_Filter().Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_NONE);
		new_texture->Get_Filter().Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);

		//
		//	Copy the contents of the texture from the surface
		//
		DX8Wrapper::_Copy_DX8_Rects (curr_surface->Peek_D3D_Surface (), nullptr, 0, texture_surface->Peek_D3D_Surface (), nullptr);
		REF_PTR_RELEASE (texture_surface);

		//
		//	Assign this texture to any renderers that need it
		//
		for (int renderer_index = 0; renderer_index < surface_info.Renderers.Count (); renderer_index ++) {
			Render2DClass *renderer = surface_info.Renderers[renderer_index];
			renderer->Set_Texture (new_texture);
		}

		//
		//	Release our hold on the objects
		//
		REF_PTR_RELEASE (new_texture);
		REF_PTR_RELEASE (curr_surface);
	}

	//
	//	Reset the list
	//
	if (PendingSurfaces.Count()>0) {
		PendingSurfaces.Delete_All ();
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Draw_Sentence
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Draw_Sentence (uint32 color)
{
	Render2DClass *curr_renderer	= nullptr;
	SurfaceClass *curr_surface		= nullptr;

	DrawExtents.Set (0, 0, 0, 0);

	int offset = 0;
	//
	//	Loop over all the parts of the sentence
	//
	for (int index = 0; index < SentenceData.Count (); index ++) {
		SentenceDataStruct &data = SentenceData[index];

		//
		//	Has the surface changed?
		//
		if (data.Surface != curr_surface) {
			curr_surface = data.Surface;

			//
			//	Try to find a renderer that uses the same "texture"
			//
			bool found = false;
			for (int renderer_index = 0; renderer_index < Renderers.Count (); renderer_index ++) {
				if (Renderers[renderer_index].Surface == curr_surface) {
					found = true;
					curr_renderer = Renderers[renderer_index].Renderer;
					break;
				}
			}

			//
			//	Create a new renderer if we couldn't find an appropriate one
			//
			if (found == false) {

				//
				//	Allocate a new renderer
				//
				curr_renderer = W3DNEW Render2DClass;
				curr_renderer->Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution ());
				ShaderClass *curr_shader = curr_renderer->Get_Shader ();
				(*curr_shader) = Shader;

				//
				//	Add it to our list
				//
				RendererDataStruct render_info;
				render_info.Renderer	= curr_renderer;
				render_info.Surface	= curr_surface;
				Renderers.Add (render_info);

				//
				//	Now, add this renderer to the surface pending list
				//
				for (int surface_index = 0; surface_index < PendingSurfaces.Count (); surface_index ++) {
					PendingSurfaceStruct &surface_info = PendingSurfaces[surface_index];
					if (surface_info.Surface == curr_surface) {
						surface_info.Renderers.Add (curr_renderer);
					}
				}
			}
		}

		//
		//	Get the dimensions of the surface
		//
		SurfaceClass::SurfaceDescription desc;
		curr_surface->Get_Description (desc);

		//
		//	Add a quad that contains this sentence chunk
		//
		RectClass screen_rect	= data.ScreenRect;
		screen_rect					+= Location;
		RectClass uv_rect			= data.UVRect;

		//
		//	Clip the quad (as necessary)
		//
		bool add_quad = true;
		if (IsClippedEnabled) {

			//
			//	Check for completely clipped
			//
			if (	screen_rect.Right <= ClipRect.Left ||
					screen_rect.Bottom <= ClipRect.Top)
			{
				add_quad = false;
			} else {

				//
				//	Clip the polygons to the specified area
				//
				RectClass clipped_rect;
				clipped_rect.Left		= max (screen_rect.Left, ClipRect.Left);
				clipped_rect.Right	= min (screen_rect.Right, ClipRect.Right);
				clipped_rect.Top		= max (screen_rect.Top, ClipRect.Top);
				clipped_rect.Bottom	= min (screen_rect.Bottom, ClipRect.Bottom);

				//
				//	Clip the texture to the specified area
				//
				RectClass clipped_uv_rect;
				float percent				= ((clipped_rect.Left - screen_rect.Left) / screen_rect.Width ());
				clipped_uv_rect.Left		= uv_rect.Left + (uv_rect.Width () * percent);

				percent						= ((clipped_rect.Right - screen_rect.Left) / screen_rect.Width ());
				clipped_uv_rect.Right	= uv_rect.Left + (uv_rect.Width () * percent);

				percent						= ((clipped_rect.Top - screen_rect.Top) / screen_rect.Height ());
				clipped_uv_rect.Top		= uv_rect.Top + (uv_rect.Height () * percent);

				percent						= ((clipped_rect.Bottom - screen_rect.Top) / screen_rect.Height ());
				clipped_uv_rect.Bottom	= uv_rect.Top + (uv_rect.Height () * percent);

				//
				//	Use the clipped rectangles to render
				//
				screen_rect = clipped_rect;
				uv_rect		= clipped_uv_rect;

				if (screen_rect.Right <= screen_rect.Left ||
						screen_rect.Bottom <= screen_rect.Top)
				{
					add_quad = false;
				}
			}
		}

		if (add_quad) {
			//uv_rect.Bottom += 0.5f;
			uv_rect *=  1.0F / ((float)desc.Width);
#ifdef TEST_PLACEMENT
			screen_rect.Left += offset*3;
			screen_rect.Right += offset*3;
#endif
			offset++;
			curr_renderer->Add_Quad (screen_rect, uv_rect, color);

			//
			//	Add this rectangle to the total draw extents
			//
			if (DrawExtents.Width () == 0) {
				DrawExtents = screen_rect;
			} else {
				DrawExtents += screen_rect;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Record_Sentence_Chunk
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Record_Sentence_Chunk ()
{
	//
	//	Do we have anything to store?
	//
	int width = TextureOffset.I - TextureStartX;
	if (width > 0) {
		float char_height = Font->Get_Char_Height ();

		//
		//	Build a structure that contains enough information
		// to hold this portion of the sentence
		//
		SentenceDataStruct sentence_data;
		sentence_data.Surface = CurSurface;
		sentence_data.Surface->Add_Ref ();
		sentence_data.ScreenRect.Left		= Cursor.X;
		sentence_data.ScreenRect.Right	= Cursor.X + width;
		sentence_data.ScreenRect.Top		= Cursor.Y;
		sentence_data.ScreenRect.Bottom	= Cursor.Y + char_height;
		sentence_data.UVRect.Left			= TextureStartX;
		sentence_data.UVRect.Top			= TextureOffset.J;
		sentence_data.UVRect.Right			= TextureOffset.I;
		sentence_data.UVRect.Bottom		= TextureOffset.J + char_height;

		//
		//	Add this information to our list
		//
		SentenceData.Add (sentence_data);
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_New_Surface
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Allocate_New_Surface (const WCHAR *text, bool justCalcExtents)
{
	if (!justCalcExtents)
	{
		//
		//	Unlock the last surface (if necessary)
		//
		if (LockedPtr != nullptr) {
			CurSurface->Unlock ();
			LockedPtr = nullptr;
		}
	}

	//
	// Calculate the width of the text and of its widest glyph
	//
	int text_width = 0;
	int max_char_width = 0;
	for (int index = 0; text[index] != 0; index ++) {
		text_width += Font->Get_Char_Spacing (text[index]);
		const int char_width = Font->Get_Char_Width (text[index]);
		max_char_width = max (max_char_width, char_width);
	}

	int char_height = Font->Get_Char_Height ();

	//
	//	TheSuperHackers @fix The texture must hold at least one row of glyphs and the widest glyph
	//	of this text, otherwise the blit runs off the surface. The widest glyph the font can produce
	//	is no use here, because fonts such as Arial report one several times wider than their text.
	//	Fonts that fit search 64 to 256 px, which bounds the texture that every display string
	//	allocates for itself. Larger fonts use the smallest texture that fits.
	//
	constexpr const int TextureSizeMinPow2 = 6; // 64 px, smallest texture
	constexpr const int TextureSizeSearchMaxPow2 = 8; // 256 px, largest texture searched for fonts that fit
	constexpr const int TextureSizeMaxPow2 = 11; // 2048 px, largest texture for any font

	const int min_extent = max (char_height + 1, max_char_width + TEXTURE_OFFSET + 1);
	int min_pow2 = TextureSizeMinPow2;
	while (min_pow2 < TextureSizeMaxPow2 && (1 << min_pow2) < min_extent) {
		min_pow2 ++;
	}

	int max_pow2 = max (TextureSizeSearchMaxPow2, min_pow2);
	if (max_pow2 > TextureSizeSearchMaxPow2) {
		//
		//	The font does not fit the search range, so use the smallest texture that fits, limited to
		//	the largest texture the device supports.
		//
		const D3DCAPS8 &dx8caps = DX8Wrapper::Get_Current_Caps ()->Get_DX8_Caps ();
		const int max_device_size = (int)min (dx8caps.MaxTextureWidth, dx8caps.MaxTextureHeight);
		while (max_pow2 > TextureSizeMinPow2 && (1 << max_pow2) > max_device_size) {
			max_pow2 --;
		}
		min_pow2 = min (min_pow2, max_pow2);
	}

	//
	//	Find the best texture size for the remaining text
	//
	CurrTextureSize = 1 << min_pow2;
	int best_tex_mem_usage = 999999999;
	for (int pow2 = min_pow2; pow2 <= max_pow2; pow2 ++) {

		int size					= 1 << pow2;
		int row_count			= (text_width / size) + 1;
		int rows_per_texture	= size / (char_height + 1);

		//
		//	Can we even fit one character on this texture?
		//
		if (rows_per_texture > 0) {

			//
			//	How many textures (at this size) would it take to render
			// the remaining text?
			//
			int texture_count	= row_count / rows_per_texture;
			texture_count		= max (texture_count, 1);

			//
			//	Is this the best usage of texture memory we've found yet?
			//
			int texture_mem_usage = (texture_count * size * size);
			if (texture_mem_usage < best_tex_mem_usage) {
				CurrTextureSize		= size;
				best_tex_mem_usage	= texture_mem_usage;
			}
		}
	}

	//
	//	Use whichever is larger, the hint or the calculated size
	//
	CurrTextureSize = max (TextureSizeHint, CurrTextureSize);

	if (!justCalcExtents)
	{
		//
		//	Release our extra hold on the old surface
		//
		REF_PTR_RELEASE (CurSurface);

		//
		//	Create the new surface
		//
		CurSurface = NEW_REF (SurfaceClass, (CurrTextureSize, CurrTextureSize, WW3D_FORMAT_A4R4G4B4));
		WWASSERT (CurSurface != nullptr);
		CurSurface->Add_Ref ();

		//
		//	Add this surface to our list
		//
		PendingSurfaceStruct surface_info;
		surface_info.Surface = CurSurface;
		PendingSurfaces.Add (surface_info);
	}

	//
	//	Reset to the upper left corner
	//
	TextureOffset.Set (0, 0);
	TextureStartX = 0;
}

float FindStartingXPos( const WCHAR *text )
{

	return 1;
}
////////////////////////////////////////////////////////////////////////////////////
//
//	Build_Sentence_Centered
//
////////////////////////////////////////////////////////////////////////////////////
void	Render2DSentenceClass::Build_Sentence_Centered (const WCHAR *text, int *hkX, int *hkY)
{
	float char_height = Font->Get_Char_Height ();
	int notCenteredHotkeyX = 0;
	int notCenteredHotkeyY = 0;
	Vector2 extent = Build_Sentence_Not_Centered(text,&notCenteredHotkeyX, &notCenteredHotkeyY, TRUE); //Get_Formatted_Text_Extents(text);

	//
	//	Start fresh
	//
	Reset_Sentence_Data ();
	Cursor.Set (0, 0);

	//
	//	Ensure we have a surface to start with
	//
	if (CurSurface == nullptr) {
		Allocate_New_Surface (text);
	}



	//
	//	Loop over all the characters in the string
	//
	bool end = false;
	const WCHAR *word;
	int word_width	= 0;
	int line_width	= 0;
	int charCount = 0;
	int wordCount = 0;
	int hotKeyPosX = 0;
	int hotKeyPosY = 0;
	bool calcHotKeyX = false;
	bool dontBlit = false;
	while (!end)
	{
		//
		// Re-init everything for the next line
		//
		word	= text;
		word_width	= 0;
		line_width	= 0;
		charCount = 0;
		wordCount = 0;
		//
		//first find the length of the line till we wrap
		//
		while ( 1 )
		{
			//
			// read a word
			//
			int word_spacing = 0;
			while ((*word != 0) && (*word > L' ') && (*word != L'\n')) {
				if( ParseHotKey && (*word == L'&') && (*word+1 != 0) && (*word+1 > L' ') && (*word+1 != L'\n'))
				{
					int offset = 0;
					if (word_width != 0 )
					{
						const WCHAR *word_back = word;
						*word_back--;
						if (*word_back == L' ')
						{
							line_width -= word_width;
							offset =-1;
						}
					}
					*word++;
					calcHotKeyX = true;
				}

				word_spacing = Font->Get_Char_Spacing (*word++);
				word_width += word_spacing;
				wordCount++;

				if (WrapWidth > 0 && word_width >= WrapWidth && useHardWordWrap)
					break;
			}
			//
			// If this word is unworthy to be on the current line, decrement the space and break
			//
			if(WrapWidth > 0 && (line_width + word_width >= WrapWidth))
			{
				//
				//Take care of the case that the word is too big for the allocated space...
				//If that's the case, drop out and process the word anyway
				//
				if(charCount == 0)
				{
					charCount +=wordCount - 1;
					line_width += word_width - word_spacing;
					if(*word == 0)
						end = true;
					break;
				}
				charCount--;
				break;
			}
			//
			// if we reached the end of the text, set the values and break, also set the end flag
			//
			if( *word == 0 )
			{
				charCount +=wordCount;
				line_width += word_width;
				end = true;
				break;
			}
			//
			// otherwise, increment the counts
			//
			charCount +=wordCount + 1;
			line_width += word_width;
			//
			// We were some a new line character break and process
			//
			if(*word != L' ')
				break;
			//
			// add the space to our width
			//
			word_width = Font->Get_Char_Spacing (*word++);
			wordCount = 0;
			line_width += word_width;
		}
		//
		// we now hold the length of the line and it's width lets set our cursor position to center it
		//
		Cursor.X = (int)((extent.X - line_width) / 2);
		if(Cursor.X < 0)
			Cursor.X = 0;
		if(calcHotKeyX)
		{
			calcHotKeyX = false;
			hotKeyPosX = Cursor.X + notCenteredHotkeyX;
		}

		for(int i = 0; i <= charCount; i++) {
			// TheSuperHackers @fix The text still to be placed, starting at ch, so a new surface is sized for ch as well.
			const WCHAR *remaining_text = text;
			WCHAR ch = *text++;
			dontBlit = false;
			//
			//	Determine how much horizontal space this character requires
			//
			if(ParseHotKey && (ch == L'&') && (*text != 0) && (*text > L' ') && (*text != L'\n'))
			{
				remaining_text = text;
				ch = *text++;
				dontBlit = true;
			}
			int char_spacing = Font->Get_Char_Spacing (ch);
			const int char_width = Font->Get_Char_Width (ch);

			bool exceeded_texture_width	= ((TextureOffset.I + char_width) >= CurrTextureSize);
			bool encountered_break_char	= (ch == L' ' || ch == L'\n' || ch == 0);

			//
			//	Do we need to record this portion of the sentence to its own chunk?
			//
			if (exceeded_texture_width || encountered_break_char) {
				Record_Sentence_Chunk ();

				//
				//	Adjust the positions
				//
				Cursor.X			+= (TextureOffset.I - TextureStartX);
				TextureStartX	= TextureOffset.I;

				//
				//	Adjust the output coordinates
				//
				if (ch == L' ') {
					Cursor.X += char_spacing;
				} else if ((ch == 0 )|| (ch == L'\n')) {
					break;
				}

				//
				//	Did the text extend past the edge of the texture?
				//
				if (exceeded_texture_width) {
					TextureStartX		= 0;
					TextureOffset.I	= TextureStartX;
					TextureOffset.J	+= char_height;

					//
					//	Did the text extent completely off the texture?
					//
					if ((TextureOffset.J + char_height) >= CurrTextureSize) {
						Allocate_New_Surface (remaining_text);
					}
				}
			}
			//
			//	Adjust the output coordinates
			//
			if (ch != L'\n' && ch != L' ') {

				//
				//	Ensure the surface is locked
				//
				if (LockedPtr == nullptr) {
					LockedPtr = (uint16 *)CurSurface->Lock (&LockedStride);
					WWASSERT (LockedPtr != nullptr);
				}

				//
				//	Check to ensure the text will fit on this texture
				//
				const bool fits_texture = ((TextureOffset.I + char_width) < CurrTextureSize) && ((TextureOffset.J + char_height) < CurrTextureSize);
				WWASSERT (fits_texture);

				//
				//	Blit the character to the surface
				//	TheSuperHackers @fix Skip a glyph that still does not fit.
				//
				if(!dontBlit && fits_texture)
					Font->Blit_Char (ch, LockedPtr, LockedStride, TextureOffset.I, TextureOffset.J);

				if (dontBlit) {
					// we don't blit for a hot key character.  So add extra spacing.
					char_spacing += Font->Get_Extra_Overlap();
					// Brutal hack #27 Gamma - Bolded M's are just a problem.	jba.
					if (ch=='M') {
						char_spacing++;
					}
				}

				TextureOffset.I += char_spacing;
			}
		}
		//
		// reset our cursor and add a line of text to the cursor position
		//
		Cursor.X = 0;
		Cursor.Y += char_height;
		line_width = 0;
		}

		if(hkX)
			*hkX = hotKeyPosX;
		if(hkX)
			*hkY = hotKeyPosY;
}
////////////////////////////////////////////////////////////////////////////////////
//
//	Build_Sentence_NotCentered
//
////////////////////////////////////////////////////////////////////////////////////
Vector2	Render2DSentenceClass::Build_Sentence_Not_Centered (const WCHAR *text, int *hkX, int *hkY, bool justCalcExtents)
{
	Vector2 cursor = Cursor;
	int textureStartX = TextureStartX;
	float maxX = 0;

	int hotKeyPosX = 0;
	int hotKeyPosY = 0;
	bool calcHotKeyX = false;
	bool dontBlit = false;
	Vector2i textureOffset = TextureOffset;
	int currTextureSize = CurrTextureSize;


	//
	//	Start fresh
	//
	if (!justCalcExtents)
	{
		Reset_Sentence_Data ();
	}
	Cursor.Set (0, 0);

	//
	//	Ensure we have a surface to start with
	//
	if (CurSurface == nullptr) {
		Allocate_New_Surface (text, justCalcExtents);
	}

	TextureOffset.Set (TEXTURE_OFFSET, 0);
	TextureStartX = TEXTURE_OFFSET;

	float char_height = Font->Get_Char_Height ();

	//
	//	Loop over all the characters in the string
	//
	while (text != nullptr) {
		// TheSuperHackers @fix The text still to be placed, starting at ch, so a new surface is sized for ch as well.
		const WCHAR *remaining_text = text;
		WCHAR ch = *text++;
		dontBlit = false;
		//
		//	Determine how much horizontal space this character requires
		//
		if(ParseHotKey && (ch == L'&') && (*text != 0) && (*text > L' ') && (*text != L'\n'))
		{
				hotKeyPosY = Cursor.Y;
			if (calcHotKeyX)
				hotKeyPosX = 0;
			else
				hotKeyPosX = Cursor.X + TextureOffset.I -TextureStartX;//TextureOffset.I;

			remaining_text = text;
			ch = *text++;
			dontBlit = true;
		}
		int char_spacing = Font->Get_Char_Spacing (ch);
		const int char_width = Font->Get_Char_Width (ch);

		bool exceeded_texture_width	= ((TextureOffset.I + char_width) >= CurrTextureSize);
		bool encountered_break_char	= (ch == L' ' || ch == L'\n' || ch == 0);
		bool wordBiggerThenLine = ((useHardWordWrap) && ( WrapWidth != 0 ) &&((Cursor.X + TextureOffset.I -TextureStartX + char_spacing) >= WrapWidth));
		//
		//	Do we need to record this portion of the sentence to its own chunk?
		//
		if (exceeded_texture_width || encountered_break_char|| wordBiggerThenLine) {
			if (!justCalcExtents)
			{
				Record_Sentence_Chunk ();
			}

			//
			//	Adjust the positions
			//
			Cursor.X			+= (TextureOffset.I - TextureStartX);
			maxX = max(maxX, Cursor.X);
			TextureStartX	= TextureOffset.I;

			//
			//	Adjust the output coordinates
			//
			if (ch == L' ') {
				//Cursor.X += char_spacing;
				//maxX = max(maxX, Cursor.X);

				//
				// Check to see if we need to wrap on this word-break
				//
				if (WrapWidth > 0) {

					//
					//	Find the length of the next word
					//
					const WCHAR *word	= text;
					float word_width	= char_spacing;
					while ((*word != 0) && (*word > L' ')) {
						if(ParseHotKey && (*word == L'&') && (*word+1 != 0) && (*word+1 > L' ') && (*word+1 != L'\n'))
							*word++;
						word_width += Font->Get_Char_Spacing (*word++);
					}

					//
					//	Should we wrap the next word?
					//
					if ((Cursor.X + word_width) >= WrapWidth) {
						Cursor.X = 0;
						Cursor.Y += char_height;
						calcHotKeyX = true;
					}
				}

			} else if (ch == L'\n') {
				Cursor.X = 0;
				Cursor.Y += char_height;
			} else if (ch == 0) {
				break;
			} else if (wordBiggerThenLine){ // we've entered this loop because we're greater then the wordwrap so we need to force a wordwrap
				Cursor.X = 0;
				Cursor.Y += char_height;
			}


			//
			//	Did the text extend past the edge of the texture?
			//
			if (exceeded_texture_width) {
				TextureStartX		= TEXTURE_OFFSET;
				TextureOffset.I	= TextureStartX;
				TextureOffset.J	+= char_height;

				//
				//	Did the text extent completely off the texture?
				//
				if ((TextureOffset.J + char_height) >= CurrTextureSize) {
					Allocate_New_Surface (remaining_text, justCalcExtents);
				}
			}
		}

		if (ch != L'\n' ) {

			//
			//	Ensure the surface is locked
			//
			if (!justCalcExtents)
			{
				if (LockedPtr == nullptr) {
					LockedPtr = (uint16 *)CurSurface->Lock (&LockedStride);
					WWASSERT (LockedPtr != nullptr);
				}
			}

			//
			//	Check to ensure the text will fit on this texture
			//
			const bool fits_texture = ((TextureOffset.I + char_width) < CurrTextureSize) && ((TextureOffset.J + char_height) < CurrTextureSize);
			WWASSERT (fits_texture);

			//
			//	Blit the character to the surface
			//	TheSuperHackers @fix Skip a glyph that still does not fit.
			//
			if (!justCalcExtents && !dontBlit && fits_texture)
			{
				Font->Blit_Char (ch, LockedPtr, LockedStride, TextureOffset.I, TextureOffset.J);
			}
			TextureOffset.I += char_spacing;
		}
	}

	Vector2 extent;
	extent.X = maxX + Font->Get_Extra_Overlap();
	extent.Y = Cursor.Y + char_height;

	Cursor = cursor;
	TextureOffset = textureOffset;
	TextureStartX = textureStartX;

	//
	//	TheSuperHackers @fix Measuring allocates no surface, so it must not leave behind the texture
	//	size of a surface it did not create. The next pass would otherwise check its glyphs against a
	//	size that differs from the surface it actually locks.
	//
	if (justCalcExtents) {
		CurrTextureSize = currTextureSize;
	}

	if(hkX)
		*hkX = hotKeyPosX;
	if(hkX)
		*hkY = hotKeyPosY;

	return extent;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Build_Sentence
//
////////////////////////////////////////////////////////////////////////////////////
void
Render2DSentenceClass::Build_Sentence (const WCHAR *text, int *hkX, int *hkY)
{
	if (text == nullptr) {
		return ;
	}

	if (Font == nullptr)
		return;

	if(Centered && (WrapWidth > 0 || wcschr(text,L'\n')))
		Build_Sentence_Centered(text, hkX, hkY);
	else
		Build_Sentence_Not_Centered(text, hkX, hkY);

}


////////////////////////////////////////////////////////////////////////////////////
//
//	FontCharsClass
//
////////////////////////////////////////////////////////////////////////////////////
FontCharsClass::FontCharsClass () :
	OldGDIFont(	nullptr ),
	OldGDIBitmap( nullptr ),
	GDIFont( nullptr ),
	GDIBitmap( nullptr ),
	GDIBitmapBits ( nullptr ),
	MemDC( nullptr ),
	CurrPixelOffset( 0 ),
	PointSize( 0 ),
	CharHeight( 0 ),
	GlyphBitmapWidth( 0 ),
	GlyphBitmapHeight( 0 ),
	GlyphCellBytes( 0 ),
	GlyphBlockBytes( 0 ),
	UnicodeCharArray( nullptr ),
	FirstUnicodeChar( 0xFFFF ),
	LastUnicodeChar( 0 ),
	IsBold (false)
{
	AlternateUnicodeFont = nullptr;
	::memset( ASCIICharArray, 0, sizeof (ASCIICharArray) );
}


////////////////////////////////////////////////////////////////////////////////////
//
//	~FontCharsClass
//
////////////////////////////////////////////////////////////////////////////////////
FontCharsClass::~FontCharsClass ()
{
	Free_Glyph_Cache();
	Free_GDI_Font();
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Glyph_Cache
//	Discards the cached glyphs but keeps the font itself, so that the pointers other
//	objects hold to this font stay valid and glyphs are rebuilt on demand.
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Free_Glyph_Cache ()
{
	while ( BufferList.Count() ) {
		delete [] BufferList[0].Buffer;
		BufferList.Delete(0);
	}

	Free_Character_Arrays();

	//
	//	The character arrays are gone, so the unicode range has to start over as well.
	//	The GDI font and the derived metrics are deliberately kept, so Store_GDI_Char
	//	can rebuild any glyph that is asked for again without recreating this object.
	//
	CurrPixelOffset = 0;
	FirstUnicodeChar = 0xFFFF;
	LastUnicodeChar = 0;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Char_Data
//
////////////////////////////////////////////////////////////////////////////////////
const FontCharsClassCharDataStruct *
FontCharsClass::Get_Char_Data (WCHAR ch)
{
	const FontCharsClassCharDataStruct *retval = nullptr;

	if ( ch < 256 )
	{
		retval = ASCIICharArray[ch];
	}
 	else if ( AlternateUnicodeFont && this != AlternateUnicodeFont )
	{
		return AlternateUnicodeFont->Get_Char_Data( ch );
	}
	else
	{
		Grow_Unicode_Array( ch );
		retval = UnicodeCharArray[ch - FirstUnicodeChar];
	}

	//
	//	If the character wasn't found, then add it to our list
	//
	if ( retval == nullptr ) {
		retval = Store_GDI_Char( ch );
	}

	WWASSERT( retval->Value == ch );
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Char_Width
//
////////////////////////////////////////////////////////////////////////////////////
int
FontCharsClass::Get_Char_Width (WCHAR ch)
{
	const FontCharsClassCharDataStruct	* data = Get_Char_Data( ch );
	if ( data != nullptr ) {
		return data->Width;
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Char_Spacing
//
////////////////////////////////////////////////////////////////////////////////////
int
FontCharsClass::Get_Char_Spacing (WCHAR ch)
{
	const FontCharsClassCharDataStruct	* data = Get_Char_Data( ch );
	if ( data != nullptr ) {
		if ( data->Width != 0 ) {
			return data->Width - PixelOverlap - CharOverhang;
		}
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Blit_Char
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Blit_Char (WCHAR ch, uint16 *dest_ptr, int dest_stride, int x, int y)
{
	const FontCharsClassCharDataStruct	* data = Get_Char_Data( ch );
	if ( data != nullptr && data->Width != 0 ) {

		//
		//	Setup the src and destination pointers
		//
		int dest_inc		= (dest_stride >> 1);
		const uint8 *src_ptr = data->Buffer;
		dest_ptr				+= (dest_inc * y) + x;

		//
		//	Copy the data from the src buffer to the destination, rebuilding the A4R4G4B4 texel
		//	from the stored coverage value the same way Store_GDI_Char used to compose it.
		//
		for ( int row = 0; row < CharHeight; row ++ ) {
			for ( int col = 0; col < data->Width; col ++ ) {
				const uint8 coverage = *src_ptr;
				uint16 curData = (coverage != 0 ? 0x0FFF : 0) | ((uint16)(coverage >> 4) << 12);
				if (col<PixelOverlap) {
					curData |= dest_ptr[col];
				}
				dest_ptr[col] = curData;
				src_ptr++;
			}
			dest_ptr	+= dest_inc;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Store_GDI_Char
//
////////////////////////////////////////////////////////////////////////////////////
const FontCharsClassCharDataStruct *
FontCharsClass::Store_GDI_Char (WCHAR ch)
{
	const int width = GlyphBitmapWidth;
	const int height = GlyphBitmapHeight;

	//
	//	Draw the character into the memory DC
	//
	RECT rect = { 0, 0, width, height };
	int xOrigin = 0;
	if (ch == 'W') {
		xOrigin = 1;
	}
	::ExtTextOutW( MemDC, xOrigin, 0, ETO_OPAQUE, &rect, &ch, 1, nullptr);

	//
	//	Get the size of the character we just drew
	//
	SIZE char_size = { 0 };
	::GetTextExtentPoint32W( MemDC, &ch, 1, &char_size );
	char_size.cx += PixelOverlap + xOrigin;

	//
	//	TheSuperHackers @fix ExtTextOutW clipped the glyph to the scratch bitmap, so the copy
	//	below must not read beyond it either, whatever extent GDI reports. A malformed font can
	//	report nothing at all, which leaves a character of zero width that everything skips.
	//
	char_size.cx = min (max ((int)char_size.cx, 0), width);
	char_size.cy = min (max ((int)char_size.cy, 0), height);

	//
	//	Get a pointer to the surface that this character should use
	//
	Update_Current_Buffer( char_size.cx );
	uint8* glyph_buffer_p = BufferList[BufferList.Count () - 1].Buffer + CurrPixelOffset;
	uint8* curr_buffer_p = glyph_buffer_p;

	//
	//	Copy the BMP contents to the buffer
	//
	int stride = (((width * 3) + 3) & ~3);
	for (int row = 0; row < char_size.cy; row ++) {

		//
		//	Compute the indices into the BMP and surface
		//
		int index = (row * stride);

		//
		//	Loop over each column
		//
		for (int col = 0; col < char_size.cx; col ++) {

			//
			//	Get the pixel color at this location
			//
			uint8 pixel_value = GDIBitmapBits[index];
			index += 3;
#ifdef TEST_PLACEMENT
 			if (row==CharHeight-1&&col==0) {
 				pixel_value = 0xff;
 			}
 			if (row==CharHeight-2&&col==1) {
 				pixel_value = 0xff;
 			}
 			if (row==0&&col==0) {
 				pixel_value = 0xff;
 			}
 			if (row==1&&col==1) {
 				pixel_value = 0xff;
 			}
 			if (row==CharHeight-1&&col==char_size.cx-1-PixelOverlap) {
 				pixel_value = 0xff;
 			}
 			if (row==CharHeight-2&&col==char_size.cx-2-PixelOverlap) {
 				pixel_value = 0xff;
 			}
 			if (row==0&&col==char_size.cx-1-PixelOverlap) {
 				pixel_value = 0xff;
 			}
 			if (row==1&&col==char_size.cx-2-PixelOverlap) {
 				pixel_value = 0xff;
 			}
 			if (pixel_value == 0x00) {
 				pixel_value = 0x40;
 			}
#endif

			//
			//	Store the raw intensity.
			//	TheSuperHackers @tweak The glyph is cached as one byte of GDI coverage per pixel.
			//	Blit_Char rebuilds the A4R4G4B4 texel from it, which is exact because the stored
			//	color only ever depends on whether the coverage is zero.
			//
			*curr_buffer_p++ = pixel_value;
		}
	}

	//
	//	TheSuperHackers @fix Blit_Char always reads CharHeight rows, so any row GDI did not
	//	report must not be left at whatever the freshly allocated block happened to contain.
	//
	if (char_size.cy < CharHeight) {
		::memset (curr_buffer_p, 0, (CharHeight - char_size.cy) * char_size.cx);
	}

	//
	//	Save information about this character in our list
	//
	FontCharsClassCharDataStruct *char_data	= W3DNEW FontCharsClassCharDataStruct;
	char_data->Value				= ch;
	char_data->Width				= char_size.cx;
	char_data->Buffer				= glyph_buffer_p;

	//
	//	Insert this character into our array
	//
	if ( ch < 256 ) {
		ASCIICharArray[ch] = char_data;
	} else {
		UnicodeCharArray[ch - FirstUnicodeChar] = char_data;
	}

	//
	//	Advance the character position. This matches both what Update_Current_Buffer reserved and
	//	what Blit_Char reads back; char_size.cx already includes PixelOverlap.
	//
	CurrPixelOffset += (char_size.cx * CharHeight);

	//
	//	Return the index of the entry we just added
	//
	return char_data;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Current_Buffer
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Update_Current_Buffer (int char_width)
{
	const int char_len = char_width * CharHeight;

	//
	//	Check to see if we need to allocate a new buffer
	//
	bool needs_new_buffer = (BufferList.Count () == 0);
	if (needs_new_buffer == false) {

		//
		//	Would we extend past this buffer?
		//
		if ( (CurrPixelOffset + char_len) > BufferList[BufferList.Count () - 1].Length ) {
			needs_new_buffer = true;
		}
	}

	//
	//	Do we need to create a new surface?
	//
	if (needs_new_buffer)
	{
		//
		//	TheSuperHackers @fix Ceil the block size to the char size to make it fit.
		//	TheSuperHackers @tweak Ramp the first blocks up to the full size, because a font whose
		//	working set is a handful of glyphs would otherwise pay for a whole block of them.
		//	Halving rather than one small first block is what keeps such a font from being pushed
		//	into a full sized second block.
		//
		const int shift = 2 - min( 2, BufferList.Count() );
		const int length = max( GlyphBlockBytes >> shift, GlyphCellBytes );
		WWASSERT( char_len <= length );

		BufferList.Add( FontCharsBuffer( length, W3DNEWARRAY uint8[length] ) );
		CurrPixelOffset = 0;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Create_GDI_Font
//
////////////////////////////////////////////////////////////////////////////////////
bool
FontCharsClass::Create_GDI_Font (const char *font_name)
{
	HDC screen_dc = ::GetDC ((HWND)WW3D::Get_Window());

	const char *fontToUseForGenerals = "Arial";
	bool doingGenerals = false;
	if (strcmp(font_name, "Generals")==0) {
		font_name = fontToUseForGenerals;
		doingGenerals = true;
	}

	//
	//	Calculate the height of the font in logical units
	//
	const int dotsPerInch = 96; // always use 96.	jba.
	int font_height = -MulDiv (PointSize, dotsPerInch, 72);

	int fontWidth = 0; // use font default.
	if (doingGenerals) {
		//fontWidth = -font_height*0.35f; //2 pixels tighter.
		fontWidth = -font_height*0.40f; // one pixel tighter
	}
	PixelOverlap = (-font_height)/8;

	// Sanity check in case of perversion. :)
	if (PixelOverlap<0) PixelOverlap = 0;
	if (PixelOverlap>4) PixelOverlap = 4;
	//
	//	Create the Windows font
	//
	DWORD bold		= IsBold ? FW_BOLD : FW_NORMAL;
	DWORD italic	= 0;
	GDIFont			= ::CreateFont (font_height, fontWidth, 0, 0, bold, italic,
								FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
								VARIABLE_PITCH, font_name);

	//
	//	Create a device context we can select the font and bitmap into
	//
	MemDC = ::CreateCompatibleDC (screen_dc);

	//
	//	TheSuperHackers @fix Select the font and read its metrics before creating the scratch
	//	bitmap below, because that bitmap is sized from them. The point size alone cannot give a
	//	safe size: a font is free to report a tmHeight or a tmMaxCharWidth larger than any guess
	//	made from it, and Store_GDI_Char copies as many rows and columns as GDI reports.
	//
	OldGDIFont = (HFONT)::SelectObject (MemDC, GDIFont);

	//
	//	Lookup the pixel height of the font
	//
	TEXTMETRIC text_metric = { 0 };
	::GetTextMetrics (MemDC, &text_metric);
	CharHeight = text_metric.tmHeight;
	CharAscent = text_metric.tmAscent;
	CharOverhang = text_metric.tmOverhang;
	if (doingGenerals) {
		CharOverhang = 0;
	}

	//
	//	The scratch bitmap must hold the widest glyph, the overlap column that Store_GDI_Char
	//	appends to it and the one pixel it shifts 'W' by.
	//
	GlyphBitmapWidth = (int)text_metric.tmMaxCharWidth + max ((int)text_metric.tmOverhang, 0) + PixelOverlap + 1;

	// Sanity check. A font reporting absurd metrics renders clipped
	// rather than allocating an absurd bitmap and absurd glyph buffers.
	const int max_glyph_extent = PointSize * 4 + 8;
	GlyphBitmapWidth = min (max (GlyphBitmapWidth, 1), max_glyph_extent);
	CharHeight = min (max (CharHeight, 1), max_glyph_extent);
	GlyphBitmapHeight = CharHeight;

	//
	//	TheSuperHackers @tweak Size the glyph cache blocks from the widest glyph this font can produce,
	//	so that a block always holds a whole number of glyphs and the space abandoned when one does not
	//	fit is at most one glyph. A block always fits at least one glyph, however large the font is.
	//
	GlyphCellBytes = GlyphBitmapWidth * GlyphBitmapHeight;
	GlyphBlockBytes = GlyphCellBytes * GLYPH_BLOCK_TARGET_CELLS;
	GlyphBlockBytes = min (max (GlyphBlockBytes, (int)GLYPH_BLOCK_MIN_BYTES), (int)GLYPH_BLOCK_MAX_BYTES);
	GlyphBlockBytes = max (GlyphBlockBytes, GlyphCellBytes);

	//
	// Set-up the fields of the BITMAPINFOHEADER
	//	Note: Top-down DIBs use negative height in Win32.
	//
	BITMAPINFOHEADER bitmap_info = { 0 };
	bitmap_info.biSize				= sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth				= GlyphBitmapWidth;
	bitmap_info.biHeight			= -GlyphBitmapHeight;
	bitmap_info.biPlanes				= 1;
	bitmap_info.biBitCount			= 24;
	bitmap_info.biCompression		= BI_RGB;
	bitmap_info.biSizeImage			= (((GlyphBitmapWidth * 3) + 3) & ~3) * GlyphBitmapHeight;
	bitmap_info.biXPelsPerMeter	= 0;
	bitmap_info.biYPelsPerMeter	= 0;
	bitmap_info.biClrUsed			= 0;
	bitmap_info.biClrImportant		= 0;

	//
	// Create a bitmap that we can access the bits directly of
	//
	GDIBitmap	= ::CreateDIBSection (	screen_dc,
													(const BITMAPINFO *)&bitmap_info,
													DIB_RGB_COLORS,
													(void **)&GDIBitmapBits,
													nullptr,
													0L);

	//
	// Release our temporary screen DC
	//
	::ReleaseDC ((HWND)WW3D::Get_Window(), screen_dc);

	//
	//	Now select the BMP into the DC
	//
	OldGDIBitmap	= (HBITMAP)::SelectObject (MemDC, GDIBitmap);
	::SetBkColor (MemDC, RGB (0, 0, 0));
	::SetTextColor (MemDC, RGB (255, 255, 255));

	return GDIFont != nullptr && GDIBitmap != nullptr;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Free_GDI_Font
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Free_GDI_Font ()
{
	//
	//	Select the old font back into the DC and delete
	// our font object
	//
	if ( GDIFont != nullptr ) {
		::SelectObject( MemDC, OldGDIFont );
		::DeleteObject( GDIFont );
		GDIFont = nullptr;
	}

	//
	//	Select the old bitmap back into the DC and delete
	// our bitmap object
	//
	if ( GDIBitmap != nullptr ) {
		::SelectObject( MemDC, OldGDIBitmap );
		::DeleteObject( GDIBitmap );
		GDIBitmap = nullptr;
	}

	//
	//	Delete our memory DC
	//
	if ( MemDC != nullptr ) {
		::DeleteDC( MemDC );
		MemDC = nullptr;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_GDI_Font
//
////////////////////////////////////////////////////////////////////////////////////
bool
FontCharsClass::Initialize_GDI_Font (const char *font_name, int point_size, bool is_bold)
{
	//
	//	Build a unique name from the font name and its size
	//
	Name.Format ("%s%d", font_name, point_size);

	//
	//	Remember these settings
	//
	GDIFontName	= font_name;
	PointSize	= point_size;
	IsBold		= is_bold;

	//
	//	Create the actual font object
	//
	return Create_GDI_Font (font_name);
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Is_Font
//
////////////////////////////////////////////////////////////////////////////////////
bool
FontCharsClass::Is_Font (const char *font_name, int point_size, bool is_bold)
{
	bool retval = false;

	//
	//	Check to see if both the name and height matches...
	//
	if (	(GDIFontName.Compare_No_Case (font_name) == 0) &&
			(point_size == PointSize) &&
			(is_bold == IsBold))
	{
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Grow_Unicode_Array
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Grow_Unicode_Array (WCHAR ch)
{
	//
	//	Don't do anything if character is in the ASCII range
	//
	if ( ch < 256 ) {
		return ;
	}

	//
	//	Don't do anything if character is in the currently allocated range
	//
	if ( ch >= FirstUnicodeChar && ch <= LastUnicodeChar ) {
		return ;
	}

	uint16 first_index	= min( FirstUnicodeChar, static_cast<uint16>(ch) );
	uint16 last_index		= max( LastUnicodeChar, static_cast<uint16>(ch) );
	uint16 count			= (last_index - first_index) + 1;

	//
	//	Allocate enough memory to hold the new cells
	//
	FontCharsClassCharDataStruct **new_array = W3DNEWARRAY FontCharsClassCharDataStruct *[count];
	::memset (new_array, 0, sizeof (FontCharsClassCharDataStruct *) * count);

	//
	//	Copy the contents of the old array into the new array
	//
	if ( UnicodeCharArray != nullptr ) {
		int start_offset	= (FirstUnicodeChar - first_index);
		int old_count		= (LastUnicodeChar - FirstUnicodeChar) + 1;
		::memcpy (&new_array[start_offset], UnicodeCharArray, sizeof (FontCharsClassCharDataStruct *) * old_count);

		//
		//	Delete the old array
		//
		delete [] UnicodeCharArray;
		UnicodeCharArray = nullptr;
	}

	FirstUnicodeChar	= first_index;
	LastUnicodeChar	= last_index;
	UnicodeCharArray	= new_array;
}


////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Character_Arrays
//
////////////////////////////////////////////////////////////////////////////////////
void
FontCharsClass::Free_Character_Arrays ()
{
	if ( UnicodeCharArray != nullptr ) {

		int count = (LastUnicodeChar - FirstUnicodeChar) + 1;

		//
		//	Delete each member of the unicode array
		//
		for (int index = 0; index < count; index ++) {
			delete UnicodeCharArray[index];
			UnicodeCharArray[index] = nullptr;
		}

		//
		//	Delete the array itself
		//
		delete [] UnicodeCharArray;
		UnicodeCharArray = nullptr;
	}

	//
	//	Delete each member of the ascii character array
	//
	for (int index = 0; index < 256; index ++) {
		delete ASCIICharArray[index];
		ASCIICharArray[index] = nullptr;
	}
}
