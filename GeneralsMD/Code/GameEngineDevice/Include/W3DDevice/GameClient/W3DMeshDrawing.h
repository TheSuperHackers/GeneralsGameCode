#pragma once
import Graphics.Scene.Models.MeshDrawing;
class W3DMeshRenderObject;
class W3DRenderContext;
bool Draw_W3D_Mesh(W3DMeshRenderObject& mesh, W3DRenderContext& info, const Graphics::ModelMeshDrawOverrides& overrides);

class W3DRenderObject;
void Flush_Before_W3D_Object_Draw(const W3DRenderObject& object);
