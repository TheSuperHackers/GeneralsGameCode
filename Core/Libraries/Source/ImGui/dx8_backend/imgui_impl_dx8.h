/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 - 2025 Omar Cornut (Based on dx9 ImGui Backend)
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

/**
 * @file imgui_impl_dx8.h
 * @brief DirectX8 renderer backend for Dear ImGui, reverse-engineered from the official DX9 backend
 */

#pragma once
#include <imgui.h>      // IMGUI_IMPL_API

struct IDirect3DDevice8;

IMGUI_IMPL_API bool     ImGui_ImplDX8_Init(IDirect3DDevice8* device);
IMGUI_IMPL_API void     ImGui_ImplDX8_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDX8_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDX8_RenderDrawData(ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing Dear ImGui state.
IMGUI_IMPL_API bool     ImGui_ImplDX8_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplDX8_InvalidateDeviceObjects();
