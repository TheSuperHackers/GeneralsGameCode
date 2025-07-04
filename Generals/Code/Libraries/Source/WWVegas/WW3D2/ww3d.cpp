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
 *                     $Archive:: /Commando/Code/ww3d2/ww3d.cpp                               $*
 *                                                                                             *
 *                   Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *								$Modtime:: 08/05/02 10:03a                                             $*
 *                                                                                             *
 *                    $Revision:: 98                                                          $*
 *                                                                                             *
 * 07/01/02 KM Scalable shader library integration				                               *
 * 08/05/02 KM Texture class redesign 
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WW3D::Init -- Initialize the WW3D Library                                                 *
 *   WW3D::Shutdown -- shutdown the WW3D Library                                               *
 *   WW3D::Set_Render_Device -- set the render device being currently used                     *
 *   WW3D::Set_Next_Render_Device -- just go to the next device in the list                    *
 *   WW3D::Set_Device_Resolution -- set the current resolution and bitdepth                    *
 *   WW3D::Get_Render_Device -- Get the index of the current render device                     *
 *   WW3D::Get_Render_Device_Desc -- returns description of the current render device          *
 *   WW3D::Get_Render_Device_Count -- returns the number of render devices available           *
 *   WW3D::Get_Render_Device_Name -- returns the name of the n-th render device                *
 *	  WW3D::Get_Render_Target_Resolution -- get the resolution and bitdepth of the current target*
 *   WW3D::Get_Device_Resolution -- get the current resolution and bitdepth                    *
 *   WW3D::Begin_Render -- mark the start of rendering for a new frame                         *
 *   WW3D::Render -- Render a 3D Scene using the given camera                                  *
 *   WW3D::Render -- Render a single render object                                             *
 *   WW3D::End_Render -- Mark the completion of a frame                                        *
 *   WW3D::Sync -- Time sychronization                                                         *
 *   WW3D::Set_Ext_Swap_Interval -- Sets the swap interval the device should aim sync for.     *
 *   WW3D::Get_Ext_Swap_Interval -- Queries the swap interval the device is aiming sync for.   *
 *   WW3D::Get_Polygon_Mode -- returns the current rendering mode                              *
 *   WW3D::Set_Collision_Box_Display_Mask -- control rendering of collision boxes              *
 *   WW3D::Get_Collision_Box_Display_Mask -- returns the current display mask for collision bo *
 *   WW3D::Normalize_Coordinates -- Convert pixel coords to normalized screen coords 0..1      *
 *   WW3D::Update_Render_Device_Description -- updates the description of the current render d *
 *   WW3D::Make_Screen_Shot -- saves a screenshot with the given base filename                 *
 *   WW3D::Start_Movie_Capture -- begins dumping frames to a movie                             *
 *   WW3D::Stop_Movie_Capture -- ends dumping frames to a movie                                *
 *   WW3D::Toggle_Movie_Capture -- toggles movie capture...                                    *
 *   WW3D::Start_Single_Frame_Movie_Capture -- starts capturing a single frame movie           *
 *   WW3D::Capture_Next_Movie_Frame -- tells ww3d to grab another frame for the movie          *
 *   WW3D::Pause_Movie -- pauses/unpauses movie capturing                                      *
 *   WW3D::Is_Movie_Paused -- returns whether the movie capture system is paused               *
 *   WW3D::Is_Recording_Next_Frame -- returns whether the next frame will be dumped to a movie *
 *   WW3D::Is_Movie_Ready -- returns whether the movie capture system is ready                 *
 *   WW3D::Update_Movie_Capture -- dumps the current frame into the movie                      *
 *   WW3D::Get_Movie_Capture_Frame_Rate -- returns the framerate at which the movie is being c *
 *   WW3D::Set_Texture_Reduction -- sets the (hacky) texture reduction factor                  *
 *   WW3D::Get_Texture_Reduction -- gets the (hacky) texture reduction factor                  *
 *   WW3D::Flush_Texture_Cache -- dump all textures from the texture cache                     *
 *   WW3D::Allocate_Debug_Resources -- allocates the debug resources					              *
 *   WW3D::Release_Debug_Resources -- releases the debug resources									  *
 *   WW3D::Get_Last_Frame_Poly_Count -- returns the number of polys submitted in the previous  *
 *   WW3D::Flush -- Process all pending rendering tasks                                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "ww3d.h"
#include "rinfo.h"
#include "assetmgr.h"
#include "boxrobj.h"
#include "predlod.h"
#include "camera.h"
#include "scene.h"
#include "registry.h"
#include "segline.h"
#include "shader.h"
#include "vertmaterial.h"
#include "wwdebug.h"
#include "wwprofile.h"
#include "wwmemlog.h"
#include "shattersystem.h"
#include "textureloader.h"
#include "statistics.h"
#include "pointgr.h"
#include "ffactory.h"
#include "INI.H"
#include "dazzle.h"
#include "meshmdl.h"
#include "dx8renderer.h"
#include "render2d.h"
#include "bound.h"
#include "rddesc.h"
#include "Vector3i.h"
#include <cstdio>
#include "dx8wrapper.h"
#include "TARGA.H"
#include "sortingrenderer.h"
#include "thread.h"
#include "cpudetect.h"
#include "dx8texman.h"
#include "formconv.h"
#include "animatedsoundmgr.h"
#include "static_sort_list.h"


#ifndef _UNIX
#include "framgrab.h"
#endif


const char* DAZZLE_INI_FILENAME="DAZZLE.INI";

#define DEFAULT_DEBUG_SHADER_BITS	(		SHADE_CNST(\
												ShaderClass::PASS_LEQUAL,\
												ShaderClass::DEPTH_WRITE_ENABLE,\
												ShaderClass::COLOR_WRITE_ENABLE,\
												ShaderClass::SRCBLEND_ONE,\
												ShaderClass::DSTBLEND_ZERO,\
												ShaderClass::FOG_DISABLE,\
												ShaderClass::GRADIENT_MODULATE,\
												ShaderClass::SECONDARY_GRADIENT_DISABLE,\
												ShaderClass::TEXTURING_DISABLE,\
												ShaderClass::ALPHATEST_DISABLE,\
												ShaderClass::CULL_MODE_ENABLE, \
												ShaderClass::DETAILCOLOR_DISABLE,\
												ShaderClass::DETAILALPHA_DISABLE) )

