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
 *                     $Archive:: /VSS_Sync/ww3d2/ww3d.h                                      $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 9:39p                                               $*
 *                                                                                             *
 *                    $Revision:: 32                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef WW3D_H
#define WW3D_H

#include "always.h"
#include "vector3.h"
#include "layer.h"
#include "w3derr.h"
#include "robjlist.h"

class		SceneClass;
class		CameraClass;
class		ShaderClass;
class		DX8Wrapper;

struct	RenderStatistics;
class		FrameGrabClass;
class		VertexMaterialClass;
class		ExtraMaterialPassClass;
class		RenderInfoClass;
class		RenderDeviceDescClass;
class		StringClass;
class		LightEnvironmentClass;
class		MaterialPassClass;
class 	StaticSortListClass;

#define SNAPSHOT_SAY(x) if (WW3D::Is_Snapshot_Activated()) { WWDEBUG_SAY(x); }
//#define SNAPSHOT_SAY(x)

/**
** WW3D
**
** This is the collection of static functions and data which initialize and
** control the behavior of the WW3D library.
*/
class WW3D
{
public:

	enum PrelitModeEnum {
		PRELIT_MODE_VERTEX,
		PRELIT_MODE_LIGHTMAP_MULTI_PASS,
		PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE
	};

	enum MeshDrawModeEnum {
		MESH_DRAW_MODE_OLD,
		MESH_DRAW_MODE_NEW,
		MESH_DRAW_MODE_DEBUG_DRAW,
		MESH_DRAW_MODE_DEBUG_CLIP,
		MESH_DRAW_MODE_DEBUG_BOX,
		MESH_DRAW_MODE_NONE,
		MESH_DRAW_MODE_DX8_ONLY
	};

	enum TextureCompressionModeEnum {
		TEXTURE_COMPRESSION_DISABLE,
		TEXTURE_COMPRESSION_ENABLE
	};

	enum NPatchesGapFillingModeEnum {
		NPATCHES_GAP_FILLING_DISABLED,
		NPATCHES_GAP_FILLING_ENABLED,
		NPATCHES_GAP_FILLING_FORCE
	};


	static WW3DErrorType		Init(void * hwnd, char *defaultpal = NULL);
	static WW3DErrorType		Shutdown(void);
	static bool					Is_Initted(void)								{ return IsInitted; }

	static const int			Get_Render_Device_Count(void);
	static const char *		Get_Render_Device_Name(int device_index);
	static const RenderDeviceDescClass &								Get_Render_Device_Desc(int device = -1);

	static int					Get_Render_Device(void);
	static WW3DErrorType		Set_Render_Device( int dev=-1, int resx=-1, int resy=-1, int bits=-1, int windowed=-1, bool resize_window = false, bool reset_device=false, bool restore_assets=true);
	static WW3DErrorType		Set_Render_Device( const char *dev_name, int resx=-1, int resy=-1, int bits=-1, int windowed=-1, bool resize_window = false  );	
	static WW3DErrorType		Set_Next_Render_Device(void);
	static WW3DErrorType		Set_Any_Render_Device( void );

	static void					Get_Pixel_Center(float &x, float &y);
	static void					Get_Render_Target_Resolution(int & set_w,int & set_h,int & get_bits,bool & get_windowed);
	static void					Get_Device_Resolution(int & set_w,int & set_h,int & get_bits,bool & get_windowed);
	static WW3DErrorType		Set_Device_Resolution(int w=-1,int h=-1,int bits=-1,int windowed=-1, bool resize_window=false );

	static bool					Is_Windowed( void );
	static WW3DErrorType		Toggle_Windowed ( void );
	static void					Set_Window( void *hwnd );
	static void *				Get_Window( void );

	static WW3DErrorType		On_Activate_App( void );
	static WW3DErrorType		On_Deactivate_App( void );

	static WW3DErrorType		Registry_Save_Render_Device( const char * sub_key );
	static WW3DErrorType		Registry_Save_Render_Device( const char * sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth );
	static WW3DErrorType		Registry_Load_Render_Device( const char * sub_key, bool resize_window = false );
	static bool					Registry_Load_Render_Device( const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int& texture_depth);		

