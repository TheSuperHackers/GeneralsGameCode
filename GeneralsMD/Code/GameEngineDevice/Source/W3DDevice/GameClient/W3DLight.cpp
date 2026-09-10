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

#include "W3DDevice/GameClient/W3DLight.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "W3DDevice/GameClient/W3DSceneClass.h"
#include "WWLib/chunkio.h"
#include "WWMath/matrix3d.h"
#include "WWMath/wwmath.h"
#include "WWSaveLoad/persistfactory.h"
#include "WWSaveLoad/saveloadids.h"

import Assets.Adapters.W3D.Light;
import Assets.Adapters.W3D.Chunks;
import Graphics.Scene.AffineTransform;

namespace
{

constexpr std::uint32_t Light_Chunk_W3D_File = 0x02157100;
constexpr std::uint32_t Light_Chunk_Variables = Light_Chunk_W3D_File + 1;
constexpr std::uint32_t Light_Variable_Transform = 0x00;
constexpr std::uint32_t Light_Persist_Chunk_Id = CHUNKID_WW3D_BEGIN + 1;

SimplePersistFactoryClass<W3DLight, Light_Persist_Chunk_Id> Light_Factory;

Assets::LightType To_Asset_Type(W3DLight::LightType type) noexcept
{
	switch (type) {
	case W3DLight::DIRECTIONAL: return Assets::LightType::Directional;
	case W3DLight::SPOT: return Assets::LightType::Spot;
	case W3DLight::POINT:
	default: return Assets::LightType::Point;
	}
}

W3DLight::LightType To_W3D_Type(Assets::LightType type) noexcept
{
	switch (type) {
	case Assets::LightType::Directional: return W3DLight::DIRECTIONAL;
	case Assets::LightType::Spot: return W3DLight::SPOT;
	case Assets::LightType::Point:
	default: return W3DLight::POINT;
	}
}

bool Write_Encoded_Chunks(ChunkSaveClass &save, Assets::W3D::W3DByteSpan bytes)
{
	return Assets::W3D::W3DVisit_Chunks(bytes, [&](const Assets::W3D::W3DChunkView &chunk) {
		const int depth = save.Cur_Chunk_Depth();
		if (!save.Begin_Chunk(chunk.id)) {
			while (save.Cur_Chunk_Depth() > depth)
				save.End_Chunk();
			return false;
		}

		bool success = true;
		if (chunk.contains_children) {
			success = Write_Encoded_Chunks(save, chunk.payload);
		} else if (chunk.payload.size() > (std::numeric_limits<std::uint32_t>::max)()) {
			success = false;
		} else if (!chunk.payload.empty()) {
			success = save.Write(chunk.payload.data(),
				static_cast<std::uint32_t>(chunk.payload.size())) == chunk.payload.size();
		}

		if (!save.End_Chunk())
			success = false;
		while (save.Cur_Chunk_Depth() > depth)
			save.End_Chunk();
		return success;
	});
}

}

W3DLight::W3DLight(LightType type) :
	m_state{}
{
	m_state.authored.type = To_Asset_Type(type);
	if (type == DIRECTIONAL)
		Set_Force_Visible(true);
}

W3DLight::W3DLight(const W3DLight &source) :
	// Deliberately omit W3DRenderObject(source). The retired light's copy
	// constructor left the base identity and scene state at their defaults.
	m_state(source.m_state)
{
}

W3DLight &W3DLight::operator=(const W3DLight &source)
{
	if (this != &source) {
		W3DRenderObject::operator=(source);
		m_state = source.m_state;
	}
	return *this;
}

W3DLight::~W3DLight() = default;

W3DRenderObject *W3DLight::Clone() const
{
	return W3DNEW W3DLight(*this);
}

W3DLight::LightType W3DLight::Get_Type() const noexcept
{
	return To_W3D_Type(m_state.authored.type);
}

