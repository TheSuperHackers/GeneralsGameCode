/*
**	Command & Conquer Generals Zero Hour(tm)
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

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

import Graphics.Materials.State;
import Graphics.Scene.Beams.SegmentedLine;

#include "WWLib/ref_ptr.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "WWMath/vector2.h"
#include "WWMath/vector3.h"

class W3DCamera;
class W3DRenderContext;
class W3DTextureHandle;

struct W3DSegmentedLineGeometrySink final {
	void *context = nullptr;
	void (*submit)(void *, const Graphics::PropVertex *, unsigned,
		const std::uint32_t *, unsigned) = nullptr;
};

// Game scene ownership and camera adapter for the segmented-line renderer.
// Geometry preparation and material submission belong to engine/graphics.
class W3DSegmentedLineRenderObject final : public W3DRenderObject
{
public:
	W3DSegmentedLineRenderObject();
	W3DSegmentedLineRenderObject(const W3DSegmentedLineRenderObject &source);
	W3DSegmentedLineRenderObject &operator=(const W3DSegmentedLineRenderObject &source);
	~W3DSegmentedLineRenderObject() override;

	W3DRenderObject *Clone() const override;
	int Class_ID() const override { return CLASSID_SEGLINE; }
	int Get_Num_Polys() const override;
	void Render(W3DRenderContext &rinfo) override;
	bool Cast_Ray(W3DRayCastQuery &raytest) override;

	void Set_Points(unsigned int count, const Vector3 *points);
	int Get_Num_Points() const noexcept;
	void Set_Point_Location(unsigned int index, const Vector3 &point);
	void Get_Point_Location(unsigned int index, Vector3 &point) const;
	void Add_Point(const Vector3 &point);
	void Delete_Point(unsigned int index);

	W3DTextureHandle *Get_Texture() const;
	void Set_Texture(W3DTextureHandle *texture);
	Graphics::MaterialState Get_Shader() const noexcept;
	void Set_Shader(Graphics::MaterialState shader) noexcept;
	float Get_Width() const noexcept;
	void Set_Width(float width) noexcept;
	void Get_Color(Vector3 &color) const;
	void Set_Color(const Vector3 &color) noexcept;
	float Get_Opacity() const noexcept;
	void Set_Opacity(float opacity) noexcept;
	float Get_Noise_Amplitude() const noexcept;
	void Set_Noise_Amplitude(float amplitude) noexcept;
	float Get_Merge_Abort_Factor() const noexcept;
	void Set_Merge_Abort_Factor(float factor) noexcept;
	unsigned Get_Subdivision_Levels() const noexcept;
	void Set_Subdivision_Levels(unsigned levels) noexcept;
	Graphics::RibbonTextureMapping Get_Texture_Mapping_Mode() const noexcept;
	void Set_Texture_Mapping_Mode(Graphics::RibbonTextureMapping mapping) noexcept;
	float Get_Texture_Tile_Factor() const noexcept;
	void Set_Texture_Tile_Factor(float factor) noexcept;
	Vector2 Get_UV_Offset_Rate() const;
	void Set_UV_Offset_Rate(const Vector2 &rate) noexcept;
	int Is_Merge_Intersections() const noexcept;
	void Set_Merge_Intersections(int enabled) noexcept;
	int Is_Freeze_Random() const noexcept;
	void Set_Freeze_Random(int enabled) noexcept;
	int Is_Sorting_Disabled() const noexcept;
	void Set_Disable_Sorting(int disabled) noexcept;
	int Are_End_Caps_Enabled() const noexcept;
	void Set_End_Caps(int enabled) noexcept;

	void Reset_Line();
	void Extract_Geometry(W3DRenderContext &rinfo,
		const W3DSegmentedLineGeometrySink &sink);

	void Prepare_LOD(W3DCamera &camera) override;
	void Increment_LOD() override;
	void Decrement_LOD() override;
	float Get_Cost() const override;
	float Get_Value() const override;
	float Get_Post_Increment_Value() const override;
	void Set_LOD_Level(int lod) override;
	int Get_LOD_Level() const override;
	int Get_LOD_Count() const override;

	void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
	void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

private:
	void Submit(W3DRenderContext &rinfo);

	Graphics::SegmentedLineRenderer m_renderer;
	std::vector<Vector3> m_points;
	RefCountPtr<W3DTextureHandle> m_texture;
	unsigned m_max_subdivision_levels = 0;
	float m_normalized_screen_area = 0.0f;
};

