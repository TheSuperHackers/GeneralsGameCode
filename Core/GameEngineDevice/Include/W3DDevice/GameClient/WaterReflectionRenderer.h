/*
** Command & Conquer Generals Zero Hour(tm)
**
** Scene-submission contract used by the water renderer.
*/

#pragma once

import Graphics.RHI;

class W3DCamera;

class WaterReflectionRenderer
{
public:
	virtual ~WaterReflectionRenderer() = default;

	virtual void Render_Water_Reflection(W3DCamera *camera,
		const Graphics::RHIViewport &viewport) = 0;
};