	/*
	** Rendering functions
	** Each frame should be bracketed by a Begin_Render and End_Render call.  Between these two calls you will
	** normally render scenes.  The render function which accepts a single render object is implemented for
	** special cases like generating a shadow texture for an object.  Basically this function will have the
	** entire scene rendering overhead.
	*/
	static WW3DErrorType		Begin_Render(bool clear = false,bool clearz = true,const Vector3 & color = Vector3(0,0,0), float dest_alpha=0.0f);

	static WW3DErrorType		Render(const LayerListClass & layerlist);
	static WW3DErrorType		Render(const LayerClass & layer);
	static WW3DErrorType		Render(SceneClass * scene,CameraClass * cam,bool clear = false,bool clearz = false,const Vector3 & color = Vector3(0,0,0));
	static WW3DErrorType		Render(RenderObjClass & obj,RenderInfoClass & rinfo);
	static void					Flush(RenderInfoClass & rinfo);	// NOTE: "normal" usage should *NEVER* require the user to call this function

	static WW3DErrorType		End_Render(bool flip_frame = true);

	static void Flip_To_Primary(void);


	/*
	** Timing
	** By calling the Sync function, the application can move the ww3d library time forward.  This
	** will control things like animated uv-offset mappers and render object animations.
	*/
	static void					Sync( unsigned int sync_time );
	static unsigned int		Get_Sync_Time(void) { return SyncTime; }
   static unsigned int     Get_Frame_Time(void) { return SyncTime - PreviousSyncTime; }
   static unsigned int     Get_Frame_Count(void) { return FrameCount; }
	static unsigned int		Get_Last_Frame_Poly_Count(void);
	static unsigned int		Get_Last_Frame_Vertex_Count(void);

	/*
	** Screen/Movie capturing
	** These functions allow you to create screenshots and movies.
	*/
	static void					Make_Screen_Shot( const char * filename = "ScreenShot");
	static void					Start_Movie_Capture( const char * filename_base = "Movie", float frame_rate = 15);
	static void					Stop_Movie_Capture( void);
	static void					Toggle_Movie_Capture( const char * filename_base = "Movie", float frame_rate = 15);
	static void					Start_Single_Frame_Movie_Capture(const char *filename_base = "Frames");
	static void					Capture_Next_Movie_Frame();
	static void					Update_Movie_Capture( void);
	static float				Get_Movie_Capture_Frame_Rate( void);
	static void					Pause_Movie(bool mode);
	static bool					Is_Movie_Paused();
	static bool					Is_Recording_Next_Frame(); 
	static bool					Is_Movie_Ready(); 

   /*
	** Set_Ext_Swap_Interval - how many vertical retraces to wait before flipping frames
	** Get_Ext_Swap_Interval - what is our current setting for the swap interval?
	*/
	static void             Set_Ext_Swap_Interval(long swap);
   static long             Get_Ext_Swap_Interval(void);

	/*
	** Texture Reduction - all currently loaded textures can be de-resed on the fly
	** by passing in a non-unit value to Set_Texture_Reduction.  Passing in 2 causes
	** all textures to be half their normal resolution.  Passing in 3 causes them to
	** be cut in half twice, etc
	*/
	static void					Set_Texture_Reduction( int value, int min_mip_levels=1 );
	static int					Get_Texture_Reduction( void );
	static int					Get_Texture_Min_Mip_Levels( void );
	static void					_Invalidate_Mesh_Cache();
	static void					_Invalidate_Textures();

	static void					Set_Thumbnail_Enabled(bool b);// { ThumbnailEnabled=b; }
	static bool					Get_Thumbnail_Enabled() { return ThumbnailEnabled; }

	static void					Enable_Sorting(bool onoff);
	static bool					Is_Sorting_Enabled(void)					{ return IsSortingEnabled; }

	static void					Set_Screen_UV_Bias( bool onoff )			{ IsScreenUVBiased = onoff; }
	static bool					Is_Screen_UV_Biased( void )				{ return IsScreenUVBiased; }

	static void					Set_Collision_Box_Display_Mask(int mask);
	static int					Get_Collision_Box_Display_Mask(void);

	static void					Set_Default_Native_Screen_Size(float dnss)	{ DefaultNativeScreenSize = dnss; }
	static float				Get_Default_Native_Screen_Size(void)			{ return DefaultNativeScreenSize; }

	static void					Normalize_Coordinates(int x, int y, float &fx, float &fy); // convert pixel coordinates to 0..1 screen coordinates

