/**
 * @file imgui_impl_dx8.cpp
 * @brief DirectX8 renderer backend for Dear ImGui, reverse-engineered from the official DX9 backend
 */

// dear imgui: Renderer Backend for DirectX8
// This needs to be used along with a Platform Backend (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE8' as ImTextureID. Read the FAQ about ImTextureID!
//  [X] Renderer: Large meshes support (64k+ vertices) with 16-bit indices.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 - 2025 Omar Cornut (Based on dx9 ImGui Backend)
 * Copyright (c) 2021 KoMaR1911
 * Copyright (c) 2025 Meigyoku-Thmn and others
 * Copyright (c) 2026 TheSuperHackers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <imgui.h>
#include "imgui_impl_dx8.h"

// DirectX
#include "d3d8.h"


// DirectX data
struct ImGui_ImplDX8_Data
{
    LPDIRECT3DDEVICE8           pd3dDevice;
    LPDIRECT3DVERTEXBUFFER8     pVB;
    LPDIRECT3DINDEXBUFFER8      pIB;
    LPDIRECT3DVERTEXBUFFER8     maskVB;
    LPDIRECT3DINDEXBUFFER8      maskIB;
    LPDIRECT3DTEXTURE8          FontTexture;
    int                         VertexBufferSize;
    int                         IndexBufferSize;
    IDirect3DSurface8*          DepthBuffer;
    IDirect3DSurface8*          realDepthStencilBuffer;

    ImGui_ImplDX8_Data() {
        memset((void*)this, 0, sizeof(*this));
        VertexBufferSize = 5000;
        IndexBufferSize = 10000;
    }
};

struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IMGUI_COL_TO_DX8_ARGB(_COL)     (_COL)
#else
#define IMGUI_COL_TO_DX8_ARGB(_COL)     (((_COL) & 0xFF00FF00) | (((_COL) & 0xFF0000) >> 16) | (((_COL) & 0xFF) << 16))
#endif

ImGui_ImplDX8_Data* ImGui_ImplDX8_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplDX8_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

// Functions
void ImGui_ImplDX8_SetupRenderState(ImDrawData* draw_data)
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();

    IDirect3DSurface8* pSurface{};
    D3DSURFACE_DESC d3dSize{};
    if (SUCCEEDED(bd->pd3dDevice->GetRenderTarget(&pSurface)) && SUCCEEDED(pSurface->GetDesc(&d3dSize)))
    {
        // Setup viewport
        D3DVIEWPORT8 vp{};
        vp.X = vp.Y = 0;
        vp.Width = d3dSize.Width;
        vp.Height = d3dSize.Height;
        vp.MinZ = 0.0f;
        vp.MaxZ = 1.0f;
        bd->pd3dDevice->SetViewport(&vp);
    }
    if (pSurface)
    {
        pSurface->Release();
        pSurface = nullptr;
    }

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient), bilinear sampling.
    // Check result to prevent crash when restoring depth buffer with SetRenderTarget
    if (bd->pd3dDevice->GetDepthStencilSurface(&bd->realDepthStencilBuffer) != D3D_OK)
    {
        bd->realDepthStencilBuffer = nullptr;
    }
    bd->pd3dDevice->SetRenderTarget(nullptr, bd->DepthBuffer);
    bd->pd3dDevice->SetPixelShader(0);
    bd->pd3dDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX);
    bd->pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    bd->pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    bd->pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    bd->pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    bd->pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    bd->pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    bd->pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    // This below is needed for the entire Touhou series from Touhou 6!
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

    // Setup orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    // Being agnostic of whether <d3dx8.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        float L = draw_data->DisplayPos.x + 0.5f;
        float R = draw_data->DisplayPos.x + d3dSize.Width + 0.5f;
        float T = draw_data->DisplayPos.y + 0.5f;
        float B = draw_data->DisplayPos.y + d3dSize.Height + 0.5f;
        D3DMATRIX mat_identity = {{{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
        }}};
        D3DMATRIX mat_projection = {{{
            2.0f / (R - L), 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / (T - B), 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            (L + R) / (L - R), (T + B) / (B - T), 0.5f, 1.0f,
        }}};
        bd->pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        bd->pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        bd->pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }
}

