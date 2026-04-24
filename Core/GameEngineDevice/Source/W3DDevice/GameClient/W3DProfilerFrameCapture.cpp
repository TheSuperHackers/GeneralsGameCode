/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#ifdef PROFILER_ENABLED

#include "../../../Include/W3DDevice/GameClient/W3DProfilerFrameCapture.h"

#include "WW3D2/dx8wrapper.h"
#include "WW3D2/surfaceclass.h"
#include "WW3D2/texture.h"
#include "WW3D2/ww3d.h"
#include "WW3D2/ww3dformat.h"
#include "WWMath/wwmath.h"
#include <cstring>
#include <d3dx8core.h>

W3DProfilerFrameCapture::W3DProfilerFrameCapture()
{
}

W3DProfilerFrameCapture::~W3DProfilerFrameCapture()
{
	if (m_swizzleShader)
	{
		DX8Wrapper::_Get_D3D_Device8()->DeletePixelShader(m_swizzleShader);
		m_swizzleShader = 0;
	}
}

bool W3DProfilerFrameCapture::ShouldReuseLastCapture(UnsignedInt currentTimeMs) const
{
	return PROFILER_FRAME_IMAGE_INTERVAL_MS > 0
		&& currentTimeMs - m_lastCaptureTimeMs < PROFILER_FRAME_IMAGE_INTERVAL_MS
		&& !m_lastCapturePixels.empty();
}

