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

#ifndef TEXTUREFILE_H
#define TEXTUREFILE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "always.h"
#include "classid.h"
#include "wwdebug.h"
#include "wwstring.h"

#ifdef WW3D_DX8

#include <srTexture.hpp>

/*
** TextureFileClass: this replaces srTextureFile - it is the primary texture
** class used in WW3D (except for special and rare cases such as animating
** textures).
** A TextureFileClass can operate in one of two modes:
** 1) A simple mode in which every time the GERD requests the texture data
**    (via getMipmapData), the texture is reloaded from its file. This is
**    useful for cases in which the texture must always be seen at its full
**    size and we do not mind waiting for it to load (on startup, and
**    whenever it gets thrown out of the GERD/API/HW texture cache).
** 2) A mode in which there is a "locked surface" which is always present in
**    the texture. This surface is reduced by some factor relative to the
**    fullsize texture (NOTE: this factor may be 0, in which case the locked
**    texture IS the fullsize texture. This is similar to the srTextureFile's
**    "cached" mode.). In this mode the texture's desired reduction factor
**    changes dynamically according to the screen size of the object to which
**    it belongs. If the difference between the desired and current reduction
**    factors is large enough, the current reduction factor is adjusted to
**    match the desired one: either by using the locked surface (if the
**    desired reduction factor is equal or greater to that of the locked
**    surface), or by asking a background thread to load the surface at the
**    desired reduction factor.
*/
class TextureFileClass : public srClassSupport<TextureFileClass,srTexture,false,ID_TEXTURE_FILE_CLASS>
{
public:

	TextureFileClass(const char * filename);
	virtual ~TextureFileClass(void);

	// Copy CTor and assignment operator assert for now - if anyone hits the
	// assert we might need to actually implement them 8^).
	TextureFileClass(const TextureFileClass & src);
	TextureFileClass & TextureFileClass::operator = (const TextureFileClass &that);

	// srClass functions:
	virtual srClass* vInstance(void) { WWASSERT(0); return W3DNEW TextureFileClass(""); }

	// srTextureIFace functions:
	virtual FrameHandle getTextureFrameHandle(void);
	virtual void getMipmapData(MultiRequest& m);

	// srTexture functions:
	virtual void invalidate(void);
protected:
	virtual void setupDefaultValues (void);
public:

	const char * Get_File_Name(void) { return FileName; }

	void				Apply_New_Surface();

	// Performance statistics:
	static void		Switch_Mipmaping_Debug();
	static int		Get_Total_Locked_Surface_Size();
	static int		Get_Total_Texture_Size();
	static int		Get_Total_Non_Reduced_Texture_Size();	// Texture size if reduction was NOT used
	static int		Get_Total_Locked_Surface_Count();
	static int		Get_Total_Texture_Count();
	static StringClass List_Missing_Files();	// Print out a list of missing files
	int Get_Texture_Size() const { return TextureSize; }	// Actual size is in bytes, reduction applied
	int Get_Non_Reduced_Texture_Size() const { return NonReducedTextureSize; }	// Size is in bytes if texture reduction wasn't used
	int Get_Locked_Surface_Size() const { return LockedSurfaceSize; }	// Size is in bytes
	int Get_Current_Reduction_Factor() const { return CurrentReductionFactor; }
	int Get_File_Error() const { return file_error; }

	// This is used by an object to request a reduction factor on all its
	// textures. Only the smallest reduction factor requested in a given frame
	// is preserved. See comments for DesiredReductionFactor below on
	// interpretation of reduction_factor and why it nees to be a float.
	void Request_Reduction(float reduction_factor);

	// This is used during rendering - if a textures desired reduction level
	// is "different enough" from its current one, we fix it: either by
	// updating from the locked surface (if the reduction factor is equal or
	// greater to that of the locked surface) or by asking the texture loader
	// to load a new reduction level of the texture in the background.
	// NOTE - if there is no locked surface, we do nothing (since textures
	// without locked surfaces do not perform reduction).
	void Process_Reduction(void);

	// If false is passed, the texture will load the largest LOD size and stick to that
	void Enable_Reduction(bool b);

	// Reset the current time stamp to the next value (see comments for
	// _CurrentTimeStamp below)
	static void _Reset_Time_Stamp(void) { _CurrentTimeStamp++; }

	// See comments for _SwitchThreshold below:
	static void				_Set_Switch_Threshold(float switch_threshold);
	static float			_Get_Switch_Threshold(void);

	// Information structure for texture loader use only
	struct TextureLoaderInfoStruct {
		srColorSurfaceIFace* new_surface;
		TextureFileClass* succ;
		bool loading;
		unsigned int reduction_factor;
		TextureLoaderInfoStruct() : succ(0), new_surface(0), loading(false), reduction_factor(0U) {}
	} texture_loader_info;

	// Debug functions
	int ID() const { return id; }
	void Set_Texture_Flash(bool b);	// Make texture flash (Warning! Slow, for debug only!!!)
	bool Get_Texture_Flash() const;	// Return texture flash state
	static TextureFileClass* Get_Texture(int id);
	static void Update_Texture_Flash();
private:

	// Utility functions
	void				Load_Temp_Surface(void);
	void				Release_Temp_Surface(void);
	void				Load_Locked_Surface();
	void static		Fill_Multi_Request_From_Surface(MultiRequest& m, srColorSurfaceIFace* surface);

	// Locked Surface
	srColorSurfaceIFace * LockedSurface;
	unsigned int LockedSurfaceReductionFactor;

	TextureFileClass * Succ;

	// For performance statistics
	int LockedSurfaceSize;
	int TextureSize;
	int NonReducedTextureSize;

	/*
	** Texture reduction stuff:
	**
	** Reduction factors are the power of two by which the texture size must
	** be reduced: e.g. 3 means a reduction by 8 in both x and y.
	*/

	bool ReductionEnabled;

	// The current reduction factor reflects the texture size currently used
	// in rendering. 
	unsigned int CurrentReductionFactor;

	// The desired reduction factor is a fractional non-negative value. It
	// needs to be fractional for proper implementation of hysteresis (the
	// thresholds going up and down are different).
	float DesiredReductionFactor;

	// The time stamp is used for two purposes:
	// 1) Determining the first Request_Reduction() call in a frame
	// 2) Determining the first Process_Reduction() call in a frame
	static unsigned int	_CurrentTimeStamp;
	unsigned int			TimeStampOfLastRequestReductionCall;
	unsigned int			TimeStampOfLastProcessReductionCall;

	// This controls the degree of hysteresis when switching reduction levels.
	// It is the minimum difference between the current and desired reduction
	// factors to cause a switch. This number must be greater or equal to 0.5
	// (no hysteresis) and less than 1 (maximum hysteresis). In theory values
	// greater than 1 should be possible, producing more hysteresis, but then
	// there is a problem when switching to 0 (since the desired reduction
	// factor is clamped to 0. If in future we want greater amounts of
	// hysteresis, we should consider not clamping.)
	static float			_SwitchThreshold;

	// The texture's file name
	char *					FileName;

	// The texture's frame handle
	FrameHandle				TextureFrameHandle;

	// A temporary surface pointer only used to pass information from
	// Apply_New_Surface() to setupDefaultValues() and getMipmapData().
	srColorSurfaceIFace* TempSurfacePtr;

	bool file_error;
	int id;	// Used for debugging purposes (unique id)
	bool flash; // If true, texture will flash (debug!)
	srColorSurfaceIFace* flash_store_surface;
};

#endif //WW3D_DX8

#endif