void build_mask_vbuffer(const RECT* rect)
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    CUSTOMVERTEX* vtx_dst{};
    // Check Lock result to prevent null pointer dereference
    if (bd->maskVB->Lock(0, (UINT)(6 * sizeof(CUSTOMVERTEX)), (BYTE**)&vtx_dst, 0) != D3D_OK)
    {
        return;
    }
    vtx_dst[0].pos[0] = (float)rect->left;
    vtx_dst[0].pos[1] = (float)rect->bottom;
    vtx_dst[0].pos[2] = 0;
    vtx_dst[1].pos[0] = (float)rect->left;
    vtx_dst[1].pos[1] = (float)rect->top;
    vtx_dst[1].pos[2] = 0;
    vtx_dst[2].pos[0] = (float)rect->right;
    vtx_dst[2].pos[1] = (float)rect->top;
    vtx_dst[2].pos[2] = 0;
    vtx_dst[3].pos[0] = (float)rect->left;
    vtx_dst[3].pos[1] = (float)rect->bottom;
    vtx_dst[3].pos[2] = 0;
    vtx_dst[4].pos[0] = (float)rect->right;
    vtx_dst[4].pos[1] = (float)rect->top;
    vtx_dst[4].pos[2] = 0;
    vtx_dst[5].pos[0] = (float)rect->right;
    vtx_dst[5].pos[1] = (float)rect->bottom;
    vtx_dst[5].pos[2] = 0;
    vtx_dst[0].col = 0xFFFFFFFF;
    vtx_dst[1].col = 0xFFFFFFFF;
    vtx_dst[2].col = 0xFFFFFFFF;
    vtx_dst[3].col = 0xFFFFFFFF;
    vtx_dst[4].col = 0xFFFFFFFF;
    vtx_dst[5].col = 0xFFFFFFFF;
    vtx_dst[0].uv[0] = 0;
    vtx_dst[0].uv[1] = 0;
    vtx_dst[1].uv[0] = 0;
    vtx_dst[1].uv[1] = 0;
    vtx_dst[2].uv[0] = 0;
    vtx_dst[2].uv[1] = 0;
    vtx_dst[3].uv[0] = 0;
    vtx_dst[3].uv[1] = 0;
    vtx_dst[4].uv[0] = 0;
    vtx_dst[4].uv[1] = 0;
    vtx_dst[5].uv[0] = 0;
    vtx_dst[5].uv[1] = 0;
    bd->maskVB->Unlock();
}

