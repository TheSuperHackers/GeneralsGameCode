#pragma once

class W3DRenderObject;
class W3DRenderContext;
class Shadow;

Shadow* Create_Directional_Shadow(W3DRenderObject* object);
void Reset_Directional_Shadows();
bool Collect_Directional_Shadow_Casters(W3DRenderContext& info);
bool Render_Directional_Shadow_Maps(W3DRenderContext& info);
