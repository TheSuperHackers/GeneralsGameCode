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

/**
 * D3DXWrapper.h - Conditional D3DX8 include
 * 
 * Include this file instead of <d3dx8.h> directly.
 * When NO_D3DX is defined, uses compatibility layer.
 * Otherwise, uses standard D3DX8 headers.
 * 
 * NOTE: This header is safe to force-include globally.
 * For C files, it does nothing when NO_D3DX is defined.
 */

#pragma once

// Only provide D3DX replacements for C++ files
// C files that don't use D3DX will be unaffected
#ifdef __cplusplus

#ifdef NO_D3DX
    // Use compatibility layer - no DLL dependency
    #include "D3DXCompat.h"
#else
    // Use standard D3DX8 headers - requires d3dx8.dll
    #include <d3dx8.h>
#endif

#endif // __cplusplus
