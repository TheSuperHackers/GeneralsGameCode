import Graphics.Frame.RenderClock;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"
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

#include <algorithm>
#include <array>
#include <cmath>
#include <climits>
#include <cstddef>
#include <optional>

import Graphics.Materials.Ordering;
import Graphics.Frame.Runtime;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.SegmentedLine;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.OrderedDraws;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Views.CameraMatrices;

#include "W3DDevice/GameClient/W3DSegmentedLineRenderObject.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

#include "WWLib/RANDOM.h"
#include "WWMath/matrix4.h"
#include "WWMath/v3_rnd.h"

namespace {

template <typename Matrix>
std::array<float, 16> Copy_Matrix(const Matrix &matrix) noexcept
{
	std::array<float, 16> result{};
	for (std::size_t row = 0; row < 4; ++row)
		for (std::size_t column = 0; column < 4; ++column)
			result[row * 4 + column] = matrix[row][column];
	return result;
}

Graphics::SegmentedLineBuildInput Make_Build_Input(
	const W3DSegmentedLineRenderObject &line)
{
	const auto &camera = Graphics::Get_Camera_Matrices();
	Graphics::SegmentedLineBuildInput input;
	input.view = camera.view.values;
	input.world = Copy_Matrix(Matrix4x4(line.Get_Transform()));
	input.time_milliseconds = Graphics::Get_Render_Clock().Logic_Time_Milliseconds();
	return input;
}

struct RandomState final {
	std::optional<Random3Class> frozen;
	std::optional<Vector3SolidBoxRandomizer> randomizer;

	void Reset(std::size_t)
	{
		frozen.emplace();
		randomizer.emplace(Vector3(1, 1, 1));
	}

	std::array<float, 3> Next(bool freeze)
	{
		Vector3 value;
		if (freeze) {
			// Keep the legacy evaluation count and axis order.  SegLineRenderer
			// converted Random3Class independently for all three components.
			const float oo_int_max = 1.0f / static_cast<float>(INT_MAX);
			value.Set(*frozen * oo_int_max, *frozen * oo_int_max, *frozen * oo_int_max);
		} else {
			randomizer->Get_Vector(value);
		}
		return {value.X, value.Y, value.Z};
	}
};

}

W3DSegmentedLineRenderObject::W3DSegmentedLineRenderObject()
	: m_renderer(Graphics::SegmentedLineDescription{}, Graphics::Get_Render_Clock().Logic_Time_Milliseconds())
{
}

W3DSegmentedLineRenderObject::W3DSegmentedLineRenderObject(
	const W3DSegmentedLineRenderObject &source)
	: m_renderer(source.m_renderer),
	  m_points(source.m_points),
	  m_texture(source.m_texture),
	  m_max_subdivision_levels(source.m_max_subdivision_levels),
	  m_normalized_screen_area(source.m_normalized_screen_area)
{
}

W3DSegmentedLineRenderObject &W3DSegmentedLineRenderObject::operator=(
	const W3DSegmentedLineRenderObject &source)
{
	if (this == &source)
		return *this;
	W3DRenderObject::operator=(source);
	m_renderer = source.m_renderer;
	m_points = source.m_points;
	m_texture = source.m_texture;
	m_max_subdivision_levels = source.m_max_subdivision_levels;
	m_normalized_screen_area = source.m_normalized_screen_area;
	return *this;
}

W3DSegmentedLineRenderObject::~W3DSegmentedLineRenderObject() = default;

W3DRenderObject *W3DSegmentedLineRenderObject::Clone() const
{
	return W3DNEW W3DSegmentedLineRenderObject(*this);
}

int W3DSegmentedLineRenderObject::Get_Num_Polys() const
{
	if (m_points.size() < 2)
		return 0;
	const auto subdivision = std::size_t{1} << m_renderer.Get_Current_Subdivision_Level();
	return static_cast<int>(2 * (m_points.size() - 1) * subdivision);
}

void W3DSegmentedLineRenderObject::Render(W3DRenderContext &rinfo)
{
	if (!Is_Not_Hidden_At_All() || m_points.size() < 2)
		return;
	if (!Graphics::Get_Render_Settings().Is_Sorting_Enabled()) {
		const auto sort_level = Graphics::Material_Ordered_Layer(m_renderer.Get_Shader());
		if (Graphics::Get_Scene_Draw_Queue().Is_Enabled() && sort_level != 0
			&& Graphics::Get_Scene_Draw_Queue().Enqueue<Extract_Ordered_Draw>(sort_level, *this))
			return;
	}
	Submit(rinfo);
}

