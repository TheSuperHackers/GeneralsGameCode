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