#define LIGHTMAP_DEBUG_SHADER_BITS	(		SHADE_CNST(\
												ShaderClass::PASS_LEQUAL,\
												ShaderClass::DEPTH_WRITE_ENABLE,\
												ShaderClass::COLOR_WRITE_ENABLE,\
												ShaderClass::SRCBLEND_ONE,\
												ShaderClass::DSTBLEND_ZERO,\
												ShaderClass::FOG_DISABLE,\
												ShaderClass::GRADIENT_DISABLE,\
												ShaderClass::SECONDARY_GRADIENT_DISABLE,\
												ShaderClass::TEXTURING_ENABLE,\
												ShaderClass::ALPHATEST_DISABLE,\
												ShaderClass::CULL_MODE_ENABLE, \
												ShaderClass::DETAILCOLOR_DISABLE,\
												ShaderClass::DETAILALPHA_DISABLE) )



/**********************************************************************************
**
**  WW3D Static Globals
**
***********************************************************************************/

unsigned int											WW3D::SyncTime = 0;
unsigned int											WW3D::PreviousSyncTime = 0;
bool														WW3D::IsSortingEnabled = true;

float														WW3D::PixelCenterX = 0.0f;
float														WW3D::PixelCenterY = 0.0f;


bool														WW3D::IsInitted = false;
bool														WW3D::IsRendering = false;
bool														WW3D::IsCapturing = false;
bool														WW3D::IsScreenUVBiased = false;

bool														WW3D::AreDecalsEnabled = true;
float														WW3D::DecalRejectionDistance = 1000000.0f;

bool														WW3D::AreStaticSortListsEnabled = false;
bool														WW3D::MungeSortOnLoad = false;

bool														WW3D::OverbrightModifyOnLoad = false;

FrameGrabClass *										WW3D::Movie = NULL;
bool														WW3D::PauseRecord;
bool														WW3D::RecordNextFrame;

int														WW3D::FrameCount = 0;
long														WW3D::UserStat0 = 0;
long														WW3D::UserStat1 = 0;
long														WW3D::UserStat2 = 0;

float														WW3D::DefaultNativeScreenSize = 1.0f;

StaticSortListClass *								WW3D::DefaultStaticSortLists = NULL;
StaticSortListClass *								WW3D::CurrentStaticSortLists = NULL;


VertexMaterialClass *								WW3D::DefaultDebugMaterial  = NULL;
ShaderClass												WW3D::DefaultDebugShader(DEFAULT_DEBUG_SHADER_BITS);
ShaderClass												WW3D::LightmapDebugShader(LIGHTMAP_DEBUG_SHADER_BITS);

WW3D::PrelitModeEnum									WW3D::PrelitMode = PRELIT_MODE_LIGHTMAP_MULTI_PASS;
bool														WW3D::ExposePrelit = false;

bool														WW3D::SnapshotActivated=false;
bool														WW3D::ThumbnailEnabled=true;

WW3D::MeshDrawModeEnum								WW3D::MeshDrawMode = MESH_DRAW_MODE_OLD;
WW3D::NPatchesGapFillingModeEnum					WW3D::NPatchesGapFillingMode = NPATCHES_GAP_FILLING_ENABLED;
unsigned													WW3D::NPatchesLevel=1;
bool														WW3D::IsTexturingEnabled=true;
bool										WW3D::IsColoringEnabled=false;

static HWND												_Hwnd = NULL;		// Not a member to hide windows from WW3D users
static int												_TextureReduction = 0;
static int												_TextureMinDim = 1;
static bool												_LargeTextureExtraReductionEnabled = false;
int														WW3D::LastFrameMemoryAllocations;
int														WW3D::LastFrameMemoryFrees;

int														WW3D::TextureFilter = 0;

bool														WW3D::Lite = false;

/**********************************************************************************
**
**  WW3D Static Functions
**
***********************************************************************************/

void WW3D::Set_NPatches_Gap_Filling_Mode(NPatchesGapFillingModeEnum mode)
{
	if (NPatchesGapFillingMode!=mode) {
		NPatchesGapFillingMode=mode;
		TheDX8MeshRenderer.Invalidate();
	}
}

void WW3D::Set_NPatches_Level(unsigned level)
{
	if (level>8) level=8;
	if (level<1) level=1;
	if (NPatchesLevel==1 && level>1) TheDX8MeshRenderer.Invalidate();
	if (NPatchesLevel>1 && level==1) TheDX8MeshRenderer.Invalidate();
	NPatchesLevel = level;
}

void WW3D::Set_Thumbnail_Enabled (bool b)
{
	if (ThumbnailEnabled!=b) {
		ThumbnailEnabled = b;
		_Invalidate_Textures();
	}
}