	static VertexMaterialClass *	Peek_Default_Debug_Material(void);
	static ShaderClass		Peek_Default_Debug_Shader(void);
	static ShaderClass		Peek_Backface_Debug_Shader(void);
	static ShaderClass		Peek_Lightmap_Debug_Shader(void);

	static void					Set_Prelit_Mode (PrelitModeEnum mode)			{ PrelitMode = mode; }
	static PrelitModeEnum 	Get_Prelit_Mode ()									{ return (PrelitMode); }
	static bool					Supports_Prelit_Mode (PrelitModeEnum mode)	{ return (true); }
	static void					Expose_Prelit (bool onoff)							{ ExposePrelit = onoff; }
	static bool					Expose_Prelit ()										{ return (ExposePrelit); }

	static void					Set_Texture_Bitdepth(int bitdepth);
	static int					Get_Texture_Bitdepth();

	static void					Set_Mesh_Draw_Mode (MeshDrawModeEnum mode)	{ MeshDrawMode = mode; }
	static MeshDrawModeEnum Get_Mesh_Draw_Mode ()								{ return (MeshDrawMode); }

	static void					Set_Texture_Compression_Mode (TextureCompressionModeEnum mode);
	static TextureCompressionModeEnum 	Get_Texture_Compression_Mode () { return (TextureCompressionMode); }

	static void					Set_NPatches_Gap_Filling_Mode (NPatchesGapFillingModeEnum mode);
	static NPatchesGapFillingModeEnum 	Get_NPatches_Gap_Filling_Mode () { return (NPatchesGapFillingMode); }

	static void					Set_NPatches_Level(unsigned level);
	static unsigned			Get_NPatches_Level() { return NPatchesLevel; }

	static void					Enable_Texturing(bool b);
	static bool					Is_Texturing_Enabled() { return IsTexturingEnabled; }
	static bool					Is_Coloring_Enabled() { return (IsColoringEnabled == 0) ? false : true; }
	static void					Enable_Coloring(unsigned int color);	///<when non-zero color is passed, it will override vertex colors

	static int					Get_Last_Frame_Memory_Allocation_Count() { return LastFrameMemoryAllocations; }
	static int					Get_Last_Frame_Memory_Free_Count() { return LastFrameMemoryFrees; }

	/*
	** Decal control
	** These global settings can control whether decals are rendered at all and
	** at what distance to stop rendering/creating decals
	*/
	static void					Enable_Decals(bool onoff)					{ AreDecalsEnabled = onoff; }
	static bool					Are_Decals_Enabled(void)					{ return AreDecalsEnabled; }
	static void					Set_Decal_Rejection_Distance(float d)	{ DecalRejectionDistance = d; }
	static float				Get_Decal_Rejection_Distance(void)		{ return DecalRejectionDistance; }

	/*
	** Static sort lists. The ability to temporarily set a different static
	** sort list from the default one and a min/max sort list range is for
	** specialised uses (such as pipctuire-in-picture windows which need to
	** sort at a certain sort level). After this override is called, the
	** default sort list must be restored.
	*/
	static void					Enable_Static_Sort_Lists(bool onoff)	{ AreStaticSortListsEnabled = onoff; }
	static bool					Are_Static_Sort_Lists_Enabled(void)		{ return AreStaticSortListsEnabled; }
	static void					Enable_Munge_Sort_On_Load(bool onoff)	{ MungeSortOnLoad=onoff; }
	static bool					Is_Munge_Sort_On_Load_Enabled(void)		{ return MungeSortOnLoad; }
	static void					Add_To_Static_Sort_List(RenderObjClass *robj, unsigned int sort_level);
	static void					Render_And_Clear_Static_Sort_Lists(RenderInfoClass & rinfo);
	static void					Override_Current_Static_Sort_Lists(StaticSortListClass * sort_list);
	static void					Reset_Current_Static_Sort_Lists_To_Default(void);

	static bool					Is_Snapshot_Activated()						{ return SnapshotActivated; }
	static void					Activate_Snapshot(bool b)					{ SnapshotActivated=b; }

	// These clock all the time under user control, and are used to update
   // Stats.UserStat* when performance sampling is enabled.
   static long             UserStat0;
   static long             UserStat1;
   static long             UserStat2;

private:

