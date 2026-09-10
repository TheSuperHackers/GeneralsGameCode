module;

#define BOOST_TEST_MODULE GraphicsRenderSettingsTests

#include <boost/test/included/unit_test.hpp>

export module Graphics.Frame.RenderSettings.Tests;

import Graphics.Frame.RenderSettings;

using Graphics::RenderSettings;
using Graphics::RenderPrelitMode;

BOOST_AUTO_TEST_CASE(settings_preserve_the_runtime_defaults)
{
	RenderSettings settings;

	BOOST_CHECK(settings.Is_Texturing_Enabled());
	BOOST_CHECK(!settings.Is_Coloring_Enabled());
	BOOST_CHECK(settings.Is_Sorting_Enabled());
	BOOST_CHECK(!settings.Is_Munge_Sort_On_Load_Enabled());
	BOOST_CHECK(!settings.Is_Overbright_Modify_On_Load_Enabled());
	BOOST_CHECK(settings.Get_Prelit_Mode() == RenderPrelitMode::LightmapMultiPass);
	BOOST_CHECK(!settings.Get_Preserve_FPU());
}

BOOST_AUTO_TEST_CASE(settings_update_each_extraction_option)
{
	RenderSettings settings;
	settings.Set_Texturing_Enabled(false);
	settings.Set_Coloring_Enabled(true);
	settings.Set_Sorting_Enabled(false);
	settings.Set_Munge_Sort_On_Load_Enabled(true);
	settings.Set_Overbright_Modify_On_Load_Enabled(true);
	settings.Set_Prelit_Mode(RenderPrelitMode::Vertex);
	settings.Set_Preserve_FPU(true);

	BOOST_CHECK(!settings.Is_Texturing_Enabled());
	BOOST_CHECK(settings.Is_Coloring_Enabled());
	BOOST_CHECK(!settings.Is_Sorting_Enabled());
	BOOST_CHECK(settings.Is_Munge_Sort_On_Load_Enabled());
	BOOST_CHECK(settings.Is_Overbright_Modify_On_Load_Enabled());
	BOOST_CHECK(settings.Get_Prelit_Mode() == RenderPrelitMode::Vertex);
	BOOST_CHECK(settings.Get_Preserve_FPU());
}