bool W3DSegmentedLineRenderObject::Cast_Ray(W3DRayCastQuery &raytest)
{
	if ((Get_Collision_Type() & raytest.CollisionType) == 0)
		return false;

	bool hit = false;
	float fraction = 1.0f;
	for (std::size_t index = 1; index < m_points.size(); ++index) {
		Vector3 transformed_points[2];
		Get_Transform().mulVector3Array(&m_points[index - 1], transformed_points, 2);
		const LineSegClass line_segment(transformed_points[0], transformed_points[1]);

		Vector3 ray_point;
		Vector3 line_point;
		if (!raytest.Ray.Find_Intersection(line_segment, &ray_point, &fraction,
			&line_point, nullptr))
			continue;

		const float distance = (ray_point - line_point).Length();
		if (distance <= m_renderer.Get_Width() && fraction >= 0.0f
			&& fraction < raytest.Result->Fraction) {
			hit = true;
			break;
		}
	}

	if (!hit)
		return false;

	raytest.Result->Fraction = fraction;
	// Picking reports the format's default surface value for procedural lines.
	constexpr std::uint32_t default_surface_type = 13;
	raytest.Result->SurfaceType = default_surface_type;
	raytest.CollidedRenderObj = this;
	return true;
}

void W3DSegmentedLineRenderObject::Submit(W3DRenderContext &rinfo)
{
	auto *device = Graphics::Shared_Frame_Device();
	if (device == nullptr)
		return;

	Graphics::SegmentedLineDrawInput input;
	input.build = Make_Build_Input(*this);
	input.material_time_milliseconds = Graphics::Get_Render_Clock().Sync_Time();
	Matrix4x4 projection;
	rinfo.Camera.Get_Backend_Projection_Matrix(&projection);
	input.projection = Copy_Matrix(projection);
	input.scene = Graphics::Get_Scene_Draw_Parameters();
	input.reflection = Get_W3D_Render_Services().Is_Reflection_Render_Pass();
	const auto shader = m_renderer.Get_Shader();
	if (!m_renderer.Is_Sorting_Disabled() && Graphics::Get_Render_Settings().Is_Sorting_Enabled()
		&& shader.Get_Dst_Blend_Func() != Graphics::MaterialState::DSTBLEND_ZERO
		&& shader.Get_Alpha_Test() == Graphics::MaterialState::ALPHATEST_DISABLE)
		input.sorting_depth = {0.0f, 0.0f, 1.0f, 0.0f};

	RandomState random;
	const auto read_point = [&](std::size_t index) {
		const Vector3 &point = m_points[index];
		return Graphics::RibbonPoint{{point.X, point.Y, point.Z}, {1, 1, 1, 1}, 0.0f};
	};
	const auto resolve = [](W3DTextureHandle *source, bool load)
		-> std::optional<Graphics::PropMaterialTexture> {
		if (load && !source->Ensure_Render_Backend_Texture())
			return std::nullopt;
		return Graphics::PropMaterialTexture{source->Peek_Graphics_Texture(), source->Get_Sampling()};
	};
	m_renderer.Submit(*device, Graphics::Get_Prop_Renderer(), Graphics::Get_Prop_Submission(),
		input, m_points.size(), m_texture.Peek(), resolve, read_point,
		[&] { return random.Next(m_renderer.Is_Freeze_Random()); },
		[&](std::size_t chunk) { random.Reset(chunk); });
}

void W3DSegmentedLineRenderObject::Set_Points(unsigned int count, const Vector3 *points)
{
	if (count < 2 || points == nullptr)
		return;
	m_points.assign(points, points + count);
	Invalidate_Cached_Bounding_Volumes();
}

int W3DSegmentedLineRenderObject::Get_Num_Points() const noexcept
{
	return static_cast<int>(m_points.size());
}

void W3DSegmentedLineRenderObject::Set_Point_Location(unsigned int index, const Vector3 &point)
{
	if (index < m_points.size())
		m_points[index] = point;
	Invalidate_Cached_Bounding_Volumes();
}

void W3DSegmentedLineRenderObject::Get_Point_Location(unsigned int index, Vector3 &point) const
{
	point = index < m_points.size() ? m_points[index] : Vector3(0, 0, 0);
}

void W3DSegmentedLineRenderObject::Add_Point(const Vector3 &point)
{
	m_points.push_back(point);
	Invalidate_Cached_Bounding_Volumes();
}

void W3DSegmentedLineRenderObject::Delete_Point(unsigned int index)
{
	if (index < m_points.size())
		m_points.erase(m_points.begin() + index);
	Invalidate_Cached_Bounding_Volumes();
}

W3DTextureHandle *W3DSegmentedLineRenderObject::Get_Texture() const
{
	if (auto *texture = m_texture.Peek())
		texture->Add_Ref();
	return m_texture.Peek();
}

void W3DSegmentedLineRenderObject::Set_Texture(W3DTextureHandle *texture)
{
	m_texture.Assign_Add_Ref(texture);
}

