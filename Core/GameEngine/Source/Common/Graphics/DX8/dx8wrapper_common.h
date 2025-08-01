bool DX8Wrapper::Init(void* hwnd, bool lite)
{
	WWASSERT(!IsInitted);

	// zero memory
	memset(Textures, 0, sizeof(IDirect3DBaseTexture8*) * MAX_TEXTURE_STAGES);
	memset(RenderStates, 0, sizeof(unsigned) * 256);
	memset(TextureStageStates, 0, sizeof(unsigned) * 32 * MAX_TEXTURE_STAGES);
	memset(Vertex_Shader_Constants, 0, sizeof(Vector4) * MAX_VERTEX_SHADER_CONSTANTS);
	memset(Pixel_Shader_Constants, 0, sizeof(Vector4) * MAX_PIXEL_SHADER_CONSTANTS);
	memset(&render_state, 0, sizeof(RenderStateStruct));
	memset(Shadow_Map, 0, sizeof(ZTextureClass*) * MAX_SHADOW_MAPS);

	/*
	** Initialize all variables!
	*/
	_Hwnd = (HWND)hwnd;
	_MainThreadID = ThreadClass::_Get_Current_Thread_ID();
	WWDEBUG_SAY(("DX8Wrapper main thread: 0x%x", _MainThreadID));
	CurRenderDevice = -1;
	ResolutionWidth = DEFAULT_RESOLUTION_WIDTH;
	ResolutionHeight = DEFAULT_RESOLUTION_HEIGHT;
	// Initialize Render2DClass Screen Resolution
	Render2DClass::Set_Screen_Resolution(RectClass(0, 0, ResolutionWidth, ResolutionHeight));
	BitDepth = DEFAULT_BIT_DEPTH;
	IsWindowed = false;
	DX8Wrapper_IsWindowed = false;

	for (int light = 0; light < 4; ++light) CurrentDX8LightEnables[light] = false;

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

		Direct3DCreate8Ptr = (Direct3DCreate8Type)GetProcAddress(D3D8Lib, "Direct3DCreate8");
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

		Set_Render_Target((IDirect3DSurface8*)NULL);
		Release_Device();
	}

	if (D3DInterface) {
		D3DInterface->Release();
		D3DInterface = NULL;

	}

	if (CurrentCaps)
	{
		int max = CurrentCaps->Get_Max_Textures_Per_Pass();
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
		UINT newRefCount = D3DInterface->Release();
		D3DInterface = NULL;
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
	TextureFilterClass::_Init_Filters((TextureFilterClass::TextureFilterMode)WW3D::Get_Texture_Filter());
	TheDX8MeshRenderer.Init();
	SHD_INIT;
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
	const D3DCAPS8& caps = Get_Current_Caps()->Get_DX8_Caps();

	Set_DX8_Render_State(D3DRS_RANGEFOGENABLE, (caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE) ? TRUE : FALSE);
	Set_DX8_Render_State(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	Set_DX8_Render_State(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	Set_DX8_Render_State(D3DRS_COLORVERTEX, TRUE);
	Set_DX8_Render_State(D3DRS_ZBIAS, 0);
	Set_DX8_Texture_Stage_State(1, D3DTSS_BUMPENVLSCALE, F2DW(1.0f));
	Set_DX8_Texture_Stage_State(1, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT00, F2DW(1.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT01, F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT10, F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT11, F2DW(1.0f));

	//	Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_CW);
		// Set dither mode here?
}

//MW: I added this for 'Generals'.
bool DX8Wrapper::Validate_Device(void)
{
	DWORD numPasses = 0;
	HRESULT hRes;

	hRes = _Get_D3D_Device8()->ValidateDevice(&numPasses);

	return (hRes == D3D_OK);
}

void DX8Wrapper::Invalidate_Cached_Render_States(void)
{
	render_state_changed = 0;

	int a;
	for (a = 0; a < sizeof(RenderStates) / sizeof(unsigned); ++a) {
		RenderStates[a] = 0x12345678;
	}
	for (a = 0; a < MAX_TEXTURE_STAGES; ++a)
	{
		for (int b = 0; b < 32; b++)
		{
			TextureStageStates[a][b] = 0x12345678;
		}
		//Need to explicitly set texture to NULL, otherwise app will not be able to
		//set it to null because of redundant state checker. MW
		if (_Get_D3D_Device8())
			_Get_D3D_Device8()->SetTexture(a, NULL);
		if (Textures[a] != NULL) {
			Textures[a]->Release();
		}
		Textures[a] = NULL;
	}

	ShaderClass::Invalidate();

	//Need to explicitly set render_state texture pointers to NULL. MW
	Release_Render_State();

	// (gth) clear the matrix shadows too
	for (int i = 0; i < D3DTS_WORLD + 1; i++) {
		DX8Transforms[i][0].Set(0, 0, 0, 0);
		DX8Transforms[i][1].Set(0, 0, 0, 0);
		DX8Transforms[i][2].Set(0, 0, 0, 0);
		DX8Transforms[i][3].Set(0, 0, 0, 0);
	}

}

void DX8Wrapper::Do_Onetime_Device_Dependent_Shutdowns(void)
{
	/*
	** Shutdown ww3d systems
	*/
	int i;
	for (i = 0; i < MAX_VERTEX_STREAMS; ++i) {
		if (render_state.vertex_buffers[i]) render_state.vertex_buffers[i]->Release_Engine_Ref();
		REF_PTR_RELEASE(render_state.vertex_buffers[i]);
	}
	if (render_state.index_buffer) render_state.index_buffer->Release_Engine_Ref();
	REF_PTR_RELEASE(render_state.index_buffer);
	REF_PTR_RELEASE(render_state.material);
	for (i = 0; i < CurrentCaps->Get_Max_Textures_Per_Pass(); ++i) REF_PTR_RELEASE(render_state.Textures[i]);


	TextureLoader::Deinit();
	SortingRendererClass::Deinit();
	DynamicVBAccessClass::_Deinit();
	DynamicIBAccessClass::_Deinit();
	ShatterSystem::Shutdown();
	PointGroupClass::_Shutdown();
	VertexMaterialClass::Shutdown();
	BoxRenderObjClass::Shutdown();
	SHD_SHUTDOWN;
	TheDX8MeshRenderer.Shutdown();
	MissingTexture::_Deinit();

	if (CurrentCaps) {
		delete CurrentCaps;
		CurrentCaps = NULL;
	}

}

bool DX8Wrapper::Create_Device(void)
{
	WWASSERT(D3DDevice == NULL);	// for now, once you've created a device, you're stuck with it!

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

#ifndef _XBOX

	Vertex_Processing_Behavior = (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) ?
		D3DCREATE_MIXED_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// enable this when all 'get' dx calls are removed KJM
	/*if (caps.DevCaps&D3DDEVCAPS_PUREDEVICE)
	{
		Vertex_Processing_Behavior|=D3DCREATE_PUREDEVICE;
	}*/

#else // XBOX
	Vertex_Processing_Behavior = D3DCREATE_PUREDEVICE;
#endif // XBOX

#ifdef CREATE_DX8_MULTI_THREADED
	Vertex_Processing_Behavior |= D3DCREATE_MULTITHREADED;
	_DX8SingleThreaded = false;
#else
	_DX8SingleThreaded = true;
#endif

	if (DX8Wrapper_PreserveFPU)
		Vertex_Processing_Behavior |= D3DCREATE_FPU_PRESERVE;

#ifdef CREATE_DX8_FPU_PRESERVE
	Vertex_Processing_Behavior |= D3DCREATE_FPU_PRESERVE;
#endif

	HRESULT hr = D3DInterface->CreateDevice
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
		// The device selection may fail because the device lied that it supports 32 bit zbuffer with 16 bit
		// display. This happens at least on Voodoo2.

		if ((_PresentParameters.BackBufferFormat == D3DFMT_R5G6B5 ||
			_PresentParameters.BackBufferFormat == D3DFMT_X1R5G5B5 ||
			_PresentParameters.BackBufferFormat == D3DFMT_A1R5G5B5) &&
			(_PresentParameters.AutoDepthStencilFormat == D3DFMT_D32 ||
				_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24S8 ||
				_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24X8))
		{
			_PresentParameters.AutoDepthStencilFormat = D3DFMT_D16;
			hr = D3DInterface->CreateDevice
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
		}
		else
		{
			return false;
		}
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
		WW3D::_Invalidate_Textures();

		for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
		{
			Set_Vertex_Buffer(NULL, i);
		}
		Set_Index_Buffer(NULL, 0);
		if (m_pCleanupHook) {
			m_pCleanupHook->ReleaseResources();
		}
		DynamicVBAccessClass::_Deinit();
		DynamicIBAccessClass::_Deinit();
		DX8TextureManagerClass::Release_Textures();
		SHD_SHUTDOWN_SHADERS;

		// Reset frame count to reflect the flipping chain being reset by Reset()
		FrameCount = 0;

		memset(Vertex_Shader_Constants, 0, sizeof(Vector4) * MAX_VERTEX_SHADER_CONSTANTS);
		memset(Pixel_Shader_Constants, 0, sizeof(Vector4) * MAX_PIXEL_SHADER_CONSTANTS);

		HRESULT hr = _Get_D3D_Device8()->TestCooperativeLevel();
		if (hr != D3DERR_DEVICELOST)
		{
			DX8CALL_HRES(Reset(&_PresentParameters), hr)
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
		SHD_INIT_SHADERS;
		WWDEBUG_SAY(("Device reset completed"));
		return true;
	}
	WWDEBUG_SAY(("Device reset failed"));
	return false;
}

void DX8Wrapper::Release_Device(void)
{
	if (D3DDevice) {

		for (int a = 0; a < MAX_TEXTURE_STAGES; ++a)
		{	//release references to any textures that were used in last rendering call
			DX8CALL(SetTexture(a, NULL));
		}

		DX8CALL(SetStreamSource(0, NULL, 0));	//release reference count on last rendered vertex buffer
		DX8CALL(SetIndices(NULL, 0));	//release reference count on last rendered index buffer


		/*
		** Release the current vertex and index buffers
		*/
		for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
		{
			if (render_state.vertex_buffers[i]) render_state.vertex_buffers[i]->Release_Engine_Ref();
			REF_PTR_RELEASE(render_state.vertex_buffers[i]);
		}
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
		D3DDevice = NULL;
	}
}

void DX8Wrapper::Enumerate_Devices()
{
	DX8_Assert();

	int adapter_count = D3DInterface->GetAdapterCount();
	for (int adapter_index = 0; adapter_index < adapter_count; adapter_index++) {

		D3DADAPTER_IDENTIFIER8 id;
		::ZeroMemory(&id, sizeof(D3DADAPTER_IDENTIFIER8));
		HRESULT res = D3DInterface->GetAdapterIdentifier(adapter_index, D3DENUM_NO_WHQL_LEVEL, &id);

		if (res == D3D_OK) {

			/*
			** Set up the render device description
			** TODO: Fill in more fields of the render device description?  (need some lookup tables)
			*/
			RenderDeviceDescClass desc;
			desc.set_device_name(id.Description);
			desc.set_driver_name(id.Driver);

			char buf[64];
			sprintf(buf, "%d.%d.%d.%d", //"%04x.%04x.%04x.%04x",
				HIWORD(id.DriverVersion.HighPart),
				LOWORD(id.DriverVersion.HighPart),
				HIWORD(id.DriverVersion.LowPart),
				LOWORD(id.DriverVersion.LowPart));

			desc.set_driver_version(buf);

			D3DInterface->GetDeviceCaps(adapter_index, WW3D_DEVTYPE, &desc.Caps);
			D3DInterface->GetAdapterIdentifier(adapter_index, D3DENUM_NO_WHQL_LEVEL, &desc.AdapterIdentifier);

			DX8Caps dx8caps(D3DInterface, desc.Caps, WW3D_FORMAT_UNKNOWN, desc.AdapterIdentifier);

			/*
			** Enumerate the resolutions
			*/
			desc.reset_resolution_list();
			int mode_count = D3DInterface->GetAdapterModeCount(adapter_index);
			for (int mode_index = 0; mode_index < mode_count; mode_index++) {
				D3DDISPLAYMODE d3dmode;
				::ZeroMemory(&d3dmode, sizeof(D3DDISPLAYMODE));
				HRESULT res = D3DInterface->EnumAdapterModes(adapter_index, mode_index, &d3dmode);

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

					// Some cards fail in certain modes, DX8Caps keeps list of those.
					if (!dx8caps.Is_Valid_Display_Format(d3dmode.Width, d3dmode.Height, D3DFormat_To_WW3DFormat(d3dmode.Format))) {
						bits = 0;
					}

					/*
					** If we recognize the format, add it to the list
					** TODO: should we handle more formats?  will any cards report more than 24 or 16 bit?
					*/
					if (bits != 0) {
						desc.add_resolution(d3dmode.Width, d3dmode.Height, bits);
					}
				}
			}

			// IML: If the device has one or more valid resolutions add it to the device list.
			// NOTE: Testing has shown that there are drivers with zero resolutions.
			if (desc.Enumerate_Resolutions().Count() > 0) {

				/*
				** Set up the device name
				*/
				StringClass device_name(id.Description, true);
				_RenderDeviceNameTable.Add(device_name);
				_RenderDeviceShortNameTable.Add(device_name);	// for now, just add the same name to the "pretty name table"

				/*
				** Add the render device to our table
				*/
				_RenderDeviceDescriptionTable.Add(desc);
			}
		}
	}
}


bool DX8Wrapper::Set_Any_Render_Device(void)
{
	// Then fullscreen
	int dev_number = 0;
	for (; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if (Set_Render_Device(dev_number, -1, -1, -1, 0, false)) {
			return true;
		}
	}

	// Try windowed first
	for (dev_number = 0; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if (Set_Render_Device(dev_number, -1, -1, -1, 1, false)) {
			return true;
		}
	}

	return false;
}

bool DX8Wrapper::Set_Render_Device
(
	const char* dev_name,
	int width,
	int height,
	int bits,
	int windowed,
	bool resize_window
)
{
	for (int dev_number = 0; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if (strcmp(dev_name, _RenderDeviceNameTable[dev_number]) == 0) {
			return Set_Render_Device(dev_number, width, height, bits, windowed, resize_window);
		}

		if (strcmp(dev_name, _RenderDeviceShortNameTable[dev_number]) == 0) {
			return Set_Render_Device(dev_number, width, height, bits, windowed, resize_window);
		}
	}
	return false;
}

void DX8Wrapper::Get_Format_Name(unsigned int format, StringClass* tex_format)
{
	*tex_format = "Unknown";
	switch (format) {
	case D3DFMT_A8R8G8B8: *tex_format = "D3DFMT_A8R8G8B8"; break;
	case D3DFMT_R8G8B8: *tex_format = "D3DFMT_R8G8B8"; break;
	case D3DFMT_A4R4G4B4: *tex_format = "D3DFMT_A4R4G4B4"; break;
	case D3DFMT_A1R5G5B5: *tex_format = "D3DFMT_A1R5G5B5"; break;
	case D3DFMT_R5G6B5: *tex_format = "D3DFMT_R5G6B5"; break;
	case D3DFMT_L8: *tex_format = "D3DFMT_L8"; break;
	case D3DFMT_A8: *tex_format = "D3DFMT_A8"; break;
	case D3DFMT_P8: *tex_format = "D3DFMT_P8"; break;
	case D3DFMT_X8R8G8B8: *tex_format = "D3DFMT_X8R8G8B8"; break;
	case D3DFMT_X1R5G5B5: *tex_format = "D3DFMT_X1R5G5B5"; break;
	case D3DFMT_R3G3B2: *tex_format = "D3DFMT_R3G3B2"; break;
	case D3DFMT_A8R3G3B2: *tex_format = "D3DFMT_A8R3G3B2"; break;
	case D3DFMT_X4R4G4B4: *tex_format = "D3DFMT_X4R4G4B4"; break;
	case D3DFMT_A8P8: *tex_format = "D3DFMT_A8P8"; break;
	case D3DFMT_A8L8: *tex_format = "D3DFMT_A8L8"; break;
	case D3DFMT_A4L4: *tex_format = "D3DFMT_A4L4"; break;
	case D3DFMT_V8U8: *tex_format = "D3DFMT_V8U8"; break;
	case D3DFMT_L6V5U5: *tex_format = "D3DFMT_L6V5U5"; break;
	case D3DFMT_X8L8V8U8: *tex_format = "D3DFMT_X8L8V8U8"; break;
	case D3DFMT_Q8W8V8U8: *tex_format = "D3DFMT_Q8W8V8U8"; break;
	case D3DFMT_V16U16: *tex_format = "D3DFMT_V16U16"; break;
	case D3DFMT_W11V11U10: *tex_format = "D3DFMT_W11V11U10"; break;
	case D3DFMT_UYVY: *tex_format = "D3DFMT_UYVY"; break;
	case D3DFMT_YUY2: *tex_format = "D3DFMT_YUY2"; break;
	case D3DFMT_DXT1: *tex_format = "D3DFMT_DXT1"; break;
	case D3DFMT_DXT2: *tex_format = "D3DFMT_DXT2"; break;
	case D3DFMT_DXT3: *tex_format = "D3DFMT_DXT3"; break;
	case D3DFMT_DXT4: *tex_format = "D3DFMT_DXT4"; break;
	case D3DFMT_DXT5: *tex_format = "D3DFMT_DXT5"; break;
	case D3DFMT_D16_LOCKABLE: *tex_format = "D3DFMT_D16_LOCKABLE"; break;
	case D3DFMT_D32: *tex_format = "D3DFMT_D32"; break;
	case D3DFMT_D15S1: *tex_format = "D3DFMT_D15S1"; break;
	case D3DFMT_D24S8: *tex_format = "D3DFMT_D24S8"; break;
	case D3DFMT_D16: *tex_format = "D3DFMT_D16"; break;
	case D3DFMT_D24X8: *tex_format = "D3DFMT_D24X8"; break;
	case D3DFMT_D24X4S4: *tex_format = "D3DFMT_D24X4S4"; break;
	default:	break;
	}
}

void DX8Wrapper::Resize_And_Position_Window()
{
	// Get the current dimensions of the 'render area' of the window
	RECT rect = { 0 };
	::GetClientRect(_Hwnd, &rect);

	// Is the window the correct size for this resolution?
	if ((rect.right - rect.left) != ResolutionWidth ||
		(rect.bottom - rect.top) != ResolutionHeight) {

		// Calculate what the main window's bounding rectangle should be to
		// accommodate this resolution
		rect.left = 0;
		rect.top = 0;
		rect.right = ResolutionWidth;
		rect.bottom = ResolutionHeight;
		DWORD dwstyle = ::GetWindowLong(_Hwnd, GWL_STYLE);
		AdjustWindowRect(&rect, dwstyle, FALSE);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

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
			MONITORINFO mi = { sizeof(MONITORINFO) };
			GetMonitorInfo(MonitorFromWindow(_Hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
			int left = (mi.rcWork.left + mi.rcWork.right - width) / 2;
			int top = (mi.rcWork.top + mi.rcWork.bottom - height) / 2;

			// TheSuperHackers @feature helmutbuhler 14/04/2025
			// Move the window to try fit it into the monitor area, if one of its dimensions is larger than the work area.
			// Otherwise align the window to the top left edges, if it is even larger than the monitor area.
			RECT rectClient;
			rectClient.left = left - rect.left;
			rectClient.top = top - rect.top;
			rectClient.right = rectClient.left + ResolutionWidth;
			rectClient.bottom = rectClient.top + ResolutionHeight;
			MoveRectIntoOtherRect(rectClient, mi.rcMonitor, &left, &top);

			::SetWindowPos(_Hwnd, NULL, left, top, width, height, SWP_NOZORDER);

			DEBUG_LOG(("Window positioned to x:%d y:%d, resized to w:%d h:%d", left, top, width, height));
		}
	}
}



bool DX8Wrapper::Set_Render_Device(int dev, int width, int height, int bits, int windowed,
	bool resize_window, bool reset_device, bool restore_assets)
{
	WWASSERT(IsInitted);
	WWASSERT(dev >= -1);
	WWASSERT(dev < _RenderDeviceNameTable.Count());

	/*
	** If user has never selected a render device, start out with device 0
	*/
	if ((CurRenderDevice == -1) && (dev == -1)) {
		CurRenderDevice = 0;
	}
	else if (dev != -1) {
		CurRenderDevice = dev;
	}

	/*
	** If user doesn't want to change res, set the res variables to match the
	** current resolution
	*/
	if (width != -1)		ResolutionWidth = width;
	if (height != -1)		ResolutionHeight = height;

	// Initialize Render2DClass Screen Resolution
	Render2DClass::Set_Screen_Resolution(RectClass(0, 0, ResolutionWidth, ResolutionHeight));

	if (bits != -1)		BitDepth = bits;
	if (windowed != -1)	IsWindowed = (windowed != 0);
	DX8Wrapper_IsWindowed = IsWindowed;

	WWDEBUG_SAY(("Attempting Set_Render_Device: name: %s (%s:%s), width: %d, height: %d, windowed: %d",
		_RenderDeviceNameTable[CurRenderDevice].str(), _RenderDeviceDescriptionTable[CurRenderDevice].Get_Driver_Name(),
		_RenderDeviceDescriptionTable[CurRenderDevice].Get_Driver_Version(), ResolutionWidth, ResolutionHeight, (IsWindowed ? 1 : 0)));

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
	//I changed this to discard all the time (even when full-screen) since that the most efficient. 07-16-03 MW:
	_PresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;//IsWindowed ? D3DSWAPEFFECT_DISCARD : D3DSWAPEFFECT_FLIP;		// Shouldn't this be D3DSWAPEFFECT_FLIP?
	_PresentParameters.hDeviceWindow = _Hwnd;
	_PresentParameters.Windowed = IsWindowed;

	_PresentParameters.EnableAutoDepthStencil = TRUE;				// Driver will attempt to match Z-buffer depth
	_PresentParameters.Flags = 0;											// We're not going to lock the backbuffer

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
		D3DInterface->GetAdapterDisplayMode(CurRenderDevice, &desktop_mode);

		DisplayFormat = _PresentParameters.BackBufferFormat = desktop_mode.Format;

		// In windowed mode, define the bitdepth from desktop mode (as it can't be changed)
		switch (_PresentParameters.BackBufferFormat) {
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8R8G8B8:
		case D3DFMT_R8G8B8: BitDepth = 32; break;
		case D3DFMT_A4R4G4B4:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_R5G6B5: BitDepth = 16; break;
		case D3DFMT_L8:
		case D3DFMT_A8:
		case D3DFMT_P8: BitDepth = 8; break;
		default:
			// Unknown backbuffer format probably means the device can't do windowed
			return false;
		}

		if (BitDepth == 32 && D3DInterface->CheckDeviceType(0, D3DDEVTYPE_HAL, desktop_mode.Format, D3DFMT_A8R8G8B8, TRUE) == D3D_OK)
		{	//promote 32-bit modes to include destination alpha
			_PresentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
		}

		/*
		** Find a appropriate Z buffer
		*/
		if (!Find_Z_Mode(DisplayFormat, _PresentParameters.BackBufferFormat, &_PresentParameters.AutoDepthStencilFormat))
		{
			// If opening 32 bit mode failed, try 16 bit, even if the desktop happens to be 32 bit
			if (BitDepth == 32) {
				BitDepth = 16;
				_PresentParameters.BackBufferFormat = D3DFMT_R5G6B5;
				if (!Find_Z_Mode(_PresentParameters.BackBufferFormat, _PresentParameters.BackBufferFormat, &_PresentParameters.AutoDepthStencilFormat)) {
					_PresentParameters.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
				}
			}
			else {
				_PresentParameters.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
			}
		}

	}
	else {

		/*
		** Try to find a mode that matches the user's desired bit-depth.
		*/
		Find_Color_And_Z_Mode(ResolutionWidth, ResolutionHeight, BitDepth, &DisplayFormat,
			&_PresentParameters.BackBufferFormat, &_PresentParameters.AutoDepthStencilFormat);
	}

	/*
	** Time to actually create the device.
	*/
	if (_PresentParameters.AutoDepthStencilFormat == D3DFMT_UNKNOWN) {
		if (BitDepth == 32) {
			_PresentParameters.AutoDepthStencilFormat = D3DFMT_D32;
		}
		else {
			_PresentParameters.AutoDepthStencilFormat = D3DFMT_D16;
		}
	}

	StringClass displayFormat;
	StringClass backbufferFormat;

	Get_Format_Name(DisplayFormat, &displayFormat);
	Get_Format_Name(_PresentParameters.BackBufferFormat, &backbufferFormat);

	WWDEBUG_SAY(("Using Display/BackBuffer Formats: %s/%s", displayFormat.str(), backbufferFormat.str()));

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
	assert(IsInitted);
	if (IsInitted) {

		// Get information about the current render device's resolutions
		const RenderDeviceDescClass& render_device = Get_Render_Device_Desc();
		const DynamicVectorClass<ResolutionDescClass>& resolutions = render_device.Enumerate_Resolutions();

		// Loop through all the resolutions supported by the current device.
		// If we aren't currently running under one of these resolutions,
		// then we should probably		 to the closest resolution before
		// toggling the windowed state.
		int curr_res = -1;
		for (int res = 0;
			(res < resolutions.Count()) && (curr_res == -1);
			res++) {

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
			return Set_Device_Resolution(resolutions[0].Width,
				resolutions[0].Height,
				resolutions[0].BitDepth,
				!IsWindowed, true);
		}
		else {

			// Toggle the windowed state
			return Set_Device_Resolution(-1, -1, -1, !IsWindowed, true);
		}
	}
#endif //WW3D_DX8

	return false;
}

void DX8Wrapper::Set_Swap_Interval(int swap)
{
	switch (swap) {
	case 0: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; break;
	case 1: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE; break;
	case 2: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_TWO; break;
	case 3: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_THREE; break;
	default: _PresentParameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE; break;
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

const RenderDeviceDescClass& DX8Wrapper::Get_Render_Device_Desc(int deviceidx)
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

const char* DX8Wrapper::Get_Render_Device_Name(int device_index)
{
	device_index = device_index % _RenderDeviceShortNameTable.Count();
	return _RenderDeviceShortNameTable[device_index];
}

bool DX8Wrapper::Set_Device_Resolution(int width, int height, int bits, int windowed, bool resize_window)
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
	}
	else {
		return false;
	}
}

void DX8Wrapper::Get_Device_Resolution(int& set_w, int& set_h, int& set_bits, bool& set_windowed)
{
	WWASSERT(IsInitted);

	set_w = ResolutionWidth;
	set_h = ResolutionHeight;
	set_bits = BitDepth;
	set_windowed = IsWindowed;

	return;
}

void DX8Wrapper::Get_Render_Target_Resolution(int& set_w, int& set_h, int& set_bits, bool& set_windowed)
{
	WWASSERT(IsInitted);

	if (CurrentRenderTarget != NULL) {
		D3DSURFACE_DESC info;
		CurrentRenderTarget->GetDesc(&info);

		set_w = info.Width;
		set_h = info.Height;
		set_bits = BitDepth;		// should we get the actual bit depth of the target?
		set_windowed = IsWindowed;	// this doesn't really make sense for render targets (shouldn't matter)...

	}
	else {
		Get_Device_Resolution(set_w, set_h, set_bits, set_windowed);
	}

	return;
}

bool DX8Wrapper::Registry_Save_Render_Device(const char* sub_key)
{
	int	width, height, depth;
	bool	windowed;
	Get_Device_Resolution(width, height, depth, windowed);
	return Registry_Save_Render_Device(sub_key, CurRenderDevice, ResolutionWidth, ResolutionHeight, BitDepth, IsWindowed, TextureBitDepth);
}

bool DX8Wrapper::Registry_Save_Render_Device(const char* sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth)
{
	RegistryClass* registry = W3DNEW RegistryClass(sub_key);
	WWASSERT(registry);

	if (!registry->Is_Valid()) {
		delete registry;
		WWDEBUG_SAY(("Error getting Registry"));
		return false;
	}

	registry->Set_String(VALUE_NAME_RENDER_DEVICE_NAME,
		_RenderDeviceShortNameTable[device]);
	registry->Set_Int(VALUE_NAME_RENDER_DEVICE_WIDTH, width);
	registry->Set_Int(VALUE_NAME_RENDER_DEVICE_HEIGHT, height);
	registry->Set_Int(VALUE_NAME_RENDER_DEVICE_DEPTH, depth);
	registry->Set_Int(VALUE_NAME_RENDER_DEVICE_WINDOWED, windowed);
	registry->Set_Int(VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, texture_depth);

	delete registry;
	return true;
}

bool DX8Wrapper::Registry_Load_Render_Device(const char* sub_key, bool resize_window)
{
	char	name[200];
	int	width, height, depth, windowed;

	if (Registry_Load_Render_Device(sub_key,
		name,
		sizeof(name),
		width,
		height,
		depth,
		windowed,
		TextureBitDepth) &&
		(*name != 0))
	{
		WWDEBUG_SAY(("Device %s (%d X %d) %d bit windowed:%d", name, width, height, depth, windowed));

		if (TextureBitDepth == 16 || TextureBitDepth == 32) {
			//			WWDEBUG_SAY(( "Texture depth %d", TextureBitDepth));
		}
		else {
			WWDEBUG_SAY(("Invalid texture depth %d, switching to 16 bits", TextureBitDepth));
			TextureBitDepth = 16;
		}


		//		_RenderDeviceDescriptionTable.


		if (Set_Render_Device(name, width, height, depth, windowed, resize_window) != true) {
			if (depth == 16) depth = 32;
			else depth = 16;
			if (Set_Render_Device(name, width, height, depth, windowed, resize_window) == true) {
				return true;
			}
			if (depth == 16) depth = 32;
			else depth = 16;
			// we'll test resolutions down, so if start is 640, increase to begin with...
			if (width == 640) {
				width = 1024;
				height = 768;
			}
			for (;;) {
				if (width > 2048) {
					width = 2048;
					height = 1536;
				}
				else if (width > 1920) {
					width = 1920;
					height = 1440;
				}
				else if (width > 1600) {
					width = 1600;
					height = 1200;
				}
				else if (width > 1280) {
					width = 1280;
					height = 1024;
				}
				else if (width > 1024) {
					width = 1024;
					height = 768;
				}
				else if (width > 800) {
					width = 800;
					height = 600;
				}
				else if (width != 640) {
					width = 640;
					height = 480;
				}
				else {
					return Set_Any_Render_Device();
				}
				for (int i = 0; i < 2; ++i) {
					if (Set_Render_Device(name, width, height, depth, windowed, resize_window) == true) {
						return true;
					}
					if (depth == 16) depth = 32;
					else depth = 16;
				}
			}
		}

		return true;
	}

	WWDEBUG_SAY(("Error getting Registry"));

	return Set_Any_Render_Device();
}

bool DX8Wrapper::Registry_Load_Render_Device(const char* sub_key, char* device, int device_len, int& width, int& height, int& depth, int& windowed, int& texture_depth)
{
	RegistryClass registry(sub_key);

	if (registry.Is_Valid()) {
		registry.Get_String(VALUE_NAME_RENDER_DEVICE_NAME,
			device, device_len);

		width = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_WIDTH, -1);
		height = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_HEIGHT, -1);
		depth = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_DEPTH, -1);
		windowed = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_WINDOWED, -1);
		texture_depth = registry.Get_Int(VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH, -1);
		return true;
	}
	*device = 0;
	width = -1;
	height = -1;
	depth = -1;
	windowed = -1;
	texture_depth = -1;
	return false;
}