/***********************************************************************************************
 * WW3D::Init -- Initialize the WW3D Library                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Init(void *hwnd, char *defaultpal, bool lite)
{
	assert(IsInitted == false);
	WWDEBUG_SAY(("WW3D::Init hwnd = %p",hwnd));
	_Hwnd = (HWND)hwnd;
	Lite = lite;

	/*
	** Initialize d3d, this also enumerates the available devices and resolutions.
	*/
	Init_D3D_To_WW3_Conversion();
	WWDEBUG_SAY(("Init DX8Wrapper"));
	if (!DX8Wrapper::Init(_Hwnd, lite)) {
		return(WW3D_ERROR_INITIALIZATION_FAILED);
	}
	WWDEBUG_SAY(("Allocate Debug Resources"));
	Allocate_Debug_Resources();

 	MMRESULT r=timeBeginPeriod(1);
	WWASSERT(r==TIMERR_NOERROR);

	/*
	** Initialize the dazzle system
	*/
	if (!lite) {
		WWDEBUG_SAY(("Init Dazzles"));
		FileClass * dazzle_ini_file = _TheFileFactory->Get_File(DAZZLE_INI_FILENAME);
		if (dazzle_ini_file) {
			INIClass dazzle_ini(*dazzle_ini_file);
			DazzleRenderObjClass::Init_From_INI(&dazzle_ini);
			_TheFileFactory->Return_File(dazzle_ini_file);
		}
	}
	/*
	** Initialize the default static sort lists
	** Note that DefaultStaticSortLists[0] is unused.
	*/
	DefaultStaticSortLists = W3DNEW DefaultStaticSortListClass();
	Reset_Current_Static_Sort_Lists_To_Default();

	/*
	** Initialize the animation-triggered sound system
	*/
	if (!lite) {
		AnimatedSoundMgrClass::Initialize ();
		IsInitted = true;
	}
	WWDEBUG_SAY(("WW3D Init completed"));
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * WW3D::Shutdown -- shutdown the WW3D Library                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Shutdown(void)
{
	assert(Lite || IsInitted == true);
//	WWDEBUG_SAY(("WW3D::Shutdown"));

#ifdef WW3D_DX8
	if (IsCapturing) {
		Stop_Movie_Capture();
	}
#endif //WW3D_DX8

	//restore the previous timer resolution
	MMRESULT r=timeEndPeriod(1);
	WWASSERT(r==TIMERR_NOERROR);
	/*
	** Free memory in predictive LOD optimizer
	*/
	PredictiveLODOptimizerClass::Free();

	/*
	** Free the DazzleRenderObject class stuff. Whatever it is. ST - 6/11/2001 8:20PM
	*/
	if (!Lite) {
		DazzleRenderObjClass::Deinit ();
	}

	/*
	** Release all of our assets
	*/
	Release_Debug_Resources();
	if (WW3DAssetManager::Get_Instance()) {
		WW3DAssetManager::Get_Instance()->Free_Assets();
	}

	DX8TextureManagerClass::Shutdown();
	if (!Lite) {
		DX8Wrapper::Shutdown();
	}

	/*
	** Clear the default static sort lists
	*/
	delete DefaultStaticSortLists;

	/*
	** Release the animation-triggered sound data
	*/
	AnimatedSoundMgrClass::Shutdown ();

	IsInitted = false;
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * WW3D::Set_Render_Device -- set the render device being currently used                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Set_Render_Device( const char * dev_name, int width, int height, int bits, int windowed, bool resize_window )
{
	bool success = DX8Wrapper::Set_Render_Device(dev_name,width,height,bits,windowed,resize_window);
	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}


/***********************************************************************************************
 * WW3D::Set_Any_Render_Device -- set any render device you can find                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Set_Any_Render_Device( void )
{
	bool success = DX8Wrapper::Set_Any_Render_Device();
	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}


/***********************************************************************************************
 * WW3D::Set_Render_Device -- set the render device being currently used                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Set_Render_Device(int dev, int width, int height, int bits, int windowed, bool resize_window, bool reset_device, bool restore_assets )
{
	bool success = DX8Wrapper::Set_Render_Device(dev,width,height,bits,windowed,resize_window,reset_device, restore_assets );
	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}


/***********************************************************************************************
 * WW3D::Set_Next_Render_Device -- just go to the next device in the list                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/26/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Set_Next_Render_Device(void)
{
	bool success = DX8Wrapper::Set_Next_Render_Device();
	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}

/***********************************************************************************************
 * WW3D::Get_Window -- returns the handle of the render window.										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/28/2001  pds : Created.                                                                 *
 *=============================================================================================*/
void *WW3D::Get_Window( void )
{
	return _Hwnd;
}

/***********************************************************************************************
 * WW3D::Is_Windowed -- returns wether we are currently in a windowed mode                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/2001  gth : Created.                                                                 *
 *=============================================================================================*/
bool WW3D::Is_Windowed( void )
{
	return DX8Wrapper::Is_Windowed();
}

/***********************************************************************************************
 * WW3D::Toggle_Windowed -- Toggle the current render device between	fullscreen and windowed	  *
 *									 mode.  Note:  Its called '_Windowed' to be consistent with the	  *
 *									 other references inside WW3D, a more descriptive name would		  *
 *									 be Toggle_Fullscreen.															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/99    PDS : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Toggle_Windowed (void)
{
	bool success = DX8Wrapper::Toggle_Windowed();
	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}


/***********************************************************************************************
 * WW3D::Get_Render_Device -- Get the index of the current render device                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *   1/25/2001  gth : converted to dx8                                                         *
 *=============================================================================================*/
int WW3D::Get_Render_Device(void)
{
	return DX8Wrapper::Get_Render_Device();
}


/***********************************************************************************************
 * WW3D::Get_Render_Device_Desc -- returns description of the current render device            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/26/98    GTH : Created.                                                                 *
 *   1/25/2001  gth : converted to dx8                                                         *
 *=============================================================================================*/
const RenderDeviceDescClass & WW3D::Get_Render_Device_Desc(int deviceidx)
{
	return DX8Wrapper::Get_Render_Device_Desc(deviceidx);
}



/***********************************************************************************************
 * WW3D::Get_Render_Device_Count -- returns the number of render devices available             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *   1/25/2001  gth : converted to DX8                                                         *
 *=============================================================================================*/
int WW3D::Get_Render_Device_Count(void)
{
	return DX8Wrapper::Get_Render_Device_Count();
}


/***********************************************************************************************
 * WW3D::Get_Render_Device_Name -- returns the name of the n-th render device                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *   1/25/2001  gth : converted to dx8                                                         *
 *=============================================================================================*/
const char * WW3D::Get_Render_Device_Name(int device_index)
{
	return DX8Wrapper::Get_Render_Device_Name(device_index);
}


/***********************************************************************************************
 * WW3D::Set_Device_Resolution -- set the current resolution and bitdepth                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Set_Device_Resolution(int width,int height,int bits,int windowed, bool resize_window)
{
	bool success = DX8Wrapper::Set_Device_Resolution(width,height,bits,windowed,resize_window);

	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}


/***********************************************************************************************
 * WW3D::Get_Render_Target_Resolution -- get the resolution and bitdepth of the current target *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *   1/25/2001  gth : converted to dx8                                                         *
 *=============================================================================================*/
void WW3D::Get_Render_Target_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed)
{
	DX8Wrapper::Get_Render_Target_Resolution(set_w,set_h,set_bits,set_windowed);
}


/***********************************************************************************************
 * WW3D::Get_Device_Resolution -- get the current resolution and bitdepth                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *   1/25/2001  gth : converted to dx8                                                         *
 *=============================================================================================*/
void WW3D::Get_Device_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed)
{
	DX8Wrapper::Get_Device_Resolution(set_w,set_h,set_bits,set_windowed);
}


/***********************************************************************************************
 * WW3D::Registry_Save_Render_Device -- Saves settings to Registry
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/3/98    BMG : Created.                                                                 *
 *   1/25/2001  gth : converted to dx8                                                         *
 *=============================================================================================*/
