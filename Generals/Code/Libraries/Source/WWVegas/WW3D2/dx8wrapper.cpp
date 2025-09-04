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
 *                     $Archive:: /VSS_Sync/ww3d2/dx8wrapper.cpp                              $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 134                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DX8Wrapper::_Update_Texture -- Copies a texture from system memory to video memory        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//#define CREATE_DX8_MULTI_THREADED
//#define CREATE_DX8_FPU_PRESERVE
#define WW3D_DEVTYPE D3DDEVTYPE_HAL

#if defined(_MSC_VER) && _MSC_VER < 1300
#undef WINVER
#define WINVER 0x0500 // Required to access GetMonitorInfo in VC6.
#endif

#include "dx8wrapper.h"
#include "dx8webbrowser.h"
#include "dx8fvf.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "ww3d.h"
#include "camera.h"
#include "wwstring.h"
#include "matrix4.h"
#include "vertmaterial.h"
#include "rddesc.h"
#include "lightenvironment.h"
#include "statistics.h"
#include "registry.h"
#include "boxrobj.h"
#include "pointgr.h"
#include "render2d.h"
#include "sortingrenderer.h"
#include "shattersystem.h"
#include "light.h"
#include "assetmgr.h"
#include "textureloader.h"
#include "missingtexture.h"
#include "thread.h"
#include <stdio.h>
#include <d3dx8core.h>
#include "pot.h"
#include "wwprofile.h"
#include "ffactory.h"
#include "dx8caps.h"
#include "formconv.h"
#include "dx8texman.h"
#include "bound.h"
#include "dx8webbrowser.h"


const int DEFAULT_RESOLUTION_WIDTH = 640;
const int DEFAULT_RESOLUTION_HEIGHT = 480;
const int DEFAULT_BIT_DEPTH = 32;
const int DEFAULT_TEXTURE_BIT_DEPTH = 16;

bool DX8Wrapper_IsWindowed = true;

// FPU_PRESERVE
int DX8Wrapper_PreserveFPU = 0;

/***********************************************************************************
**
** DX8Wrapper Static Variables
**
***********************************************************************************/

static HWND						_Hwnd															= NULL;
bool								DX8Wrapper::IsInitted									= false;
bool								DX8Wrapper::_EnableTriangleDraw						= true;

int								DX8Wrapper::CurRenderDevice							= -1;
int								DX8Wrapper::ResolutionWidth							= DEFAULT_RESOLUTION_WIDTH;
int								DX8Wrapper::ResolutionHeight							= DEFAULT_RESOLUTION_HEIGHT;
int								DX8Wrapper::BitDepth										= DEFAULT_BIT_DEPTH;
int								DX8Wrapper::TextureBitDepth							= DEFAULT_TEXTURE_BIT_DEPTH;
bool								DX8Wrapper::IsWindowed									= false;
D3DFORMAT					DX8Wrapper::DisplayFormat	= D3DFMT_UNKNOWN;

D3DMATRIX						DX8Wrapper::old_world;
D3DMATRIX						DX8Wrapper::old_view;
D3DMATRIX						DX8Wrapper::old_prj;

// shader system additions KJM v
DWORD								DX8Wrapper::Vertex_Shader								= 0;
DWORD								DX8Wrapper::Pixel_Shader								= 0;

Vector4							DX8Wrapper::Vertex_Shader_Constants[MAX_VERTEX_SHADER_CONSTANTS];
Vector4							DX8Wrapper::Pixel_Shader_Constants[MAX_PIXEL_SHADER_CONSTANTS];

LightEnvironmentClass*		DX8Wrapper::Light_Environment							= NULL;
RenderInfoClass*				DX8Wrapper::Render_Info									= NULL;

DWORD								DX8Wrapper::Vertex_Processing_Behavior				= 0;
Vector3							DX8Wrapper::Ambient_Color;
// shader system additions KJM ^
bool								DX8Wrapper::world_identity;
unsigned							DX8Wrapper::RenderStates[256];
unsigned							DX8Wrapper::TextureStageStates[MAX_TEXTURE_STAGES][32];
IDirect3DBaseTexture8 *		DX8Wrapper::Textures[MAX_TEXTURE_STAGES];
RenderStateStruct				DX8Wrapper::render_state;
unsigned							DX8Wrapper::render_state_changed;

bool								DX8Wrapper::FogEnable									= false;
D3DCOLOR							DX8Wrapper::FogColor										= 0;

IDirect3D8 *					DX8Wrapper::D3DInterface								= NULL;
IDirect3DDevice8 *			DX8Wrapper::D3DDevice									= NULL;
IDirect3DSurface8 *			DX8Wrapper::CurrentRenderTarget						= NULL;
IDirect3DSurface8 *			DX8Wrapper::DefaultRenderTarget						= NULL;
bool								DX8Wrapper::IsRenderToTexture							= false;

unsigned							DX8Wrapper::matrix_changes								= 0;
unsigned							DX8Wrapper::material_changes							= 0;
unsigned							DX8Wrapper::vertex_buffer_changes					= 0;
unsigned							DX8Wrapper::index_buffer_changes                = 0;
unsigned							DX8Wrapper::light_changes								= 0;
unsigned							DX8Wrapper::texture_changes							= 0;
unsigned							DX8Wrapper::render_state_changes						= 0;
unsigned							DX8Wrapper::texture_stage_state_changes			= 0;
unsigned							DX8Wrapper::draw_calls									= 0;
unsigned							DX8Wrapper::_MainThreadID								= 0;
bool								DX8Wrapper::CurrentDX8LightEnables[4];
bool								DX8Wrapper::IsDeviceLost;
int								DX8Wrapper::ZBias;
float								DX8Wrapper::ZNear;
float								DX8Wrapper::ZFar;
Matrix4x4						DX8Wrapper::ProjectionMatrix;
Matrix4x4						DX8Wrapper::DX8Transforms[D3DTS_WORLD+1];

DX8Caps*							DX8Wrapper::CurrentCaps = 0;

// Hack test... this disables rendering of batches of too few polygons.
unsigned							DX8Wrapper::DrawPolygonLowBoundLimit=0;

D3DADAPTER_IDENTIFIER8		DX8Wrapper::CurrentAdapterIdentifier;

unsigned long DX8Wrapper::FrameCount = 0;

bool								_DX8SingleThreaded										= false;

unsigned							number_of_DX8_calls										= 0;
static unsigned				last_frame_matrix_changes								= 0;
static unsigned				last_frame_material_changes							= 0;
static unsigned				last_frame_vertex_buffer_changes						= 0;
static unsigned				last_frame_index_buffer_changes						= 0;
static unsigned				last_frame_light_changes								= 0;
static unsigned				last_frame_texture_changes								= 0;
static unsigned				last_frame_render_state_changes						= 0;
static unsigned				last_frame_texture_stage_state_changes				= 0;
static unsigned				last_frame_number_of_DX8_calls						= 0;
static unsigned				last_frame_draw_calls									= 0;

static D3DPRESENT_PARAMETERS								_PresentParameters;
static DynamicVectorClass<StringClass>					_RenderDeviceNameTable;
static DynamicVectorClass<StringClass>					_RenderDeviceShortNameTable;
static DynamicVectorClass<RenderDeviceDescClass>	_RenderDeviceDescriptionTable;


typedef IDirect3D8* (WINAPI *Direct3DCreate8Type) (UINT SDKVersion);
Direct3DCreate8Type	Direct3DCreate8Ptr = NULL;
HINSTANCE D3D8Lib = NULL;

DX8_CleanupHook	 *DX8Wrapper::m_pCleanupHook=NULL;
#ifdef EXTENDED_STATS
DX8_Stats	 DX8Wrapper::stats;
#endif
/***********************************************************************************
**
** DX8Wrapper Implementation
**
***********************************************************************************/

void Log_DX8_ErrorCode(unsigned res)
{
	char tmp[256]="";

	HRESULT new_res=D3DXGetErrorStringA(
		res,
		tmp,
		sizeof(tmp));

	if (new_res==D3D_OK) {
		WWDEBUG_SAY((tmp));
	}

	WWASSERT(0);
}

void Non_Fatal_Log_DX8_ErrorCode(unsigned res,const char * file,int line)
{
	char tmp[256]="";

	HRESULT new_res=D3DXGetErrorStringA(
		res,
		tmp,
		sizeof(tmp));

	if (new_res==D3D_OK) {
		WWDEBUG_SAY(("DX8 Error: %s, File: %s, Line: %d",tmp,file,line));
	}
}

// TheSuperHackers @info helmutbuhler 14/04/2025
// Helper function that moves x and y such that the inner rect fits into the outer rect.
// If the inner rect already is in the outer rect, then this does nothing.
// If the inner rect is larger than the outer rect, then the inner rect will be aligned to the top left of the outer rect.
void MoveRectIntoOtherRect(const RECT& inner, const RECT& outer, int* x, int* y)
{
	int dx = 0;
	if (inner.right > outer.right)
		dx = outer.right-inner.right;
	if (inner.left < outer.left)
		dx = outer.left-inner.left;

	int dy = 0;
	if (inner.bottom > outer.bottom)
		dy = outer.bottom-inner.bottom;
	if (inner.top < outer.top)
		dy = outer.top-inner.top;

	*x += dx;
	*y += dy;
}


bool DX8Wrapper::Init(void * hwnd, bool lite)
{
	WWASSERT(!IsInitted);

	memset(Vertex_Shader_Constants,0,sizeof(Vector4)*MAX_VERTEX_SHADER_CONSTANTS);
	memset(Pixel_Shader_Constants,0,sizeof(Vector4)*MAX_PIXEL_SHADER_CONSTANTS);
	/*
	** Initialize all variables!
	*/
	_Hwnd = (HWND)hwnd;
	_MainThreadID=ThreadClass::_Get_Current_Thread_ID();
	WWDEBUG_SAY(("DX8Wrapper main thread: 0x%x",_MainThreadID));
	CurRenderDevice = -1;
	ResolutionWidth = DEFAULT_RESOLUTION_WIDTH;
	ResolutionHeight = DEFAULT_RESOLUTION_HEIGHT;
	// Initialize Render2DClass Screen Resolution
	Render2DClass::Set_Screen_Resolution( RectClass( 0, 0, ResolutionWidth, ResolutionHeight ) );
	BitDepth = DEFAULT_BIT_DEPTH;
	IsWindowed = false;
	DX8Wrapper_IsWindowed = false;

	for (int light=0;light<4;++light) CurrentDX8LightEnables[light]=false;

	::ZeroMemory(&old_world, sizeof(D3DMATRIX));
	::ZeroMemory(&old_view, sizeof(D3DMATRIX));
	::ZeroMemory(&old_prj, sizeof(D3DMATRIX));

	//old_vertex_shader; TODO
	//old_sr_shader;
	//current_shader;

	//world_identity;
	//CurrentFogColor;

	D3DInterface = NULL;
	D3DDevice = NULL;

	WWDEBUG_SAY(("Reset DX8Wrapper statistics"));
	Reset_Statistics();

	Invalidate_Cached_Render_States();

	if (!lite) {
		D3D8Lib = LoadLibrary("D3D8.DLL");

		if (D3D8Lib == NULL) return false;	// Return false at this point if init failed

		Direct3DCreate8Ptr = (Direct3DCreate8Type) GetProcAddress(D3D8Lib, "Direct3DCreate8");
		if (Direct3DCreate8Ptr == NULL) return false;

		/*
		** Create the D3D interface object
		*/
		WWDEBUG_SAY(("Create Direct3D8"));
		D3DInterface = Direct3DCreate8Ptr(D3D_SDK_VERSION);		// TODO: handle failure cases...
		if (D3DInterface == NULL) {
			return(false);
		}
		IsInitted = true;

		/*
		** Enumerate the available devices
		*/
		WWDEBUG_SAY(("Enumerate devices"));
		Enumerate_Devices();
		WWDEBUG_SAY(("DX8Wrapper Init completed"));
	}

	return(true);
}

void DX8Wrapper::Shutdown(void)
{
	if (D3DDevice) {

		Set_Render_Target ((IDirect3DSurface8 *)NULL);
		Release_Device();
	}

	if (D3DInterface) {
		D3DInterface->Release();
		D3DInterface=NULL;

	}

	if (CurrentCaps)
	{
		int max=CurrentCaps->Get_Max_Textures_Per_Pass();
		for (int i = 0; i < max; i++)
		{
			if (Textures[i])
			{
				Textures[i]->Release();
				Textures[i] = NULL;
			}
		}
	}

	if (D3DInterface) {
		UINT newRefCount=D3DInterface->Release();
		D3DInterface=NULL;
	}

	if (D3D8Lib) {
		FreeLibrary(D3D8Lib);
		D3D8Lib = NULL;
	}

	_RenderDeviceNameTable.Clear();		 // note - Delete_All() resizes the vector, causing a reallocation.  Clear is better. jba.
	_RenderDeviceShortNameTable.Clear();
	_RenderDeviceDescriptionTable.Clear();

	DX8Caps::Shutdown();
	IsInitted = false;		// 010803 srj
}

void DX8Wrapper::Do_Onetime_Device_Dependent_Inits(void)
{
	/*
	** Set Global render states (some of which depend on caps)
	*/
	Compute_Caps(D3DFormat_To_WW3DFormat(DisplayFormat));

   /*
	** Initalize any other subsystems inside of WW3D
	*/
	MissingTexture::_Init();
	TextureFilterClass::_Init_Filters();
	TheDX8MeshRenderer.Init();
	BoxRenderObjClass::Init();
	VertexMaterialClass::Init();
	PointGroupClass::_Init(); // This needs the VertexMaterialClass to be initted
	ShatterSystem::Init();
	TextureLoader::Init();

	Set_Default_Global_Render_States();
}

