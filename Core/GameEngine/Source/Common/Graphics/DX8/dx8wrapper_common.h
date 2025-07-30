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