bool DX8Wrapper::Find_Color_And_Z_Mode(int resx, int resy, int bitdepth, D3DFORMAT* set_colorbuffer, D3DFORMAT* set_backbuffer, D3DFORMAT* set_zmode)
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
	D3DFORMAT* format_table = NULL;
	int format_count = 0;

	if (BitDepth == 16) {
		format_table = _formats16;
		format_count = sizeof(_formats16) / sizeof(D3DFORMAT);
	}
	else {
		format_table = _formats32;
		format_count = sizeof(_formats32) / sizeof(D3DFORMAT);
	}

	/*
	** now search for a valid format
	*/
	bool found = false;
	unsigned int mode = 0;

	int format_index = 0;
	for (; format_index < format_count; format_index++) {
		found |= Find_Color_Mode(format_table[format_index], resx, resy, &mode);
		if (found) break;
	}

	if (!found) {
		return false;
	}
	else {
		*set_backbuffer = *set_colorbuffer = format_table[format_index];
	}

	if (bitdepth == 32 && *set_colorbuffer == D3DFMT_X8R8G8B8 && D3DInterface->CheckDeviceType(0, D3DDEVTYPE_HAL, *set_colorbuffer, D3DFMT_A8R8G8B8, TRUE) == D3D_OK)
	{	//promote 32-bit modes to include destination alpha when supported
		*set_backbuffer = D3DFMT_A8R8G8B8;
	}

	/*
	** We found a backbuffer format, now find a zbuffer format
	*/
	return Find_Z_Mode(*set_colorbuffer, *set_backbuffer, set_zmode);
};


