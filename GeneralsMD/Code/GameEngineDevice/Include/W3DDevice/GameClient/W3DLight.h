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

#include "W3DDevice/GameClient/W3DRenderObject.h"

import Graphics.Scene.Lighting.Local;
import Graphics.Scene.Lighting.State;

class ChunkLoadClass;
class ChunkSaveClass;

// Game-facing light object. Graphics owns the value conversion while this
// adapter retains scene membership, authored persistence, and transform data.
class W3DLight : public W3DRenderObject
{
public:
	enum LightType
	{
		POINT = 0,
		DIRECTIONAL,
		SPOT
	};

	// Keep the legacy numeric encoding: NEAR_ATTENUATION is zero and therefore
	// does not set a bit when passed to Set_Flag.
	enum FlagsType
	{
		NEAR_ATTENUATION = 0,
		FAR_ATTENUATION
	};

	explicit W3DLight(LightType type = POINT);
	W3DLight(const W3DLight &source);
	W3DLight &operator=(const W3DLight &source);
	~W3DLight() override;

	W3DRenderObject *Clone() const override;
	int Class_ID() const override { return CLASSID_LIGHT; }

	void Render(W3DRenderContext &) override { }
	bool Is_Vertex_Processor() { return true; }

	void Notify_Added(W3DScene *scene) override;
	void Notify_Removed(W3DScene *scene) override;

	void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
	void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

	LightType Get_Type() const noexcept;

	void Set_Intensity(float intensity) noexcept { m_state.authored.intensity = intensity; }
	float Get_Intensity() const noexcept { return m_state.authored.intensity; }

	void Set_Ambient(const Vector3 &color) noexcept;
	void Get_Ambient(Vector3 *color) const noexcept;
	void Set_Diffuse(const Vector3 &color) noexcept;
	void Get_Diffuse(Vector3 *color) const noexcept;
	void Set_Specular(const Vector3 &color) noexcept;
	void Get_Specular(Vector3 *color) const noexcept;

	void Set_Far_Attenuation_Range(double start, double end) noexcept;
	void Get_Far_Attenuation_Range(double &start, double &end) const noexcept;
	void Get_Far_Attenuation_Range(float &start, float &end) const noexcept;
	float Get_Attenuation_Range() const noexcept;
	void Set_Near_Attenuation_Range(double start, double end) noexcept;
	void Get_Near_Attenuation_Range(double &start, double &end) const noexcept;

	void Set_Flag(FlagsType flag, bool onoff) noexcept;
	int Get_Flag(FlagsType flag) const noexcept;
	void Enable_Shadows(bool onoff) noexcept { m_state.authored.cast_shadows = onoff; }
	bool Are_Shadows_Enabled() const noexcept { return m_state.authored.cast_shadows; }

	void Set_Spot_Angle(float angle) noexcept;
	float Get_Spot_Angle() const noexcept { return m_state.authored.spot_angle; }
	float Get_Spot_Angle_Cos() const noexcept { return m_state.spot_angle_cosine; }
	void Set_Spot_Direction(const Vector3 &direction) noexcept;
	void Get_Spot_Direction(Vector3 &direction) const noexcept;
	void Set_Spot_Exponent(float exponent) noexcept { m_state.authored.spot_exponent = exponent; }
	float Get_Spot_Exponent() const noexcept { return m_state.authored.spot_exponent; }

	bool Load_W3D(ChunkLoadClass &load);
	bool Save_W3D(ChunkSaveClass &save);

	const PersistFactoryClass &Get_Factory() const override;
	bool Save(ChunkSaveClass &save) override;
	bool Load(ChunkLoadClass &load) override;

	bool Get_Light_Description(Graphics::MaterialLightSource &result) const override;
	const Graphics::LightState &Get_Light_State() const noexcept { return m_state; }

private:
	Graphics::LightState m_state;
};