WW3DErrorType WW3D::Registry_Save_Render_Device( const char * sub_key )
{
	bool success = DX8Wrapper::Registry_Save_Render_Device(sub_key);
	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}

/***********************************************************************************************
 * WW3D::Registry_Save_Render_Device -- Saves settings to Registry
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/3/98    BMG : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Registry_Save_Render_Device( const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth )
{
	bool success = DX8Wrapper::Registry_Save_Render_Device(sub_key,device,width,height,depth,windowed,texture_depth);
	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}


/***********************************************************************************************
 * WW3D::Registry_Load_Render_Device -- Loads settings from Registry
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/3/98    BMG : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Registry_Load_Render_Device( const char * sub_key, bool resize_window )
{
	bool success = DX8Wrapper::Registry_Load_Render_Device(sub_key,resize_window);
	if (success) {
		return WW3D_ERROR_OK;
	} else {
		return WW3D_ERROR_INITIALIZATION_FAILED;
	}
}

bool WW3D::Registry_Load_Render_Device( const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth)
{
	return DX8Wrapper::Registry_Load_Render_Device(sub_key,device,device_len,width,height,depth,windowed,texture_depth);
}

void WW3D::_Invalidate_Mesh_Cache()
{
	TheDX8MeshRenderer.Invalidate();
}

void WW3D::_Invalidate_Textures()
{
	if (!WW3DAssetManager::Get_Instance()) return;

	TextureLoader::Flush_Pending_Load_Tasks();

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());

	// Loop through all the textures in the manager
	for (ite.First();!ite.Is_Done();ite.Next()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		tex->Invalidate();
	}
}

void WW3D::Set_Texture_Filter(int texture_filter)
{
	if (texture_filter<0) texture_filter=0;
	if (texture_filter>TextureFilterClass::TEXTURE_FILTER_ANISOTROPIC) texture_filter=TextureFilterClass::TEXTURE_FILTER_ANISOTROPIC;
	TextureFilter=texture_filter;
	TextureFilterClass::_Init_Filters();
}


/***********************************************************************************************
 * WW3D::Begin_Render -- mark the start of rendering for a new frame                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Begin_Render(bool clear,bool clearz,const Vector3 & color, float dest_alpha, void(*network_callback)(void))
{
	if (!IsInitted) {
		return(WW3D_ERROR_OK);
	}

	WWPROFILE("WW3D::Begin_Render");
	WWASSERT(IsInitted);
	HRESULT hr;

	SNAPSHOT_SAY(("==========================================\r\n"));
	SNAPSHOT_SAY(("========== WW3D::Begin_Render ============\r\n"));
	SNAPSHOT_SAY(("==========================================\r\n\r\n"));

	if (DX8Wrapper::_Get_D3D_Device8() && (hr=DX8Wrapper::_Get_D3D_Device8()->TestCooperativeLevel()) != D3D_OK)
	{
        // If the device was lost, do not render until we get it back
        if( D3DERR_DEVICELOST == hr )
            return WW3D_ERROR_GENERIC;	//other app has the device

        // Check if the device needs to be reset
        if( D3DERR_DEVICENOTRESET == hr )
        {
            WWDEBUG_SAY(("WW3D::Begin_Render is resetting the device."));
            DX8Wrapper::Reset_Device();
        }

		return WW3D_ERROR_GENERIC;
	}

	// Memory allocation statistics
	LastFrameMemoryAllocations=WWMemoryLogClass::Get_Allocate_Count();
	LastFrameMemoryFrees=WWMemoryLogClass::Get_Free_Count();
	WWMemoryLogClass::Reset_Counters();

	TextureLoader::Update();
//	TextureClass::_Reset_Time_Stamp();
	DynamicVBAccessClass::_Reset(true);
	DynamicIBAccessClass::_Reset(true);

	Debug_Statistics::Begin_Statistics();

	if (IsCapturing && (!PauseRecord || RecordNextFrame)) {
		Update_Movie_Capture();
		RecordNextFrame = false;
	}

	WWASSERT(!IsRendering);
	IsRendering = true;

	// If we want to clear the screen, we need to set the viewport to include the entire screen:
	if (clear || clearz) {
		D3DVIEWPORT8 vp;
		int width, height, bits;
		bool windowed;
		WW3D::Get_Render_Target_Resolution(width, height, bits, windowed);
		vp.X = 0;
		vp.Y = 0;
		vp.Width = width;
		vp.Height = height;
		vp.MinZ = 0.0f;;
		vp.MaxZ = 1.0f;
		DX8Wrapper::Set_Viewport(&vp);
		DX8Wrapper::Clear(clear, clearz, color, dest_alpha);
	}

	// Notify D3D that we are beginning to render the frame
	DX8Wrapper::Begin_Scene();

	return WW3D_ERROR_OK;
}

/***********************************************************************************************
 * WW3D::Render -- Render a list of layers, starting at the back.                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/2/98    EHC : Created.                                                                  *
 *=============================================================================================*/
WW3DErrorType WW3D::Render(const LayerListClass &LayerList)
{
	if (!IsInitted) {
		return(WW3D_ERROR_OK);
	}

	WWASSERT(IsRendering);

	LayerClass *layer = LayerList.Last();

	while (layer->Is_Valid()) {
		WW3DErrorType result = Render(*layer);

		if (result != WW3D_ERROR_OK) {
			return result;
		}

		layer = layer->Prev();
	}

	return WW3D_ERROR_OK;
}

/***********************************************************************************************
 * WW3D::Render -- Render a Layer                                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/2/98    EHC : Created.                                                                  *
 *=============================================================================================*/
WW3DErrorType WW3D::Render(const LayerClass &Layer)
{
	if (!IsInitted) {
		return(WW3D_ERROR_OK);
	}

	WWASSERT(IsRendering);
	return Render(Layer.Scene, Layer.Camera, Layer.Clear, Layer.ClearZ, Layer.ClearColor);

}