// find the resolution mode with at least resx,resy with the highest supported
// refresh rate
bool DX8Wrapper::Find_Color_Mode(D3DFORMAT colorbuffer, int resx, int resy, UINT* mode)
{
	UINT i, j, modemax;
	UINT rx, ry;
	D3DDISPLAYMODE dmode;
	::ZeroMemory(&dmode, sizeof(D3DDISPLAYMODE));

	rx = (unsigned int)resx;
	ry = (unsigned int)resy;

	bool found = false;

	modemax = D3DInterface->GetAdapterModeCount(D3DADAPTER_DEFAULT);

	i = 0;

	while (i < modemax && !found)
	{
		D3DInterface->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &dmode);
		if (dmode.Width == rx && dmode.Height == ry && dmode.Format == colorbuffer) {
			WWDEBUG_SAY(("Found valid color mode.  Width = %d Height = %d Format = %d", dmode.Width, dmode.Height, dmode.Format));
			found = true;
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
	bool stillok = true;

	j = i;
	while (j < modemax && stillok)
	{
		D3DInterface->EnumAdapterModes(D3DADAPTER_DEFAULT, j, &dmode);
		if (dmode.Width == rx && dmode.Height == ry && dmode.Format == colorbuffer)
			stillok = true; else stillok = false;
		j++;
	}

	if (stillok == false) *mode = j - 2;
	else *mode = i;

	return true;
}

// Helper function to find a Z buffer mode for the colorbuffer
// Will look for greatest Z precision
bool DX8Wrapper::Find_Z_Mode(D3DFORMAT colorbuffer, D3DFORMAT backbuffer, D3DFORMAT* zmode)
{
	//MW: Swapped the next 2 tests so that Stencil modes get tested first.
	if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D24S8))
	{
		*zmode = D3DFMT_D24S8;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24S8"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D32))
	{
		*zmode = D3DFMT_D32;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D32"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D24X8))
	{
		*zmode = D3DFMT_D24X8;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24X8"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D24X4S4))
	{
		*zmode = D3DFMT_D24X4S4;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24X4S4"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D16))
	{
		*zmode = D3DFMT_D16;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D16"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer, backbuffer, D3DFMT_D15S1))
	{
		*zmode = D3DFMT_D15S1;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D15S1"));
		return true;
	}

	// can't find a match
	WWDEBUG_SAY(("Failed to find a valid zbuffer mode"));
	return false;
}

bool DX8Wrapper::Test_Z_Mode(D3DFORMAT colorbuffer, D3DFORMAT backbuffer, D3DFORMAT zmode)
{
	// See if we have this mode first
	if (FAILED(D3DInterface->CheckDeviceFormat(D3DADAPTER_DEFAULT, WW3D_DEVTYPE,
		colorbuffer, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, zmode)))
	{
		WWDEBUG_SAY(("CheckDeviceFormat failed.  Colorbuffer format = %d  Zbufferformat = %d", colorbuffer, zmode));
		return false;
	}

	// Then see if it matches the color buffer
	if (FAILED(D3DInterface->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, WW3D_DEVTYPE,
		colorbuffer, backbuffer, zmode)))
	{
		WWDEBUG_SAY(("CheckDepthStencilMatch failed.  Colorbuffer format = %d  Backbuffer format = %d Zbufferformat = %d", colorbuffer, backbuffer, zmode));
		return false;
	}
	return true;
}


void DX8Wrapper::Reset_Statistics()
{
	matrix_changes = 0;
	material_changes = 0;
	vertex_buffer_changes = 0;
	index_buffer_changes = 0;
	light_changes = 0;
	texture_changes = 0;
	render_state_changes = 0;
	texture_stage_state_changes = 0;
	draw_calls = 0;

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
	last_frame_draw_calls = 0;
}

void DX8Wrapper::Begin_Statistics()
{
	matrix_changes = 0;
	material_changes = 0;
	vertex_buffer_changes = 0;
	index_buffer_changes = 0;
	light_changes = 0;
	texture_changes = 0;
	render_state_changes = 0;
	texture_stage_state_changes = 0;
	number_of_DX8_calls = 0;
	draw_calls = 0;
}

void DX8Wrapper::End_Statistics()
{
	last_frame_matrix_changes = matrix_changes;
	last_frame_material_changes = material_changes;
	last_frame_vertex_buffer_changes = vertex_buffer_changes;
	last_frame_index_buffer_changes = index_buffer_changes;
	last_frame_light_changes = light_changes;
	last_frame_texture_changes = texture_changes;
	last_frame_render_state_changes = render_state_changes;
	last_frame_texture_stage_state_changes = texture_stage_state_changes;
	last_frame_number_of_DX8_calls = number_of_DX8_calls;
	last_frame_draw_calls = draw_calls;
}

unsigned DX8Wrapper::Get_Last_Frame_Matrix_Changes() { return last_frame_matrix_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Material_Changes() { return last_frame_material_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Vertex_Buffer_Changes() { return last_frame_vertex_buffer_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Index_Buffer_Changes() { return last_frame_index_buffer_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Light_Changes() { return last_frame_light_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Texture_Changes() { return last_frame_texture_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Render_State_Changes() { return last_frame_render_state_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Texture_Stage_State_Changes() { return last_frame_texture_stage_state_changes; }
unsigned DX8Wrapper::Get_Last_Frame_DX8_Calls() { return last_frame_number_of_DX8_calls; }
unsigned DX8Wrapper::Get_Last_Frame_Draw_Calls() { return last_frame_draw_calls; }
unsigned long DX8Wrapper::Get_FrameCount(void) { return FrameCount; }

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
			hr = _Get_D3D_Device8()->Present(NULL, NULL, NULL, NULL);
		}

		number_of_DX8_calls++;

		if (SUCCEEDED(hr)) {
#ifdef EXTENDED_STATS
			if (stats.m_sleepTime) {
				::Sleep(stats.m_sleepTime);
			}
#endif
			IsDeviceLost = false;
			FrameCount++;
		}
		else {
			IsDeviceLost = true;
		}

		// If the device was lost we need to check for cooperative level and possibly reset the device
		if (hr == D3DERR_DEVICELOST) {
			hr = _Get_D3D_Device8()->TestCooperativeLevel();
			if (hr == D3DERR_DEVICENOTRESET) {
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
	Set_Index_Buffer(NULL, 0);
	for (int i = 0; i < CurrentCaps->Get_Max_Textures_Per_Pass(); ++i) Set_Texture(i, NULL);
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
					IsDeviceLost = true;
					WWDEBUG_SAY(("DEVICELOST: Cannot flip to primary."));
					return;
				}
				IsDeviceLost = false;

				if (D3DERR_DEVICENOTRESET == hr) {
					WWDEBUG_SAY(("DEVICENOTRESET"));
					Reset_Device();
					resetAttempts++;
				}
			}
			else {
				WWDEBUG_SAY(("Flipping: %ld", FrameCount));
				hr = _Get_D3D_Device8()->Present(NULL, NULL, NULL, NULL);

				if (SUCCEEDED(hr)) {
					IsDeviceLost = false;
					FrameCount++;
					WWDEBUG_SAY(("Flip to primary succeeded %ld", FrameCount));
				}
				else {
					IsDeviceLost = true;
				}
			}

			--flipCount;
		}
	}
}



//**********************************************************************************************
//! Clear current render device
/*! KM
/* 5/17/02 KM Fixed support for render to texture with depth/stencil buffers
*/
void DX8Wrapper::Clear(bool clear_color, bool clear_z_stencil, const Vector3& color, float dest_alpha, float z, unsigned int stencil)
{
	DX8_THREAD_ASSERT();

	// If we try to clear a stencil buffer which is not there, the entire call will fail
	// KJM fixed this to get format from back buffer (incase render to texture is used)
	/*bool has_stencil = (	_PresentParameters.AutoDepthStencilFormat == D3DFMT_D15S1 ||
								_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24S8 ||
								_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24X4S4);*/
	bool has_stencil = false;
	IDirect3DSurface8* depthbuffer;

	_Get_D3D_Device8()->GetDepthStencilSurface(&depthbuffer);
	number_of_DX8_calls++;

	if (depthbuffer)
	{
		D3DSURFACE_DESC desc;
		depthbuffer->GetDesc(&desc);
		has_stencil =
			(
				desc.Format == D3DFMT_D15S1 ||
				desc.Format == D3DFMT_D24S8 ||
				desc.Format == D3DFMT_D24X4S4
				);

		// release ref
		depthbuffer->Release();
	}

	DWORD flags = 0;
	if (clear_color) flags |= D3DCLEAR_TARGET;
	if (clear_z_stencil) flags |= D3DCLEAR_ZBUFFER;
	if (clear_z_stencil && has_stencil) flags |= D3DCLEAR_STENCIL;
	if (flags)
	{
		DX8CALL(Clear(0, NULL, flags, Convert_Color(color, dest_alpha), z, stencil));
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

void DX8Wrapper::Set_Vertex_Buffer(const VertexBufferClass* vb, unsigned stream)
{
	render_state.vba_offset = 0;
	render_state.vba_count = 0;
	if (render_state.vertex_buffers[stream]) {
		render_state.vertex_buffers[stream]->Release_Engine_Ref();
	}
	REF_PTR_SET(render_state.vertex_buffers[stream], const_cast<VertexBufferClass*>(vb));
	if (vb) {
		vb->Add_Engine_Ref();
		render_state.vertex_buffer_types[stream] = vb->Type();
	}
	else {
		render_state.vertex_buffer_types[stream] = BUFFER_TYPE_INVALID;
	}
	render_state_changed |= VERTEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
//
// Set index buffer. A reference to previous index buffer is released and
// this one is assigned the current index buffer. The DX8 index buffer will
// actually be set in Apply() which is called by Draw_Indexed_Triangles().
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset)
{
	render_state.iba_offset = 0;
	if (render_state.index_buffer) {
		render_state.index_buffer->Release_Engine_Ref();
	}
	REF_PTR_SET(render_state.index_buffer, const_cast<IndexBufferClass*>(ib));
	render_state.index_base_offset = index_base_offset;
	if (ib) {
		ib->Add_Engine_Ref();
		render_state.index_buffer_type = ib->Type();
	}
	else {
		render_state.index_buffer_type = BUFFER_TYPE_INVALID;
	}
	render_state_changed |= INDEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
//
// Set vertex buffer using dynamic access object.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Vertex_Buffer(const DynamicVBAccessClass& vba_)
{
	// Release all streams (only one stream allowed in the legacy pipeline)
	for (int i = 1; i < MAX_VERTEX_STREAMS; ++i) {
		DX8Wrapper::Set_Vertex_Buffer(NULL, i);
	}

	if (render_state.vertex_buffers[0]) render_state.vertex_buffers[0]->Release_Engine_Ref();
	DynamicVBAccessClass& vba = const_cast<DynamicVBAccessClass&>(vba_);
	render_state.vertex_buffer_types[0] = vba.Get_Type();
	render_state.vba_offset = vba.VertexBufferOffset;
	render_state.vba_count = vba.Get_Vertex_Count();
	REF_PTR_SET(render_state.vertex_buffers[0], vba.VertexBuffer);
	render_state.vertex_buffers[0]->Add_Engine_Ref();
	render_state_changed |= VERTEX_BUFFER_CHANGED;
	render_state_changed |= INDEX_BUFFER_CHANGED;		// vba_offset changes so index buffer needs to be reset as well.
}

// ----------------------------------------------------------------------------
//
// Set index buffer using dynamic access object.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Index_Buffer(const DynamicIBAccessClass& iba_, unsigned short index_base_offset)
{
	if (render_state.index_buffer) render_state.index_buffer->Release_Engine_Ref();

	DynamicIBAccessClass& iba = const_cast<DynamicIBAccessClass&>(iba_);
	render_state.index_base_offset = index_base_offset;
	render_state.index_buffer_type = iba.Get_Type();
	render_state.iba_offset = iba.IndexBufferOffset;
	REF_PTR_SET(render_state.index_buffer, iba.IndexBuffer);
	render_state.index_buffer->Add_Engine_Ref();
	render_state_changed |= INDEX_BUFFER_CHANGED;
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
	WWASSERT(render_state.vertex_buffer_types[0] == BUFFER_TYPE_SORTING || render_state.vertex_buffer_types[0] == BUFFER_TYPE_DYNAMIC_SORTING);
	WWASSERT(render_state.index_buffer_type == BUFFER_TYPE_SORTING || render_state.index_buffer_type == BUFFER_TYPE_DYNAMIC_SORTING);

	// Fill dynamic vertex buffer with sorting vertex buffer vertices
	DynamicVBAccessClass dyn_vb_access(BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, vertex_count);
	{
		DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
		VertexFormatXYZNDUV2* src = static_cast<SortingVertexBufferClass*>(render_state.vertex_buffers[0])->VertexBuffer;
		VertexFormatXYZNDUV2* dest = lock.Get_Formatted_Vertex_Array();
		src += render_state.vba_offset + render_state.index_base_offset + min_vertex_index;
		unsigned  size = dyn_vb_access.FVF_Info().Get_FVF_Size() * vertex_count / sizeof(unsigned);
		unsigned* dest_u = (unsigned*)dest;
		unsigned* src_u = (unsigned*)src;

		for (unsigned i = 0; i < size; ++i) {
			*dest_u++ = *src_u++;
		}
	}

	DX8CALL(SetStreamSource(
		0,
		static_cast<DX8VertexBufferClass*>(dyn_vb_access.VertexBuffer)->Get_DX8_Vertex_Buffer(),
		dyn_vb_access.FVF_Info().Get_FVF_Size()));
	// If using FVF format VB, set the FVF as vertex shader (may not be needed here KM)
	unsigned fvf = dyn_vb_access.FVF_Info().Get_FVF();
	if (fvf != 0) {
		DX8CALL(SetVertexShader(fvf));
	}
	DX8_RECORD_VERTEX_BUFFER_CHANGE();

	unsigned index_count = 0;
	switch (primitive_type) {
	case D3DPT_TRIANGLELIST: index_count = polygon_count * 3; break;
	case D3DPT_TRIANGLESTRIP: index_count = polygon_count + 2; break;
	case D3DPT_TRIANGLEFAN: index_count = polygon_count + 2; break;
	default: WWASSERT(0); break; // Unsupported primitive type
	}

	// Fill dynamic index buffer with sorting index buffer vertices
	DynamicIBAccessClass dyn_ib_access(BUFFER_TYPE_DYNAMIC_DX8, index_count);
	{
		DynamicIBAccessClass::WriteLockClass lock(&dyn_ib_access);
		unsigned short* dest = lock.Get_Index_Array();
		unsigned short* src = NULL;
		src = static_cast<SortingIndexBufferClass*>(render_state.index_buffer)->index_buffer;
		src += render_state.iba_offset + start_index;

		for (unsigned short i = 0; i < index_count; ++i) {
			unsigned short index = *src++;
			index -= min_vertex_index;
			WWASSERT(index < vertex_count);
			*dest++ = index;
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

	DX8_RECORD_RENDER(polygon_count, vertex_count, render_state.shader);
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
	if (DrawPolygonLowBoundLimit && DrawPolygonLowBoundLimit >= polygon_count) return;

	DX8_THREAD_ASSERT();
	SNAPSHOT_SAY(("DX8 - draw"));

	Apply_Render_State_Changes();

	// Debug feature to disable triangle drawing...
	if (!_Is_Triangle_Draw_Enabled()) return;

#ifdef MESH_RENDER_SNAPSHOT_ENABLED
	if (WW3D::Is_Snapshot_Activated()) {
		unsigned long passes = 0;
		SNAPSHOT_SAY(("ValidateDevice:"));
		HRESULT res = D3DDevice->ValidateDevice(&passes);
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


	SNAPSHOT_SAY(("DX8 - draw %d polygons (%d vertices)", polygon_count, vertex_count));

	if (vertex_count < 3) {
		min_vertex_index = 0;
		switch (render_state.vertex_buffer_types[0]) {
		case BUFFER_TYPE_DX8:
		case BUFFER_TYPE_SORTING:
			vertex_count = render_state.vertex_buffers[0]->Get_Vertex_Count() - render_state.index_base_offset - render_state.vba_offset - min_vertex_index;
			break;
		case BUFFER_TYPE_DYNAMIC_DX8:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			vertex_count = render_state.vba_count;
			break;
		}
	}

	switch (render_state.vertex_buffer_types[0]) {
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
			DX8_RECORD_RENDER(polygon_count, vertex_count, render_state.shader);
			DX8_RECORD_DRAW_CALLS();
			DX8CALL(DrawIndexedPrimitive(
				(D3DPRIMITIVETYPE)primitive_type,
				min_vertex_index,
				vertex_count,
				start_index + render_state.iba_offset,
				polygon_count));
		}
		break;
		case BUFFER_TYPE_SORTING:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			WWASSERT_PRINT(0, "VB and IB must of same type (sorting or dx8)");
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
			WWASSERT_PRINT(0, "VB and IB must of same type (sorting or dx8)");
			break;
		case BUFFER_TYPE_SORTING:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			Draw_Sorting_IB_VB(primitive_type, start_index, polygon_count, min_vertex_index, vertex_count);
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
	if (buffer_type == BUFFER_TYPE_SORTING || buffer_type == BUFFER_TYPE_DYNAMIC_SORTING) {
		SortingRendererClass::Insert_Triangles(start_index, polygon_count, min_vertex_index, vertex_count);
	}
	else {
		Draw(D3DPT_TRIANGLELIST, start_index, polygon_count, min_vertex_index, vertex_count);
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
	Draw(D3DPT_TRIANGLELIST, start_index, polygon_count, min_vertex_index, vertex_count);
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
	Draw(D3DPT_TRIANGLESTRIP, start_index, polygon_count, min_vertex_index, vertex_count);
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
	if (render_state_changed & SHADER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply shader"));
		render_state.shader.Apply();
	}

	unsigned mask = TEXTURE0_CHANGED;
	int i = 0;
	for (; i < CurrentCaps->Get_Max_Textures_Per_Pass(); ++i, mask <<= 1)
	{
		if (render_state_changed & mask)
		{
			SNAPSHOT_SAY(("DX8 - apply texture %d (%s)", i, render_state.Textures[i] ? render_state.Textures[i]->Get_Full_Path().str() : "NULL"));

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

	if (render_state_changed & MATERIAL_CHANGED)
	{
		SNAPSHOT_SAY(("DX8 - apply material"));
		VertexMaterialClass* material = const_cast<VertexMaterialClass*>(render_state.material);
		if (material)
		{
			material->Apply();
		}
		else VertexMaterialClass::Apply_Null();
	}

	if (render_state_changed & LIGHTS_CHANGED)
	{
		unsigned mask = LIGHT0_CHANGED;
		for (unsigned index = 0; index < 4; ++index, mask <<= 1) {
			if (render_state_changed & mask) {
				SNAPSHOT_SAY(("DX8 - apply light %d", index));
				if (render_state.LightEnable[index]) {
#ifdef MESH_RENDER_SNAPSHOT_ENABLED		
					if (WW3D::Is_Snapshot_Activated()) {
						D3DLIGHT8* light = &(render_state.Lights[index]);
						static const char* _light_types[] = { "Unknown", "Point","Spot", "Directional" };
						WWASSERT((light->Type >= 0) && (light->Type <= 3));

						SNAPSHOT_SAY((" type = %s amb = %4.2f,%4.2f,%4.2f  diff = %4.2f,%4.2f,%4.2f spec = %4.2f, %4.2f, %4.2f",
							_light_types[light->Type],
							light->Ambient.r, light->Ambient.g, light->Ambient.b,
							light->Diffuse.r, light->Diffuse.g, light->Diffuse.b,
							light->Specular.r, light->Specular.g, light->Specular.b));
						SNAPSHOT_SAY((" pos = %f, %f, %f  dir = %f, %f, %f",
							light->Position.x, light->Position.y, light->Position.z,
							light->Direction.x, light->Direction.y, light->Direction.z));
					}
#endif

					Set_DX8_Light(index, &render_state.Lights[index]);
				}
				else {
					Set_DX8_Light(index, NULL);
					SNAPSHOT_SAY((" clearing light to NULL"));
				}
			}
		}
	}

	if (render_state_changed & WORLD_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply world matrix"));
		_Set_DX8_Transform(D3DTS_WORLD, render_state.world);
	}
	if (render_state_changed & VIEW_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply view matrix"));
		_Set_DX8_Transform(D3DTS_VIEW, render_state.view);
	}
	if (render_state_changed & VERTEX_BUFFER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply vb change"));
		for (i = 0; i < MAX_VERTEX_STREAMS; ++i) {
			if (render_state.vertex_buffers[i]) {
				switch (render_state.vertex_buffer_types[i]) {//->Type()) {
				case BUFFER_TYPE_DX8:
				case BUFFER_TYPE_DYNAMIC_DX8:
					DX8CALL(SetStreamSource(
						i,
						static_cast<DX8VertexBufferClass*>(render_state.vertex_buffers[i])->Get_DX8_Vertex_Buffer(),
						render_state.vertex_buffers[i]->FVF_Info().Get_FVF_Size()));
					DX8_RECORD_VERTEX_BUFFER_CHANGE();
					{
						// If the VB format is FVF, set the FVF as a vertex shader
						unsigned fvf = render_state.vertex_buffers[i]->FVF_Info().Get_FVF();
						if (fvf != 0) {
							Set_Vertex_Shader(fvf);
						}
					}
					break;
				case BUFFER_TYPE_SORTING:
				case BUFFER_TYPE_DYNAMIC_SORTING:
					break;
				default:
					WWASSERT(0);
				}
			}
			else {
				DX8CALL(SetStreamSource(i, NULL, 0));
				DX8_RECORD_VERTEX_BUFFER_CHANGE();
			}
		}
	}
	if (render_state_changed & INDEX_BUFFER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply ib change"));
		if (render_state.index_buffer) {
			switch (render_state.index_buffer_type) {//->Type()) {
			case BUFFER_TYPE_DX8:
			case BUFFER_TYPE_DYNAMIC_DX8:
				DX8CALL(SetIndices(
					static_cast<DX8IndexBufferClass*>(render_state.index_buffer)->Get_DX8_Index_Buffer(),
					render_state.index_base_offset + render_state.vba_offset));
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

	render_state_changed &= ((unsigned)WORLD_IDENTITY | (unsigned)VIEW_IDENTITY);

	SNAPSHOT_SAY(("DX8Wrapper::Apply_Render_State_Changes() - finished"));
}

IDirect3DTexture8* DX8Wrapper::_Create_DX8_Texture
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
	IDirect3DTexture8* texture = NULL;

	// Paletted textures not supported!
	WWASSERT(format != D3DFMT_P8);

	// NOTE: If 'format' is not supported as a texture format, this function will find the closest
	// format that is supported and use that instead.

	// Render target may return NOTAVAILABLE, in
	// which case we return NULL.
	if (rendertarget) {
		unsigned ret = D3DXCreateTexture(
			DX8Wrapper::_Get_D3D_Device8(),
			width,
			height,
			mip_level_count,
			D3DUSAGE_RENDERTARGET,
			WW3DFormat_To_D3DFormat(format),
			pool,
			&texture);

		if (ret == D3DERR_NOTAVAILABLE) {
			Non_Fatal_Log_DX8_ErrorCode(ret, __FILE__, __LINE__);
			return NULL;
		}

		// If ran out of texture ram, try invalidating some textures and mesh cache.
		if (ret == D3DERR_OUTOFVIDEOMEMORY) {
			WWDEBUG_SAY(("Error: Out of memory while creating render target. Trying to release assets..."));
			// Free all textures that haven't been used in the last 5 seconds
			TextureClass::Invalidate_Old_Unused_Textures(5000);

			// Invalidate the mesh cache
			WW3D::_Invalidate_Mesh_Cache();

			ret = D3DXCreateTexture(
				DX8Wrapper::_Get_D3D_Device8(),
				width,
				height,
				mip_level_count,
				D3DUSAGE_RENDERTARGET,
				WW3DFormat_To_D3DFormat(format),
				pool,
				&texture);

			if (SUCCEEDED(ret)) {
				WWDEBUG_SAY(("...Render target creation succesful."));
			}
			else {
				WWDEBUG_SAY(("...Render target creation failed."));
			}
			if (ret == D3DERR_OUTOFVIDEOMEMORY) {
				Non_Fatal_Log_DX8_ErrorCode(ret, __FILE__, __LINE__);
				return NULL;
			}
		}

		DX8_ErrorCode(ret);
		// Just return the texture, no reduction
		// allowed for render targets.
		return texture;
	}

	// We should never run out of video memory when allocating a non-rendertarget texture.
	// However, it seems to happen sometimes when there are a lot of textures in memory and so
	// if it happens we'll release assets and try again (anything is better than crashing).
	unsigned ret = D3DXCreateTexture(
		DX8Wrapper::_Get_D3D_Device8(),
		width,
		height,
		mip_level_count,
		0,
		WW3DFormat_To_D3DFormat(format),
		pool,
		&texture);

	// If ran out of texture ram, try invalidating some textures and mesh cache.
	if (ret == D3DERR_OUTOFVIDEOMEMORY) {
		WWDEBUG_SAY(("Error: Out of memory while creating texture. Trying to release assets..."));
		// Free all textures that haven't been used in the last 5 seconds
		TextureClass::Invalidate_Old_Unused_Textures(5000);

		// Invalidate the mesh cache
		WW3D::_Invalidate_Mesh_Cache();

		ret = D3DXCreateTexture(
			DX8Wrapper::_Get_D3D_Device8(),
			width,
			height,
			mip_level_count,
			0,
			WW3DFormat_To_D3DFormat(format),
			pool,
			&texture);
		if (SUCCEEDED(ret)) {
			WWDEBUG_SAY(("...Texture creation succesful."));
		}
		else {
			StringClass format_name(0, true);
			Get_WW3D_Format_Name(format, format_name);
			WWDEBUG_SAY(("...Texture creation failed. (%d x %d, format: %s, mips: %d", width, height, format_name.str(), mip_level_count));
		}

	}
	DX8_ErrorCode(ret);

	return texture;
}

IDirect3DTexture8* DX8Wrapper::_Create_DX8_Texture
(
	const char* filename,
	MipCountType mip_level_count
)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture8* texture = NULL;

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
	texture->GetLevelDesc(0, &desc);
	if (desc.Format == D3DFMT_P8) {
		texture->Release();
		return MissingTexture::_Get_Missing_Texture();
	}
	return texture;
}

IDirect3DTexture8* DX8Wrapper::_Create_DX8_Texture
(
	IDirect3DSurface8* surface,
	MipCountType mip_level_count
)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture8* texture = NULL;

	D3DSURFACE_DESC surface_desc;
	::ZeroMemory(&surface_desc, sizeof(D3DSURFACE_DESC));
	surface->GetDesc(&surface_desc);

	// This function will create a texture with a different (but similar) format if the surface is
	// not in a supported texture format.
	WW3DFormat format = D3DFormat_To_WW3DFormat(surface_desc.Format);
	texture = _Create_DX8_Texture(surface_desc.Width, surface_desc.Height, format, mip_level_count);

	// Copy the surface to the texture
	IDirect3DSurface8* tex_surface = NULL;
	texture->GetSurfaceLevel(0, &tex_surface);
	DX8_ErrorCode(D3DXLoadSurfaceFromSurface(tex_surface, NULL, NULL, surface, NULL, NULL, D3DX_FILTER_BOX, 0));
	tex_surface->Release();

	// Create mipmaps if needed
	if (mip_level_count != MIP_LEVELS_1)
	{
		DX8_ErrorCode(D3DXFilterTexture(texture, NULL, 0, D3DX_FILTER_BOX));
	}

	return texture;

}

/*!
 * KJM create depth stencil texture
 */
IDirect3DTexture8* DX8Wrapper::_Create_DX8_ZTexture
(
	unsigned int width,
	unsigned int height,
	WW3DZFormat zformat,
	MipCountType mip_level_count,
	D3DPOOL pool
)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture8* texture = NULL;

	D3DFORMAT zfmt = WW3DZFormat_To_D3DFormat(zformat);

	unsigned ret = DX8Wrapper::_Get_D3D_Device8()->CreateTexture
	(
		width,
		height,
		mip_level_count,
		D3DUSAGE_DEPTHSTENCIL,
		zfmt,
		pool,
		&texture
	);

	if (ret == D3DERR_NOTAVAILABLE)
	{
		Non_Fatal_Log_DX8_ErrorCode(ret, __FILE__, __LINE__);
		return NULL;
	}

	// If ran out of texture ram, try invalidating some textures and mesh cache.
	if (ret == D3DERR_OUTOFVIDEOMEMORY)
	{
		WWDEBUG_SAY(("Error: Out of memory while creating render target. Trying to release assets..."));
		// Free all textures that haven't been used in the last 5 seconds
		TextureClass::Invalidate_Old_Unused_Textures(5000);

		// Invalidate the mesh cache
		WW3D::_Invalidate_Mesh_Cache();

		ret = DX8Wrapper::_Get_D3D_Device8()->CreateTexture
		(
			width,
			height,
			mip_level_count,
			D3DUSAGE_DEPTHSTENCIL,
			zfmt,
			pool,
			&texture
		);

		if (SUCCEEDED(ret))
		{
			WWDEBUG_SAY(("...Render target creation succesful."));
		}
		else
		{
			WWDEBUG_SAY(("...Render target creation failed."));
		}
		if (ret == D3DERR_OUTOFVIDEOMEMORY)
		{
			Non_Fatal_Log_DX8_ErrorCode(ret, __FILE__, __LINE__);
			return NULL;
		}
	}

	DX8_ErrorCode(ret);

	texture->AddRef(); // don't release this texture

	// Just return the texture, no reduction
	// allowed for render targets.

	return texture;
}

/*!
 * KJM create cube map texture
 */
IDirect3DCubeTexture8* DX8Wrapper::_Create_DX8_Cube_Texture
(
	unsigned int width,
	unsigned int height,
	WW3DFormat format,
	MipCountType mip_level_count,
	D3DPOOL pool,
	bool rendertarget
)
{
	WWASSERT(width == height);
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DCubeTexture8* texture = NULL;

	// Paletted textures not supported!
	WWASSERT(format != D3DFMT_P8);

	// NOTE: If 'format' is not supported as a texture format, this function will find the closest
	// format that is supported and use that instead.

	// Render target may return NOTAVAILABLE, in
	// which case we return NULL.
	if (rendertarget)
	{
		unsigned ret = D3DXCreateCubeTexture
		(
			DX8Wrapper::_Get_D3D_Device8(),
			width,
			mip_level_count,
			D3DUSAGE_RENDERTARGET,
			WW3DFormat_To_D3DFormat(format),
			pool,
			&texture
		);

		if (ret == D3DERR_NOTAVAILABLE)
		{
			Non_Fatal_Log_DX8_ErrorCode(ret, __FILE__, __LINE__);
			return NULL;
		}

		// If ran out of texture ram, try invalidating some textures and mesh cache.
		if (ret == D3DERR_OUTOFVIDEOMEMORY)
		{
			WWDEBUG_SAY(("Error: Out of memory while creating render target. Trying to release assets..."));
			// Free all textures that haven't been used in the last 5 seconds
			TextureClass::Invalidate_Old_Unused_Textures(5000);

			// Invalidate the mesh cache
			WW3D::_Invalidate_Mesh_Cache();

			ret = D3DXCreateCubeTexture
			(
				DX8Wrapper::_Get_D3D_Device8(),
				width,
				mip_level_count,
				D3DUSAGE_RENDERTARGET,
				WW3DFormat_To_D3DFormat(format),
				pool,
				&texture
			);

			if (SUCCEEDED(ret))
			{
				WWDEBUG_SAY(("...Render target creation succesful."));
			}
			else
			{
				WWDEBUG_SAY(("...Render target creation failed."));
			}
			if (ret == D3DERR_OUTOFVIDEOMEMORY)
			{
				Non_Fatal_Log_DX8_ErrorCode(ret, __FILE__, __LINE__);
				return NULL;
			}
		}

		DX8_ErrorCode(ret);
		// Just return the texture, no reduction
		// allowed for render targets.
		return texture;
	}

	// We should never run out of video memory when allocating a non-rendertarget texture.
	// However, it seems to happen sometimes when there are a lot of textures in memory and so
	// if it happens we'll release assets and try again (anything is better than crashing).
	unsigned ret = D3DXCreateCubeTexture
	(
		DX8Wrapper::_Get_D3D_Device8(),
		width,
		mip_level_count,
		0,
		WW3DFormat_To_D3DFormat(format),
		pool,
		&texture
	);

	// If ran out of texture ram, try invalidating some textures and mesh cache.
	if (ret == D3DERR_OUTOFVIDEOMEMORY)
	{
		WWDEBUG_SAY(("Error: Out of memory while creating texture. Trying to release assets..."));
		// Free all textures that haven't been used in the last 5 seconds
		TextureClass::Invalidate_Old_Unused_Textures(5000);

		// Invalidate the mesh cache
		WW3D::_Invalidate_Mesh_Cache();

		ret = D3DXCreateCubeTexture
		(
			DX8Wrapper::_Get_D3D_Device8(),
			width,
			mip_level_count,
			0,
			WW3DFormat_To_D3DFormat(format),
			pool,
			&texture
		);
		if (SUCCEEDED(ret))
		{
			WWDEBUG_SAY(("...Texture creation succesful."));
		}
		else
		{
			StringClass format_name(0, true);
			Get_WW3D_Format_Name(format, format_name);
			WWDEBUG_SAY(("...Texture creation failed. (%d x %d, format: %s, mips: %d", width, height, format_name.str(), mip_level_count));
		}

	}
	DX8_ErrorCode(ret);

	return texture;
}

/*!
 * KJM create volume texture
 */
IDirect3DVolumeTexture8* DX8Wrapper::_Create_DX8_Volume_Texture
(
	unsigned int width,
	unsigned int height,
	unsigned int depth,
	WW3DFormat format,
	MipCountType mip_level_count,
	D3DPOOL pool
)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DVolumeTexture8* texture = NULL;

	// Paletted textures not supported!
	WWASSERT(format != D3DFMT_P8);

	// NOTE: If 'format' is not supported as a texture format, this function will find the closest
	// format that is supported and use that instead.


	// We should never run out of video memory when allocating a non-rendertarget texture.
	// However, it seems to happen sometimes when there are a lot of textures in memory and so
	// if it happens we'll release assets and try again (anything is better than crashing).
	unsigned ret = D3DXCreateVolumeTexture
	(
		DX8Wrapper::_Get_D3D_Device8(),
		width,
		height,
		depth,
		mip_level_count,
		0,
		WW3DFormat_To_D3DFormat(format),
		pool,
		&texture
	);

	// If ran out of texture ram, try invalidating some textures and mesh cache.
	if (ret == D3DERR_OUTOFVIDEOMEMORY)
	{
		WWDEBUG_SAY(("Error: Out of memory while creating texture. Trying to release assets..."));
		// Free all textures that haven't been used in the last 5 seconds
		TextureClass::Invalidate_Old_Unused_Textures(5000);

		// Invalidate the mesh cache
		WW3D::_Invalidate_Mesh_Cache();

		ret = D3DXCreateVolumeTexture
		(
			DX8Wrapper::_Get_D3D_Device8(),
			width,
			height,
			depth,
			mip_level_count,
			0,
			WW3DFormat_To_D3DFormat(format),
			pool,
			&texture
		);
		if (SUCCEEDED(ret))
		{
			WWDEBUG_SAY(("...Texture creation succesful."));
		}
		else
		{
			StringClass format_name(0, true);
			Get_WW3D_Format_Name(format, format_name);
			WWDEBUG_SAY(("...Texture creation failed. (%d x %d, format: %s, mips: %d", width, height, format_name.str(), mip_level_count));
		}

	}
	DX8_ErrorCode(ret);

	return texture;
}


IDirect3DSurface8* DX8Wrapper::_Create_DX8_Surface(unsigned int width, unsigned int height, WW3DFormat format)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();

	IDirect3DSurface8* surface = NULL;

	// Paletted surfaces not supported!
	WWASSERT(format != D3DFMT_P8);

	DX8CALL(CreateImageSurface(width, height, WW3DFormat_To_D3DFormat(format), &surface));

	return surface;
}

IDirect3DSurface8* DX8Wrapper::_Create_DX8_Surface(const char* filename_)
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
	IDirect3DSurface8* surface = NULL;

	{

		file_auto_ptr myfile(_TheFileFactory, filename_);
		// If file not found, create a surface with missing texture in it

		if (!myfile->Is_Available()) {
			// If file not found, try the dds format
			// else create a surface with missing texture in it
			char compressed_name[200];
			strncpy(compressed_name, filename_, ARRAY_SIZE(compressed_name));
			compressed_name[ARRAY_SIZE(compressed_name) - 1] = '\0';
			char* ext = strstr(compressed_name, ".");
			if (ext && (strlen(ext) == 4) &&
				((ext[1] == 't') || (ext[1] == 'T')) &&
				((ext[2] == 'g') || (ext[2] == 'G')) &&
				((ext[3] == 'a') || (ext[3] == 'A'))) {
				ext[1] = 'd';
				ext[2] = 'd';
				ext[3] = 's';
			}
			file_auto_ptr myfile2(_TheFileFactory, compressed_name);
			if (!myfile2->Is_Available())
				return MissingTexture::_Create_Missing_Surface();
		}
	}

	StringClass filename_string(filename_, true);
	surface = TextureLoader::Load_Surface_Immediate(
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
void DX8Wrapper::_Update_Texture(TextureClass* system, TextureClass* video)
{
	WWASSERT(system);
	WWASSERT(video);
	WWASSERT(system->Get_Pool() == TextureClass::POOL_SYSTEMMEM);
	WWASSERT(video->Get_Pool() == TextureClass::POOL_DEFAULT);
	DX8CALL(UpdateTexture(system->Peek_D3D_Base_Texture(), video->Peek_D3D_Base_Texture()));
}

void DX8Wrapper::Compute_Caps(WW3DFormat display_format)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	delete CurrentCaps;
	CurrentCaps = new DX8Caps(_Get_D3D8(), D3DDevice, display_format, Get_Current_Adapter_Identifier());
}


void DX8Wrapper::Set_Light(unsigned index, const D3DLIGHT8* light)
{
	if (light) {
		render_state.Lights[index] = *light;
		render_state.LightEnable[index] = true;
	}
	else {
		render_state.LightEnable[index] = false;
	}
	render_state_changed |= (LIGHT0_CHANGED << index);
}

void DX8Wrapper::Set_Light(unsigned index, const LightClass& light)
{
	D3DLIGHT8 dlight;
	Vector3 temp;
	memset(&dlight, 0, sizeof(D3DLIGHT8));

	switch (light.Get_Type())
	{
	case LightClass::POINT:
	{
		dlight.Type = D3DLIGHT_POINT;
	}
	break;
	case LightClass::DIRECTIONAL:
	{
		dlight.Type = D3DLIGHT_DIRECTIONAL;
	}
	break;
	case LightClass::SPOT:
	{
		dlight.Type = D3DLIGHT_SPOT;
	}
	break;
	}

	light.Get_Diffuse(&temp);
	temp *= light.Get_Intensity();
	dlight.Diffuse.r = temp.X;
	dlight.Diffuse.g = temp.Y;
	dlight.Diffuse.b = temp.Z;
	dlight.Diffuse.a = 1.0f;

	light.Get_Specular(&temp);
	temp *= light.Get_Intensity();
	dlight.Specular.r = temp.X;
	dlight.Specular.g = temp.Y;
	dlight.Specular.b = temp.Z;
	dlight.Specular.a = 1.0f;

	light.Get_Ambient(&temp);
	temp *= light.Get_Intensity();
	dlight.Ambient.r = temp.X;
	dlight.Ambient.g = temp.Y;
	dlight.Ambient.b = temp.Z;
	dlight.Ambient.a = 1.0f;

	temp = light.Get_Position();
	dlight.Position = *(D3DVECTOR*)&temp;

	light.Get_Spot_Direction(temp);
	dlight.Direction = *(D3DVECTOR*)&temp;

	dlight.Range = light.Get_Attenuation_Range();
	dlight.Falloff = light.Get_Spot_Exponent();
	dlight.Theta = light.Get_Spot_Angle();
	dlight.Phi = light.Get_Spot_Angle();

	// Inverse linear light 1/(1+D)
	double a, b;
	light.Get_Far_Attenuation_Range(a, b);
	dlight.Attenuation0 = 1.0f;
	if (fabs(a - b) < 1e-5)
		// if the attenuation range is too small assume uniform with cutoff
		dlight.Attenuation1 = 0.0f;
	else
		// this will cause the light to drop to half intensity at the first far attenuation
		dlight.Attenuation1 = (float)1.0 / a;
	dlight.Attenuation2 = 0.0f;

	Set_Light(index, &dlight);
}