// Render function.
void ImGui_ImplDX8_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    // Create and grow buffers if needed
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (!bd->pVB || bd->VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (bd->pVB)
        {
            bd->pVB->Release();
            bd->pVB = nullptr;
        }
        bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;
        if (bd->pd3dDevice->CreateVertexBuffer(bd->VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &bd->pVB) < 0)
            return;
    }
    if (!bd->pIB || bd->IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (bd->pIB)
        {
            bd->pIB->Release();
            bd->pIB = nullptr;
        }
        bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;
        if (bd->pd3dDevice->CreateIndexBuffer(bd->IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &bd->pIB) < 0)
            return;
    }

    if (!bd->maskVB && !bd->maskIB)
    {
        if (bd->pd3dDevice->CreateVertexBuffer(6 * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &bd->maskVB) < 0)
            return;
        if (bd->pd3dDevice->CreateIndexBuffer(6, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &bd->maskIB) < 0)
            return;
        ImDrawIdx* idx_dst{};
        // Check Lock result to prevent null pointer dereference
        if (bd->maskIB->Lock(0, 6 * sizeof(ImDrawIdx), (BYTE**)&idx_dst, D3DLOCK_DISCARD) != D3D_OK)
        {
            return;
        }
        idx_dst[0] = 0;
        idx_dst[1] = 1;
        idx_dst[2] = 2;
        idx_dst[3] = 0;
        idx_dst[4] = 2;
        idx_dst[5] = 3;
        bd->maskIB->Unlock();
    }

    // Backup the DX8 state
    DWORD d3d8_state_block;
    if (bd->pd3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d8_state_block) < 0)
        return;
    if (bd->pd3dDevice->CaptureStateBlock(d3d8_state_block) < 0)
    {
        bd->pd3dDevice->DeleteStateBlock(d3d8_state_block);
        return;
    }

    // Backup the DX8 transform
    D3DMATRIX last_world, last_view, last_projection;
    bd->pd3dDevice->GetTransform(D3DTS_WORLD, &last_world);
    bd->pd3dDevice->GetTransform(D3DTS_VIEW, &last_view);
    bd->pd3dDevice->GetTransform(D3DTS_PROJECTION, &last_projection);

    // Allocate buffers
    CUSTOMVERTEX* vtx_dst{};
    ImDrawIdx* idx_dst{};
    if (bd->pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (BYTE**)&vtx_dst, D3DLOCK_DISCARD) < 0)
    {
        bd->pd3dDevice->DeleteStateBlock(d3d8_state_block);
        return;
    }
    if (bd->pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (BYTE**)&idx_dst, D3DLOCK_DISCARD) < 0)
    {
        bd->pVB->Unlock();
        bd->pd3dDevice->DeleteStateBlock(d3d8_state_block);
        return;
    }

    // Copy and convert all vertices into a single contiguous buffer, convert colors to DX8 default format.
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
        for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
        {
            vtx_dst->pos[0] = vtx_src->pos.x;
            vtx_dst->pos[1] = vtx_src->pos.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = IMGUI_COL_TO_DX8_ARGB(vtx_src->col);
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    bd->pVB->Unlock();
    bd->pIB->Unlock();
    bd->pd3dDevice->SetStreamSource(0, bd->pVB, sizeof(CUSTOMVERTEX));
    bd->pd3dDevice->SetIndices(bd->pIB, 0);
    bd->pd3dDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX);

    // Setup desired DX state
    ImGui_ImplDX8_SetupRenderState(draw_data);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplDX8_SetupRenderState(draw_data);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project clipping rectangles into framebuffer space
                ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
                ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply clipping rectangle, Bind texture, Draw
                const RECT r = {(LONG)clip_min.x, (LONG)clip_min.y, (LONG)clip_max.x, (LONG)clip_max.y};
                const LPDIRECT3DTEXTURE8 texture = (LPDIRECT3DTEXTURE8)pcmd->GetTexID();
                bd->pd3dDevice->SetTexture(0, texture);
                build_mask_vbuffer(&r);
                bd->pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
                bd->pd3dDevice->SetRenderState(D3DRS_ZENABLE, true);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, true);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFF);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
                bd->pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILREF, 0xFF);
                bd->pd3dDevice->Clear(0, nullptr, D3DCLEAR_STENCIL, 0, 1.0f, 0);
                bd->pd3dDevice->SetStreamSource(0, bd->maskVB, sizeof(CUSTOMVERTEX));
                bd->pd3dDevice->SetIndices(bd->maskIB, 0);
                bd->pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
                bd->pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
                bd->pd3dDevice->SetStreamSource(0, bd->pVB, sizeof(CUSTOMVERTEX));
                bd->pd3dDevice->SetIndices(bd->pIB, global_vtx_offset);
                bd->pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
                bd->pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, true);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
                bd->pd3dDevice->SetRenderState(D3DRS_STENCILREF, 0xFF);
                bd->pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (UINT)cmd_list->VtxBuffer.Size, pcmd->IdxOffset + global_idx_offset, pcmd->ElemCount / 3);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore the DX8 transform
    bd->pd3dDevice->SetTransform(D3DTS_WORLD, &last_world);
    bd->pd3dDevice->SetTransform(D3DTS_VIEW, &last_view);
    bd->pd3dDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

    // Restore the DX8 state
    bd->pd3dDevice->SetRenderTarget(nullptr, bd->realDepthStencilBuffer);
    if (bd->realDepthStencilBuffer)
    {
        bd->realDepthStencilBuffer->Release();
        bd->realDepthStencilBuffer = nullptr;
    }
    bd->pd3dDevice->ApplyStateBlock(d3d8_state_block);
    bd->pd3dDevice->DeleteStateBlock(d3d8_state_block);
}

bool ImGui_ImplDX8_Init(IDirect3DDevice8* device)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    ImGui_ImplDX8_Data* bd = IM_NEW(ImGui_ImplDX8_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_dx8";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

    bd->pd3dDevice = device;
    bd->pd3dDevice->AddRef();

    return true;
}

