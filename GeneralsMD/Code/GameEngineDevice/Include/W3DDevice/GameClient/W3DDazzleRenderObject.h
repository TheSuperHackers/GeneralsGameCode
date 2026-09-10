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
#include <array>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "WWLib/ref_ptr.h"
import Graphics.Scene.Models.Factory;
import Graphics.Scene.Dazzles.State;
import Graphics.Scene.Dazzles.Layer;
import Graphics.Scene.Dazzles.Drawing;

class W3DDazzleLayer;
class W3DDazzleVisibility;
class W3DCamera;

class W3DDazzleRenderObject final : public W3DRenderObject {
public:
    explicit W3DDazzleRenderObject(unsigned type);
    explicit W3DDazzleRenderObject(const char* name);
    W3DDazzleRenderObject(const W3DDazzleRenderObject& source);
    W3DDazzleRenderObject& operator=(const W3DDazzleRenderObject& source);
    W3DRenderObject* Clone() const override;
    int Class_ID() const override { return CLASSID_DAZZLE; }
    void Render(W3DRenderContext& info) override;
    void Set_Transform(const Matrix3D& transform) override;
    void Get_Obj_Space_Bounding_Sphere(SphereClass& sphere) const override;
    void Get_Obj_Space_Bounding_Box(AABoxClass& box) const override;
    void Scale(float scale) override { m_state.scale *= scale; }
    void Set_Dazzle_Color(const Vector3& color) { m_state.color = {color.X, color.Y, color.Z}; }
    void Set_Halo_Color(const Vector3& color) { m_state.halo_color = {color.X, color.Y, color.Z}; }
    void Set_Lensflare_Intensity(float intensity) { m_state.lens_flare_intensity = intensity; }
    unsigned Get_Dazzle_Type() const { return m_type; }
    void Set_Layer(W3DDazzleLayer* layer);
    const PersistFactoryClass& Get_Factory() const override;
    static unsigned Get_Type_ID(const char* name);
    static const char* Get_Type_Name(unsigned type);
    static void Set_Current_Dazzle_Layer(W3DDazzleLayer* layer);
    static void Install_Dazzle_Visibility_Handler(const W3DDazzleVisibility* handler);
    static void Enable_Dazzle_Rendering(bool enabled);
    static bool Is_Dazzle_Rendering_Enabled();
private:
    friend class W3DDazzleLayer;
    unsigned m_type;
    float m_radius = 0;
    Graphics::DazzleState m_state;
    Graphics::DazzleMembership m_membership;
};

class W3DDazzleLayer final {
public:
    W3DDazzleLayer();
    ~W3DDazzleLayer();
    void Render(W3DCamera* camera);
private:
    friend class W3DDazzleRenderObject;
    Graphics::DazzleLayer<RefCountPtr<W3DDazzleRenderObject>> m_layer;
    Graphics::DazzleDrawing m_drawing;
};

class W3DDazzleVisibility {
public:
    virtual ~W3DDazzleVisibility() = default;
    virtual float Compute_Dazzle_Visibility(W3DRenderContext& info, W3DDazzleRenderObject* dazzle, const Vector3& point) const;
};
Graphics::ModelFactory<W3DRenderObject>* Load_Dazzle_Factory(ChunkLoadClass& chunks);