	enum
	{
		DEFAULT_RESOLUTION_WIDTH =			640,
		DEFAULT_RESOLUTION_HEIGHT =		480,
		DEFAULT_BIT_DEPTH =					16
	};

	static void					Read_Gerd_Render_Device_Description(RenderDeviceDescClass &desc);
	static void					Update_Pixel_Center(void);
	static void					Set_Polygon_Mode(int mode);
	static void					Allocate_Debug_Resources(void);
	static void					Release_Debug_Resources(void);

	// Timing info:
   // The absolute synchronized frame time (in milliseconds) supplied by the
   // application at the start of every frame. Note that wraparound cases
   // etc. need to be considered.
	static unsigned int				SyncTime;

   // The previously set absolute sync time - this is used to get the interval between
   // the most recently set sync time and the previous one. Assuming the
   // application sets sync time at the start of every frame, this represents
   // the frame interval.
   static unsigned int           PreviousSyncTime;

	static float						PixelCenterX;
	static float						PixelCenterY;

	static bool							IsInitted;
	static bool							IsRendering;
	static bool							IsCapturing;
	static bool							IsSortingEnabled;
	static bool							IsScreenUVBiased;
	static bool							IsBackfaceDebugEnabled;

	static bool							AreDecalsEnabled;
	static float						DecalRejectionDistance;

	static bool							AreStaticSortListsEnabled;
	static bool							MungeSortOnLoad;

	static FrameGrabClass *			Movie;
	static bool							PauseRecord;
	static bool							RecordNextFrame;
	static int							FrameCount;

	static VertexMaterialClass *	DefaultDebugMaterial;
	static VertexMaterialClass *	BackfaceDebugMaterial;
	static ShaderClass				DefaultDebugShader;
	static ShaderClass				LightmapDebugShader;

	static PrelitModeEnum			PrelitMode;
	static bool							ExposePrelit;

	static bool							SnapshotActivated;
	static bool							ThumbnailEnabled;

	static MeshDrawModeEnum			MeshDrawMode;
	static TextureCompressionModeEnum TextureCompressionMode;
	static NPatchesGapFillingModeEnum NPatchesGapFillingMode;
	static unsigned NPatchesLevel;
	static bool							IsTexturingEnabled;
	static bool							IsColoringEnabled;

	// This is the default native screen size which will be set for each
	// RenderObject on construction. The native screen size is the screen size
	// at which the object was designed to be viewed, and it is used in the
	// texture resizing algorithm (may be used in future for other things).
	// If the default is overriden, it will usually be in the asset manager
	// post-load callback.
	static float						DefaultNativeScreenSize;

	// For meshes which have a static sorting order. These will get drawn
	// after opaque meshes and before normally sorted meshes. The 'current'
	// pointer is so the application can temporarily set a different set of
	// static sort lists to be used temporarily. This is for specialised uses.
	static StaticSortListClass * DefaultStaticSortLists;
	static StaticSortListClass * CurrentStaticSortLists;

	// Memory allocation statistics
	static int							LastFrameMemoryAllocations;
	static int							LastFrameMemoryFrees;
};


/*
** RenderStatistics
** This struct holds the results of a performance sampling.  The WW3D object returns
** its statistics packaged up in one of these structures.
*/
struct RenderStatistics
{
		// General statistics
		double	ElapsedSeconds;
      int      FramesRendered;

		// Geometry engine statistics
		double	TrianglesReceived;
		double	TrianglesSubmitted;
		double	TrianglesSorted;
		double	VerticesReceived;
		double	VerticesSubmitted;

		// State change statistics
		double	ViewStateChanges;
		double	DrawStateChanges;
		double	TextureChanges;
		double	TextureParameterChanges;
		double	TexturesCreated;
		double	PaletteChanges;
		double	ShaderChanges;
		double	DrawCommands;
		double	TrianglesClipRemoved;
		double	TrianglesClipCreated;
		double	DeviceDriverCalls;

		// Rendering device statistics
		double	TextureTransfers;
		double	PixelsDrawn;
		double	PixelsRejected;

		// Surface cache statistics
		long		Hits;
		long		Misses;
		long		Insertions;
		long		Removals;
		long		MemUsed;
		long		MaxMemory;

      // User stats (can be used to see how often a function is called, etc.)
      long     UserStat0;
      long     UserStat1;
      long     UserStat2;
};



#endif