/***********************************************************************************************
 * WW3D::Render -- Render a 3D Scene using the given camera                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Render(SceneClass * scene,CameraClass * cam,bool clear,bool clearz,const Vector3 & color)
{
	if (!IsInitted) {
		return(WW3D_ERROR_OK);
	}

	WWPROFILE("WW3D::Render");
	WWMEMLOG(MEM_GAMEDATA);
	WWASSERT(IsInitted);
	WWASSERT(IsRendering);
	WWASSERT(scene);
	WWASSERT(cam);

	cam->On_Frame_Update();
	RenderInfoClass rinfo(*cam);

	// Apply the camera and viewport (including depth range)
	cam->Apply();

	// Clear the viewport
	if (clear || clearz) {
		DX8Wrapper::Clear(clear, clearz, color);
	}

	// set the rendering mode
	switch(scene->Get_Polygon_Mode()) {
		case SceneClass::POINT:
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_POINT);
			break;
		case SceneClass::LINE:
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
			break;
		case SceneClass::FILL:
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_SOLID);
			break;
	}

	// Set the global ambient light value here.  If the scene is using the LightEnvironment system
	// this setting will get overriden.
	DX8Wrapper::Set_Ambient(scene->Get_Ambient_Light());

	// render the scene

	TheDX8MeshRenderer.Set_Camera(&rinfo.Camera);

	scene->Render(rinfo);

	Flush(rinfo);

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * WW3D::Render -- Render a single render object                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/4/2001   gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::Render(
	RenderObjClass & obj,
	RenderInfoClass & rinfo
)
{
	if (!IsInitted) {
		return(WW3D_ERROR_OK);
	}

	WWPROFILE("WW3D::Render");
	WWASSERT(IsInitted);
	WWASSERT(IsRendering);

	{
		WWPROFILE("On_Frame_Update");
		rinfo.Camera.On_Frame_Update();
	}

	// Apply the camera and viewport (including depth range)
	rinfo.Camera.Apply();

	// set the rendering mode
	DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_SOLID);

	// Install the lighting environment if one is supplied
	if (rinfo.light_environment != NULL) {
		DX8Wrapper::Set_Light_Environment(rinfo.light_environment);
	}

	// Render the object
	TheDX8MeshRenderer.Set_Camera(&rinfo.Camera);

	obj.Render(rinfo);

	Flush(rinfo);

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * WW3D::Flush -- Process all pending rendering tasks                                          *
 *                                                                                             *
 *    NOTE: This normally happens AUTOMATICALLY. The user should almost *NEVER* have to call   *
 *    this function.  Anyway, this function causes all of the deferred rendering systems to    *
 *    actually perform all of their rendering tasks.  This includes the DX8MeshRenderer and    *
 *    the sorting system.                                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    Don't call this unless you know what you're doing                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/17/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Flush(RenderInfoClass & rinfo)
{
	TheDX8MeshRenderer.Flush();
	WW3D::Render_And_Clear_Static_Sort_Lists(rinfo);	//draws things like water

	SortingRendererClass::Flush();
	TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
}


/***********************************************************************************************
 * WW3D::End_Render -- Mark the completion of a frame                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType WW3D::End_Render(bool flip_frame)
{
	if (!IsInitted) {
		return(WW3D_ERROR_OK);
	}

	WWPROFILE("WW3D::End_Render");

	WWASSERT(IsRendering);
	WWASSERT(IsInitted);

	// If sorting renderer flush isn't called from within any of the render functions
	// the sorting arrays will overflow!

	SortingRendererClass::Flush();

	IsRendering = false;

	{
		WWPROFILE("DX8Wrapper::End_Scene");
		DX8Wrapper::End_Scene(flip_frame);
	}

	FrameCount++;

	{
		WWPROFILE("End_Statistics");
		Debug_Statistics::End_Statistics();
	}

	SNAPSHOT_SAY(("==========================================\r\n"));
	SNAPSHOT_SAY(("========== WW3D::End_Render ==============\r\n"));
	SNAPSHOT_SAY(("==========================================\r\n\r\n"));

	Activate_Snapshot(false);
	
	// (gth) I've found some cases where its not safe to rely on our "shadow" copy (of 
	// matrices for example) across multiple frames.  So even though this is slightly
	// less "optimal", lets just reset the caches each frame.
	DX8Wrapper::Invalidate_Cached_Render_States();

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * WW3D::Flip_To_Primary                                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/20/01    DEL : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Flip_To_Primary(void)
{
	DX8Wrapper::Flip_To_Primary();
}


/***********************************************************************************************
 * WW3D::Get_Last_Frame_Poly_Count -- returns the number of polys submitted in the previous fr *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/28/99    GTH : Created.                                                                 *
 *=============================================================================================*/
unsigned int WW3D::Get_Last_Frame_Poly_Count(void)
{
	return Debug_Statistics::Get_DX8_Polygons();
}

unsigned int WW3D::Get_Last_Frame_Vertex_Count(void)
{
	return Debug_Statistics::Get_DX8_Vertices();
}


/***********************************************************************************************
 * WW3D::Sync -- Time sychronization                                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Sync(unsigned int sync_time)
{
	PreviousSyncTime = SyncTime;
   SyncTime = sync_time;
}


/***********************************************************************************************
 * WW3D::Set_Ext_Swap_Interval -- Sets the swap interval the device should aim sync for.       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:   Not supported by all rendering devices.                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/07/98    NH : Created.                                                                  *
 *=============================================================================================*/
void WW3D::Set_Ext_Swap_Interval(long swap)
{
	DX8Wrapper::Set_Swap_Interval(swap);
}


/***********************************************************************************************
 * WW3D::Get_Ext_Swap_Interval -- Queries the swap interval the device is aiming sync for.     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:   Not supported by all rendering devices.                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/07/98    NH : Created.                                                                  *
 *=============================================================================================*/
long WW3D::Get_Ext_Swap_Interval(void)
{
	return DX8Wrapper::Get_Swap_Interval();
}


/***********************************************************************************************
 * WW3D::Set_Collision_Box_Display_Mask -- control rendering of collision boxes                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/17/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Set_Collision_Box_Display_Mask(int mask)
{
	BoxRenderObjClass::Set_Box_Display_Mask(mask);
}

/***********************************************************************************************
 * WW3D::Get_Collision_Box_Display_Mask -- returns the current display mask for collision boxe *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
int WW3D::Get_Collision_Box_Display_Mask(void)
{
	return BoxRenderObjClass::Get_Box_Display_Mask();
}


/***********************************************************************************************
 * WW3D::Normalize_Coordinates -- Convert pixel coords to normalized screen coords 0..1        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    EHC : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Normalize_Coordinates(int x, int y, float &fx, float &fy)
{
	// clip the coordinates back into the resolution of the screen
	x = Bound(x, 0, DX8Wrapper::Get_Device_Resolution_Width());
	y = Bound(y, 0, DX8Wrapper::Get_Device_Resolution_Height());

	// now that the coordinates are clipped convert them to their normalized values.
	fx = (float)x / DX8Wrapper::Get_Device_Resolution_Width();
	fy = (float)y / DX8Wrapper::Get_Device_Resolution_Height();
}


/***********************************************************************************************
 * WW3D::Make_Screen_Shot -- saves a screenshot with the given base filename                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *   2/26/2001  hy : Updated to DX8                                                            *
 *=============================================================================================*/