void W3DLight::Notify_Added(W3DScene *scene)
{
	W3DRenderObject::Notify_Added(scene);
	scene->Register(this, W3DScene::LIGHT);
}

void W3DLight::Notify_Removed(W3DScene *scene)
{
	scene->Unregister(this, W3DScene::LIGHT);
	W3DRenderObject::Notify_Removed(scene);
}

void W3DLight::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
	sphere.Center.Set(0, 0, 0);
	sphere.Radius = Get_Attenuation_Range();
}

void W3DLight::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
	const float range = Get_Attenuation_Range();
	box.Center.Set(0, 0, 0);
	box.Extent.Set(range, range, range);
}

void W3DLight::Set_Ambient(const Vector3 &color) noexcept
{
	m_state.authored.ambient = {color.X, color.Y, color.Z};
}

void W3DLight::Get_Ambient(Vector3 *color) const noexcept
{
	if (color != nullptr)
		color->Set(m_state.authored.ambient.x, m_state.authored.ambient.y,
			m_state.authored.ambient.z);
}

void W3DLight::Set_Diffuse(const Vector3 &color) noexcept
{
	m_state.authored.diffuse = {color.X, color.Y, color.Z};
}

void W3DLight::Get_Diffuse(Vector3 *color) const noexcept
{
	if (color != nullptr)
		color->Set(m_state.authored.diffuse.x, m_state.authored.diffuse.y,
			m_state.authored.diffuse.z);
}

void W3DLight::Set_Specular(const Vector3 &color) noexcept
{
	m_state.authored.specular = {color.X, color.Y, color.Z};
}

void W3DLight::Get_Specular(Vector3 *color) const noexcept
{
	if (color != nullptr)
		color->Set(m_state.authored.specular.x, m_state.authored.specular.y,
			m_state.authored.specular.z);
}

void W3DLight::Set_Far_Attenuation_Range(double start, double end) noexcept
{
	m_state.authored.far_attenuation_start = static_cast<float>(start);
	m_state.authored.far_attenuation_end = static_cast<float>(end);
}

void W3DLight::Get_Far_Attenuation_Range(double &start, double &end) const noexcept
{
	start = m_state.authored.far_attenuation_start;
	end = m_state.authored.far_attenuation_end;
}

void W3DLight::Get_Far_Attenuation_Range(float &start, float &end) const noexcept
{
	start = m_state.authored.far_attenuation_start;
	end = m_state.authored.far_attenuation_end;
}

float W3DLight::Get_Attenuation_Range() const noexcept
{
	return m_state.authored.far_attenuation_end;
}

void W3DLight::Set_Near_Attenuation_Range(double start, double end) noexcept
{
	m_state.authored.near_attenuation_start = static_cast<float>(start);
	m_state.authored.near_attenuation_end = static_cast<float>(end);
}

void W3DLight::Get_Near_Attenuation_Range(double &start, double &end) const noexcept
{
	start = m_state.authored.near_attenuation_start;
	end = m_state.authored.near_attenuation_end;
}

void W3DLight::Set_Flag(FlagsType flag, bool onoff) noexcept
{
	// Keep the legacy enum encoding. NEAR_ATTENUATION is zero, so either
	// operation is a no-op and never changes the far enable bit.
	if (flag == NEAR_ATTENUATION)
		return;
	if (onoff)
		m_state.authored.far_attenuation_enabled = true;
	else
		m_state.authored.far_attenuation_enabled = false;
}

int W3DLight::Get_Flag(FlagsType flag) const noexcept
{
	return flag == FAR_ATTENUATION && m_state.authored.far_attenuation_enabled;
}

void W3DLight::Set_Spot_Angle(float angle) noexcept
{
	m_state.authored.spot_angle = angle;
	m_state.spot_angle_cosine = WWMath::Fast_Cos(angle);
}

void W3DLight::Set_Spot_Direction(const Vector3 &direction) noexcept
{
	m_state.authored.spot_direction = {direction.X, direction.Y, direction.Z};
}