void ImGui_ImplDX8_Shutdown()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplDX8_InvalidateDeviceObjects();
    if (bd->pd3dDevice)
        bd->pd3dDevice->Release();
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    IM_DELETE(bd);
}

bool ImGui_ImplDX8_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    unsigned char* pixels;
    int width, height, bytes_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

    // Convert RGBA32 to BGRA32
#ifndef IMGUI_USE_BGRA_PACKED_COLOR
    if (io.Fonts->TexPixelsUseColors)
    {
        ImU32* dst_start = (ImU32*)ImGui::MemAlloc((size_t)width * height * bytes_per_pixel);
        for (ImU32* src = (ImU32*)pixels, *dst = dst_start, *dst_end = dst_start + (size_t)width * height; dst < dst_end; src++, dst++)
            *dst = IMGUI_COL_TO_DX8_ARGB(*src);
        pixels = (unsigned char*)dst_start;
    }
#endif

    // Upload texture to graphics system
    bd->FontTexture = nullptr;
    if (bd->pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &bd->FontTexture) < 0)
        return false;
    D3DLOCKED_RECT tex_locked_rect;
    if (bd->FontTexture->LockRect(0, &tex_locked_rect, nullptr, 0) != D3D_OK)
        return false;
    for (int y = 0; y < height; y++)
        memcpy((unsigned char*)tex_locked_rect.pBits + (size_t)tex_locked_rect.Pitch * y, pixels + (size_t)width * bytes_per_pixel * y, (size_t)width * bytes_per_pixel);
    bd->FontTexture->UnlockRect(0);

    // Store our identifier
    io.Fonts->SetTexID((ImTextureID)bd->FontTexture);

#ifndef IMGUI_USE_BGRA_PACKED_COLOR
    if (io.Fonts->TexPixelsUseColors)
        ImGui::MemFree(pixels);
#endif

    return true;
}

bool ImGui_ImplD3D8_CreateDepthStencilBuffer()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (bd->pd3dDevice == nullptr)
    {
        return false;
    }
    if (bd->DepthBuffer == nullptr)
    {
        IDirect3DSurface8* realDepth;
        D3DSURFACE_DESC sfcDesc;

        if (bd->pd3dDevice->GetDepthStencilSurface(&realDepth) != D3D_OK || ! realDepth)
        {
            return false;
        }
        if (realDepth->GetDesc(&sfcDesc) != 0)
        {
            return false;
        }
        realDepth->Release();
        realDepth = nullptr;
        if (bd->pd3dDevice->CreateDepthStencilSurface(sfcDesc.Width, sfcDesc.Height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, &bd->DepthBuffer) != 0)
        {
            return false;
        }
    }

    return true;
}

bool ImGui_ImplDX8_CreateDeviceObjects()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (!bd || !bd->pd3dDevice)
        return false;
    if (!ImGui_ImplDX8_CreateFontsTexture())
        return false;
    if (!ImGui_ImplD3D8_CreateDepthStencilBuffer())
        return false;
    return true;
}

void ImGui_ImplDX8_InvalidateDeviceObjects()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (!bd || !bd->pd3dDevice)
        return;
    if (bd->pVB)
    {
        bd->pVB->Release();
        bd->pVB = nullptr;
    }
    if (bd->pIB)
    {
        bd->pIB->Release();
        bd->pIB = nullptr;
    }
    if (bd->maskVB)
    {
        bd->maskVB->Release();
        bd->maskVB = nullptr;
    }
    if (bd->maskIB)
    {
        bd->maskIB->Release();
        bd->maskIB = nullptr;
    }
    if (bd->DepthBuffer)
    {
        bd->DepthBuffer->Release();
        bd->DepthBuffer = nullptr;
    }
    if (bd->FontTexture)
    {
        bd->FontTexture->Release();
        bd->FontTexture = nullptr;
        ImGui::GetIO().Fonts->SetTexID(0);
    } // We copied bd->pFontTextureView to io.Fonts->TexID so let's clear that as well.
}

void ImGui_ImplDX8_NewFrame()
{
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplDX8_Init()?");

    if (!bd->FontTexture || !bd->DepthBuffer)
        ImGui_ImplDX8_CreateDeviceObjects();
}