void WW3D::Make_Screen_Shot( const char * filename_base , const float gamma, const ScreenShotFormatEnum format)
{

	WWASSERT(!IsRendering);

	char filename[80];

	char ext[4];
	switch (format) {
		case TGA:
			sprintf(ext,"tga");
			break;
		case BMP:
			sprintf(ext,"bmp");
			break;
		default:
			WWASSERT(0);
			return;
			break;
	}

	static int frame_number = 1;

	bool done = false;
	while (!done) {
		sprintf( filename, "%s%.2d.%s", filename_base, frame_number++, ext);
		FileClass*file=_TheFileFactory->Get_File( filename );
		if ( file ) {
			file->Open();
			done = !file->Is_Available();
			_TheFileFactory->Return_File( file );
		} else {
			done = true;
		}
	}

	WWDEBUG_SAY(( "Creating Screen Shot %s", filename ));

	// make the gamma look up table
	int i;
	unsigned char gamma_lut[256];
	float recip = 1.0f;
	if (gamma > WWMATH_EPSILON) {
		recip = 1.0f / gamma;
	}
	for (i = 0; i < 256; i++) {
		gamma_lut[i] = (unsigned char) (256.0f * powf(i / 256.0f, recip));
	}

	// TheSuperHackers @bugfix xezon 21/05/2025 Get the back buffer and create a copy of the surface.
	// Originally this code took the front buffer and tried to lock it. This does not work when the
	// render view clips outside the desktop boundaries. It crashed the game.
	SurfaceClass* surface = DX8Wrapper::_Get_DX8_Back_Buffer();

	SurfaceClass::SurfaceDescription surfaceDesc;
	surface->Get_Description(surfaceDesc);

	SurfaceClass* surfaceCopy = NEW_REF(SurfaceClass, (DX8Wrapper::_Create_DX8_Surface(surfaceDesc.Width, surfaceDesc.Height, surfaceDesc.Format)));
	DX8Wrapper::_Copy_DX8_Rects(surface->Peek_D3D_Surface(), NULL, 0, surfaceCopy->Peek_D3D_Surface(), NULL);

	surface->Release_Ref();
	surface = NULL;

	struct Rect
	{
		int Pitch;
		void* pBits;
	} lrect;

	lrect.pBits = surfaceCopy->Lock(&lrect.Pitch);
	if (lrect.pBits == NULL)
	{
		surfaceCopy->Release_Ref();
		return;
	}

	unsigned int x,y,index,index2,width,height;

	width = surfaceDesc.Width;
	height = surfaceDesc.Height;

	unsigned char *image=W3DNEWARRAY unsigned char[3*width*height];

	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			// index for image
			index=3*(x+y*width);
			// index for fb
			index2=y*lrect.Pitch+4*x;

			image[index]   = gamma_lut[*((unsigned char *) lrect.pBits + index2+2)];
			image[index+1] = gamma_lut[*((unsigned char *) lrect.pBits + index2+1)];
			image[index+2] = gamma_lut[*((unsigned char *) lrect.pBits + index2+0)];
		}
	}

	surfaceCopy->Unlock();
	surfaceCopy->Release_Ref();
	surfaceCopy = NULL;

	switch (format) {
		case TGA:
			{
				Targa targ;
				memset(&targ.Header,0,sizeof(targ.Header));
				targ.Header.Width=width;
				targ.Header.Height=height;
				targ.Header.PixelDepth=24;
				targ.Header.ImageType=TGA_TRUECOLOR;
				targ.SetImage((char *) image);
				targ.YFlip();

				FileClass*file=_TheWritingFileFactory->Get_File( filename );
				if ( file ) {
					file->Create();
					file->Close();
					_TheWritingFileFactory->Return_File( file );
				}

				targ.Save(filename,TGAF_IMAGE,false);
			}
		break;
		case BMP:
			{
				BITMAPFILEHEADER fileheader;
				BITMAPINFOHEADER header;
				memset(&header, 0, sizeof(BITMAPINFOHEADER));
				header.biSize = sizeof(BITMAPINFOHEADER);
				header.biWidth = width;
				header.biHeight = height;
				header.biPlanes = 1;
				header.biBitCount = 24;
				header.biCompression = BI_RGB;
				header.biXPelsPerMeter = 0xB12;
				header.biYPelsPerMeter = 0xB12;
				int len = ((width * 24 +31) & ~31) /8;
    
				memset(&fileheader, 0, sizeof(BITMAPFILEHEADER));
				fileheader.bfType = 19778; // BM
				fileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
				fileheader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 3 * len * height * sizeof(char);

				FileClass *file = _TheWritingFileFactory->Get_File( filename );
				if ( file ) {
					file->Create();
					file->Open(FileClass::WRITE);
					int num;
					num = file->Write(&fileheader, sizeof(BITMAPFILEHEADER));
					WWASSERT(num == sizeof(BITMAPFILEHEADER));
					num = file->Write(&header, sizeof(BITMAPINFOHEADER));
					WWASSERT(num == sizeof(BITMAPINFOHEADER));
					char *temp = new char [3 * len];
					memset(temp, 0, 3 * len * sizeof(char));
					// invert image, pad and swap R and B
					for (y = 0; y < (int) height; y++) {
						memcpy(&temp[0], &image[ 3 * width * (height - y - 1)], 3 * width * sizeof(char));
						for (x = 0; x < width; x++) {
							char t2 = temp[3 * x];
							temp[3 * x] = temp[3 * x + 2];
							temp[3 * x + 2] = t2;
						}
						num = file->Write(&temp[0], len * sizeof(char));
						WWASSERT(num == len * (int)sizeof(char));
					}
					delete [] temp;
					file->Close();
					_TheWritingFileFactory->Return_File( file );
				}
			}
			break;
	}

	delete [] image;
}