Graphics::MaterialState W3DSegmentedLineRenderObject::Get_Shader() const noexcept
{
	return m_renderer.Get_Shader();
}

void W3DSegmentedLineRenderObject::Set_Shader(Graphics::MaterialState shader) noexcept
{
	m_renderer.Set_Shader(shader);
}

float W3DSegmentedLineRenderObject::Get_Width() const noexcept { return m_renderer.Get_Width(); }

void W3DSegmentedLineRenderObject::Set_Width(float width) noexcept
{
	m_renderer.Set_Width((std::max)(width, 0.0f));
	Invalidate_Cached_Bounding_Volumes();
}

void W3DSegmentedLineRenderObject::Get_Color(Vector3 &color) const
{
	const auto value = m_renderer.Get_Color();
	color.Set(value[0], value[1], value[2]);
}

void W3DSegmentedLineRenderObject::Set_Color(const Vector3 &color) noexcept
{
	m_renderer.Set_Color({color.X, color.Y, color.Z});
}

float W3DSegmentedLineRenderObject::Get_Opacity() const noexcept { return m_renderer.Get_Opacity(); }
void W3DSegmentedLineRenderObject::Set_Opacity(float opacity) noexcept { m_renderer.Set_Opacity(opacity); }
float W3DSegmentedLineRenderObject::Get_Noise_Amplitude() const noexcept { return m_renderer.Get_Noise_Amplitude(); }

void W3DSegmentedLineRenderObject::Set_Noise_Amplitude(float amplitude) noexcept
{
	m_renderer.Set_Noise_Amplitude(std::abs(amplitude));
	Invalidate_Cached_Bounding_Volumes();
}

float W3DSegmentedLineRenderObject::Get_Merge_Abort_Factor() const noexcept
{
	return m_renderer.Get_Merge_Abort_Factor();
}
void W3DSegmentedLineRenderObject::Set_Merge_Abort_Factor(float factor) noexcept
{
	m_renderer.Set_Merge_Abort_Factor(factor);
}

unsigned W3DSegmentedLineRenderObject::Get_Subdivision_Levels() const noexcept
{
	return m_max_subdivision_levels;
}

void W3DSegmentedLineRenderObject::Set_Subdivision_Levels(unsigned levels) noexcept
{
	m_max_subdivision_levels = (std::min)(levels, Graphics::RibbonPipelineMaximumSubdivisionLevel);
	m_renderer.Set_Current_Subdivision_Level(
		(std::min)(m_renderer.Get_Current_Subdivision_Level(), m_max_subdivision_levels));
	Invalidate_Cached_Bounding_Volumes();
}

Graphics::RibbonTextureMapping W3DSegmentedLineRenderObject::Get_Texture_Mapping_Mode() const noexcept
{
	return m_renderer.Get_Texture_Mapping_Mode();
}
void W3DSegmentedLineRenderObject::Set_Texture_Mapping_Mode(Graphics::RibbonTextureMapping mapping) noexcept
{
	m_renderer.Set_Texture_Mapping_Mode(mapping);
}
float W3DSegmentedLineRenderObject::Get_Texture_Tile_Factor() const noexcept
{
	return m_renderer.Get_Texture_Tile_Factor();
}
void W3DSegmentedLineRenderObject::Set_Texture_Tile_Factor(float factor) noexcept
{
	m_renderer.Set_Texture_Tile_Factor(factor);
}
Vector2 W3DSegmentedLineRenderObject::Get_UV_Offset_Rate() const
{
	const auto rate = m_renderer.Get_UV_Offset_Rate();
	return Vector2(rate[0], rate[1]);
}
void W3DSegmentedLineRenderObject::Set_UV_Offset_Rate(const Vector2 &rate) noexcept
{
	m_renderer.Set_UV_Offset_Rate({rate.X, rate.Y});
}
int W3DSegmentedLineRenderObject::Is_Merge_Intersections() const noexcept
{
	return m_renderer.Is_Merge_Intersections();
}
void W3DSegmentedLineRenderObject::Set_Merge_Intersections(int enabled) noexcept
{
	m_renderer.Set_Merge_Intersections(enabled != 0);
}
int W3DSegmentedLineRenderObject::Is_Freeze_Random() const noexcept
{
	return m_renderer.Is_Freeze_Random();
}
void W3DSegmentedLineRenderObject::Set_Freeze_Random(int enabled) noexcept
{
	m_renderer.Set_Freeze_Random(enabled != 0);
}
int W3DSegmentedLineRenderObject::Is_Sorting_Disabled() const noexcept
{
	return m_renderer.Is_Sorting_Disabled();
}
void W3DSegmentedLineRenderObject::Set_Disable_Sorting(int disabled) noexcept
{
	m_renderer.Set_Disable_Sorting(disabled != 0);
}
int W3DSegmentedLineRenderObject::Are_End_Caps_Enabled() const noexcept
{
	return m_renderer.Are_End_Caps_Enabled();
}
void W3DSegmentedLineRenderObject::Set_End_Caps(int enabled) noexcept
{
	m_renderer.Set_End_Caps(enabled != 0);
}