void W3DLight::Get_Spot_Direction(Vector3 &direction) const noexcept
{
	direction.Set(m_state.authored.spot_direction.x, m_state.authored.spot_direction.y,
		m_state.authored.spot_direction.z);
}

bool W3DLight::Get_Light_Description(Graphics::MaterialLightSource &result) const
{
	const Graphics::RenderTransform transform =
		Graphics::Import_Affine_Transform(Get_Transform());
	result = Graphics::Make_Material_Light(m_state, transform);
	return true;
}

bool W3DLight::Load_W3D(ChunkLoadClass &load)
{
	const std::uint32_t length = load.Cur_Chunk_Length();
	std::vector<std::byte> bytes(length);
	if (load.Read(bytes.data(), length) != length)
		return false;

	Assets::LightAssetDesc decoded = m_state.authored;
	Assets::W3D::W3DLightDecodeMetadata metadata;
	std::string error;
	if (!Assets::W3D::W3DRead_Light(bytes, decoded, error, &metadata))
		return false;

	// The old NEAR_ATTENUATION value is zero and therefore never enabled by
	// Set_Flag. Preserve a decoded near range while retaining that behavior.
	decoded.near_attenuation_enabled = false;
	m_state.authored = decoded;
	if (metadata.spot_info_present)
		m_state.spot_angle_cosine = WWMath::Fast_Cos(m_state.authored.spot_angle);
	return true;
}

bool W3DLight::Save_W3D(ChunkSaveClass &save)
{
	Assets::LightAssetDesc encoded = m_state.authored;
	// See Load_W3D: the legacy zero-valued near flag never enables this record.
	encoded.near_attenuation_enabled = false;

	std::vector<std::byte> bytes;
	std::string error;
	if (!Assets::W3D::W3DWrite_Light(encoded, bytes, error))
		return false;
	return Write_Encoded_Chunks(save, bytes);
}

const PersistFactoryClass &W3DLight::Get_Factory() const
{
	return Light_Factory;
}

bool W3DLight::Save(ChunkSaveClass &save)
{
	const int w3d_depth = save.Cur_Chunk_Depth();
	if (!save.Begin_Chunk(Light_Chunk_W3D_File)) {
		while (save.Cur_Chunk_Depth() > w3d_depth)
			save.End_Chunk();
		return false;
	}
	bool success = Save_W3D(save);
	if (!save.End_Chunk())
		success = false;
	while (save.Cur_Chunk_Depth() > w3d_depth)
		save.End_Chunk();
	if (!success)
		return false;

	const Matrix3D transform = Get_Transform();
	const int variables_depth = save.Cur_Chunk_Depth();
	if (!save.Begin_Chunk(Light_Chunk_Variables)) {
		while (save.Cur_Chunk_Depth() > variables_depth)
			save.End_Chunk();
		return false;
	}
	WRITE_MICRO_CHUNK(save, Light_Variable_Transform, transform);
	success = save.End_Chunk();
	while (save.Cur_Chunk_Depth() > variables_depth)
		save.End_Chunk();
	return success;
}

bool W3DLight::Load(ChunkLoadClass &load)
{
	Matrix3D transform(1);
	while (load.Open_Chunk()) {
		switch (load.Cur_Chunk_ID()) {
		case Light_Chunk_W3D_File:
			if (!load.Open_Chunk()
				|| !Load_W3D(load)
				|| !load.Close_Chunk())
				return false;
			break;
		case Light_Chunk_Variables:
			while (load.Open_Micro_Chunk()) {
				switch (load.Cur_Micro_Chunk_ID()) {
					READ_MICRO_CHUNK(load, Light_Variable_Transform, transform);
				}
				load.Close_Micro_Chunk();
			}
			break;
		default:
			WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d", __FILE__, __LINE__));
			break;
		}
		load.Close_Chunk();
	}
	Set_Transform(transform);
	return true;
}