/***********************************************************************************************
 * WW3D::Start_Movie_Capture -- begins dumping frames to a movie                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *   2/26/2001  hy : updated to dx8                                                            *
 *=============================================================================================*/
void WW3D::Start_Movie_Capture( const char * filename_base, float frame_rate )
{
#ifdef _WINDOWS
	if (IsCapturing) {
		Stop_Movie_Capture();
	}
	WWASSERT( !IsCapturing);
	IsCapturing = true;

	RECT bounds;
	GetWindowRect(_Hwnd,&bounds);
	int height=bounds.bottom-bounds.top;
	int width=bounds.right-bounds.left;
	int depth=24;

	WWASSERT( Movie == NULL);

	if (frame_rate == 0.0f) {
		frame_rate = 1.0f;
		PauseRecord = true;
	} else {
		PauseRecord = false;
	}

	Movie = W3DNEW FrameGrabClass( filename_base, FrameGrabClass::AVI, width, height, depth, frame_rate);

	WWDEBUG_SAY(( "Starting Movie %s", filename_base ));
#endif
}


/***********************************************************************************************
 * WW3D::Stop_Movie_Capture -- ends dumping frames to a movie                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Stop_Movie_Capture( void )
{
#ifdef _WINDOWS
	if (IsCapturing) {
		IsCapturing = false;
		WWDEBUG_SAY(( "Stoping Movie" ));

		WWASSERT( Movie != NULL);
		delete Movie;
		Movie = NULL;
	}
#endif
}


/***********************************************************************************************
 * WW3D::Toggle_Movie_Capture -- toggles movie capture...                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Toggle_Movie_Capture( const char * filename_base, float frame_rate )
{
	if (IsCapturing) {
		Stop_Movie_Capture();
	} else {
		Start_Movie_Capture( filename_base, frame_rate);
	}
}


/***********************************************************************************************
 * WW3D::Start_Single_Frame_Movie_Capture -- starts capturing a single frame movie             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Start_Single_Frame_Movie_Capture(const char *filename_base)
{
	Start_Movie_Capture(filename_base, 0.0f);
}


/***********************************************************************************************
 * WW3D::Capture_Next_Movie_Frame -- tells ww3d to grab another frame for the movie            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Capture_Next_Movie_Frame()
{
	RecordNextFrame = true;
}


/***********************************************************************************************
 * WW3D::Pause_Movie -- pauses/unpauses movie capturing                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Pause_Movie(bool mode)
{
	PauseRecord = mode;
}


/***********************************************************************************************
 * WW3D::Is_Movie_Paused -- returns whether the movie capture system is paused                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
bool WW3D::Is_Movie_Paused()
{
	return PauseRecord;
}


/***********************************************************************************************
 * WW3D::Is_Recording_Next_Frame -- returns whether the next frame will be dumped to a movie   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
bool WW3D::Is_Recording_Next_Frame()
{
	return (Movie != 0) && (!PauseRecord || RecordNextFrame);
}


/***********************************************************************************************
 * WW3D::Is_Movie_Ready -- returns whether the movie capture system is ready                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
bool WW3D::Is_Movie_Ready()
{
	return Movie != 0;
}


/***********************************************************************************************
 * WW3D::Update_Movie_Capture -- dumps the current frame into the movie                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *   2/26/2001  hy : Updated to dx8                                                            *
 *=============================================================================================*/
void WW3D::Update_Movie_Capture( void )
{
#ifdef _WINDOWS
	WWASSERT( IsCapturing);
	WWPROFILE("WW3D::Update_Movie_Capture");
	WWDEBUG_SAY(( "Updating"));

	// TheSuperHackers @bugfix xezon 21/05/2025 Get the back buffer and create a copy of the surface.
	// Originally this code took the front buffer and tried to lock it. This does not work when the
	// render view clips outside the desktop boundaries. It crashed the game.
	SurfaceClass* surface = DX8Wrapper::_Get_DX8_Back_Buffer();

	SurfaceClass::SurfaceDescription surfaceDesc;
	surface->Get_Description(surfaceDesc);

	SurfaceClass* surfaceCopy = NEW_REF(SurfaceClass, (DX8Wrapper::_Create_DX8_Surface(surfaceDesc.Width, surfaceDesc.Height, surfaceDesc.Format)));
	DX8Wrapper::_Copy_DX8_Rects(surface->Peek_D3D_Surface(), NULL, 0, surfaceCopy->Peek_D3D_Surface(), NULL);

	surface->Release_Ref();
	surface = NULL;

	struct Rect
	{
		int Pitch;
		void* pBits;
	} lrect;

	lrect.pBits = surfaceCopy->Lock(&lrect.Pitch);
	if (lrect.pBits == NULL)
	{
		surfaceCopy->Release_Ref();
		return;
	}

	unsigned int x,y,index,index2,width,height;

	width = surfaceDesc.Width;
	height = surfaceDesc.Height;

	char *image=(char *)Movie->GetBuffer();

	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			// index for image
			index=3*(x+(height-y-1)*width);
			// index for fb
			index2=y*lrect.Pitch+4*x;

			image[index]=*((char *) lrect.pBits + index2+0);
			image[index+1]=*((char *) lrect.pBits + index2+1);
			image[index+2]=*((char *) lrect.pBits + index2+2);
		}
	}

	surfaceCopy->Unlock();
	surfaceCopy->Release_Ref();
	surfaceCopy = NULL;

	Movie->Grab(image);
#endif
}


/***********************************************************************************************
 * WW3D::Get_Movie_Capture_Frame_Rate -- returns the framerate at which the movie is being cap *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
float	WW3D::Get_Movie_Capture_Frame_Rate( void )
{
#ifdef _WINDOWS
	if (IsCapturing) {
		return Movie->GetFrameRate();
	}
#endif
	return 0;
}


/***********************************************************************************************
 * WW3D::Set_Texture_Reduction -- sets the (hacky) texture reduction factor                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void	WW3D::Set_Texture_Reduction( int value, int minDim )
{
	if (_TextureReduction != value || _TextureMinDim != minDim) {
		_TextureReduction=value;
		_TextureMinDim=minDim;
		_Invalidate_Textures();
	}
}


void WW3D::Enable_Texturing(bool b)
{
	if (b==IsTexturingEnabled) return;
	IsTexturingEnabled=b;
//	_Invalidate_Textures();
}

void WW3D::Enable_Coloring(unsigned int color)
{
	IsColoringEnabled = (color == 0) ? false : true;
}

/***********************************************************************************************
 * WW3D::Get_Texture_Reduction -- gets the (hacky) texture reduction factor                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/25/99    TSS : Created.                                                                 *
 *=============================================================================================*/