inline DWORD F2DW(float f) { return *((unsigned*)&f); }
void DX8Wrapper::Set_Default_Global_Render_States(void)
{
	DX8_THREAD_ASSERT();
	const D3DCAPS8 &caps = Get_Current_Caps()->Get_DX8_Caps();

	Set_DX8_Render_State(D3DRS_RANGEFOGENABLE, (caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE) ? TRUE : FALSE);
	Set_DX8_Render_State(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	Set_DX8_Render_State(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	Set_DX8_Render_State(D3DRS_COLORVERTEX, TRUE);
	Set_DX8_Render_State(D3DRS_ZBIAS,0);
	Set_DX8_Texture_Stage_State(1, D3DTSS_BUMPENVLSCALE, F2DW(1.0f));
	Set_DX8_Texture_Stage_State(1, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT00,F2DW(1.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT01,F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT10,F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT11,F2DW(1.0f));

//	Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_CW);
	// Set dither mode here?
}

//MW: I added this for 'Generals'.
bool DX8Wrapper::Validate_Device(void)
{	DWORD numPasses=0;
	HRESULT hRes;

	hRes=_Get_D3D_Device8()->ValidateDevice(&numPasses);

	return (hRes == D3D_OK);
}

void DX8Wrapper::Invalidate_Cached_Render_States(void)
{
	render_state_changed=0;

	int a;
	for (a=0;a<sizeof(RenderStates)/sizeof(unsigned);++a) {
		RenderStates[a]=0x12345678;
	}
	for (a=0;a<MAX_TEXTURE_STAGES;++a)
	{
		for (int b=0; b<32;b++)
		{
			TextureStageStates[a][b]=0x12345678;
		}
		//Need to explicitly set texture to NULL, otherwise app will not be able to
		//set it to null because of redundant state checker. MW
		if (_Get_D3D_Device8())
			_Get_D3D_Device8()->SetTexture(a,NULL);
		if (Textures[a] != NULL) {
			Textures[a]->Release();
		}
		Textures[a]=NULL;
	}

	ShaderClass::Invalidate();

	//Need to explicitly set render_state texture pointers to NULL. MW
	Release_Render_State();

	// (gth) clear the matrix shadows too
	for (int i=0; i<D3DTS_WORLD+1; i++) {
		DX8Transforms[i][0].Set(0,0,0,0);
		DX8Transforms[i][1].Set(0,0,0,0);
		DX8Transforms[i][2].Set(0,0,0,0);
		DX8Transforms[i][3].Set(0,0,0,0);
	}

}

void DX8Wrapper::Do_Onetime_Device_Dependent_Shutdowns(void)
{
	/*
	** Shutdown ww3d systems
	*/
	int i;
	if (render_state.vertex_buffer) render_state.vertex_buffer->Release_Engine_Ref();
	REF_PTR_RELEASE(render_state.vertex_buffer);
	if (render_state.index_buffer) render_state.index_buffer->Release_Engine_Ref();
	REF_PTR_RELEASE(render_state.index_buffer);
	REF_PTR_RELEASE(render_state.material);
	for (i=0;i<CurrentCaps->Get_Max_Textures_Per_Pass();++i) REF_PTR_RELEASE(render_state.Textures[i]);


	TextureLoader::Deinit();
	SortingRendererClass::Deinit();
	DynamicVBAccessClass::_Deinit();
	DynamicIBAccessClass::_Deinit();
	ShatterSystem::Shutdown();
	PointGroupClass::_Shutdown();
	VertexMaterialClass::Shutdown();
	BoxRenderObjClass::Shutdown();
	TheDX8MeshRenderer.Shutdown();
	MissingTexture::_Deinit();

	delete CurrentCaps;
	CurrentCaps=NULL;

}


bool DX8Wrapper::Create_Device(void)
{
	WWASSERT(D3DDevice==NULL);	// for now, once you've created a device, you're stuck with it!

	D3DCAPS8 caps;
	if
	(
		FAILED
		(
			D3DInterface->GetDeviceCaps
			(
				CurRenderDevice,
				WW3D_DEVTYPE,
				&caps
			)
		)
	)
	{
		return false;
	}

	::ZeroMemory(&CurrentAdapterIdentifier, sizeof(D3DADAPTER_IDENTIFIER8));

	if
	(
		FAILED
		(
			D3DInterface->GetAdapterIdentifier
			(
				CurRenderDevice,
				D3DENUM_NO_WHQL_LEVEL,
				&CurrentAdapterIdentifier
			)
			)
	)
	{
		return false;
	}

	Vertex_Processing_Behavior=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if (caps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		Vertex_Processing_Behavior=D3DCREATE_MIXED_VERTEXPROCESSING;
	}

#ifdef CREATE_DX8_MULTI_THREADED
	Vertex_Processing_Behavior|=D3DCREATE_MULTITHREADED;
	_DX8SingleThreaded=false;
#else
	_DX8SingleThreaded=true;
#endif

	if (DX8Wrapper_PreserveFPU)
		Vertex_Processing_Behavior |= D3DCREATE_FPU_PRESERVE;

#ifdef CREATE_DX8_FPU_PRESERVE
	Vertex_Processing_Behavior|=D3DCREATE_FPU_PRESERVE;
#endif

	HRESULT hr=D3DInterface->CreateDevice
	(
		CurRenderDevice,
		WW3D_DEVTYPE,
		_Hwnd,
		Vertex_Processing_Behavior,
		&_PresentParameters,
		&D3DDevice
	);

	if (FAILED(hr))
	{
		return false;
	}

	/*
	** Initialize all subsystems
	*/
	Do_Onetime_Device_Dependent_Inits();
	return true;
}

bool DX8Wrapper::Reset_Device(bool reload_assets)
{
	WWDEBUG_SAY(("Resetting device."));
	DX8_THREAD_ASSERT();
	if ((IsInitted) && (D3DDevice != NULL)) {
		// Release all non-MANAGED stuff
		Set_Vertex_Buffer (NULL);
		Set_Index_Buffer (NULL, 0);
		if (m_pCleanupHook) {
			m_pCleanupHook->ReleaseResources();
		}
		DynamicVBAccessClass::_Deinit();
		DynamicIBAccessClass::_Deinit();
		DX8TextureManagerClass::Release_Textures();

		memset(Vertex_Shader_Constants,0,sizeof(Vector4)*MAX_VERTEX_SHADER_CONSTANTS);
		memset(Pixel_Shader_Constants,0,sizeof(Vector4)*MAX_PIXEL_SHADER_CONSTANTS);

		HRESULT hr=_Get_D3D_Device8()->TestCooperativeLevel();
		if (hr != D3DERR_DEVICELOST )
		{	DX8CALL_HRES(Reset(&_PresentParameters),hr)
			if (hr != D3D_OK)
				return false;	//reset failed.
		}
		else
			return false;	//device is lost and can't be reset.

		if (reload_assets)
		{
			DX8TextureManagerClass::Recreate_Textures();
			if (m_pCleanupHook) {
				m_pCleanupHook->ReAcquireResources();
			}
		}
		Invalidate_Cached_Render_States();
		Set_Default_Global_Render_States();
		WWDEBUG_SAY(("Device reset completed"));
		return true;
	}
	WWDEBUG_SAY(("Device reset failed"));
	return false;
}

void DX8Wrapper::Release_Device(void)
{
	if (D3DDevice) {

		for (int a=0;a<MAX_TEXTURE_STAGES;++a)
		{	//release references to any textures that were used in last rendering call
			DX8CALL(SetTexture(a,NULL));
		}

		DX8CALL(SetStreamSource(0, NULL, 0));	//release reference count on last rendered vertex buffer
		DX8CALL(SetIndices(NULL,0));	//release reference count on last rendered index buffer


		/*
		** Release the current vertex and index buffers
		*/
		if (render_state.vertex_buffer) render_state.vertex_buffer->Release_Engine_Ref();
		REF_PTR_RELEASE(render_state.vertex_buffer);
		if (render_state.index_buffer) render_state.index_buffer->Release_Engine_Ref();
		REF_PTR_RELEASE(render_state.index_buffer);

		/*
		** Shutdown all subsystems
		*/
		Do_Onetime_Device_Dependent_Shutdowns();

		/*
		** Release the device
		*/

		D3DDevice->Release();
		D3DDevice=NULL;
	}
}

void DX8Wrapper::Enumerate_Devices()
{
	DX8_Assert();

	int adapter_count = D3DInterface->GetAdapterCount();
	for (int adapter_index=0; adapter_index<adapter_count; adapter_index++) {

		D3DADAPTER_IDENTIFIER8 id;
		::ZeroMemory(&id, sizeof(D3DADAPTER_IDENTIFIER8));
		HRESULT res = D3DInterface->GetAdapterIdentifier(adapter_index,D3DENUM_NO_WHQL_LEVEL,&id);

		if (res == D3D_OK) {

			/*
			** Set up the device name
			*/
			StringClass device_name = id.Description;
			_RenderDeviceNameTable.Add(device_name);
			_RenderDeviceShortNameTable.Add(device_name);	// for now, just add the same name to the "pretty name table"

			/*
			** Set up the render device description
			** TODO: Fill in more fields of the render device description?  (need some lookup tables)
			*/
			RenderDeviceDescClass desc;
			desc.set_device_name(id.Description);
			desc.set_driver_name(id.Driver);

			char buf[64];
			sprintf(buf,"%d.%d.%d.%d", //"%04x.%04x.%04x.%04x",
				HIWORD(id.DriverVersion.HighPart),
				LOWORD(id.DriverVersion.HighPart),
				HIWORD(id.DriverVersion.LowPart),
				LOWORD(id.DriverVersion.LowPart));

			desc.set_driver_version(buf);

			/*
			** Enumerate the resolutions
			*/
			desc.reset_resolution_list();
			int mode_count = D3DInterface->GetAdapterModeCount(adapter_index);
			for (int mode_index=0; mode_index<mode_count; mode_index++) {
				D3DDISPLAYMODE d3dmode;
				::ZeroMemory(&d3dmode, sizeof(D3DDISPLAYMODE));
				HRESULT res = D3DInterface->EnumAdapterModes(adapter_index,mode_index,&d3dmode);

				if (res == D3D_OK) {
					int bits = 0;
					switch (d3dmode.Format)
					{
						case D3DFMT_R8G8B8:
						case D3DFMT_A8R8G8B8:
						case D3DFMT_X8R8G8B8:		bits = 32; break;

						case D3DFMT_R5G6B5:
						case D3DFMT_X1R5G5B5:		bits = 16; break;
					}

					/*
					** If we recognize the format, add it to the list
					** TODO: should we handle more formats?  will any cards report more than 24 or 16 bit?
					*/
					if (bits != 0) {
						desc.add_resolution(d3dmode.Width,d3dmode.Height,bits);
					}
				}
			}

			/*
			** Add the render device to our table
			*/
			_RenderDeviceDescriptionTable.Add(desc);
		}
	}
}

bool DX8Wrapper::Set_Any_Render_Device(void)
{
	// Try windowed first
	int dev_number = 0;
	for (; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if (Set_Render_Device(dev_number,-1,-1,-1,1,false)) {
			return true;
		}
	}

	// Then fullscreen
	for (dev_number = 0; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if (Set_Render_Device(dev_number,-1,-1,-1,0,false)) {
			return true;
		}
	}

	return false;
}

bool DX8Wrapper::Set_Render_Device
(
	const char * dev_name,
	int width,
	int height,
	int bits,
	int windowed,
	bool resize_window
)
{
	for ( int dev_number = 0; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if ( strcmp( dev_name, _RenderDeviceNameTable[dev_number]) == 0) {
			return Set_Render_Device( dev_number, width, height, bits, windowed, resize_window );
		}

		if ( strcmp( dev_name, _RenderDeviceShortNameTable[dev_number]) == 0) {
			return Set_Render_Device( dev_number, width, height, bits, windowed, resize_window );
		}
	}
	return false;
}

void DX8Wrapper::Get_Format_Name(unsigned int format, StringClass *tex_format)
{
		*tex_format="Unknown";
		switch (format) {
		case D3DFMT_A8R8G8B8: *tex_format="D3DFMT_A8R8G8B8"; break;
		case D3DFMT_R8G8B8: *tex_format="D3DFMT_R8G8B8"; break;
		case D3DFMT_A4R4G4B4: *tex_format="D3DFMT_A4R4G4B4"; break;
		case D3DFMT_A1R5G5B5: *tex_format="D3DFMT_A1R5G5B5"; break;
		case D3DFMT_R5G6B5: *tex_format="D3DFMT_R5G6B5"; break;
		case D3DFMT_L8: *tex_format="D3DFMT_L8"; break;
		case D3DFMT_A8: *tex_format="D3DFMT_A8"; break;
		case D3DFMT_P8: *tex_format="D3DFMT_P8"; break;
		case D3DFMT_X8R8G8B8: *tex_format="D3DFMT_X8R8G8B8"; break;
		case D3DFMT_X1R5G5B5: *tex_format="D3DFMT_X1R5G5B5"; break;
		case D3DFMT_R3G3B2: *tex_format="D3DFMT_R3G3B2"; break;
		case D3DFMT_A8R3G3B2: *tex_format="D3DFMT_A8R3G3B2"; break;
		case D3DFMT_X4R4G4B4: *tex_format="D3DFMT_X4R4G4B4"; break;
		case D3DFMT_A8P8: *tex_format="D3DFMT_A8P8"; break;
		case D3DFMT_A8L8: *tex_format="D3DFMT_A8L8"; break;
		case D3DFMT_A4L4: *tex_format="D3DFMT_A4L4"; break;
		case D3DFMT_V8U8: *tex_format="D3DFMT_V8U8"; break;
		case D3DFMT_L6V5U5: *tex_format="D3DFMT_L6V5U5"; break;
		case D3DFMT_X8L8V8U8: *tex_format="D3DFMT_X8L8V8U8"; break;
		case D3DFMT_Q8W8V8U8: *tex_format="D3DFMT_Q8W8V8U8"; break;
		case D3DFMT_V16U16: *tex_format="D3DFMT_V16U16"; break;
		case D3DFMT_W11V11U10: *tex_format="D3DFMT_W11V11U10"; break;
		case D3DFMT_UYVY: *tex_format="D3DFMT_UYVY"; break;
		case D3DFMT_YUY2: *tex_format="D3DFMT_YUY2"; break;
		case D3DFMT_DXT1: *tex_format="D3DFMT_DXT1"; break;
		case D3DFMT_DXT2: *tex_format="D3DFMT_DXT2"; break;
		case D3DFMT_DXT3: *tex_format="D3DFMT_DXT3"; break;
		case D3DFMT_DXT4: *tex_format="D3DFMT_DXT4"; break;
		case D3DFMT_DXT5: *tex_format="D3DFMT_DXT5"; break;
		case D3DFMT_D16_LOCKABLE: *tex_format="D3DFMT_D16_LOCKABLE"; break;
		case D3DFMT_D32: *tex_format="D3DFMT_D32"; break;
		case D3DFMT_D15S1: *tex_format="D3DFMT_D15S1"; break;
		case D3DFMT_D24S8: *tex_format="D3DFMT_D24S8"; break;
		case D3DFMT_D16: *tex_format="D3DFMT_D16"; break;
		case D3DFMT_D24X8: *tex_format="D3DFMT_D24X8"; break;
		case D3DFMT_D24X4S4: *tex_format="D3DFMT_D24X4S4"; break;
		default:	break;
		}
}

void DX8Wrapper::Resize_And_Position_Window()
{
	// Get the current dimensions of the 'render area' of the window
	RECT rect = { 0 };
	::GetClientRect (_Hwnd, &rect);

	// Is the window the correct size for this resolution?
	if ((rect.right-rect.left) != ResolutionWidth ||
			(rect.bottom-rect.top) != ResolutionHeight) {

		// Calculate what the main window's bounding rectangle should be to
		// accommodate this resolution
		rect.left = 0;
		rect.top = 0;
		rect.right = ResolutionWidth;
		rect.bottom = ResolutionHeight;
		DWORD dwstyle = ::GetWindowLong (_Hwnd, GWL_STYLE);
		AdjustWindowRect (&rect, dwstyle, FALSE);
		int width = rect.right-rect.left;
		int height = rect.bottom-rect.top;

		// Resize the window to fit this resolution
		if (!IsWindowed)
		{
			::SetWindowPos(_Hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_NOSIZE | SWP_NOMOVE);

			DEBUG_LOG(("Window resized to w:%d h:%d", width, height));
		}
		else
		{
			// TheSuperHackers @feature helmutbuhler 14/04/2025
			// Center the window in the workarea of the monitor it is on.
			MONITORINFO mi = {sizeof(MONITORINFO)};
			GetMonitorInfo(MonitorFromWindow(_Hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
			int left = (mi.rcWork.left + mi.rcWork.right - width) / 2;
			int top  = (mi.rcWork.top + mi.rcWork.bottom - height) / 2;

			// TheSuperHackers @feature helmutbuhler 14/04/2025
			// Move the window to try fit it into the monitor area, if one of its dimensions is larger than the work area.
			// Otherwise align the window to the top left edges, if it is even larger than the monitor area.
			RECT rectClient;
			rectClient.left = left - rect.left;
			rectClient.top = top - rect.top;
			rectClient.right = rectClient.left + ResolutionWidth;
			rectClient.bottom = rectClient.top + ResolutionHeight;
			MoveRectIntoOtherRect(rectClient, mi.rcMonitor, &left, &top);

			::SetWindowPos (_Hwnd, NULL, left, top, width, height, SWP_NOZORDER);

			DEBUG_LOG(("Window positioned to x:%d y:%d, resized to w:%d h:%d", left, top, width, height));
		}
	}
}

bool DX8Wrapper::Set_Render_Device(int dev, int width, int height, int bits, int windowed,
								   bool resize_window,bool reset_device, bool restore_assets)
{
	WWASSERT(IsInitted);
	WWASSERT(dev >= -1);
	WWASSERT(dev < _RenderDeviceNameTable.Count());

	/*
	** If user has never selected a render device, start out with device 0
	*/
	if ((CurRenderDevice == -1) && (dev == -1)) {
		CurRenderDevice = 0;
	} else if (dev != -1) {
		CurRenderDevice = dev;
	}

	/*
	** If user doesn't want to change res, set the res variables to match the
	** current resolution
	*/
	if (width != -1)		ResolutionWidth = width;
	if (height != -1)		ResolutionHeight = height;

	// Initialize Render2DClass Screen Resolution
	Render2DClass::Set_Screen_Resolution( RectClass( 0, 0, ResolutionWidth, ResolutionHeight ) );

	if (bits != -1)		BitDepth = bits;
	if (windowed != -1)	IsWindowed = (windowed != 0);
	DX8Wrapper_IsWindowed = IsWindowed;

	WWDEBUG_SAY(("Attempting Set_Render_Device: name: %s (%s:%s), width: %d, height: %d, windowed: %d",
		_RenderDeviceNameTable[CurRenderDevice].str(),_RenderDeviceDescriptionTable[CurRenderDevice].Get_Driver_Name(),
		_RenderDeviceDescriptionTable[CurRenderDevice].Get_Driver_Version(),ResolutionWidth,ResolutionHeight,(IsWindowed ? 1 : 0)));

#ifdef _WINDOWS
	// PWG 4/13/2000 - changed so that if you say to resize the window it resizes
	// regardless of whether its windowed or not as OpenGL resizes its self around
	// the caption and edges of the window type you provide, so its important to
	// push the client area to be the size you really want.
	// if ( resize_window && windowed ) {
	if (resize_window) {
		Resize_And_Position_Window();
	}
#endif
	//must be either resetting existing device or creating a new one.
	WWASSERT(reset_device || D3DDevice == NULL);

	/*
	** Initialize values for D3DPRESENT_PARAMETERS members.
	*/
	::ZeroMemory(&_PresentParameters, sizeof(D3DPRESENT_PARAMETERS));

	_PresentParameters.BackBufferWidth = ResolutionWidth;
	_PresentParameters.BackBufferHeight = ResolutionHeight;
	_PresentParameters.BackBufferCount = IsWindowed ? 1 : 2;

	_PresentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	_PresentParameters.SwapEffect = IsWindowed ? D3DSWAPEFFECT_DISCARD : D3DSWAPEFFECT_FLIP;		// Shouldn't this be D3DSWAPEFFECT_FLIP?
	_PresentParameters.hDeviceWindow = _Hwnd;
	_PresentParameters.Windowed = IsWindowed;

	_PresentParameters.EnableAutoDepthStencil = TRUE;				// Driver will attempt to match Z-buffer depth
	_PresentParameters.Flags=0;											// We're not going to lock the backbuffer

	_PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	_PresentParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	/*
	** Set up the buffer formats.  Several issues here:
	** - if in windowed mode, the backbuffer must use the current display format.
	** - the depth buffer must use
	*/
	if (IsWindowed) {

		D3DDISPLAYMODE desktop_mode;
		::ZeroMemory(&desktop_mode, sizeof(D3DDISPLAYMODE));
		D3DInterface->GetAdapterDisplayMode( CurRenderDevice, &desktop_mode );

		DisplayFormat=_PresentParameters.BackBufferFormat = desktop_mode.Format;

		// In windowed mode, define the bitdepth from desktop mode (as it can't be changed)
		switch (_PresentParameters.BackBufferFormat) {
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8R8G8B8:
		case D3DFMT_R8G8B8: BitDepth=32; break;
		case D3DFMT_A4R4G4B4:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_R5G6B5: BitDepth=16; break;
		case D3DFMT_L8:
		case D3DFMT_A8:
		case D3DFMT_P8: BitDepth=8; break;
		default:
			// Unknown backbuffer format probably means the device can't do windowed
			return false;
		}

		if (BitDepth==32 && D3DInterface->CheckDeviceType(0,D3DDEVTYPE_HAL,desktop_mode.Format,D3DFMT_A8R8G8B8, TRUE) == D3D_OK)
		{	//promote 32-bit modes to include destination alpha
			_PresentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
		}

		/*
		** Find a appropriate Z buffer
		*/
		if (!Find_Z_Mode(DisplayFormat,_PresentParameters.BackBufferFormat,&_PresentParameters.AutoDepthStencilFormat))
		{
			// If opening 32 bit mode failed, try 16 bit, even if the desktop happens to be 32 bit
			if (BitDepth==32) {
				BitDepth=16;
				_PresentParameters.BackBufferFormat=D3DFMT_R5G6B5;
				if (!Find_Z_Mode(_PresentParameters.BackBufferFormat,_PresentParameters.BackBufferFormat,&_PresentParameters.AutoDepthStencilFormat)) {
					_PresentParameters.AutoDepthStencilFormat=D3DFMT_UNKNOWN;
				}
			}
			else {
				_PresentParameters.AutoDepthStencilFormat=D3DFMT_UNKNOWN;
			}
		}

	} else {

		/*
		** Try to find a mode that matches the user's desired bit-depth.
		*/
		Find_Color_And_Z_Mode(ResolutionWidth,ResolutionHeight,BitDepth,&DisplayFormat,
			&_PresentParameters.BackBufferFormat,&_PresentParameters.AutoDepthStencilFormat);
	}

	/*
	** Time to actually create the device.
	*/
	if (_PresentParameters.AutoDepthStencilFormat==D3DFMT_UNKNOWN) {
		if (BitDepth==32) {
			_PresentParameters.AutoDepthStencilFormat=D3DFMT_D32;
		}
		else {
			_PresentParameters.AutoDepthStencilFormat=D3DFMT_D16;
		}
	}

	StringClass displayFormat;
	StringClass backbufferFormat;

	Get_Format_Name(DisplayFormat,&displayFormat);
	Get_Format_Name(_PresentParameters.BackBufferFormat,&backbufferFormat);

	WWDEBUG_SAY(("Using Display/BackBuffer Formats: %s/%s",displayFormat.str(),backbufferFormat.str()));

	bool ret;

	if (reset_device)
	{
		WWDEBUG_SAY(("DX8Wrapper::Set_Render_Device is resetting the device."));
		ret = Reset_Device(restore_assets);	//reset device without restoring data - we're likely switching out of the app.
	}
	else
		ret = Create_Device();

	WWDEBUG_SAY(("Reset/Create_Device done, reset_device=%d, restore_assets=%d", reset_device, restore_assets));

	return ret;
}

bool DX8Wrapper::Set_Next_Render_Device(void)
{
	int new_dev = (CurRenderDevice + 1) % _RenderDeviceNameTable.Count();
	return Set_Render_Device(new_dev);
}

bool DX8Wrapper::Toggle_Windowed(void)
{
#ifdef WW3D_DX8
	// State OK?
	assert (IsInitted);
	if (IsInitted) {

		// Get information about the current render device's resolutions
		const RenderDeviceDescClass &render_device = Get_Render_Device_Desc ();
		const DynamicVectorClass<ResolutionDescClass> &resolutions = render_device.Enumerate_Resolutions ();

		// Loop through all the resolutions supported by the current device.
		// If we aren't currently running under one of these resolutions,
		// then we should probably		 to the closest resolution before
		// toggling the windowed state.
		int curr_res = -1;
		for (int res = 0;
		     (res < resolutions.Count ()) && (curr_res == -1);
			  res ++) {

			// Is this the resolution we are looking for?
			if ((resolutions[res].Width == ResolutionWidth) &&
				 (resolutions[res].Height == ResolutionHeight) &&
				 (resolutions[res].BitDepth == BitDepth)) {
				curr_res = res;
			}
		}

		if (curr_res == -1) {

			// We don't match any of the standard resolutions,
			// so set the first resolution and toggle the windowed state.
			return Set_Device_Resolution (resolutions[0].Width,
								 resolutions[0].Height,
								 resolutions[0].BitDepth,
								 !IsWindowed, true);
		} else {

			// Toggle the windowed state
			return Set_Device_Resolution (-1, -1, -1, !IsWindowed, true);
		}
	}
#endif //WW3D_DX8

	return false;
}

void DX8Wrapper::Set_Swap_Interval(int swap)
{
	switch (swap) {
		case 0: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; break;
		case 1: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE ; break;
		case 2: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_TWO; break;
		case 3: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_THREE; break;
		default: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE ; break;
	}

	WWDEBUG_SAY(("DX8Wrapper::Set_Swap_Interval is resetting the device."));
	Reset_Device();
}

int DX8Wrapper::Get_Swap_Interval(void)
{
	return _PresentParameters.FullScreen_PresentationInterval;
}

bool DX8Wrapper::Has_Stencil(void)
{
	bool has_stencil = (_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24S8 ||
						_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24X4S4);
	return has_stencil;
}

int DX8Wrapper::Get_Render_Device_Count(void)
{
	return _RenderDeviceNameTable.Count();

}
int DX8Wrapper::Get_Render_Device(void)
{
	assert(IsInitted);
	return CurRenderDevice;
}

const RenderDeviceDescClass & DX8Wrapper::Get_Render_Device_Desc(int deviceidx)
{
	WWASSERT(IsInitted);

	if ((deviceidx == -1) && (CurRenderDevice == -1)) {
		CurRenderDevice = 0;
	}

	// if the device index is -1 then we want the current device
	if (deviceidx == -1) {
		WWASSERT(CurRenderDevice >= 0);
		WWASSERT(CurRenderDevice < _RenderDeviceNameTable.Count());
		return _RenderDeviceDescriptionTable[CurRenderDevice];
	}

	// We can only ask for multiple device information if the devices
	// have been detected.
	WWASSERT(deviceidx >= 0);
	WWASSERT(deviceidx < _RenderDeviceNameTable.Count());
	return _RenderDeviceDescriptionTable[deviceidx];
}

const char * DX8Wrapper::Get_Render_Device_Name(int device_index)
{
	device_index = device_index % _RenderDeviceShortNameTable.Count();
	return _RenderDeviceShortNameTable[device_index];
}

bool DX8Wrapper::Set_Device_Resolution(int width,int height,int bits,int windowed, bool resize_window)
{
	if (D3DDevice != NULL) {

		if (width != -1) {
			_PresentParameters.BackBufferWidth = ResolutionWidth = width;
		}
		if (height != -1) {
			_PresentParameters.BackBufferHeight = ResolutionHeight = height;
		}
		if (resize_window)
		{
			Resize_And_Position_Window();
		}
#pragma message("TODO: support changing windowed status and changing the bit depth")
		WWDEBUG_SAY(("DX8Wrapper::Set_Device_Resolution is resetting the device."));
		return Reset_Device();
	} else {
		return false;
	}
}

void DX8Wrapper::Get_Device_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed)
{
	WWASSERT(IsInitted);

	set_w = ResolutionWidth;
	set_h = ResolutionHeight;
	set_bits = BitDepth;
	set_windowed = IsWindowed;

	return ;
}

void DX8Wrapper::Get_Render_Target_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed)
{
	WWASSERT(IsInitted);

	if (CurrentRenderTarget != NULL) {
		D3DSURFACE_DESC info;
		CurrentRenderTarget->GetDesc (&info);

		set_w				= info.Width;
		set_h				= info.Height;
		set_bits			= BitDepth;		// should we get the actual bit depth of the target?
		set_windowed	= IsWindowed;	// this doesn't really make sense for render targets (shouldn't matter)...

	} else {
		Get_Device_Resolution (set_w, set_h, set_bits, set_windowed);
	}

	return ;
}

bool DX8Wrapper::Registry_Save_Render_Device( const char * sub_key )
{
	int	width, height, depth;
	bool	windowed;
	Get_Device_Resolution(width, height, depth, windowed);
	return Registry_Save_Render_Device(sub_key, CurRenderDevice, ResolutionWidth, ResolutionHeight, BitDepth, IsWindowed, TextureBitDepth);
}

bool DX8Wrapper::Registry_Save_Render_Device( const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth)
{
	RegistryClass * registry = W3DNEW RegistryClass( sub_key );
	WWASSERT( registry );

	if ( !registry->Is_Valid() ) {
		delete registry;
		WWDEBUG_SAY(( "Error getting Registry" ));
		return false;
	}

	registry->Set_String( VALUE_NAME_RENDER_DEVICE_NAME,
		_RenderDeviceShortNameTable[device] );
	registry->Set_Int( VALUE_NAME_RENDER_DEVICE_WIDTH,	width );
	registry->Set_Int( VALUE_NAME_RENDER_DEVICE_HEIGHT, height );
	registry->Set_Int( VALUE_NAME_RENDER_DEVICE_DEPTH, depth );
	registry->Set_Int( VALUE_NAME_RENDER_DEVICE_WINDOWED, windowed );
	registry->Set_Int( VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, texture_depth );

	delete registry;
	return true;
}

bool DX8Wrapper::Registry_Load_Render_Device( const char * sub_key, bool resize_window )
{
	char	name[ 200 ];
	int	width,height,depth,windowed;

	if (	Registry_Load_Render_Device(	sub_key,
													name,
													sizeof(name),
													width,
													height,
													depth,
													windowed,
													TextureBitDepth) &&
			(*name != 0))
	{
		WWDEBUG_SAY(( "Device %s (%d X %d) %d bit windowed:%d", name,width,height,depth,windowed));

		if (TextureBitDepth==16 || TextureBitDepth==32) {
//			WWDEBUG_SAY(( "Texture depth %d", TextureBitDepth));
		} else {
			WWDEBUG_SAY(( "Invalid texture depth %d, switching to 16 bits", TextureBitDepth));
			TextureBitDepth=16;
		}

		if ( Set_Render_Device( name, width,height,depth,windowed, resize_window ) != true) {
			return Set_Any_Render_Device();
		}

		return true;
	}

	WWDEBUG_SAY(( "Error getting Registry" ));

	return Set_Any_Render_Device();
}

bool DX8Wrapper::Registry_Load_Render_Device( const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth)
{
	RegistryClass registry( sub_key );

	if ( registry.Is_Valid() ) {
		registry.Get_String( VALUE_NAME_RENDER_DEVICE_NAME,
			device, device_len);

		width =		registry.Get_Int( VALUE_NAME_RENDER_DEVICE_WIDTH, -1 );
		height =		registry.Get_Int( VALUE_NAME_RENDER_DEVICE_HEIGHT, -1 );
		depth =		registry.Get_Int( VALUE_NAME_RENDER_DEVICE_DEPTH, -1 );
		windowed =	registry.Get_Int( VALUE_NAME_RENDER_DEVICE_WINDOWED, -1 );
		texture_depth = registry.Get_Int( VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, -1 );
		return true;
	}
	*device=0;
	width=-1;
	height=-1;
	depth=-1;
	windowed=-1;
	texture_depth=-1;
	return false;
}


bool DX8Wrapper::Find_Color_And_Z_Mode(int resx,int resy,int bitdepth,D3DFORMAT * set_colorbuffer,D3DFORMAT * set_backbuffer,D3DFORMAT * set_zmode)
{
	static D3DFORMAT _formats16[] =
	{
		D3DFMT_R5G6B5,
		D3DFMT_X1R5G5B5,
		D3DFMT_A1R5G5B5
	};

	static D3DFORMAT _formats32[] =
	{
		D3DFMT_A8R8G8B8,
		D3DFMT_X8R8G8B8,
		D3DFMT_R8G8B8,
	};

	/*
	** Select the table that we're going to use to search for a valid backbuffer format
	*/
	D3DFORMAT * format_table = NULL;
	int format_count = 0;

	if (BitDepth == 16) {
		format_table = _formats16;
		format_count = sizeof(_formats16) / sizeof(D3DFORMAT);
	} else {
		format_table = _formats32;
		format_count = sizeof(_formats32) / sizeof(D3DFORMAT);
	}

	/*
	** now search for a valid format
	*/
	bool found = false;
	unsigned int mode = 0;

	int format_index=0;
	for (; format_index < format_count; format_index++) {
		found |= Find_Color_Mode(format_table[format_index],resx,resy,&mode);
		if (found) break;
	}

	if (!found) {
		return false;
	} else {
		*set_backbuffer=*set_colorbuffer = format_table[format_index];
	}

	if (bitdepth==32 && *set_colorbuffer == D3DFMT_X8R8G8B8 && D3DInterface->CheckDeviceType(0,D3DDEVTYPE_HAL,*set_colorbuffer,D3DFMT_A8R8G8B8, TRUE) == D3D_OK)
	{	//promote 32-bit modes to include destination alpha when supported
		*set_backbuffer = D3DFMT_A8R8G8B8;
	}

	/*
	** We found a backbuffer format, now find a zbuffer format
	*/
	return Find_Z_Mode(*set_colorbuffer,*set_backbuffer, set_zmode);
};


// find the resolution mode with at least resx,resy with the highest supported
// refresh rate
bool DX8Wrapper::Find_Color_Mode(D3DFORMAT colorbuffer, int resx, int resy, UINT *mode)
{
	UINT i,j,modemax;
	UINT rx,ry;
	D3DDISPLAYMODE dmode;
	::ZeroMemory(&dmode, sizeof(D3DDISPLAYMODE));

	rx=(unsigned int) resx;
	ry=(unsigned int) resy;

	bool found=false;

	modemax=D3DInterface->GetAdapterModeCount(D3DADAPTER_DEFAULT);

	i=0;

	while (i<modemax && !found)
	{
		D3DInterface->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &dmode);
		if (dmode.Width==rx && dmode.Height==ry && dmode.Format==colorbuffer) {
			WWDEBUG_SAY(("Found valid color mode.  Width = %d Height = %d Format = %d",dmode.Width,dmode.Height,dmode.Format));
			found=true;
		}
		i++;
	}

	i--; // this is the first valid mode

	// no match
	if (!found) {
		WWDEBUG_SAY(("Failed to find a valid color mode"));
		return false;
	}

	// go to the highest refresh rate in this mode
	bool stillok=true;

	j=i;
	while (j<modemax && stillok)
	{
		D3DInterface->EnumAdapterModes(D3DADAPTER_DEFAULT, j, &dmode);
		if (dmode.Width==rx && dmode.Height==ry && dmode.Format==colorbuffer)
			stillok=true; else stillok=false;
		j++;
	}

	if (stillok==false) *mode=j-2;
	else *mode=i;

	return true;
}

// Helper function to find a Z buffer mode for the colorbuffer
// Will look for greatest Z precision
bool DX8Wrapper::Find_Z_Mode(D3DFORMAT colorbuffer,D3DFORMAT backbuffer, D3DFORMAT *zmode)
{
	//MW: Swapped the next 2 tests so that Stencil modes get tested first.
	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D24S8))
	{
		*zmode=D3DFMT_D24S8;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24S8"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D32))
	{
		*zmode=D3DFMT_D32;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D32"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D24X8))
	{
		*zmode=D3DFMT_D24X8;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24X8"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D24X4S4))
	{
		*zmode=D3DFMT_D24X4S4;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24X4S4"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D16))
	{
		*zmode=D3DFMT_D16;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D16"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D15S1))
	{
		*zmode=D3DFMT_D15S1;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D15S1"));
		return true;
	}

	// can't find a match
	WWDEBUG_SAY(("Failed to find a valid zbuffer mode"));
	return false;
}

bool DX8Wrapper::Test_Z_Mode(D3DFORMAT colorbuffer,D3DFORMAT backbuffer, D3DFORMAT zmode)
{
	// See if we have this mode first
	if (FAILED(D3DInterface->CheckDeviceFormat(D3DADAPTER_DEFAULT,WW3D_DEVTYPE,
		colorbuffer,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,zmode)))
	{
		WWDEBUG_SAY(("CheckDeviceFormat failed.  Colorbuffer format = %d  Zbufferformat = %d",colorbuffer,zmode));
		return false;
	}

	// Then see if it matches the color buffer
	if(FAILED(D3DInterface->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, WW3D_DEVTYPE,
		colorbuffer,backbuffer,zmode)))
	{
		WWDEBUG_SAY(("CheckDepthStencilMatch failed.  Colorbuffer format = %d  Backbuffer format = %d Zbufferformat = %d",colorbuffer,backbuffer,zmode));
		return false;
	}
	return true;
}


void DX8Wrapper::Reset_Statistics()
{
	matrix_changes	= 0;
	material_changes = 0;
	vertex_buffer_changes = 0;
	index_buffer_changes = 0;
	light_changes = 0;
	texture_changes = 0;
	render_state_changes =0;
	texture_stage_state_changes =0;
	draw_calls =0;

	number_of_DX8_calls = 0;
	last_frame_matrix_changes = 0;
	last_frame_material_changes = 0;
	last_frame_vertex_buffer_changes = 0;
	last_frame_index_buffer_changes = 0;
	last_frame_light_changes = 0;
	last_frame_texture_changes = 0;
	last_frame_render_state_changes = 0;
	last_frame_texture_stage_state_changes = 0;
	last_frame_number_of_DX8_calls = 0;
	last_frame_draw_calls =0;
}

void DX8Wrapper::Begin_Statistics()
{
	matrix_changes=0;
	material_changes=0;
	vertex_buffer_changes=0;
	index_buffer_changes=0;
	light_changes=0;
	texture_changes = 0;
	render_state_changes =0;
	texture_stage_state_changes =0;
	number_of_DX8_calls=0;
	draw_calls=0;
}

void DX8Wrapper::End_Statistics()
{
	last_frame_matrix_changes=matrix_changes;
	last_frame_material_changes=material_changes;
	last_frame_vertex_buffer_changes=vertex_buffer_changes;
	last_frame_index_buffer_changes=index_buffer_changes;
	last_frame_light_changes=light_changes;
	last_frame_texture_changes = texture_changes;
	last_frame_render_state_changes = render_state_changes;
	last_frame_texture_stage_state_changes = texture_stage_state_changes;
	last_frame_number_of_DX8_calls=number_of_DX8_calls;
	last_frame_draw_calls=draw_calls;
}

unsigned DX8Wrapper::Get_Last_Frame_Matrix_Changes()			{ return last_frame_matrix_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Material_Changes()		{ return last_frame_material_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Vertex_Buffer_Changes()	{ return last_frame_vertex_buffer_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Index_Buffer_Changes()	{ return last_frame_index_buffer_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Light_Changes()			{ return last_frame_light_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Texture_Changes()			{ return last_frame_texture_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Render_State_Changes()	{ return last_frame_render_state_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Texture_Stage_State_Changes()	{ return last_frame_texture_stage_state_changes; }
unsigned DX8Wrapper::Get_Last_Frame_DX8_Calls()					{ return last_frame_number_of_DX8_calls; }
unsigned DX8Wrapper::Get_Last_Frame_Draw_Calls()				{ return last_frame_draw_calls; }
unsigned long DX8Wrapper::Get_FrameCount(void) {return FrameCount;}

void DX8_Assert()
{
	WWASSERT(DX8Wrapper::_Get_D3D8());
	DX8_THREAD_ASSERT();
}

void DX8Wrapper::Begin_Scene(void)
{
	DX8_THREAD_ASSERT();

#if ENABLE_EMBEDDED_BROWSER
	DX8WebBrowser::Update();
#endif

	DX8CALL(BeginScene());

	DX8WebBrowser::Update();
}

void DX8Wrapper::End_Scene(bool flip_frames)
{
	DX8_THREAD_ASSERT();
	DX8CALL(EndScene());

	DX8WebBrowser::Render(0);

	if (flip_frames) {
		DX8_Assert();
		HRESULT hr;
		{
			WWPROFILE("DX8Device::Present()");
			hr=_Get_D3D_Device8()->Present(NULL, NULL, NULL, NULL);
		}

		number_of_DX8_calls++;

		if (SUCCEEDED(hr)) {
#ifdef EXTENDED_STATS
			if (stats.m_sleepTime) {
				::Sleep(stats.m_sleepTime);
			}
#endif
			IsDeviceLost=false;
			FrameCount++;
		}
		else {
			IsDeviceLost=true;
		}

		// If the device was lost we need to check for cooperative level and possibly reset the device
		if (hr==D3DERR_DEVICELOST) {
			hr=_Get_D3D_Device8()->TestCooperativeLevel();
			if (hr==D3DERR_DEVICENOTRESET) {
				WWDEBUG_SAY(("DX8Wrapper::End_Scene is resetting the device."));
				Reset_Device();
			}
			else {
				// Sleep it not active
				ThreadClass::Sleep_Ms(200);
			}
		}
		else {
			DX8_ErrorCode(hr);
		}
	}

	// Each frame, release all of the buffers and textures.
	Set_Vertex_Buffer(NULL);
	Set_Index_Buffer(NULL,0);
	for (int i=0;i<CurrentCaps->Get_Max_Textures_Per_Pass();++i) Set_Texture(i,NULL);
	Set_Material(NULL);
}


void DX8Wrapper::Flip_To_Primary(void)
{
	// If we are fullscreen and the current frame is odd then we need
	// to force a page flip to ensure that the first buffer in the flipping
	// chain is the one visible.
	if (!IsWindowed) {
		DX8_Assert();

		int numBuffers = (_PresentParameters.BackBufferCount + 1);
		int visibleBuffer = (FrameCount % numBuffers);
		int flipCount = ((numBuffers - visibleBuffer) % numBuffers);
		int resetAttempts = 0;

		while ((flipCount > 0) && (resetAttempts < 3)) {
			HRESULT hr = _Get_D3D_Device8()->TestCooperativeLevel();

			if (FAILED(hr)) {
				WWDEBUG_SAY(("TestCooperativeLevel Failed!"));

				if (D3DERR_DEVICELOST == hr) {
					IsDeviceLost=true;
					WWDEBUG_SAY(("DEVICELOST: Cannot flip to primary."));
					return;
				}
				IsDeviceLost=false;

				if (D3DERR_DEVICENOTRESET == hr) {
					WWDEBUG_SAY(("DEVICENOTRESET"));
					Reset_Device();
					resetAttempts++;
				}
			} else {
				WWDEBUG_SAY(("Flipping: %ld", FrameCount));
				hr = _Get_D3D_Device8()->Present(NULL, NULL, NULL, NULL);

				if (SUCCEEDED(hr)) {
					IsDeviceLost=false;
					FrameCount++;
					WWDEBUG_SAY(("Flip to primary succeeded %ld", FrameCount));
				}
				else {
					IsDeviceLost=true;
				}
			}

			--flipCount;
		}
	}
}


void DX8Wrapper::Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float dest_alpha, float z, unsigned int stencil)
{
	DX8_THREAD_ASSERT();
	// If we try to clear a stencil buffer which is not there, the entire call will fail
	bool has_stencil = (	_PresentParameters.AutoDepthStencilFormat == D3DFMT_D15S1 ||
								_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24S8 ||
								_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24X4S4);

	DWORD flags = 0;
	if (clear_color) flags |= D3DCLEAR_TARGET;
	if (clear_z_stencil) flags |= D3DCLEAR_ZBUFFER;
	if (clear_z_stencil && has_stencil) flags |= D3DCLEAR_STENCIL;
	if (flags)
	{
		DX8CALL(Clear(0, NULL, flags, Convert_Color(color,dest_alpha), z, stencil));
	}
}

void DX8Wrapper::Set_Viewport(CONST D3DVIEWPORT8* pViewport)
{
	DX8_THREAD_ASSERT();
	DX8CALL(SetViewport(pViewport));
}

// ----------------------------------------------------------------------------
//
// Set vertex buffer. A reference to previous vertex buffer is released and
// this one is assigned the current vertex buffer. The DX8 vertex buffer will
// actually be set in Apply() which is called by Draw_Indexed_Triangles().
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Vertex_Buffer(const VertexBufferClass* vb)
{
	render_state.vba_offset=0;
	render_state.vba_count=0;
	if (render_state.vertex_buffer) {
		render_state.vertex_buffer->Release_Engine_Ref();
	}
	REF_PTR_SET(render_state.vertex_buffer,const_cast<VertexBufferClass*>(vb));
	if (vb) {
		vb->Add_Engine_Ref();
		render_state.vertex_buffer_type=vb->Type();
	}
	else {
		render_state.index_buffer_type=BUFFER_TYPE_INVALID;
	}
	render_state_changed|=VERTEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
//
// Set index buffer. A reference to previous index buffer is released and
// this one is assigned the current index buffer. The DX8 index buffer will
// actually be set in Apply() which is called by Draw_Indexed_Triangles().
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Index_Buffer(const IndexBufferClass* ib,unsigned short index_base_offset)
{
	render_state.iba_offset=0;
	if (render_state.index_buffer) {
		render_state.index_buffer->Release_Engine_Ref();
	}
	REF_PTR_SET(render_state.index_buffer,const_cast<IndexBufferClass*>(ib));
	render_state.index_base_offset=index_base_offset;
	if (ib) {
		ib->Add_Engine_Ref();
		render_state.index_buffer_type=ib->Type();
	}
	else {
		render_state.index_buffer_type=BUFFER_TYPE_INVALID;
	}
	render_state_changed|=INDEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
//
// Set vertex buffer using dynamic access object.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Vertex_Buffer(const DynamicVBAccessClass& vba_)
{
	if (render_state.vertex_buffer) render_state.vertex_buffer->Release_Engine_Ref();

	DynamicVBAccessClass& vba=const_cast<DynamicVBAccessClass&>(vba_);
	render_state.vertex_buffer_type=vba.Get_Type();
	render_state.vba_offset=vba.VertexBufferOffset;
	render_state.vba_count=vba.Get_Vertex_Count();
	REF_PTR_SET(render_state.vertex_buffer,vba.VertexBuffer);
	render_state.vertex_buffer->Add_Engine_Ref();
	render_state_changed|=VERTEX_BUFFER_CHANGED;
	render_state_changed|=INDEX_BUFFER_CHANGED;		// vba_offset changes so index buffer needs to be reset as well.
}

// ----------------------------------------------------------------------------
//
// Set index buffer using dynamic access object.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Index_Buffer(const DynamicIBAccessClass& iba_,unsigned short index_base_offset)
{
	if (render_state.index_buffer) render_state.index_buffer->Release_Engine_Ref();

	DynamicIBAccessClass& iba=const_cast<DynamicIBAccessClass&>(iba_);
	render_state.index_base_offset=index_base_offset;
	render_state.index_buffer_type=iba.Get_Type();
	render_state.iba_offset=iba.IndexBufferOffset;
	REF_PTR_SET(render_state.index_buffer,iba.IndexBuffer);
	render_state.index_buffer->Add_Engine_Ref();
	render_state_changed|=INDEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
//
// Private function for the special case of rendering polygons from sorting
// index and vertex buffers.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw_Sorting_IB_VB(
	unsigned primitive_type,
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	WWASSERT(render_state.vertex_buffer_type==BUFFER_TYPE_SORTING || render_state.vertex_buffer_type==BUFFER_TYPE_DYNAMIC_SORTING);
	WWASSERT(render_state.index_buffer_type==BUFFER_TYPE_SORTING || render_state.index_buffer_type==BUFFER_TYPE_DYNAMIC_SORTING);

	// Fill dynamic vertex buffer with sorting vertex buffer vertices
	DynamicVBAccessClass dyn_vb_access(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,vertex_count);
	{
		DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
		VertexFormatXYZNDUV2* src = static_cast<SortingVertexBufferClass*>(render_state.vertex_buffer)->VertexBuffer;
		VertexFormatXYZNDUV2* dest= lock.Get_Formatted_Vertex_Array();
		src += render_state.vba_offset + render_state.index_base_offset + min_vertex_index;
		unsigned  size = dyn_vb_access.FVF_Info().Get_FVF_Size()*vertex_count/sizeof(unsigned);
		unsigned *dest_u =(unsigned*) dest;
		unsigned *src_u = (unsigned*) src;

		for (unsigned i=0;i<size;++i) {
			*dest_u++=*src_u++;
		}
	}

	DX8CALL(SetStreamSource(
		0,
		static_cast<DX8VertexBufferClass*>(dyn_vb_access.VertexBuffer)->Get_DX8_Vertex_Buffer(),
		dyn_vb_access.FVF_Info().Get_FVF_Size()));
	DX8CALL(SetVertexShader(dyn_vb_access.FVF_Info().Get_FVF()));
	DX8_RECORD_VERTEX_BUFFER_CHANGE();

	unsigned index_count=0;
	switch (primitive_type) {
	case D3DPT_TRIANGLELIST: index_count=polygon_count*3; break;
	case D3DPT_TRIANGLESTRIP: index_count=polygon_count+2; break;
	case D3DPT_TRIANGLEFAN: index_count=polygon_count+2; break;
	default: WWASSERT(0); break; // Unsupported primitive type
	}

	// Fill dynamic index buffer with sorting index buffer vertices
	DynamicIBAccessClass dyn_ib_access(BUFFER_TYPE_DYNAMIC_DX8,index_count);
	{
		DynamicIBAccessClass::WriteLockClass lock(&dyn_ib_access);
		unsigned short* dest=lock.Get_Index_Array();
		unsigned short* src=NULL;
		src=static_cast<SortingIndexBufferClass*>(render_state.index_buffer)->index_buffer;
		src+=render_state.iba_offset+start_index;

		for (unsigned short i=0;i<index_count;++i) {
			unsigned short index=*src++;
			index-=min_vertex_index;
			WWASSERT(index<vertex_count);
			*dest++=index;
		}
	}

	DX8CALL(SetIndices(
		static_cast<DX8IndexBufferClass*>(dyn_ib_access.IndexBuffer)->Get_DX8_Index_Buffer(),
		dyn_vb_access.VertexBufferOffset));
	DX8_RECORD_INDEX_BUFFER_CHANGE();

	DX8_RECORD_DRAW_CALLS();
	DX8CALL(DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,		// start vertex
		vertex_count,
		dyn_ib_access.IndexBufferOffset,
		polygon_count));

	DX8_RECORD_RENDER(polygon_count,vertex_count,render_state.shader);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw(
	unsigned primitive_type,
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	if (DrawPolygonLowBoundLimit && DrawPolygonLowBoundLimit>=polygon_count) return;

	DX8_THREAD_ASSERT();
	SNAPSHOT_SAY(("DX8 - draw"));

	Apply_Render_State_Changes();

	// Debug feature to disable triangle drawing...
	if (!_Is_Triangle_Draw_Enabled()) return;

#ifdef MESH_RENDER_SNAPSHOT_ENABLED
	if (WW3D::Is_Snapshot_Activated()) {
		unsigned long passes=0;
		SNAPSHOT_SAY(("ValidateDevice:"));
		HRESULT res=D3DDevice->ValidateDevice(&passes);
		switch (res) {
		case D3D_OK:
			SNAPSHOT_SAY(("OK"));
			break;

		case D3DERR_CONFLICTINGTEXTUREFILTER:
			SNAPSHOT_SAY(("D3DERR_CONFLICTINGTEXTUREFILTER"));
			break;
		case D3DERR_CONFLICTINGTEXTUREPALETTE:
			SNAPSHOT_SAY(("D3DERR_CONFLICTINGTEXTUREPALETTE"));
			break;
		case D3DERR_DEVICELOST:
			SNAPSHOT_SAY(("D3DERR_DEVICELOST"));
			break;
		case D3DERR_TOOMANYOPERATIONS:
			SNAPSHOT_SAY(("D3DERR_TOOMANYOPERATIONS"));
			break;
		case D3DERR_UNSUPPORTEDALPHAARG:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDALPHAARG"));
			break;
		case D3DERR_UNSUPPORTEDALPHAOPERATION:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDALPHAOPERATION"));
			break;
		case D3DERR_UNSUPPORTEDCOLORARG:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDCOLORARG"));
			break;
		case D3DERR_UNSUPPORTEDCOLOROPERATION:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDCOLOROPERATION"));
			break;
		case D3DERR_UNSUPPORTEDFACTORVALUE:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDFACTORVALUE"));
			break;
		case D3DERR_UNSUPPORTEDTEXTUREFILTER:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDTEXTUREFILTER"));
			break;
		case D3DERR_WRONGTEXTUREFORMAT:
			SNAPSHOT_SAY(("D3DERR_WRONGTEXTUREFORMAT"));
			break;
		default:
			SNAPSHOT_SAY(("UNKNOWN Error"));
			break;
		}
	}
#endif	// MESH_RENDER_SHAPSHOT_ENABLED


	SNAPSHOT_SAY(("DX8 - draw %d polygons (%d vertices)",polygon_count,vertex_count));

	if (vertex_count<3) {
		min_vertex_index=0;
		switch (render_state.vertex_buffer_type) {
		case BUFFER_TYPE_DX8:
		case BUFFER_TYPE_SORTING:
			vertex_count=render_state.vertex_buffer->Get_Vertex_Count()-render_state.index_base_offset-render_state.vba_offset-min_vertex_index;
			break;
		case BUFFER_TYPE_DYNAMIC_DX8:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			vertex_count=render_state.vba_count;
			break;
		}
	}

	switch (render_state.vertex_buffer_type) {
	case BUFFER_TYPE_DX8:
	case BUFFER_TYPE_DYNAMIC_DX8:
		switch (render_state.index_buffer_type) {
		case BUFFER_TYPE_DX8:
		case BUFFER_TYPE_DYNAMIC_DX8:
			{
/*				if ((start_index+render_state.iba_offset+polygon_count*3) > render_state.index_buffer->Get_Index_Count())
				{	WWASSERT_PRINT(0,"OVERFLOWING INDEX BUFFER");
					///@todo: MUST FIND OUT WHY THIS HAPPENS WITH LOTS OF PARTICLES ON BIG FIGHT!  -MW
					break;
				}*/
				DX8_RECORD_RENDER(polygon_count,vertex_count,render_state.shader);
				DX8_RECORD_DRAW_CALLS();
				DX8CALL(DrawIndexedPrimitive(
					(D3DPRIMITIVETYPE)primitive_type,
					min_vertex_index,
					vertex_count,
					start_index+render_state.iba_offset,
					polygon_count));
			}
			break;
		case BUFFER_TYPE_SORTING:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			WWASSERT_PRINT(0,"VB and IB must of same type (sorting or dx8)");
			break;
		case BUFFER_TYPE_INVALID:
			WWASSERT(0);
			break;
		}
		break;
	case BUFFER_TYPE_SORTING:
	case BUFFER_TYPE_DYNAMIC_SORTING:
		switch (render_state.index_buffer_type) {
		case BUFFER_TYPE_DX8:
		case BUFFER_TYPE_DYNAMIC_DX8:
			WWASSERT_PRINT(0,"VB and IB must of same type (sorting or dx8)");
			break;
		case BUFFER_TYPE_SORTING:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			Draw_Sorting_IB_VB(primitive_type,start_index,polygon_count,min_vertex_index,vertex_count);
			break;
		case BUFFER_TYPE_INVALID:
			WWASSERT(0);
			break;
		}
		break;
	case BUFFER_TYPE_INVALID:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw_Triangles(
	unsigned buffer_type,
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	if (buffer_type==BUFFER_TYPE_SORTING || buffer_type==BUFFER_TYPE_DYNAMIC_SORTING) {
		SortingRendererClass::Insert_Triangles(start_index,polygon_count,min_vertex_index,vertex_count);
	}
	else {
		Draw(D3DPT_TRIANGLELIST,start_index,polygon_count,min_vertex_index,vertex_count);
	}
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw_Triangles(
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	Draw(D3DPT_TRIANGLELIST,start_index,polygon_count,min_vertex_index,vertex_count);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw_Strip(
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	Draw(D3DPT_TRIANGLESTRIP,start_index,polygon_count,min_vertex_index,vertex_count);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Apply_Render_State_Changes()
{
	SNAPSHOT_SAY(("DX8Wrapper::Apply_Render_State_Changes()"));

	if (!render_state_changed) return;
	if (render_state_changed&SHADER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply shader"));
		render_state.shader.Apply();
	}

	unsigned mask=TEXTURE0_CHANGED;
	int i=0;
	for (;i<CurrentCaps->Get_Max_Textures_Per_Pass();++i,mask<<=1)
	{
		if (render_state_changed&mask)
		{
			SNAPSHOT_SAY(("DX8 - apply texture %d",i));

			if (render_state.Textures[i])
			{
				render_state.Textures[i]->Apply(i);
			}
			else
			{
				TextureBaseClass::Apply_Null(i);
			}
		}
	}

	if (render_state_changed&MATERIAL_CHANGED)
	{
		SNAPSHOT_SAY(("DX8 - apply material"));
		VertexMaterialClass* material=const_cast<VertexMaterialClass*>(render_state.material);
		if (material)
		{
			material->Apply();
		}
		else VertexMaterialClass::Apply_Null();
	}

	if (render_state_changed&LIGHTS_CHANGED)
	{
		unsigned mask=LIGHT0_CHANGED;
		for (unsigned index=0;index<4;++index,mask<<=1) {
			if (render_state_changed&mask) {
				SNAPSHOT_SAY(("DX8 - apply light %d",index));
				if (render_state.LightEnable[index]) {
#ifdef MESH_RENDER_SNAPSHOT_ENABLED
					if ( WW3D::Is_Snapshot_Activated() ) {
						D3DLIGHT8 * light = &(render_state.Lights[index]);
						static const char * _light_types[] = { "Unknown", "Point","Spot", "Directional" };
						WWASSERT((light->Type >= 0) && (light->Type <= 3));

						SNAPSHOT_SAY((" type = %s amb = %4.2f,%4.2f,%4.2f  diff = %4.2f,%4.2f,%4.2f spec = %4.2f, %4.2f, %4.2f",
							_light_types[light->Type],
							light->Ambient.r,light->Ambient.g,light->Ambient.b,
							light->Diffuse.r,light->Diffuse.g,light->Diffuse.b,
							light->Specular.r,light->Specular.g,light->Specular.b ));
						SNAPSHOT_SAY((" pos = %f, %f, %f  dir = %f, %f, %f",
							light->Position.x, light->Position.y, light->Position.z,
							light->Direction.x, light->Direction.y, light->Direction.z ));
					}
#endif

					Set_DX8_Light(index,&render_state.Lights[index]);
				}
				else {
					Set_DX8_Light(index,NULL);
					SNAPSHOT_SAY((" clearing light to NULL"));
				}
			}
		}
	}

	if (render_state_changed&WORLD_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply world matrix"));
		_Set_DX8_Transform(D3DTS_WORLD,render_state.world);
	}
	if (render_state_changed&VIEW_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply view matrix"));
		_Set_DX8_Transform(D3DTS_VIEW,render_state.view);
	}
	if (render_state_changed&VERTEX_BUFFER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply vb change"));
		if (render_state.vertex_buffer) {
			switch (render_state.vertex_buffer_type) {//->Type()) {
			case BUFFER_TYPE_DX8:
			case BUFFER_TYPE_DYNAMIC_DX8:
				DX8CALL(SetStreamSource(
					0,
					static_cast<DX8VertexBufferClass*>(render_state.vertex_buffer)->Get_DX8_Vertex_Buffer(),
					render_state.vertex_buffer->FVF_Info().Get_FVF_Size()));
				DX8_RECORD_VERTEX_BUFFER_CHANGE();
				DX8CALL(SetVertexShader(render_state.vertex_buffer->FVF_Info().Get_FVF()));
				break;
			case BUFFER_TYPE_SORTING:
			case BUFFER_TYPE_DYNAMIC_SORTING:
				break;
			default:
				WWASSERT(0);
			}
		} else {
			DX8CALL(SetStreamSource(0,NULL,0));
			DX8_RECORD_VERTEX_BUFFER_CHANGE();
		}
	}
	if (render_state_changed&INDEX_BUFFER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply ib change"));
		if (render_state.index_buffer) {
			switch (render_state.index_buffer_type) {//->Type()) {
			case BUFFER_TYPE_DX8:
			case BUFFER_TYPE_DYNAMIC_DX8:
				DX8CALL(SetIndices(
					static_cast<DX8IndexBufferClass*>(render_state.index_buffer)->Get_DX8_Index_Buffer(),
					render_state.index_base_offset+render_state.vba_offset));
				DX8_RECORD_INDEX_BUFFER_CHANGE();
				break;
			case BUFFER_TYPE_SORTING:
			case BUFFER_TYPE_DYNAMIC_SORTING:
				break;
			default:
				WWASSERT(0);
			}
		}
		else {
			DX8CALL(SetIndices(
				NULL,
				0));
			DX8_RECORD_INDEX_BUFFER_CHANGE();
		}
	}

	render_state_changed&=((unsigned)WORLD_IDENTITY|(unsigned)VIEW_IDENTITY);

	SNAPSHOT_SAY(("DX8Wrapper::Apply_Render_State_Changes() - finished"));
}

IDirect3DTexture8 * DX8Wrapper::_Create_DX8_Texture
(
	unsigned int width,
	unsigned int height,
	WW3DFormat format,
	MipCountType mip_level_count,
	D3DPOOL pool,
	bool rendertarget
)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture8 *texture = NULL;

	// Paletted textures not supported!
	WWASSERT(format!=D3DFMT_P8);

	// NOTE: If 'format' is not supported as a texture format, this function will find the closest
	// format that is supported and use that instead.

	// Render target may return NOTAVAILABLE, in
	// which case we return NULL.
	if (rendertarget) {
		unsigned ret=D3DXCreateTexture(
			DX8Wrapper::_Get_D3D_Device8(),
			width,
			height,
			mip_level_count,
			D3DUSAGE_RENDERTARGET,
			WW3DFormat_To_D3DFormat(format),
			pool,
			&texture);

		if (ret==D3DERR_NOTAVAILABLE) {
			Non_Fatal_Log_DX8_ErrorCode(ret,__FILE__,__LINE__);
			return NULL;
		}

		if (ret==D3DERR_OUTOFVIDEOMEMORY) {
			Non_Fatal_Log_DX8_ErrorCode(ret,__FILE__,__LINE__);
			return NULL;
		}

		DX8_ErrorCode(ret);
		// Just return the texture, no reduction
		// allowed for render targets.
		return texture;
	}

	// Don't allow any errors in non-render target
	// texture creation.
	DX8_ErrorCode(D3DXCreateTexture(
		DX8Wrapper::_Get_D3D_Device8(),
		width,
		height,
		mip_level_count,
		0,
		WW3DFormat_To_D3DFormat(format),
		pool,
		&texture));

//	unsigned reduction=WW3D::Get_Texture_Reduction();
//	unsigned level_count=texture->GetLevelCount();
//	if (reduction>=level_count) reduction=level_count-1;
//	texture->SetLOD(reduction);

	return texture;
}

IDirect3DTexture8 * DX8Wrapper::_Create_DX8_Texture
(
	const char *filename,
	MipCountType mip_level_count
)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture8 *texture = NULL;

	// NOTE: If the original image format is not supported as a texture format, it will
	// automatically be converted to an appropriate format.
	// NOTE: It is possible to get the size and format of the original image file from this
	// function as well, so if we later want to second-guess D3DX's format conversion decisions
	// we can do so after this function is called..
	unsigned result = D3DXCreateTextureFromFileExA(
		_Get_D3D_Device8(),
		filename,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		mip_level_count,//create_mipmaps ? 0 : 1,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_FILTER_BOX,
		D3DX_FILTER_BOX,
		0,
		NULL,
		NULL,
		&texture);

	if (result != D3D_OK) {
		return MissingTexture::_Get_Missing_Texture();
	}

	// Make sure texture wasn't paletted!
	D3DSURFACE_DESC desc;
	texture->GetLevelDesc(0,&desc);
	if (desc.Format==D3DFMT_P8) {
		texture->Release();
		return MissingTexture::_Get_Missing_Texture();
	}
	return texture;
}

IDirect3DTexture8 * DX8Wrapper::_Create_DX8_Texture
(
	IDirect3DSurface8 *surface,
	MipCountType mip_level_count
)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture8 *texture = NULL;

	D3DSURFACE_DESC surface_desc;
	::ZeroMemory(&surface_desc, sizeof(D3DSURFACE_DESC));
	surface->GetDesc(&surface_desc);

	// This function will create a texture with a different (but similar) format if the surface is
	// not in a supported texture format.
	WW3DFormat format=D3DFormat_To_WW3DFormat(surface_desc.Format);
	texture = _Create_DX8_Texture(surface_desc.Width, surface_desc.Height, format, mip_level_count);

	// Copy the surface to the texture
	IDirect3DSurface8 *tex_surface = NULL;
	texture->GetSurfaceLevel(0, &tex_surface);
	DX8_ErrorCode(D3DXLoadSurfaceFromSurface(tex_surface, NULL, NULL, surface, NULL, NULL, D3DX_FILTER_BOX, 0));
	tex_surface->Release();

	// Create mipmaps if needed
	if (mip_level_count!=MIP_LEVELS_1)
	{
		DX8_ErrorCode(D3DXFilterTexture(texture, NULL, 0, D3DX_FILTER_BOX));
	}

	return texture;

}

IDirect3DSurface8 * DX8Wrapper::_Create_DX8_Surface(unsigned int width, unsigned int height, WW3DFormat format)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();

	IDirect3DSurface8 *surface = NULL;

	// Paletted surfaces not supported!
	WWASSERT(format!=D3DFMT_P8);

	DX8CALL(CreateImageSurface(width, height, WW3DFormat_To_D3DFormat(format), &surface));

	return surface;
}

IDirect3DSurface8 * DX8Wrapper::_Create_DX8_Surface(const char *filename_)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();

	// Note: Since there is no "D3DXCreateSurfaceFromFile" and no "GetSurfaceInfoFromFile" (the
	// latter is supposed to be added to D3DX in a future version), we create a texture from the
	// file (w/o mipmaps), check that its surface is equal to the original file data (which it
	// will not be if the file is not in a texture-supported format or size). If so, copy its
	// surface (we might be able to just get its surface and add a ref to it but I'm not sure so
	// I'm not going to risk it) and release the texture. If not, create a surface according to
	// the file data and use D3DXLoadSurfaceFromFile. This is a horrible hack, but it saves us
	// having to write file loaders. Will fix this when D3DX provides us with the right functions.
	// Create a surface the size of the file image data
	IDirect3DSurface8 *surface = NULL;

	{

		file_auto_ptr myfile(_TheFileFactory,filename_);
		// If file not found, create a surface with missing texture in it

		if (!myfile->Is_Available()) {
			// If file not found, try the dds format
			// else create a surface with missing texture in it
			char compressed_name[200];
			strlcpy(compressed_name,filename_, sizeof(compressed_name));
			char *ext = strstr(compressed_name, ".");
			if ( ext && (strlen(ext)==4) &&
				  ( (ext[1] == 't') || (ext[1] == 'T') ) &&
				  ( (ext[2] == 'g') || (ext[2] == 'G') ) &&
				  ( (ext[3] == 'a') || (ext[3] == 'A') ) ) {
				ext[1]='d';
				ext[2]='d';
				ext[3]='s';
			}
			file_auto_ptr myfile2(_TheFileFactory,compressed_name);
			if (!myfile2->Is_Available())
				return MissingTexture::_Create_Missing_Surface();
		}
	}

	StringClass filename_string(filename_,true);
	surface=TextureLoader::Load_Surface_Immediate(
		filename_string,
		WW3D_FORMAT_UNKNOWN,
		true);
	return surface;
}


/***********************************************************************************************
 * DX8Wrapper::_Update_Texture -- Copies a texture from system memory to video memory          *
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
 *   4/26/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void DX8Wrapper::_Update_Texture(TextureClass *system, TextureClass *video)
{
	WWASSERT(system);
	WWASSERT(video);
	WWASSERT(system->Get_Pool()==TextureClass::POOL_SYSTEMMEM);
	WWASSERT(video->Get_Pool()==TextureClass::POOL_DEFAULT);
	DX8CALL(UpdateTexture(system->Peek_D3D_Base_Texture(),video->Peek_D3D_Base_Texture()));
}

void DX8Wrapper::Compute_Caps(WW3DFormat display_format)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	delete CurrentCaps;
	CurrentCaps=new DX8Caps(_Get_D3D8(),D3DDevice,display_format,Get_Current_Adapter_Identifier());
}


void DX8Wrapper::Set_Light(unsigned index, const D3DLIGHT8* light)
{
	if (light) {
		render_state.Lights[index]=*light;
		render_state.LightEnable[index]=true;
	}
	else {
		render_state.LightEnable[index]=false;
	}
	render_state_changed|=(LIGHT0_CHANGED<<index);
}

void DX8Wrapper::Set_Light(unsigned index,const LightClass &light)
{
	D3DLIGHT8 dlight;
	Vector3 temp;
	memset(&dlight,0,sizeof(D3DLIGHT8));

	switch (light.Get_Type())
	{
	case LightClass::POINT:
		{
			dlight.Type=D3DLIGHT_POINT;
		}
		break;
	case LightClass::DIRECTIONAL:
		{
			dlight.Type=D3DLIGHT_DIRECTIONAL;
		}
		break;
	case LightClass::SPOT:
		{
			dlight.Type=D3DLIGHT_SPOT;
		}
		break;
	}

	light.Get_Diffuse(&temp);
	temp*=light.Get_Intensity();
	dlight.Diffuse.r=temp.X;
	dlight.Diffuse.g=temp.Y;
	dlight.Diffuse.b=temp.Z;
	dlight.Diffuse.a=1.0f;

	light.Get_Specular(&temp);
	temp*=light.Get_Intensity();
	dlight.Specular.r=temp.X;
	dlight.Specular.g=temp.Y;
	dlight.Specular.b=temp.Z;
	dlight.Specular.a=1.0f;

	light.Get_Ambient(&temp);
	temp*=light.Get_Intensity();
	dlight.Ambient.r=temp.X;
	dlight.Ambient.g=temp.Y;
	dlight.Ambient.b=temp.Z;
	dlight.Ambient.a=1.0f;

	temp=light.Get_Position();
	dlight.Position=*(D3DVECTOR*) &temp;

	light.Get_Spot_Direction(temp);
	dlight.Direction=*(D3DVECTOR*) &temp;

	dlight.Range=light.Get_Attenuation_Range();
	dlight.Falloff=light.Get_Spot_Exponent();
	dlight.Theta=light.Get_Spot_Angle();
	dlight.Phi=light.Get_Spot_Angle();

	// Inverse linear light 1/(1+D)
	double a,b;
	light.Get_Far_Attenuation_Range(a,b);
	dlight.Attenuation0=1.0f;
	if (fabs(a-b)<1e-5)
		// if the attenuation range is too small assume uniform with cutoff
		dlight.Attenuation1=0.0f;
	else
		// this will cause the light to drop to half intensity at the first far attenuation
		dlight.Attenuation1=(float) 1.0/a;
	dlight.Attenuation2=0.0f;

	Set_Light(index,&dlight);
}

// ----------------------------------------------------------------------------
//
// Set the light environment. This is a lighting model which used up to four
// directional lights to produce the lighting.
//
void DX8Wrapper::Set_Light_Environment(LightEnvironmentClass* light_env)
{
	if (light_env)
	{
		int light_count = light_env->Get_Light_Count();
		unsigned int color=Convert_Color(light_env->Get_Equivalent_Ambient(),0.0f);
		if (RenderStates[D3DRS_AMBIENT]!=color)
		{
			Set_DX8_Render_State(D3DRS_AMBIENT,color);
//buggy Radeon 9700 driver doesn't apply new ambient unless the material also changes.
#if 1
			render_state_changed|=MATERIAL_CHANGED;
#endif
		}

		D3DLIGHT8 light;
		int l=0;
		for (;l<light_count;++l) {

			::ZeroMemory(&light, sizeof(D3DLIGHT8));

			light.Type=D3DLIGHT_DIRECTIONAL;
			(Vector3&)light.Diffuse=light_env->Get_Light_Diffuse(l);
			Vector3 dir=-light_env->Get_Light_Direction(l);
			light.Direction=(const D3DVECTOR&)(dir);
			if (light_env->isPointLight(l)) {
				light.Type = D3DLIGHT_POINT;
				(Vector3&)light.Diffuse=light_env->getPointDiffuse(l);
				(Vector3&)light.Ambient=light_env->getPointAmbient(l);
				light.Position = (const D3DVECTOR&)light_env->getPointCenter(l);
				light.Range = light_env->getPointOrad(l);

				// Inverse linear light 1/(1+D)
				double a,b;
				b = light_env->getPointOrad(l);
				a = light_env->getPointIrad(l);
				light.Attenuation0=0.01f;
				if (fabs(a-b)<1e-5)
					// if the attenuation range is too small assume uniform with cutoff
					light.Attenuation1=0.0f;
				else
					// this will cause the light to drop to half intensity at the first far attenuation
					light.Attenuation1=(float) 0.1/a;

				light.Attenuation2=8.0f/(b*b);
			}

			Set_Light(l,&light);
		}

		for (;l<4;++l) {
			Set_Light(l,NULL);
		}
	}
/*	else {
		for (int l=0;l<4;++l) {
			Set_Light(l,NULL);
		}
	}
*/
}

IDirect3DSurface8 * DX8Wrapper::_Get_DX8_Front_Buffer()
{
	DX8_THREAD_ASSERT();
	D3DDISPLAYMODE mode;

	DX8CALL(GetDisplayMode(&mode));

	IDirect3DSurface8 * fb=NULL;

	DX8CALL(CreateImageSurface(mode.Width,mode.Height,D3DFMT_A8R8G8B8,&fb));

	DX8CALL(GetFrontBuffer(fb));
	return fb;
}

SurfaceClass * DX8Wrapper::_Get_DX8_Back_Buffer(unsigned int num)
{
	DX8_THREAD_ASSERT();

	IDirect3DSurface8 * bb;
	SurfaceClass *surf=NULL;
	DX8CALL(GetBackBuffer(num,D3DBACKBUFFER_TYPE_MONO,&bb));
	if (bb)
	{
		surf=NEW_REF(SurfaceClass,(bb));
		bb->Release();
	}

	return surf;
}


TextureClass *
DX8Wrapper::Create_Render_Target (int width, int height, bool alpha)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();

	//
	//	Note: We're going to force the width and height to be powers of two and equal
	//
	const D3DCAPS8& dx8caps=Get_Current_Caps()->Get_DX8_Caps();
	float poweroftwosize = width;
	if (height > 0 && height < width) {
		poweroftwosize = height;
	}
	poweroftwosize = ::Find_POT (poweroftwosize);

	if (poweroftwosize>dx8caps.MaxTextureWidth) {
		poweroftwosize=dx8caps.MaxTextureWidth;
	}
	if (poweroftwosize>dx8caps.MaxTextureHeight) {
		poweroftwosize=dx8caps.MaxTextureHeight;
	}

	width = height = poweroftwosize;

	//
	//	Get the current format of the display
	//
	D3DDISPLAYMODE mode;
	DX8CALL(GetDisplayMode(&mode));

	// If the user requested a render-target texture and this device does not support that
	// feature, return NULL
	HRESULT hr;

	if (alpha)
	{
			//user wants a texture with destination alpha channel - only 1 such format
			//ever exists on current hardware	- D3DFMT_A8R8G8B8
			hr = D3DInterface->CheckDeviceFormat(	D3DADAPTER_DEFAULT,
																	WW3D_DEVTYPE,
																	mode.Format,
																	D3DUSAGE_RENDERTARGET,
																	D3DRTYPE_TEXTURE,
																	D3DFMT_A8R8G8B8 );
			mode.Format=D3DFMT_A8R8G8B8;
	}
	else
	{
			hr = D3DInterface->CheckDeviceFormat(	D3DADAPTER_DEFAULT,
																	WW3D_DEVTYPE,
																	mode.Format,
																	D3DUSAGE_RENDERTARGET,
																	D3DRTYPE_TEXTURE,
																	mode.Format );
	}

	number_of_DX8_calls++;
	if (hr != D3D_OK) {
		WWDEBUG_SAY(("DX8Wrapper - Driver cannot create render target!"));
		return NULL;
	}

	//
	//	Attempt to create the render target
	//
	DX8_Assert();
	WW3DFormat format=D3DFormat_To_WW3DFormat(mode.Format);
	TextureClass * tex = NEW_REF(TextureClass,(width,height,format,MIP_LEVELS_1,TextureClass::POOL_DEFAULT,true));

	// 3dfx drivers are lying in the CheckDeviceFormat call and claiming
	// that they support render targets!
	if (tex->Peek_D3D_Base_Texture() == NULL)
	{
		WWDEBUG_SAY(("DX8Wrapper - Render target creation failed!"));
		REF_PTR_RELEASE(tex);
	}

	return tex;
}


void
DX8Wrapper::Set_Render_Target
(TextureClass * texture)
{
	WWASSERT(texture != NULL);
	SurfaceClass * surf = texture->Get_Surface_Level();
	WWASSERT(surf != NULL);
	Set_Render_Target(surf->Peek_D3D_Surface());
	REF_PTR_RELEASE(surf);
}

void
DX8Wrapper::Set_Render_Target(IDirect3DSwapChain8 *swap_chain)
{
	DX8_THREAD_ASSERT();
	WWASSERT (swap_chain != NULL);

	//
	//	Get the back buffer for the swap chain
	//
	LPDIRECT3DSURFACE8 render_target = NULL;
	swap_chain->GetBackBuffer (0, D3DBACKBUFFER_TYPE_MONO, &render_target);

	//
	//	Set this back buffer as the render targer
	//
	Set_Render_Target (render_target);

	//
	//	Release our hold on the back buffer
	//
	if (render_target != NULL) {
		render_target->Release ();
		render_target = NULL;
	}

	IsRenderToTexture = false;

	return ;
}

void
DX8Wrapper::Set_Render_Target(IDirect3DSurface8 *render_target)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();

	//
	//	We'll need the depth buffer later...
	//
	IDirect3DSurface8 *depth_buffer = NULL;
	DX8CALL(GetDepthStencilSurface (&depth_buffer));

	//
	//	Should we restore the default render target set a new one?
	//
	if (render_target == NULL || render_target == DefaultRenderTarget)
	{

		//
		//	Restore the default render target
		//
		if (DefaultRenderTarget != NULL)
		{
			DX8CALL(SetRenderTarget (DefaultRenderTarget, depth_buffer));
			DefaultRenderTarget->Release ();
			DefaultRenderTarget = NULL;
		}

		//
		//	Release our hold on the "current" render target
		//
		if (CurrentRenderTarget != NULL)
		{
			CurrentRenderTarget->Release ();
			CurrentRenderTarget = NULL;
		}

	}
	else if (render_target != CurrentRenderTarget)
	{

		//
		//	Get a pointer to the default render target (if necessary)
		//
		if (DefaultRenderTarget == NULL)
		{
			DX8CALL(GetRenderTarget (&DefaultRenderTarget));
		}

		//
		//	Release our hold on the old "current" render target
		//
		if (CurrentRenderTarget != NULL)
		{
			CurrentRenderTarget->Release ();
			CurrentRenderTarget = NULL;
		}

		//
		//	Keep a copy of the current render target (for housekeeping)
		//
		CurrentRenderTarget = render_target;
		WWASSERT (CurrentRenderTarget != NULL);
		if (CurrentRenderTarget != NULL)
		{
			CurrentRenderTarget->AddRef ();

			//
			//	Switch render targets
			//
			DX8CALL(SetRenderTarget (CurrentRenderTarget, depth_buffer));
		}
	}

	//
	//	Free our hold on the depth buffer
	//
	if (depth_buffer != NULL) {
		depth_buffer->Release ();
		depth_buffer = NULL;
	}

	IsRenderToTexture = false;
	return ;
}


IDirect3DSwapChain8 *
DX8Wrapper::Create_Additional_Swap_Chain (HWND render_window)
{
	DX8_Assert();

	//
	//	Configure the presentation parameters for a windowed render target
	//
	D3DPRESENT_PARAMETERS params				= { 0 };
	params.BackBufferFormat						= _PresentParameters.BackBufferFormat;
	params.BackBufferCount						= 1;
	params.MultiSampleType						= D3DMULTISAMPLE_NONE;
	params.SwapEffect								= D3DSWAPEFFECT_COPY_VSYNC;
	params.hDeviceWindow							= render_window;
	params.Windowed								= TRUE;
	params.EnableAutoDepthStencil				= TRUE;
	params.AutoDepthStencilFormat				= _PresentParameters.AutoDepthStencilFormat;
	params.Flags									= 0;
	params.FullScreen_RefreshRateInHz		= D3DPRESENT_RATE_DEFAULT;
	params.FullScreen_PresentationInterval	= D3DPRESENT_INTERVAL_DEFAULT;

	//
	//	Create the swap chain
	//
	IDirect3DSwapChain8 *swap_chain = NULL;
	DX8CALL(CreateAdditionalSwapChain(&params, &swap_chain));
	return swap_chain;
}

void DX8Wrapper::Flush_DX8_Resource_Manager(unsigned int bytes)
{
	DX8_Assert();
	DX8CALL(ResourceManagerDiscardBytes(bytes));
}

unsigned int DX8Wrapper::Get_Free_Texture_RAM()
{
	DX8_Assert();
	number_of_DX8_calls++;
	return DX8Wrapper::_Get_D3D_Device8()->GetAvailableTextureMem();
}

// Converts a linear gamma ramp to one that is controlled by:
// Gamma - controls the curvature of the middle of the curve
// Bright - controls the minimum value of the curve
// Contrast - controls the difference between the maximum and the minimum of the curve
void DX8Wrapper::Set_Gamma(float gamma,float bright,float contrast,bool calibrate,bool uselimit)
{
	gamma=Bound(gamma,0.6f,6.0f);
	bright=Bound(bright,-0.5f,0.5f);
	contrast=Bound(contrast,0.5f,2.0f);
	float oo_gamma=1.0f/gamma;

	DX8_Assert();
	number_of_DX8_calls++;

	DWORD flag=(calibrate?D3DSGR_CALIBRATE:D3DSGR_NO_CALIBRATION);

	D3DGAMMARAMP ramp;
	float			 limit;

	// IML: I'm not really sure what the intent of the 'limit' variable is. It does not produce useful results for my purposes.
	if (uselimit) {
		limit=(contrast-1)/2*contrast;
	} else {
		limit = 0.0f;
	}

	// HY - arrived at this equation after much trial and error.
	for (int i=0; i<256; i++) {
		float in,out;
		in=i/256.0f;
		float x=in-limit;
		x=Bound(x,0.0f,1.0f);
		x=powf(x,oo_gamma);
		out=contrast*x+bright;
		out=Bound(out,0.0f,1.0f);
		ramp.red[i]=(WORD) (out*65535);
		ramp.green[i]=(WORD) (out*65535);
		ramp.blue[i]=(WORD) (out*65535);
	}

	if (Get_Current_Caps()->Support_Gamma())	{
		DX8Wrapper::_Get_D3D_Device8()->SetGammaRamp(flag,&ramp);
	} else {
		HWND hwnd = GetDesktopWindow();
		HDC hdc = GetDC(hwnd);
		if (hdc)
		{
			SetDeviceGammaRamp (hdc, &ramp);
			ReleaseDC (hwnd, hdc);
		}
	}
}

//**********************************************************************************************
//! Resets render device to default state
/*!
*/
void DX8Wrapper::Apply_Default_State()
{
	SNAPSHOT_SAY(("DX8Wrapper::Apply_Default_State()"));

	// only set states used in game
	Set_DX8_Render_State(D3DRS_ZENABLE, TRUE);
//	Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_SOLID);
	Set_DX8_Render_State(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	//Set_DX8_Render_State(D3DRS_LINEPATTERN, 0);
	Set_DX8_Render_State(D3DRS_ZWRITEENABLE, TRUE);
	Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, FALSE);
	//Set_DX8_Render_State(D3DRS_LASTPIXEL, FALSE);
	Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_ONE);
	Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_CW);
	Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	Set_DX8_Render_State(D3DRS_ALPHAREF, 0);
	Set_DX8_Render_State(D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL);
	Set_DX8_Render_State(D3DRS_DITHERENABLE, FALSE);
	Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);
	Set_DX8_Render_State(D3DRS_FOGENABLE, FALSE);
	Set_DX8_Render_State(D3DRS_SPECULARENABLE, FALSE);
//	Set_DX8_Render_State(D3DRS_ZVISIBLE, FALSE);
//	Set_DX8_Render_State(D3DRS_FOGCOLOR, 0);
//	Set_DX8_Render_State(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
//	Set_DX8_Render_State(D3DRS_FOGSTART, 0);

//	Set_DX8_Render_State(D3DRS_FOGEND, WWMath::Float_As_Int(1.0f));
//	Set_DX8_Render_State(D3DRS_FOGDENSITY, WWMath::Float_As_Int(1.0f));

	//Set_DX8_Render_State(D3DRS_EDGEANTIALIAS, FALSE);
	Set_DX8_Render_State(D3DRS_ZBIAS, 0);
//	Set_DX8_Render_State(D3DRS_RANGEFOGENABLE, FALSE);
	Set_DX8_Render_State(D3DRS_STENCILENABLE, FALSE);
	Set_DX8_Render_State(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	Set_DX8_Render_State(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	Set_DX8_Render_State(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	Set_DX8_Render_State(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	Set_DX8_Render_State(D3DRS_STENCILREF, 0);
	Set_DX8_Render_State(D3DRS_STENCILMASK, 0xffffffff);
	Set_DX8_Render_State(D3DRS_STENCILWRITEMASK, 0xffffffff);
	Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, 0);
/*	Set_DX8_Render_State(D3DRS_WRAP0, D3DWRAP_U| D3DWRAP_V);
	Set_DX8_Render_State(D3DRS_WRAP1, D3DWRAP_U| D3DWRAP_V);
	Set_DX8_Render_State(D3DRS_WRAP2, D3DWRAP_U| D3DWRAP_V);
	Set_DX8_Render_State(D3DRS_WRAP3, D3DWRAP_U| D3DWRAP_V);
	Set_DX8_Render_State(D3DRS_WRAP4, D3DWRAP_U| D3DWRAP_V);
	Set_DX8_Render_State(D3DRS_WRAP5, D3DWRAP_U| D3DWRAP_V);
	Set_DX8_Render_State(D3DRS_WRAP6, D3DWRAP_U| D3DWRAP_V);
	Set_DX8_Render_State(D3DRS_WRAP7, D3DWRAP_U| D3DWRAP_V);*/
	Set_DX8_Render_State(D3DRS_CLIPPING, TRUE);
	Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);
	//Set_DX8_Render_State(D3DRS_AMBIENT, 0);
//	Set_DX8_Render_State(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
	Set_DX8_Render_State(D3DRS_COLORVERTEX, TRUE);
/*	Set_DX8_Render_State(D3DRS_LOCALVIEWER, TRUE);
	Set_DX8_Render_State(D3DRS_NORMALIZENORMALS, FALSE);
	Set_DX8_Render_State(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
	Set_DX8_Render_State(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	Set_DX8_Render_State(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	Set_DX8_Render_State(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);*/
	//Set_DX8_Render_State(D3DRS_CLIPPLANEENABLE, 0);
	Set_DX8_Render_State(D3DRS_SOFTWAREVERTEXPROCESSING, FALSE);
	//Set_DX8_Render_State(D3DRS_POINTSIZE, 0x3f800000);
	//Set_DX8_Render_State(D3DRS_POINTSIZE_MIN, 0);
	//Set_DX8_Render_State(D3DRS_POINTSPRITEENABLE, FALSE);
	//Set_DX8_Render_State(D3DRS_POINTSCALEENABLE, FALSE);
	//Set_DX8_Render_State(D3DRS_POINTSCALE_A, 0);
	//Set_DX8_Render_State(D3DRS_POINTSCALE_B, 0);
	//Set_DX8_Render_State(D3DRS_POINTSCALE_C, 0);
	//Set_DX8_Render_State(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	//Set_DX8_Render_State(D3DRS_MULTISAMPLEMASK, 0xffffffff);
	//Set_DX8_Render_State(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
	//Set_DX8_Render_State(D3DRS_PATCHSEGMENTS, 0x3f800000);
	//Set_DX8_Render_State(D3DRS_DEBUGMONITORTOKEN, D3DDMT_ENABLE);
	//Set_DX8_Render_State(D3DRS_POINTSIZE_MAX, Float_At_Int(64.0f));
	//Set_DX8_Render_State(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 0x0000000f);
	//Set_DX8_Render_State(D3DRS_TWEENFACTOR, 0);
	Set_DX8_Render_State(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	//Set_DX8_Render_State(D3DRS_POSITIONORDER, D3DORDER_CUBIC);
	//Set_DX8_Render_State(D3DRS_NORMALORDER, D3DORDER_LINEAR);

	// disable TSS stages
	int i;
	for (i=0; i<CurrentCaps->Get_Max_Textures_Per_Pass(); i++)
	{
		Set_DX8_Texture_Stage_State(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
		Set_DX8_Texture_Stage_State(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		Set_DX8_Texture_Stage_State(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		Set_DX8_Texture_Stage_State(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		Set_DX8_Texture_Stage_State(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		Set_DX8_Texture_Stage_State(i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		/*Set_DX8_Texture_Stage_State(i, D3DTSS_BUMPENVMAT00, 0);
		Set_DX8_Texture_Stage_State(i, D3DTSS_BUMPENVMAT01, 0);
		Set_DX8_Texture_Stage_State(i, D3DTSS_BUMPENVMAT10, 0);
		Set_DX8_Texture_Stage_State(i, D3DTSS_BUMPENVMAT11, 0);
		Set_DX8_Texture_Stage_State(i, D3DTSS_BUMPENVLSCALE, 0);
		Set_DX8_Texture_Stage_State(i, D3DTSS_BUMPENVLOFFSET, 0);*/

		Set_DX8_Texture_Stage_State(i, D3DTSS_TEXCOORDINDEX, i);


		Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		Set_DX8_Texture_Stage_State(i, D3DTSS_BORDERCOLOR, 0);
//		Set_DX8_Texture_Stage_State(i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
//		Set_DX8_Texture_Stage_State(i, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
//		Set_DX8_Texture_Stage_State(i, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
//		Set_DX8_Texture_Stage_State(i, D3DTSS_MIPMAPLODBIAS, 0);
//		Set_DX8_Texture_Stage_State(i, D3DTSS_MAXMIPLEVEL, 0);
//		Set_DX8_Texture_Stage_State(i, D3DTSS_MAXANISOTROPY, 1);
		//Set_DX8_Texture_Stage_State(i, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP);
		//Set_DX8_Texture_Stage_State(i, D3DTSS_COLORARG0, D3DTA_CURRENT);
		//Set_DX8_Texture_Stage_State(i, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
		//Set_DX8_Texture_Stage_State(i, D3DTSS_RESULTARG, D3DTA_CURRENT);

		Set_DX8_Texture_Stage_State(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		Set_Texture(i,NULL);
	}

//	DX8Wrapper::Set_Material(NULL);
	VertexMaterialClass::Apply_Null();

	for (unsigned index=0;index<4;++index) {
		SNAPSHOT_SAY(("Clearing light %d to NULL",index));
		Set_DX8_Light(index,NULL);
	}

	// set up simple default TSS
	Vector4 vconst[MAX_VERTEX_SHADER_CONSTANTS];
	memset(vconst,0,sizeof(Vector4)*MAX_VERTEX_SHADER_CONSTANTS);
	Set_Vertex_Shader_Constant(0, vconst, MAX_VERTEX_SHADER_CONSTANTS);

	Vector4 pconst[MAX_PIXEL_SHADER_CONSTANTS];
	memset(pconst,0,sizeof(Vector4)*MAX_PIXEL_SHADER_CONSTANTS);
	Set_Pixel_Shader_Constant(0, pconst, MAX_PIXEL_SHADER_CONSTANTS);

	Set_Vertex_Shader(DX8_FVF_XYZNDUV2);
	Set_Pixel_Shader(0);

	ShaderClass::Invalidate();
}

const char* DX8Wrapper::Get_DX8_Render_State_Name(D3DRENDERSTATETYPE state)
{
	switch (state) {
	case D3DRS_ZENABLE                       : return "D3DRS_ZENABLE";
	case D3DRS_FILLMODE                      : return "D3DRS_FILLMODE";
	case D3DRS_SHADEMODE                     : return "D3DRS_SHADEMODE";
	case D3DRS_LINEPATTERN                   : return "D3DRS_LINEPATTERN";
	case D3DRS_ZWRITEENABLE                  : return "D3DRS_ZWRITEENABLE";
	case D3DRS_ALPHATESTENABLE               : return "D3DRS_ALPHATESTENABLE";
	case D3DRS_LASTPIXEL                     : return "D3DRS_LASTPIXEL";
	case D3DRS_SRCBLEND                      : return "D3DRS_SRCBLEND";
	case D3DRS_DESTBLEND                     : return "D3DRS_DESTBLEND";
	case D3DRS_CULLMODE                      : return "D3DRS_CULLMODE";
	case D3DRS_ZFUNC                         : return "D3DRS_ZFUNC";
	case D3DRS_ALPHAREF                      : return "D3DRS_ALPHAREF";
	case D3DRS_ALPHAFUNC                     : return "D3DRS_ALPHAFUNC";
	case D3DRS_DITHERENABLE                  : return "D3DRS_DITHERENABLE";
	case D3DRS_ALPHABLENDENABLE              : return "D3DRS_ALPHABLENDENABLE";
	case D3DRS_FOGENABLE                     : return "D3DRS_FOGENABLE";
	case D3DRS_SPECULARENABLE                : return "D3DRS_SPECULARENABLE";
	case D3DRS_ZVISIBLE                      : return "D3DRS_ZVISIBLE";
	case D3DRS_FOGCOLOR                      : return "D3DRS_FOGCOLOR";
	case D3DRS_FOGTABLEMODE                  : return "D3DRS_FOGTABLEMODE";
	case D3DRS_FOGSTART                      : return "D3DRS_FOGSTART";
	case D3DRS_FOGEND                        : return "D3DRS_FOGEND";
	case D3DRS_FOGDENSITY                    : return "D3DRS_FOGDENSITY";
	case D3DRS_EDGEANTIALIAS                 : return "D3DRS_EDGEANTIALIAS";
	case D3DRS_ZBIAS                         : return "D3DRS_ZBIAS";
	case D3DRS_RANGEFOGENABLE                : return "D3DRS_RANGEFOGENABLE";
	case D3DRS_STENCILENABLE                 : return "D3DRS_STENCILENABLE";
	case D3DRS_STENCILFAIL                   : return "D3DRS_STENCILFAIL";
	case D3DRS_STENCILZFAIL                  : return "D3DRS_STENCILZFAIL";
	case D3DRS_STENCILPASS                   : return "D3DRS_STENCILPASS";
	case D3DRS_STENCILFUNC                   : return "D3DRS_STENCILFUNC";
	case D3DRS_STENCILREF                    : return "D3DRS_STENCILREF";
	case D3DRS_STENCILMASK                   : return "D3DRS_STENCILMASK";
	case D3DRS_STENCILWRITEMASK              : return "D3DRS_STENCILWRITEMASK";
	case D3DRS_TEXTUREFACTOR                 : return "D3DRS_TEXTUREFACTOR";
	case D3DRS_WRAP0                         : return "D3DRS_WRAP0";
	case D3DRS_WRAP1                         : return "D3DRS_WRAP1";
	case D3DRS_WRAP2                         : return "D3DRS_WRAP2";
	case D3DRS_WRAP3                         : return "D3DRS_WRAP3";
	case D3DRS_WRAP4                         : return "D3DRS_WRAP4";
	case D3DRS_WRAP5                         : return "D3DRS_WRAP5";
	case D3DRS_WRAP6                         : return "D3DRS_WRAP6";
	case D3DRS_WRAP7                         : return "D3DRS_WRAP7";
	case D3DRS_CLIPPING                      : return "D3DRS_CLIPPING";
	case D3DRS_LIGHTING                      : return "D3DRS_LIGHTING";
	case D3DRS_AMBIENT                       : return "D3DRS_AMBIENT";
	case D3DRS_FOGVERTEXMODE                 : return "D3DRS_FOGVERTEXMODE";
	case D3DRS_COLORVERTEX                   : return "D3DRS_COLORVERTEX";
	case D3DRS_LOCALVIEWER                   : return "D3DRS_LOCALVIEWER";
	case D3DRS_NORMALIZENORMALS              : return "D3DRS_NORMALIZENORMALS";
	case D3DRS_DIFFUSEMATERIALSOURCE         : return "D3DRS_DIFFUSEMATERIALSOURCE";
	case D3DRS_SPECULARMATERIALSOURCE        : return "D3DRS_SPECULARMATERIALSOURCE";
	case D3DRS_AMBIENTMATERIALSOURCE         : return "D3DRS_AMBIENTMATERIALSOURCE";
	case D3DRS_EMISSIVEMATERIALSOURCE        : return "D3DRS_EMISSIVEMATERIALSOURCE";
	case D3DRS_VERTEXBLEND                   : return "D3DRS_VERTEXBLEND";
	case D3DRS_CLIPPLANEENABLE               : return "D3DRS_CLIPPLANEENABLE";
	case D3DRS_SOFTWAREVERTEXPROCESSING      : return "D3DRS_SOFTWAREVERTEXPROCESSING";
	case D3DRS_POINTSIZE                     : return "D3DRS_POINTSIZE";
	case D3DRS_POINTSIZE_MIN                 : return "D3DRS_POINTSIZE_MIN";
	case D3DRS_POINTSPRITEENABLE             : return "D3DRS_POINTSPRITEENABLE";
	case D3DRS_POINTSCALEENABLE              : return "D3DRS_POINTSCALEENABLE";
	case D3DRS_POINTSCALE_A                  : return "D3DRS_POINTSCALE_A";
	case D3DRS_POINTSCALE_B                  : return "D3DRS_POINTSCALE_B";
	case D3DRS_POINTSCALE_C                  : return "D3DRS_POINTSCALE_C";
	case D3DRS_MULTISAMPLEANTIALIAS          : return "D3DRS_MULTISAMPLEANTIALIAS";
	case D3DRS_MULTISAMPLEMASK               : return "D3DRS_MULTISAMPLEMASK";
	case D3DRS_PATCHEDGESTYLE                : return "D3DRS_PATCHEDGESTYLE";
	case D3DRS_PATCHSEGMENTS                 : return "D3DRS_PATCHSEGMENTS";
	case D3DRS_DEBUGMONITORTOKEN             : return "D3DRS_DEBUGMONITORTOKEN";
	case D3DRS_POINTSIZE_MAX                 : return "D3DRS_POINTSIZE_MAX";
	case D3DRS_INDEXEDVERTEXBLENDENABLE      : return "D3DRS_INDEXEDVERTEXBLENDENABLE";
	case D3DRS_COLORWRITEENABLE              : return "D3DRS_COLORWRITEENABLE";
	case D3DRS_TWEENFACTOR                   : return "D3DRS_TWEENFACTOR";
	case D3DRS_BLENDOP                       : return "D3DRS_BLENDOP";
//	case D3DRS_POSITIONORDER                 : return "D3DRS_POSITIONORDER";
//	case D3DRS_NORMALORDER                   : return "D3DRS_NORMALORDER";
	default											  : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Stage_State_Name(D3DTEXTURESTAGESTATETYPE state)
{
	switch (state) {
	case D3DTSS_COLOROP                   : return "D3DTSS_COLOROP";
	case D3DTSS_COLORARG1                 : return "D3DTSS_COLORARG1";
	case D3DTSS_COLORARG2                 : return "D3DTSS_COLORARG2";
	case D3DTSS_ALPHAOP                   : return "D3DTSS_ALPHAOP";
	case D3DTSS_ALPHAARG1                 : return "D3DTSS_ALPHAARG1";
	case D3DTSS_ALPHAARG2                 : return "D3DTSS_ALPHAARG2";
	case D3DTSS_BUMPENVMAT00              : return "D3DTSS_BUMPENVMAT00";
	case D3DTSS_BUMPENVMAT01              : return "D3DTSS_BUMPENVMAT01";
	case D3DTSS_BUMPENVMAT10              : return "D3DTSS_BUMPENVMAT10";
	case D3DTSS_BUMPENVMAT11              : return "D3DTSS_BUMPENVMAT11";
	case D3DTSS_TEXCOORDINDEX             : return "D3DTSS_TEXCOORDINDEX";
	case D3DTSS_ADDRESSU                  : return "D3DTSS_ADDRESSU";
	case D3DTSS_ADDRESSV                  : return "D3DTSS_ADDRESSV";
	case D3DTSS_BORDERCOLOR               : return "D3DTSS_BORDERCOLOR";
	case D3DTSS_MAGFILTER                 : return "D3DTSS_MAGFILTER";
	case D3DTSS_MINFILTER                 : return "D3DTSS_MINFILTER";
	case D3DTSS_MIPFILTER                 : return "D3DTSS_MIPFILTER";
	case D3DTSS_MIPMAPLODBIAS             : return "D3DTSS_MIPMAPLODBIAS";
	case D3DTSS_MAXMIPLEVEL               : return "D3DTSS_MAXMIPLEVEL";
	case D3DTSS_MAXANISOTROPY             : return "D3DTSS_MAXANISOTROPY";
	case D3DTSS_BUMPENVLSCALE             : return "D3DTSS_BUMPENVLSCALE";
	case D3DTSS_BUMPENVLOFFSET            : return "D3DTSS_BUMPENVLOFFSET";
	case D3DTSS_TEXTURETRANSFORMFLAGS     : return "D3DTSS_TEXTURETRANSFORMFLAGS";
	case D3DTSS_ADDRESSW                  : return "D3DTSS_ADDRESSW";
	case D3DTSS_COLORARG0                 : return "D3DTSS_COLORARG0";
	case D3DTSS_ALPHAARG0                 : return "D3DTSS_ALPHAARG0";
	case D3DTSS_RESULTARG                 : return "D3DTSS_RESULTARG";
	default										  : return "UNKNOWN";
	}
}

void DX8Wrapper::Get_DX8_Render_State_Value_Name(StringClass& name, D3DRENDERSTATETYPE state, unsigned value)
{
	switch (state) {
	case D3DRS_ZENABLE:
		name=Get_DX8_ZBuffer_Type_Name(value);
		break;

	case D3DRS_FILLMODE:
		name=Get_DX8_Fill_Mode_Name(value);
		break;

	case D3DRS_SHADEMODE:
		name=Get_DX8_Shade_Mode_Name(value);
		break;

	case D3DRS_LINEPATTERN:
	case D3DRS_FOGCOLOR:
	case D3DRS_ALPHAREF:
	case D3DRS_STENCILMASK:
	case D3DRS_STENCILWRITEMASK:
	case D3DRS_TEXTUREFACTOR:
	case D3DRS_AMBIENT:
	case D3DRS_CLIPPLANEENABLE:
	case D3DRS_MULTISAMPLEMASK:
		name.Format("0x%x",value);
		break;

	case D3DRS_ZWRITEENABLE:
	case D3DRS_ALPHATESTENABLE:
	case D3DRS_LASTPIXEL:
	case D3DRS_DITHERENABLE:
	case D3DRS_ALPHABLENDENABLE:
	case D3DRS_FOGENABLE:
	case D3DRS_SPECULARENABLE:
	case D3DRS_STENCILENABLE:
	case D3DRS_RANGEFOGENABLE:
	case D3DRS_EDGEANTIALIAS:
	case D3DRS_CLIPPING:
	case D3DRS_LIGHTING:
	case D3DRS_COLORVERTEX:
	case D3DRS_LOCALVIEWER:
	case D3DRS_NORMALIZENORMALS:
	case D3DRS_SOFTWAREVERTEXPROCESSING:
	case D3DRS_POINTSPRITEENABLE:
	case D3DRS_POINTSCALEENABLE:
	case D3DRS_MULTISAMPLEANTIALIAS:
	case D3DRS_INDEXEDVERTEXBLENDENABLE:
		name=value ? "TRUE" : "FALSE";
		break;

	case D3DRS_SRCBLEND:
	case D3DRS_DESTBLEND:
		name=Get_DX8_Blend_Name(value);
		break;

	case D3DRS_CULLMODE:
		name=Get_DX8_Cull_Mode_Name(value);
		break;

	case D3DRS_ZFUNC:
	case D3DRS_ALPHAFUNC:
	case D3DRS_STENCILFUNC:
		name=Get_DX8_Cmp_Func_Name(value);
		break;

	case D3DRS_ZVISIBLE:
		name="NOTSUPPORTED";
		break;

	case D3DRS_FOGTABLEMODE:
	case D3DRS_FOGVERTEXMODE:
		name=Get_DX8_Fog_Mode_Name(value);
		break;

	case D3DRS_FOGSTART:
	case D3DRS_FOGEND:
	case D3DRS_FOGDENSITY:
	case D3DRS_POINTSIZE:
	case D3DRS_POINTSIZE_MIN:
	case D3DRS_POINTSCALE_A:
	case D3DRS_POINTSCALE_B:
	case D3DRS_POINTSCALE_C:
	case D3DRS_PATCHSEGMENTS:
	case D3DRS_POINTSIZE_MAX:
	case D3DRS_TWEENFACTOR:
		name.Format("%f",*(float*)&value);
		break;

	case D3DRS_ZBIAS:
	case D3DRS_STENCILREF:
		name.Format("%d",value);
		break;

	case D3DRS_STENCILFAIL:
	case D3DRS_STENCILZFAIL:
	case D3DRS_STENCILPASS:
		name=Get_DX8_Stencil_Op_Name(value);
		break;

	case D3DRS_WRAP0:
	case D3DRS_WRAP1:
	case D3DRS_WRAP2:
	case D3DRS_WRAP3:
	case D3DRS_WRAP4:
	case D3DRS_WRAP5:
	case D3DRS_WRAP6:
	case D3DRS_WRAP7:
		name="0";
		if (value&D3DWRAP_U) name+="|D3DWRAP_U";
		if (value&D3DWRAP_V) name+="|D3DWRAP_V";
		if (value&D3DWRAP_W) name+="|D3DWRAP_W";
		break;

	case D3DRS_DIFFUSEMATERIALSOURCE:
	case D3DRS_SPECULARMATERIALSOURCE:
	case D3DRS_AMBIENTMATERIALSOURCE:
	case D3DRS_EMISSIVEMATERIALSOURCE:
		name=Get_DX8_Material_Source_Name(value);
		break;

	case D3DRS_VERTEXBLEND:
		name=Get_DX8_Vertex_Blend_Flag_Name(value);
		break;

	case D3DRS_PATCHEDGESTYLE:
		name=Get_DX8_Patch_Edge_Style_Name(value);
		break;

	case D3DRS_DEBUGMONITORTOKEN:
		name=Get_DX8_Debug_Monitor_Token_Name(value);
		break;

	case D3DRS_COLORWRITEENABLE:
		name="0";
		if (value&D3DCOLORWRITEENABLE_RED) name+="|D3DCOLORWRITEENABLE_RED";
		if (value&D3DCOLORWRITEENABLE_GREEN) name+="|D3DCOLORWRITEENABLE_GREEN";
		if (value&D3DCOLORWRITEENABLE_BLUE) name+="|D3DCOLORWRITEENABLE_BLUE";
		if (value&D3DCOLORWRITEENABLE_ALPHA) name+="|D3DCOLORWRITEENABLE_ALPHA";
		break;
	case D3DRS_BLENDOP:
		name=Get_DX8_Blend_Op_Name(value);
		break;
	default:
		name.Format("UNKNOWN (%d)",value);
		break;
	}
}

void DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name(StringClass& name, D3DTEXTURESTAGESTATETYPE state, unsigned value)
{
	switch (state) {
	case D3DTSS_COLOROP:
	case D3DTSS_ALPHAOP:
		name=Get_DX8_Texture_Op_Name(value);
		break;

	case D3DTSS_COLORARG0:
	case D3DTSS_COLORARG1:
	case D3DTSS_COLORARG2:
	case D3DTSS_ALPHAARG0:
	case D3DTSS_ALPHAARG1:
	case D3DTSS_ALPHAARG2:
	case D3DTSS_RESULTARG:
		name=Get_DX8_Texture_Arg_Name(value);
		break;

	case D3DTSS_ADDRESSU:
	case D3DTSS_ADDRESSV:
	case D3DTSS_ADDRESSW:
		name=Get_DX8_Texture_Address_Name(value);
		break;

	case D3DTSS_MAGFILTER:
	case D3DTSS_MINFILTER:
	case D3DTSS_MIPFILTER:
		name=Get_DX8_Texture_Filter_Name(value);
		break;

	case D3DTSS_TEXTURETRANSFORMFLAGS:
		name=Get_DX8_Texture_Transform_Flag_Name(value);
		break;

	// Floating point values
	case D3DTSS_MIPMAPLODBIAS:
	case D3DTSS_BUMPENVMAT00:
	case D3DTSS_BUMPENVMAT01:
	case D3DTSS_BUMPENVMAT10:
	case D3DTSS_BUMPENVMAT11:
	case D3DTSS_BUMPENVLSCALE:
	case D3DTSS_BUMPENVLOFFSET:
		name.Format("%f",*(float*)&value);
		break;

	case D3DTSS_TEXCOORDINDEX:
		if ((value&0xffff0000)==D3DTSS_TCI_CAMERASPACENORMAL) {
			name.Format("D3DTSS_TCI_CAMERASPACENORMAL|%d",value&0xffff);
		}
		else if ((value&0xffff0000)==D3DTSS_TCI_CAMERASPACEPOSITION) {
			name.Format("D3DTSS_TCI_CAMERASPACEPOSITION|%d",value&0xffff);
		}
		else if ((value&0xffff0000)==D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR) {
			name.Format("D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR|%d",value&0xffff);
		}
		else {
			name.Format("%d",value);
		}
		break;

	// Integer value
	case D3DTSS_MAXMIPLEVEL:
	case D3DTSS_MAXANISOTROPY:
		name.Format("%d",value);
		break;
	// Hex values
	case D3DTSS_BORDERCOLOR:
		name.Format("0x%x",value);
		break;

	default:
		name.Format("UNKNOWN (%d)",value);
		break;
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Op_Name(unsigned value)
{
	switch (value) {
	case D3DTOP_DISABLE                      : return "D3DTOP_DISABLE";
	case D3DTOP_SELECTARG1                   : return "D3DTOP_SELECTARG1";
	case D3DTOP_SELECTARG2                   : return "D3DTOP_SELECTARG2";
	case D3DTOP_MODULATE                     : return "D3DTOP_MODULATE";
	case D3DTOP_MODULATE2X                   : return "D3DTOP_MODULATE2X";
	case D3DTOP_MODULATE4X                   : return "D3DTOP_MODULATE4X";
	case D3DTOP_ADD                          : return "D3DTOP_ADD";
	case D3DTOP_ADDSIGNED                    : return "D3DTOP_ADDSIGNED";
	case D3DTOP_ADDSIGNED2X                  : return "D3DTOP_ADDSIGNED2X";
	case D3DTOP_SUBTRACT                     : return "D3DTOP_SUBTRACT";
	case D3DTOP_ADDSMOOTH                    : return "D3DTOP_ADDSMOOTH";
	case D3DTOP_BLENDDIFFUSEALPHA            : return "D3DTOP_BLENDDIFFUSEALPHA";
	case D3DTOP_BLENDTEXTUREALPHA            : return "D3DTOP_BLENDTEXTUREALPHA";
	case D3DTOP_BLENDFACTORALPHA             : return "D3DTOP_BLENDFACTORALPHA";
	case D3DTOP_BLENDTEXTUREALPHAPM          : return "D3DTOP_BLENDTEXTUREALPHAPM";
	case D3DTOP_BLENDCURRENTALPHA            : return "D3DTOP_BLENDCURRENTALPHA";
	case D3DTOP_PREMODULATE                  : return "D3DTOP_PREMODULATE";
	case D3DTOP_MODULATEALPHA_ADDCOLOR       : return "D3DTOP_MODULATEALPHA_ADDCOLOR";
	case D3DTOP_MODULATECOLOR_ADDALPHA       : return "D3DTOP_MODULATECOLOR_ADDALPHA";
	case D3DTOP_MODULATEINVALPHA_ADDCOLOR    : return "D3DTOP_MODULATEINVALPHA_ADDCOLOR";
	case D3DTOP_MODULATEINVCOLOR_ADDALPHA    : return "D3DTOP_MODULATEINVCOLOR_ADDALPHA";
	case D3DTOP_BUMPENVMAP                   : return "D3DTOP_BUMPENVMAP";
	case D3DTOP_BUMPENVMAPLUMINANCE          : return "D3DTOP_BUMPENVMAPLUMINANCE";
	case D3DTOP_DOTPRODUCT3                  : return "D3DTOP_DOTPRODUCT3";
	case D3DTOP_MULTIPLYADD                  : return "D3DTOP_MULTIPLYADD";
	case D3DTOP_LERP                         : return "D3DTOP_LERP";
	default										     : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Arg_Name(unsigned value)
{
	switch (value) {
	case D3DTA_CURRENT			: return "D3DTA_CURRENT";
	case D3DTA_DIFFUSE			: return "D3DTA_DIFFUSE";
	case D3DTA_SELECTMASK		: return "D3DTA_SELECTMASK";
	case D3DTA_SPECULAR			: return "D3DTA_SPECULAR";
	case D3DTA_TEMP				: return "D3DTA_TEMP";
	case D3DTA_TEXTURE			: return "D3DTA_TEXTURE";
	case D3DTA_TFACTOR			: return "D3DTA_TFACTOR";
	case D3DTA_ALPHAREPLICATE	: return "D3DTA_ALPHAREPLICATE";
	case D3DTA_COMPLEMENT		: return "D3DTA_COMPLEMENT";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Filter_Name(unsigned value)
{
	switch (value) {
	case D3DTEXF_NONE				: return "D3DTEXF_NONE";
	case D3DTEXF_POINT			: return "D3DTEXF_POINT";
	case D3DTEXF_LINEAR			: return "D3DTEXF_LINEAR";
	case D3DTEXF_ANISOTROPIC	: return "D3DTEXF_ANISOTROPIC";
	case D3DTEXF_FLATCUBIC		: return "D3DTEXF_FLATCUBIC";
	case D3DTEXF_GAUSSIANCUBIC	: return "D3DTEXF_GAUSSIANCUBIC";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Address_Name(unsigned value)
{
	switch (value) {
	case D3DTADDRESS_WRAP		: return "D3DTADDRESS_WRAP";
	case D3DTADDRESS_MIRROR		: return "D3DTADDRESS_MIRROR";
	case D3DTADDRESS_CLAMP		: return "D3DTADDRESS_CLAMP";
	case D3DTADDRESS_BORDER		: return "D3DTADDRESS_BORDER";
	case D3DTADDRESS_MIRRORONCE: return "D3DTADDRESS_MIRRORONCE";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Transform_Flag_Name(unsigned value)
{
	switch (value) {
	case D3DTTFF_DISABLE			: return "D3DTTFF_DISABLE";
	case D3DTTFF_COUNT1			: return "D3DTTFF_COUNT1";
	case D3DTTFF_COUNT2			: return "D3DTTFF_COUNT2";
	case D3DTTFF_COUNT3			: return "D3DTTFF_COUNT3";
	case D3DTTFF_COUNT4			: return "D3DTTFF_COUNT4";
	case D3DTTFF_PROJECTED		: return "D3DTTFF_PROJECTED";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_ZBuffer_Type_Name(unsigned value)
{
	switch (value) {
	case D3DZB_FALSE				: return "D3DZB_FALSE";
	case D3DZB_TRUE				: return "D3DZB_TRUE";
	case D3DZB_USEW				: return "D3DZB_USEW";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Fill_Mode_Name(unsigned value)
{
	switch (value) {
	case D3DFILL_POINT			: return "D3DFILL_POINT";
	case D3DFILL_WIREFRAME		: return "D3DFILL_WIREFRAME";
	case D3DFILL_SOLID			: return "D3DFILL_SOLID";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Shade_Mode_Name(unsigned value)
{
	switch (value) {
	case D3DSHADE_FLAT			: return "D3DSHADE_FLAT";
	case D3DSHADE_GOURAUD		: return "D3DSHADE_GOURAUD";
	case D3DSHADE_PHONG			: return "D3DSHADE_PHONG";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Blend_Name(unsigned value)
{
	switch (value) {
	case D3DBLEND_ZERO                : return "D3DBLEND_ZERO";
	case D3DBLEND_ONE                 : return "D3DBLEND_ONE";
	case D3DBLEND_SRCCOLOR            : return "D3DBLEND_SRCCOLOR";
	case D3DBLEND_INVSRCCOLOR         : return "D3DBLEND_INVSRCCOLOR";
	case D3DBLEND_SRCALPHA            : return "D3DBLEND_SRCALPHA";
	case D3DBLEND_INVSRCALPHA         : return "D3DBLEND_INVSRCALPHA";
	case D3DBLEND_DESTALPHA           : return "D3DBLEND_DESTALPHA";
	case D3DBLEND_INVDESTALPHA        : return "D3DBLEND_INVDESTALPHA";
	case D3DBLEND_DESTCOLOR           : return "D3DBLEND_DESTCOLOR";
	case D3DBLEND_INVDESTCOLOR        : return "D3DBLEND_INVDESTCOLOR";
	case D3DBLEND_SRCALPHASAT         : return "D3DBLEND_SRCALPHASAT";
	case D3DBLEND_BOTHSRCALPHA        : return "D3DBLEND_BOTHSRCALPHA";
	case D3DBLEND_BOTHINVSRCALPHA     : return "D3DBLEND_BOTHINVSRCALPHA";
	default									 : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Cull_Mode_Name(unsigned value)
{
	switch (value) {
	case D3DCULL_NONE				: return "D3DCULL_NONE";
	case D3DCULL_CW				: return "D3DCULL_CW";
	case D3DCULL_CCW				: return "D3DCULL_CCW";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Cmp_Func_Name(unsigned value)
{
	switch (value) {
	case D3DCMP_NEVER          : return "D3DCMP_NEVER";
	case D3DCMP_LESS           : return "D3DCMP_LESS";
	case D3DCMP_EQUAL          : return "D3DCMP_EQUAL";
	case D3DCMP_LESSEQUAL      : return "D3DCMP_LESSEQUAL";
	case D3DCMP_GREATER        : return "D3DCMP_GREATER";
	case D3DCMP_NOTEQUAL       : return "D3DCMP_NOTEQUAL";
	case D3DCMP_GREATEREQUAL   : return "D3DCMP_GREATEREQUAL";
	case D3DCMP_ALWAYS         : return "D3DCMP_ALWAYS";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Fog_Mode_Name(unsigned value)
{
	switch (value) {
	case D3DFOG_NONE				: return "D3DFOG_NONE";
	case D3DFOG_EXP				: return "D3DFOG_EXP";
	case D3DFOG_EXP2				: return "D3DFOG_EXP2";
	case D3DFOG_LINEAR			: return "D3DFOG_LINEAR";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Stencil_Op_Name(unsigned value)
{
	switch (value) {
	case D3DSTENCILOP_KEEP		: return "D3DSTENCILOP_KEEP";
	case D3DSTENCILOP_ZERO		: return "D3DSTENCILOP_ZERO";
	case D3DSTENCILOP_REPLACE	: return "D3DSTENCILOP_REPLACE";
	case D3DSTENCILOP_INCRSAT	: return "D3DSTENCILOP_INCRSAT";
	case D3DSTENCILOP_DECRSAT	: return "D3DSTENCILOP_DECRSAT";
	case D3DSTENCILOP_INVERT	: return "D3DSTENCILOP_INVERT";
	case D3DSTENCILOP_INCR		: return "D3DSTENCILOP_INCR";
	case D3DSTENCILOP_DECR		: return "D3DSTENCILOP_DECR";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Material_Source_Name(unsigned value)
{
	switch (value) {
	case D3DMCS_MATERIAL			: return "D3DMCS_MATERIAL";
	case D3DMCS_COLOR1			: return "D3DMCS_COLOR1";
	case D3DMCS_COLOR2			: return "D3DMCS_COLOR2";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Vertex_Blend_Flag_Name(unsigned value)
{
	switch (value) {
	case D3DVBF_DISABLE			: return "D3DVBF_DISABLE";
	case D3DVBF_1WEIGHTS			: return "D3DVBF_1WEIGHTS";
	case D3DVBF_2WEIGHTS			: return "D3DVBF_2WEIGHTS";
	case D3DVBF_3WEIGHTS			: return "D3DVBF_3WEIGHTS";
	case D3DVBF_TWEENING			: return "D3DVBF_TWEENING";
	case D3DVBF_0WEIGHTS			: return "D3DVBF_0WEIGHTS";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Patch_Edge_Style_Name(unsigned value)
{
	switch (value) {
	case D3DPATCHEDGE_DISCRETE	: return "D3DPATCHEDGE_DISCRETE";
   case D3DPATCHEDGE_CONTINUOUS:return "D3DPATCHEDGE_CONTINUOUS";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Debug_Monitor_Token_Name(unsigned value)
{
	switch (value) {
	case D3DDMT_ENABLE			: return "D3DDMT_ENABLE";
	case D3DDMT_DISABLE			: return "D3DDMT_DISABLE";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Blend_Op_Name(unsigned value)
{
	switch (value) {
	case D3DBLENDOP_ADD			: return "D3DBLENDOP_ADD";
	case D3DBLENDOP_SUBTRACT	: return "D3DBLENDOP_SUBTRACT";
	case D3DBLENDOP_REVSUBTRACT: return "D3DBLENDOP_REVSUBTRACT";
	case D3DBLENDOP_MIN			: return "D3DBLENDOP_MIN";
	case D3DBLENDOP_MAX			: return "D3DBLENDOP_MAX";
	default							: return "UNKNOWN";
	}
}


//============================================================================
// DX8Wrapper::getBackBufferFormat
//============================================================================

WW3DFormat	DX8Wrapper::getBackBufferFormat( void )
{
	return D3DFormat_To_WW3DFormat( _PresentParameters.BackBufferFormat );
}
