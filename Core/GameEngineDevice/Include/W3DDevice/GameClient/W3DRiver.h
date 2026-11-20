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

#pragma once

#include "Lib/BaseType.h"

class PolygonTrigger;
class TextureClass;
struct IDirect3DDevice8;

// TheSuperHackers @refactor Keep river rendering and its resources together; flat water borrows the base texture.
class W3DRiver
{
public:
	W3DRiver();
	~W3DRiver();

	void init(); // Acquire the base water texture and river edge texture.
	void updateMapOverrides();
	void ReleaseResources(IDirect3DDevice8 *device); // Call before reset or destruction; safe to repeat.
	void ReAcquireResources(IDirect3DDevice8 *device);
	/// Borrowed texture; owned here and also used by flat water.
	TextureClass *getTexture() const { return m_texture; }
	Bool hasPixelShader() const { return m_pixelShader != 0; }

	struct RenderSettings
	{
		TextureClass *noise;
		TextureClass *sparkles;
		Real phase;
		UnsignedInt waterDiffuse;
	};

	/// Returns whether the river path replaces the optional water grid.
	Bool draw(IDirect3DDevice8 *device, PolygonTrigger *polygon, const RenderSettings &settings) const;

private:
	W3DRiver(const W3DRiver &);
	W3DRiver &operator=(const W3DRiver &);

	static UnsignedInt createPixelShader(IDirect3DDevice8 *device);
	void setupShader(IDirect3DDevice8 *device, const RenderSettings &settings) const;

	TextureClass *m_texture;
	TextureClass *m_alphaEdge;
	UnsignedInt m_pixelShader;
};