int	WW3D::Get_Texture_Reduction( void )
{
	return _TextureReduction;
}

/***********************************************************************************************
 * WW3D::Get_Texture_Min_Mip_Levels -- gets the minimum number of mip levels permitted		   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/25/99    TSS : Created.                                                                 *
 *=============================================================================================*/
int	WW3D::Get_Texture_Min_Dimension( void )
{
	return _TextureMinDim;
}

void WW3D::Enable_Large_Texture_Extra_Reduction(bool onoff)
{
	if (_LargeTextureExtraReductionEnabled != onoff) {
		_LargeTextureExtraReductionEnabled = onoff;
		_Invalidate_Textures();
	}
}

bool WW3D::Is_Large_Texture_Extra_Reduction_Enabled(void)
{
	return _LargeTextureExtraReductionEnabled;
}

/***********************************************************************************************
 * WW3D::Peek_Default_Debug_Material -- returns a pointer to the default debug mtl				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/21/99    GTH : Created.                                                                 *
 *=============================================================================================*/
VertexMaterialClass * WW3D::Peek_Default_Debug_Material(void)
{
#ifdef WWDEBUG
	WWASSERT(DefaultDebugMaterial);
	return DefaultDebugMaterial;
#else
	return NULL;
#endif
}

/***********************************************************************************************
 * WW3D::Peek_Default_Debug_Shader -- returns the default shader for debugging.	              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/21/99    GTH : Created.                                                                 *
 *=============================================================================================*/
ShaderClass	WW3D::Peek_Default_Debug_Shader(void)
{
	return DefaultDebugShader;
}

/***********************************************************************************************
 * WW3D::Peek_Lightmap_Debug_Shader -- returns the shader for lightmap debugging.              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/21/99    GTH : Created.                                                                 *
 *=============================================================================================*/
ShaderClass	WW3D::Peek_Lightmap_Debug_Shader(void)
{
	return LightmapDebugShader;
}

/***********************************************************************************************
 * WW3D::Allocate_Debug_Resources -- allocates the debug resources									  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/21/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Allocate_Debug_Resources(void)
{
#ifdef WWDEBUG
	WWASSERT(DefaultDebugMaterial == NULL);
	DefaultDebugMaterial = W3DNEW VertexMaterialClass;
	DefaultDebugMaterial->Set_Shininess(0.0f);
	DefaultDebugMaterial->Set_Opacity(1.0f);
	DefaultDebugMaterial->Set_Ambient(0,0,0);
	DefaultDebugMaterial->Set_Diffuse(0,0,0);
	DefaultDebugMaterial->Set_Specular(0,0,0);
	DefaultDebugMaterial->Set_Emissive(0,0,0);
#endif
}

/***********************************************************************************************
 * WW3D::Release_Debug_Resources -- releases the debug resources										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/21/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3D::Release_Debug_Resources(void)
{
#ifdef WWDEBUG
	WWASSERT(DefaultDebugMaterial);
	REF_PTR_RELEASE(DefaultDebugMaterial);
#endif
}


WW3DErrorType WW3D::On_Deactivate_App(void)
{
	_Invalidate_Textures();
	_Invalidate_Mesh_Cache();

	return WW3D_ERROR_OK;
}


WW3DErrorType WW3D::On_Activate_App(void)
{
	return WW3D_ERROR_OK;
}


void WW3D::Get_Pixel_Center(float &x, float &y)
{
	x = PixelCenterX; y = PixelCenterY;
}


void WW3D::Update_Pixel_Center(void)
{
#ifdef WW3D_DX8
	const char *name = _RenderDeviceShortNameTable.getString(CurRenderDevice);
	if ( strstr(name, "OpenGL") ) {
		PixelCenterX = 0.0f; PixelCenterY = 0.0f;
	} else if ( strstr(name, "Glide") ) {
		PixelCenterX = 0.0f; PixelCenterY = 0.0f;
	} else if ( strstr(name, "DirectX") ) {
		PixelCenterX = 0.5f; PixelCenterY = 0.5f;
	} else if ( strstr(name, "Software") ) {
		PixelCenterX = 0.0f; PixelCenterY = 0.0f;
	} else if ( strstr(name, "Null") ) {
		PixelCenterX = 0.0f; PixelCenterY = 0.0f;
	} else {
		// unknown device
		PixelCenterX = 0.0f; PixelCenterY = 0.0f;
	}
#endif //WW3D_DX8
}

void WW3D::Set_Texture_Bitdepth(int bitdepth)
{
	DX8Wrapper::Set_Texture_Bitdepth(bitdepth);
}

int WW3D::Get_Texture_Bitdepth()
{
	return DX8Wrapper::Get_Texture_Bitdepth();
}

void WW3D::Add_To_Static_Sort_List(RenderObjClass *robj, unsigned int sort_level)
{
	CurrentStaticSortLists->Add_To_List(robj, sort_level);
}

void WW3D::Render_And_Clear_Static_Sort_Lists(RenderInfoClass & rinfo)
{
	// The ststic sort lists need to be disabled while we are rendering from them otherwise the
	// Render() function will just dump the objects right back on the same lists.
	bool old_enable = AreStaticSortListsEnabled;
	AreStaticSortListsEnabled = false;
	CurrentStaticSortLists->Render_And_Clear(rinfo);
	AreStaticSortListsEnabled = old_enable;
}

void WW3D::Enable_Sorting(bool onoff)
{
	IsSortingEnabled = onoff;
	// Have to invalidate mesh rendering system because
	// meshes are put into different fvfs depending on their sort state
	TheDX8MeshRenderer.Invalidate();
}

void WW3D::Override_Current_Static_Sort_Lists(StaticSortListClass * sort_list)
{
	if (sort_list) {
		CurrentStaticSortLists = sort_list;
	} else {
		WWASSERT(sort_list);
	}
}


void WW3D::Reset_Current_Static_Sort_Lists_To_Default(void)
{
	CurrentStaticSortLists = DefaultStaticSortLists;
}

void WW3D::Set_Gamma(float gamma,float bright,float contrast,bool calibrate)
{
	DX8Wrapper::Set_Gamma(gamma,bright,contrast,calibrate);
}