void W3DSegmentedLineRenderObject::Reset_Line()
{
	m_renderer.Reset_Line(Graphics::Get_Render_Clock().Logic_Time_Milliseconds());
}

void W3DSegmentedLineRenderObject::Extract_Geometry(
	W3DRenderContext &rinfo, const W3DSegmentedLineGeometrySink &sink)
{
	if (!Is_Not_Hidden_At_All() || m_points.size() < 2 || sink.submit == nullptr)
		return;
	Graphics::SegmentedLineBuildInput input = Make_Build_Input(*this);
	RandomState random;
	const auto read_point = [&](std::size_t index) {
		const Vector3 &point = m_points[index];
		return Graphics::RibbonPoint{{point.X, point.Y, point.Z}, {1, 1, 1, 1}, 0.0f};
	};
	m_renderer.Extract_Geometry(input, m_points.size(), read_point,
		[&] { return random.Next(m_renderer.Is_Freeze_Random()); },
		[&](std::size_t chunk) { random.Reset(chunk); },
		[&](const Graphics::RibbonPipelineChunk &chunk) {
			sink.submit(sink.context, chunk.vertices.data(),
				static_cast<unsigned>(chunk.vertices.size()),
				chunk.indices.data(),
				static_cast<unsigned>(chunk.indices.size()));
		});
}

void W3DSegmentedLineRenderObject::Prepare_LOD(W3DCamera &camera)
{
	m_normalized_screen_area = Get_Screen_Size(camera);
	m_renderer.Set_Current_Subdivision_Level(
		(std::min)(m_renderer.Get_Current_Subdivision_Level(), m_max_subdivision_levels));
}

void W3DSegmentedLineRenderObject::Increment_LOD()
{
	m_renderer.Set_Current_Subdivision_Level((std::min)(
		m_renderer.Get_Current_Subdivision_Level() + 1, m_max_subdivision_levels));
}

void W3DSegmentedLineRenderObject::Decrement_LOD()
{
	const auto level = m_renderer.Get_Current_Subdivision_Level();
	if (level != 0)
		m_renderer.Set_Current_Subdivision_Level(level - 1);
}

float W3DSegmentedLineRenderObject::Get_Cost() const { return Get_Num_Polys(); }

float W3DSegmentedLineRenderObject::Get_Value() const
{
	if (m_renderer.Get_Current_Subdivision_Level() == 0)
		return AT_MIN_LOD;
	const float polygons = static_cast<float>(Get_Num_Polys());
	const float benefit = 1.0f - 0.5f / (polygons * polygons);
	return benefit * m_normalized_screen_area / Get_Cost();
}

float W3DSegmentedLineRenderObject::Get_Post_Increment_Value() const
{
	if (m_renderer.Get_Current_Subdivision_Level() == m_max_subdivision_levels)
		return AT_MAX_LOD;
	const float polygons = 2.0f * static_cast<float>(Get_Num_Polys());
	const float benefit = 1.0f - 0.5f / (polygons * polygons);
	return benefit * m_normalized_screen_area / polygons;
}

void W3DSegmentedLineRenderObject::Set_LOD_Level(int lod)
{
	lod = (std::max)(0, (std::min)(lod, static_cast<int>(m_max_subdivision_levels)));
	m_renderer.Set_Current_Subdivision_Level(static_cast<unsigned>(lod));
}

int W3DSegmentedLineRenderObject::Get_LOD_Level() const
{
	return static_cast<int>(m_renderer.Get_Current_Subdivision_Level());
}

int W3DSegmentedLineRenderObject::Get_LOD_Count() const
{
	return static_cast<int>(m_max_subdivision_levels);
}

void W3DSegmentedLineRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
	AABoxClass box;
	Get_Obj_Space_Bounding_Box(box);
	sphere.Center = box.Center;
	sphere.Radius = box.Extent.Length();
}

void W3DSegmentedLineRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
	const auto bounds = m_renderer.Get_Bounds(m_points.size(), m_max_subdivision_levels,
		[&](std::size_t index) {
			const Vector3 &point = m_points[index];
			return std::array<float, 3>{point.X, point.Y, point.Z};
		});
	if (!bounds.valid) {
		box.Init(Vector3(0, 0, 0), Vector3(1, 1, 1));
		return;
	}
	box.Init_Min_Max(Vector3(bounds.minimum[0], bounds.minimum[1], bounds.minimum[2]),
		Vector3(bounds.maximum[0], bounds.maximum[1], bounds.maximum[2]));
}