void W3DProfilerFrameCapture::Capture(UnsignedInt displayWidth, UnsignedInt displayHeight)
{
	if (!PROFILER_IS_CONNECTED)
		return;

	// the profiler expects an image every render frame. resend the last capture if we're inside the capture interval.
	const UnsignedInt currentTimeMs = WW3D::Get_Logic_Time_Milliseconds();
	if (ShouldReuseLastCapture(currentTimeMs))
	{
		PROFILER_FRAME_IMAGE(m_lastCapturePixels.data(), PROFILER_FRAME_IMAGE_SIZE, m_lastCaptureHeight, 0, false);
		return;
	}

	// compile swizzle shader convert BGRA to RGBA
	// TheSuperHackers @todo In DX9 with ps2.0 this shader will be much simpler
	if (!m_swizzleShader)
	{
		ID3DXBuffer *compiledShader = nullptr;
		const char *shader =
			"ps.1.4\n"
			"texld r0, t0\n"
			"mov r1.a, r0.r\n"
			"mov r2.a, r0.g\n"
			"mov r3.a, r0.b\n"
			"mul r0.rgb, r3.a, c0\n"
			"mad r0.rgb, r2.a, c1, r0\n"
			"mad r0.rgb, r1.a, c2, r0\n";

		HRESULT hr = D3DXAssembleShader(shader, strlen(shader), 0, nullptr, &compiledShader, nullptr);
		if (FAILED(hr))
			return;

		hr = DX8Wrapper::_Get_D3D_Device8()->CreatePixelShader((DWORD *)compiledShader->GetBufferPointer(), &m_swizzleShader);
		compiledShader->Release();

		if (FAILED(hr))
			return;
	}

	// allocate render target
	TextureClass *renderTarget = DX8Wrapper::Create_Render_Target(PROFILER_FRAME_IMAGE_SIZE, PROFILER_FRAME_IMAGE_SIZE, WW3D_FORMAT_A8R8G8B8);
	if (!renderTarget)
		return;

	// allocate surface class
	const Real aspectRatio = (Real)displayHeight / (Real)displayWidth;
	unsigned int profilerImageHeight = min((int)WWMath::Round(PROFILER_FRAME_IMAGE_SIZE * aspectRatio), PROFILER_FRAME_IMAGE_SIZE);
	SurfaceClass *surfaceClass = NEW_REF(SurfaceClass, (PROFILER_FRAME_IMAGE_SIZE, profilerImageHeight, WW3D_FORMAT_A8R8G8B8));
	if (!surfaceClass)
	{
		REF_PTR_RELEASE(renderTarget);
		return;
	}

	// get the backbuffer
	SurfaceClass *backBuffer = DX8Wrapper::_Get_DX8_Back_Buffer();
	if (!backBuffer)
	{
		REF_PTR_RELEASE(surfaceClass);
		REF_PTR_RELEASE(renderTarget);
		return;
	}

	IDirect3DSurface8 *backBufferSurface = backBuffer->Peek_D3D_Surface();
	D3DSURFACE_DESC backBufferSurfaceDesc;
	HRESULT hr = backBufferSurface->GetDesc(&backBufferSurfaceDesc);
	if (FAILED(hr))
	{
		REF_PTR_RELEASE(backBuffer);
		REF_PTR_RELEASE(surfaceClass);
		REF_PTR_RELEASE(renderTarget);
		return;
	}

	// allocate intermediate texture
	IDirect3DTexture8 *intermediateTexture = nullptr;
	hr = DX8Wrapper::_Get_D3D_Device8()->CreateTexture(
		backBufferSurfaceDesc.Width,
		backBufferSurfaceDesc.Height,
		1,
		D3DUSAGE_RENDERTARGET,
		backBufferSurfaceDesc.Format,
		D3DPOOL_DEFAULT,
		&intermediateTexture);
	if (FAILED(hr))
	{
		REF_PTR_RELEASE(backBuffer);
		REF_PTR_RELEASE(surfaceClass);
		REF_PTR_RELEASE(renderTarget);
		return;
	}

	// draw backbuffer to intermediate texture
	IDirect3DSurface8 *intermediateTextureSurface;
	hr = intermediateTexture->GetSurfaceLevel(0, &intermediateTextureSurface);
	if (FAILED(hr))
	{
		REF_PTR_RELEASE(backBuffer);
		REF_PTR_RELEASE(surfaceClass);
		REF_PTR_RELEASE(renderTarget);
		intermediateTexture->Release();
		return;
	}
	DX8Wrapper::_Copy_DX8_Rects(backBufferSurface, nullptr, 0, intermediateTextureSurface, nullptr);
	intermediateTextureSurface->Release();
	intermediateTextureSurface = nullptr;

	// release the backbuffer
	backBufferSurface = nullptr;
	REF_PTR_RELEASE(backBuffer);

	// set render target to a small surface
	IDirect3DSurface8 *smallRenderTargetSurface = renderTarget->Get_D3D_Surface_Level();
	WWASSERT(smallRenderTargetSurface != nullptr);
	DX8Wrapper::Set_Render_Target(smallRenderTargetSurface, false);

	// set viewport
	IDirect3DDevice8 *device = DX8Wrapper::_Get_D3D_Device8();
	D3DVIEWPORT8 restoreViewport;
	device->GetViewport(&restoreViewport);

	SurfaceClass::SurfaceDescription smallRenderDesc;
	surfaceClass->Get_Description(smallRenderDesc);

	D3DVIEWPORT8 viewport;
	viewport.X = 0;
	viewport.Y = 0;
	viewport.Width = PROFILER_FRAME_IMAGE_SIZE;
	viewport.Height = smallRenderDesc.Height;
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	DX8Wrapper::Set_Viewport(&viewport);

	// bind swizzle shader
	DX8Wrapper::Set_Pixel_Shader(m_swizzleShader);
	static const Real kMaskR[4] = {1.0f, 0.0f, 0.0f, 0.0f};
	static const Real kMaskG[4] = {0.0f, 1.0f, 0.0f, 0.0f};
	static const Real kMaskB[4] = {0.0f, 0.0f, 1.0f, 0.0f};
	device->SetPixelShaderConstant(0, kMaskR, 1);
	device->SetPixelShaderConstant(1, kMaskG, 1);
	device->SetPixelShaderConstant(2, kMaskB, 1);

	// draw texture scaled-down onto a small surface
	struct QuadVertex
	{
		Real x, y, z, rhw;
		Real u, v;
	} vtx[4];
	const Real left = -0.5f;
	const Real top = -0.5f;
	const Real right = (Real)PROFILER_FRAME_IMAGE_SIZE - 0.5f;
	const Real bottom = (Real)smallRenderDesc.Height - 0.5f;
	vtx[0] = {right, bottom, 0.0f, 1.0f, 1.0f, 1.0f};
	vtx[1] = {right, top,    0.0f, 1.0f, 1.0f, 0.0f};
	vtx[2] = {left,  bottom, 0.0f, 1.0f, 0.0f, 1.0f};
	vtx[3] = {left,  top,    0.0f, 1.0f, 0.0f, 0.0f};
	DX8Wrapper::Set_DX8_Texture(0, intermediateTexture);
	DX8Wrapper::Set_Vertex_Shader(D3DFVF_XYZRHW | D3DFVF_TEX1);
	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vtx, sizeof(QuadVertex));
	DX8Wrapper::Set_Pixel_Shader(0);
	DX8Wrapper::Set_DX8_Texture(0, nullptr);
	DX8Wrapper::Set_Viewport(&restoreViewport);
	DX8Wrapper::Set_Render_Target(static_cast<IDirect3DSurface8 *>(nullptr));

	// copy the small surface pixels from GPU to CPU
	RECT srcRect = { 0, 0, PROFILER_FRAME_IMAGE_SIZE, smallRenderDesc.Height };
	POINT dstPoint = { 0, 0 };
	DX8Wrapper::_Copy_DX8_Rects(
		smallRenderTargetSurface,
		&srcRect,
		1,
		surfaceClass->Peek_D3D_Surface(),
		&dstPoint);
	smallRenderTargetSurface->Release();

	// send pixels to the profiler backend
	int pitch = 0;
	void *bits = surfaceClass->Lock(&pitch);
	if (bits)
	{
		const size_t rowBytes = (size_t)PROFILER_FRAME_IMAGE_SIZE * 4;
		m_lastCaptureHeight = smallRenderDesc.Height;
		m_lastCapturePixels.resize(rowBytes * m_lastCaptureHeight);

		const UnsignedByte *source = static_cast<const UnsignedByte *>(bits);
		UnsignedByte *destination = m_lastCapturePixels.data();
		for (UnsignedInt row = 0; row < m_lastCaptureHeight; ++row)
		{
			std::memcpy(destination + row * rowBytes, source + row * pitch, rowBytes);
		}

		PROFILER_FRAME_IMAGE(m_lastCapturePixels.data(), PROFILER_FRAME_IMAGE_SIZE, m_lastCaptureHeight, 0, false);
		surfaceClass->Unlock();
		m_lastCaptureTimeMs = currentTimeMs;
	}

	// cleanup
	intermediateTexture->Release();
	intermediateTexture = nullptr;
	REF_PTR_RELEASE(surfaceClass);
	REF_PTR_RELEASE(renderTarget);
}

#endif // PROFILER_ENABLED
