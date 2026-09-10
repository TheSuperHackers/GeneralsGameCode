/*
** Command & Conquer Generals Zero Hour(tm)
**
** Isolated adapter for the existing skybox asset. WaterRenderSystem does not
** expose or own the legacy scene object used by that asset.
*/

#pragma once

class W3DRenderContext;

class WaterSkyboxSystem final
{
public:
	WaterSkyboxSystem();
	~WaterSkyboxSystem();

	WaterSkyboxSystem(const WaterSkyboxSystem &) = delete;
	WaterSkyboxSystem &operator=(const WaterSkyboxSystem &) = delete;

	bool Initialize(float scale);
	void Render(W3DRenderContext &rinfo, float x, float y, float z);
	void Replace_Texture(const char *old_name, const char *new_name);

private:
	struct State;
	State *m_state;
};
