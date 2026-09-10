#include <winsock2.h>
#include <functional>
import Graphics.Frame.RenderClock;
import Graphics.Scene.OrderedDraws;
import Graphics.Frame.RenderSettings;
#include "W3DDevice/GameClient/W3DRenderServices.h"
#include "W3DDevice/GameClient/W3DDazzleRenderObject.h"
#include "W3DDevice/GameClient/W3DDazzleResources.h"
#include <climits>
import Graphics.Presentation.DisplayModes;
import Graphics.Resources.Textures.Quality;
import Graphics.Diagnostics.Render;
import Graphics.Frame.SubmissionStatistics;
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DDisplay.cpp ///////////////////////////////////////////////////////
//
// W3D Implementation for the Game Display which is responsible for creating
// and maintaining the entire visual display
//
// Author: Colin Day, April 2001
//
///////////////////////////////////////////////////////////////////////////////

static void drawFramerateBar();

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Debug.CollisionBox;
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <numeric>
#include <stdlib.h>
#include "Platform/SDLPlatformWindow.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

import Graphics.Frame.Runtime;
import Graphics.Renderer2D;
import Graphics.Frame.SceneRenderers;
import Graphics.Passes.Bloom;
import Graphics.Passes.LightRays;
import Graphics.Passes.SSAO;
import Engine.UI.WND;
import Graphics.Scene.Beams;
import Graphics.Scene.Lighting.Renderer;
import Graphics.Scene.Particles.Renderer;
import Graphics.Scene.Screen.Distortion;
import Graphics.Scene.Screen.FullscreenOverlay;
import Graphics.Scene.Ring;
import Graphics.Scene.WorldQuads;
import Graphics.Scene.Trees.Renderer;
import Graphics.Scene.Water.Renderer;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Screen.Filters;

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/FramePacer.h"
#include "Common/ThingFactory.h"
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"
#include "Common/FileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingTemplate.h"
#include "Common/NameKeyGenerator.h"
#include "Common/GameLOD.h"
#include "Common/DrawModule.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Module/PhysicsUpdate.h"

#include "GameClient/Drawable.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GraphDraw.h"
#include "GameClient/Line2D.h"
#include "GameClient/Mouse.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/GameWindow.h"
#include "GameClient/VideoRuntime.h"
#include "GameClient/Water.h"

#include "GameNetwork/NetworkInterface.h"
#include "Common/ModelState.h"
#include "Lib/BaseType.h"
#include "W3DDevice/Common/W3DConvert.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/CollisionBoxRenderObject.h"
#include "W3DDevice/GameClient/NullRenderObject.h"
#include "W3DDevice/GameClient/W3DRingLoader.h"
#include "W3DDevice/GameClient/W3DSphereLoader.h"
#include "W3DDevice/GameClient/W3DAssetRuntime.h"
#include "W3DDevice/GameClient/W3DBibBuffer.h"
#include "W3DDevice/GameClient/W3DTerrainGraphics.h"
#include "W3DDevice/GameClient/W3DGraphicsResources.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/W3DFileSystem.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/W3DLight.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DDebugDisplay.h"
#include "W3DDevice/GameClient/W3DProjectedShadow.h"
#include "W3DDevice/GameClient/W3DScreenshot.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "WWMath/wwmath.h"

#include "WWMath/matrix4.h"
#include "W3DDevice/GameClient/W3DEmitterLoader.h"
import Assets.Images.PixelEncoding;
import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.Aggregate;
import Assets.Adapters.W3D.Assembly;
import Assets.Adapters.W3D.Box;
import Assets.Adapters.W3D.Collection;
import Assets.Adapters.W3D.Dazzle;
import Assets.Adapters.W3D.LevelSet;
import Assets.Adapters.W3D.Null;
import Assets.Adapters.W3D.Particles;
import Assets.Adapters.W3D.Ring;
import Assets.Adapters.W3D.Sphere;
import Graphics.RHI;
#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DHierarchyRenderObject.h"
#include "W3DDevice/GameClient/W3DCollectionRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"

extern "C" bool Graphics_Begin_Frame() noexcept;
extern "C" bool Graphics_Execute_Queued_Draws() noexcept;
extern "C" bool Graphics_End_Frame() noexcept;
extern "C" bool Graphics_Present() noexcept;
extern "C" void Graphics_Abort_Frame() noexcept;

#include "GameLogic/ScriptEngine.h"		// For TheScriptEngine - jkmcd
#include "GameLogic/GameLogic.h"

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GENERALS_GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GENERALS_GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif
#ifdef DUMP_PERF_STATS
#include "GameLogic/PartitionManager.h"

#endif



// DEFINE AND ENUMS ///////////////////////////////////////////////////////////

import Graphics.Capture.MovieCapture;
import Graphics.Capture.FramePreview;
static Graphics::MovieCapture displayMovieCapture;

#define no_SAMPLE_DYNAMIC_LIGHT	1
static bool graphicsRendererAvailable = false;
static bool uiFrameActive = false;
static Graphics::BeamView graphicsBeamView;
static Graphics::View graphicsParticleView;
static Graphics::LightRaysInput graphicsLightRaysInput;
static Graphics::SSAOInput graphicsSSAOInput;
#ifdef SAMPLE_DYNAMIC_LIGHT
static W3DDynamicLight * theDynamicLight = nullptr;
static Real theLightXOffset = 0.1f;
static Real theLightYOffset = 0.07f;
static Int theFlashCount = 0;
#endif

static Graphics::Color2D To_UI_Color(UnsignedInt color) noexcept
{
	return {
		static_cast<float>((color >> 16) & 0xff) / 255.0f,
		static_cast<float>((color >> 8) & 0xff) / 255.0f,
		static_cast<float>(color & 0xff) / 255.0f,
		static_cast<float>((color >> 24) & 0xff) / 255.0f};
}

static Graphics::Renderer2DBlendMode To_UI_Blend_Mode(Display::DrawImageMode mode) noexcept
{
	switch (mode) {
	case Display::DRAW_IMAGE_SOLID:
		return Graphics::Renderer2DBlendMode::Solid;
	case Display::DRAW_IMAGE_ADDITIVE:
		return Graphics::Renderer2DBlendMode::Additive;
	default:
		return Graphics::Renderer2DBlendMode::Alpha;
	}
}

static void Set_UI_Clip(Bool enabled, const IRegion2D &region) noexcept
{
	Graphics::Get_Renderer2D().Set_Clip(
		enabled != FALSE,
		{static_cast<float>(region.lo.x), static_cast<float>(region.lo.y),
			static_cast<float>(region.hi.x), static_cast<float>(region.hi.y)});
}

static bool initializeGraphicsSceneRenderers(Graphics::Device &device)
{
    const auto shader_directory = Graphics::Frame_Shader_Directory(std::filesystem::path("GraphicsShaders"));
    return Graphics::Initialize_Scene_Renderers(device, shader_directory)
        && Graphics::Get_Bloom_Renderer().Initialize(device, shader_directory)
        && Graphics::Get_Light_Rays_Renderer().Initialize(device, shader_directory)
        && Graphics::Get_SSAO_Renderer().Initialize(device, shader_directory)
        && Initialize_Video_Presentation(device, shader_directory);
}

static bool executeGraphicsFramePasses(Graphics::Device &device, Graphics::CommandList &commands, const Graphics::FrameTargets &targets) noexcept
{
    const auto detail = TheGameLODManager != nullptr
        ? TheGameLODManager->getStaticLODLevel() : STATIC_GAME_LOD_UNKNOWN;
    if (!Graphics::Get_SSAO_Renderer().Render(commands, targets,
        Graphics::RHITextureFormat::BGRA8_UNorm, Graphics::RHITextureFormat::D24_UNorm_S8, graphicsSSAOInput,
        TheTerrainRenderObject != nullptr && (detail == STATIC_GAME_LOD_HIGH || detail == STATIC_GAME_LOD_VERY_HIGH)))
        return false;

	if (!Graphics::GetLightRenderer().Sync())
		return false;


	if (!Graphics::SetBeamView(graphicsBeamView))
		return false;

	if (!Graphics::GetBeamRenderer().Render(
		commands,
		targets.backbuffer.texture,
		targets.depth.texture,
		{0, 0, targets.backbuffer.width, targets.backbuffer.height, 0.0f, 1.0f}))
		return false;

	if (TheParticleSystemManager != nullptr) {
		W3DParticleSystemManager *particle_manager = static_cast<W3DParticleSystemManager *>(TheParticleSystemManager);
		if (!particle_manager->Render_Graphics_Particles(commands, targets))
			return false;
	}



	if (!Graphics::Get_Bloom_Renderer().Render(commands, targets, Graphics::RHITextureFormat::BGRA8_UNorm,
		detail == STATIC_GAME_LOD_HIGH || detail == STATIC_GAME_LOD_VERY_HIGH))
		return false;
	// Composite after bloom to keep added ray light inside the shroud mask.
	if (TheTerrainRenderObject != nullptr
		&& (detail == STATIC_GAME_LOD_HIGH || detail == STATIC_GAME_LOD_VERY_HIGH)) {
		Graphics::SurfaceParameters surface;
		auto rays = graphicsLightRaysInput;
		const auto& sunlight = TheGlobalData->m_terrainDiffuse[0];
		rays.brightness[0] *= std::clamp(sunlight.red, 0.0f, 1.0f);
		rays.brightness[1] *= std::clamp(sunlight.green, 0.0f, 1.0f);
		rays.brightness[2] *= std::clamp(sunlight.blue, 0.0f, 1.0f);
		rays.shroud_texture = Set_Surface_Shroud(surface, TheTerrainRenderObject->getShroud());
		rays.shroud_projection = surface.shroud_projection;
		if (!Graphics::Get_Light_Rays_Renderer().Render(commands, targets,
			Graphics::RHITextureFormat::BGRA8_UNorm, Graphics::RHITextureFormat::D24_UNorm_S8, rays, true))
			return false;
	}
	if (!Render_Videos(commands, targets))
		return false;

	if (!Graphics::GetRingRenderer().Render(
		commands,
		targets.backbuffer.texture,
		{0, 0, targets.backbuffer.width, targets.backbuffer.height, 0.0f, 1.0f}))
		return false;

	if (!Graphics::GetFullscreenOverlayRenderer().Render(
		commands,
		targets.backbuffer.texture,
		{0, 0, targets.backbuffer.width, targets.backbuffer.height, 0.0f, 1.0f}))
		return false;

	PROFILER_PLOT("Graphics.UI.Active", static_cast<int64_t>(uiFrameActive));
	PROFILER_PLOT("Graphics.UI.Vertices", static_cast<int64_t>(Graphics::Get_Renderer2D().Vertex_Count()));
	PROFILER_PLOT("Graphics.UI.Batches", static_cast<int64_t>(Graphics::Get_Renderer2D().Batch_Count()));
	return !uiFrameActive || Graphics::Get_Renderer2D().Execute(
		device,
		commands,
		targets.backbuffer.texture,
		targets.depth.texture,
		{0, 0, targets.backbuffer.width, targets.backbuffer.height, 0.0f, 1.0f});
}

static void shutdownGraphicsRenderer() noexcept;

static bool initializeGraphicsRenderer()
{
	if (!W3DAssetRuntime::Initialize())
			return false;

    if (Graphics::Shared_Frame_Device() == nullptr) return false;

	if (Graphics::Register_Frame_Draw_Executor(
		&initializeGraphicsSceneRenderers,
		&executeGraphicsFramePasses))
		return true;

	shutdownGraphicsRenderer();
	return false;
}

static void shutdownGraphicsRenderer() noexcept
{
	if (TheParticleSystemManager != nullptr)
		static_cast<W3DParticleSystemManager *>(TheParticleSystemManager)->Reset_Graphics_Particle_Bindings();
    uiFrameActive = false;
    Shutdown_Video_Presentation();
    W3DBibBuffer::Release_Graphics_Bibs();
    W3DTerrainGraphics::Release_Graphics();
    Release_Graphics_Textures();
    Graphics::Get_Prop_Submission().Clear();
    Graphics::Shutdown_Scene_Renderers();
    Graphics::Detach_Frame_Draw_Executor();
}

static bool beginGraphicsFrame()
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.Frame.Begin");
    if (!Graphics_Begin_Frame()) {
        Graphics_Abort_Frame();
        return false;
    }
    auto* device = Graphics::Shared_Frame_Device();
    const auto resources = device->Get_Swap_Chain().Backbuffer();
	Begin_Video_Frame();
	Graphics::Get_Renderer2D().Begin(resources.width, resources.height);
	uiFrameActive = Graphics::Get_Renderer2D().Is_Initialized();
	return true;
}

static void updateGraphicsView(W3DCamera *camera)
{
	if (camera == nullptr || Graphics::Shared_Frame_Device() == nullptr)
		return;

	Matrix3D camera_view;
	Matrix4x4 projection;
	camera->Get_View_Matrix(&camera_view);
	camera->Get_Backend_Projection_Matrix(&projection);
	const Matrix4x4 view(camera_view);
	const Matrix4x4 viewProjection = projection * view;
	const Matrix4x4 inverseViewProjection = viewProjection.Inverse();
    const Matrix4x4 inverseProjection = projection.Inverse();
	float viewProjectionElements[16] = {};
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			viewProjectionElements[row * 4 + column] = viewProjection[row][column];
			graphicsLightRaysInput.inverse_view_projection[row * 4 + column] = inverseViewProjection[row][column];
            graphicsSSAOInput.projection[row * 4 + column] = projection[row][column];
            graphicsSSAOInput.inverse_projection[row * 4 + column] = inverseProjection[row][column];
		}
	}

	const Vector3 right = camera->Get_Right_Dir();
	const Vector3 up = camera->Get_Up_Dir();
	const Vector3 forward = camera->Get_Forward_Dir();
	for (std::size_t index = 0; index < graphicsBeamView.view_projection.size(); ++index)
		graphicsBeamView.view_projection[index] = viewProjectionElements[index];
	graphicsBeamView.camera_right = {right.X, right.Y, right.Z};
	graphicsBeamView.camera_up = {up.X, up.Y, up.Z};
	graphicsBeamView.camera_forward = {forward.X, forward.Y, forward.Z};
	Graphics::Matrix4x4 graphics_view_matrix;
	Graphics::Matrix4x4 graphics_projection_matrix;
	for (std::size_t row = 0; row < 4; ++row) {
		for (std::size_t column = 0; column < 4; ++column) {
			graphics_view_matrix.values[row * 4 + column] = view[row][column];
			graphics_projection_matrix.values[row * 4 + column] = projection[row][column];
		}
	}
	graphicsParticleView = Graphics::View(
		graphics_view_matrix,
		graphics_projection_matrix,
		{camera->Get_Position().X, camera->Get_Position().Y, camera->Get_Position().Z},
		{0.0f, 0.0f, static_cast<float>(TheDisplay->getWidth()), static_cast<float>(TheDisplay->getHeight()), 0.0f, 1.0f});
	Graphics::GetParticleRenderer().Set_View(graphicsParticleView);
	Graphics::GetWorldQuadRenderer().Set_View(graphicsParticleView);
	Graphics::GetScreenDistortionRenderer().Set_View(graphicsParticleView);
	if (TheParticleSystemManager != nullptr)
		static_cast<W3DParticleSystemManager *>(TheParticleSystemManager)->Set_Graphics_Particle_View(graphicsParticleView);
}

static bool renderGraphicsScenePasses(W3DCamera *camera)
{
	GENERALS_GRAPHICS_PROFILE_SCOPE("W3DDisplay::renderGraphicsScenePasses");
	updateGraphicsView(camera);
	if (!Graphics_Execute_Queued_Draws()) {
		uiFrameActive = false;
		Graphics_Abort_Frame();
		Graphics::Get_Frame_Submission_Statistics().Cancel();
		return false;
	}

	if (!Graphics_End_Frame()) {
		uiFrameActive = false;
		Graphics_Abort_Frame();
		Graphics::Get_Frame_Submission_Statistics().Cancel();
		return false;
	}

	if (displayMovieCapture.Is_Active()) {
		auto* device = Graphics::Shared_Frame_Device();
		if (device == nullptr || !displayMovieCapture.Capture(*device, device->Get_Swap_Chain().Backbuffer(), Graphics::RHITextureFormat::BGRA8_UNorm)) {
			displayMovieCapture.Stop();
			DEBUG_LOG(("Movie capture stopped: frame readback or AVI write failed.\n"));
		}
	}

	if (!Graphics_Present()) {
		uiFrameActive = false;
		Graphics_Abort_Frame();
		Graphics::Get_Frame_Submission_Statistics().Cancel();
		return false;
	}

	if (auto* device = Graphics::Shared_Frame_Device()) {
		auto& commands = device->Immediate_Command_List();
		Graphics::Get_Frame_Submission_Statistics().Complete(&commands, commands.Submission_Counts());
	}
	uiFrameActive = false;
	return true;
}

//*****************************************************************************************
//*****************************************************************************************
//**** Start Statistical Dump *************************************************************
//*****************************************************************************************

#ifdef DUMP_PERF_STATS

#include <cstdarg>

class StatDumpClass
{
public:
	StatDumpClass( const char *fname );
	~StatDumpClass();
	void dumpStats( Bool brief = FALSE, Bool flagSpikes = FALSE );

protected:
	FILE *m_fp;
};

//=============================================================================
//Open the file once at the beginning of the game -- everything appends to it.
//=============================================================================
StatDumpClass::StatDumpClass( const char *fname )
{
	const char *basePath = SDL_GetBasePath();
	const char *path = basePath != nullptr ? basePath : "";
	// TheSuperHackers @fix Caball009 03/06/2025 Don't use AsciiString here anymore because its memory allocator may not have been initialized yet.
	const std::string fullPath = std::string(path) + fname;
	m_fp = fopen(fullPath.c_str(), "wt");
}

//=============================================================================
//Close the file at the end of the application
//=============================================================================
StatDumpClass::~StatDumpClass()
{
	if( m_fp )
	{
		fclose( m_fp );
	}
}

static const char *getCurrentTimeString()
{
	time_t aclock;
	time(&aclock);
	struct tm *newtime = localtime(&aclock);
	return asctime(newtime);
}

//=============================================================================
//Dump the stats
//=============================================================================


static Bool s_notFirstDump = FALSE;

void StatDumpClass::dumpStats( Bool brief, Bool flagSpikes )
{
	if( !m_fp )
	{
		return;
	}


  Bool beBrief = brief & s_notFirstDump;
  s_notFirstDump = TRUE;

	fprintf( m_fp, "----------------------------------------------------------------\n" );
	fprintf( m_fp, "Performance Statistical Dump -- Frame %d\n", TheGameLogic->getFrame() );
  if ( ! beBrief )
  {
	  //static char buf[1024];
	  fprintf( m_fp, "Time:\t%s", getCurrentTimeString() );
	  fprintf( m_fp, "Map:\t%s\n", TheGlobalData->m_mapName.str());
	  fprintf( m_fp, "Side:\t%s\n", ThePlayerList->getLocalPlayer()->getSide().str());
	  fprintf( m_fp, "----------------------------------------------------------------\n" );
  }

	//FPS
	Real fps = TheDisplay->getAverageFPS();
	fprintf( m_fp, "Average FPS: %.1f (%.5f msec)\n", fps, 1000.0f / fps );
  if ( flagSpikes && fps<20.0f )
  	fprintf( m_fp, "                                                                      FPS OUT OF TOLERANCE\n" );


	//Rendering stats
	fprintf( m_fp, "Draws: %llu\n", static_cast<unsigned long long>(Graphics::Get_Frame_Submission_Statistics().Last_Frame().draw_calls));

	Int onScreenParticleCount = TheParticleSystemManager->getOnScreenParticleCount();

  if ( flagSpikes )
  {
    if ( Graphics::Get_Frame_Submission_Statistics().Last_Frame().draw_calls>2000 )
  	  fprintf( m_fp, "                                                                      DRAWS OUT OF TOLERANCE(2000)\n" );
  }


	//Object stats
	UnsignedInt objCount = TheGameLogic->getObjectCount();
	UnsignedInt objScreenCount = TheGameClient->getRenderedObjectCount();
	fprintf( m_fp, "Objects: %d in world (%d onscreen)\n", objCount, objScreenCount );
  if ( flagSpikes && objCount > 800 )
  	fprintf( m_fp, "                                                                      OBJS OUT OF TOLERANCE(800)\n" );

	//AI stats
	UnsignedInt numAI, numMoving, numAttacking, numWaitingForPath, overallFailedPathfinds;
	TheGameLogic->getAIMetricsStatistics( &numAI, &numMoving, &numAttacking, &numWaitingForPath, &overallFailedPathfinds );
	fprintf( m_fp, "\n" );
	fprintf( m_fp, "AI Statistics:\n" );
	fprintf( m_fp, "  Total AI Objects: %d\n", numAI );
	fprintf( m_fp, "    -moving: %d\n", numMoving );
	fprintf( m_fp, "    -attacking: %d\n", numAttacking );
	fprintf( m_fp, "    -waiting for path: %d\n", numWaitingForPath );
	fprintf( m_fp, "  Total failed pathfinds: %d\n", overallFailedPathfinds );
  if ( flagSpikes && overallFailedPathfinds > 0 )
  	fprintf( m_fp, "                                                                      FAILEDPATHFINDS OUT OF TOLERANCE(0)\n" );
	fprintf( m_fp, "\n" );

	// Script stats
	Real timeLastFrame, slowScript1, slowScript2;
	AsciiString slowScripts = TheScriptEngine->getStats(&timeLastFrame, &slowScript1, &slowScript2);
	fprintf( m_fp, "\n" );
	fprintf( m_fp, "Script Engine Statistics:\n" );
	fprintf( m_fp, "  Total time last frame: %.5f msec\n", timeLastFrame*1000 );
	fprintf( m_fp, "    -Slowest 2 scripts      %s\n", slowScripts.str() );
	fprintf( m_fp, "    -Slowest 2 script times %.5f msec, %.5f msec \n", slowScript1*1000, slowScript2*1000 );
  if ( flagSpikes && slowScript1*1000 > 0.2f || slowScript2*1000 > 0.2f )
  	fprintf( m_fp, "                                                                      SLOW SCRIPT OUT OF TOLERANCE(0.2)\n" );
	fprintf( m_fp, "\n" );



	//PartitionMgr stats
	double gcoTimeThisFrameTotal, gcoTimeThisFrameAvg;
	ThePartitionManager->getPMStats(gcoTimeThisFrameTotal, gcoTimeThisFrameAvg);
	fprintf(m_fp, "Partition Manager Statistics:\n");
	fprintf(m_fp, "  Total time for object scans this frame is %.5f msec\n", gcoTimeThisFrameTotal);
	fprintf(m_fp, "  Avg time per object scan this frame is %.5f msec\n", gcoTimeThisFrameAvg);
	fprintf( m_fp, "\n" );

	// setup texture stats

	fprintf( m_fp, "Video Statistics:\n" );
	//Particle system stats
	fprintf( m_fp, "  Particle Systems: %d\n", TheParticleSystemManager->getParticleSystemCount() );
	Int totalParticles = TheParticleSystemManager->getParticleCount();
	fprintf( m_fp, "  Particles: %d in world (%d onscreen)\n", totalParticles, onScreenParticleCount );

  if ( flagSpikes && totalParticles > TheGlobalData->m_maxParticleCount - 10 )
  	fprintf( m_fp, "                                                                      PARTICLES OUT OF TOLERANCE(CAP-10)\n" );
  if ( flagSpikes && onScreenParticleCount > TheGlobalData->m_maxParticleCount - 10 )
  	fprintf( m_fp, "                                                                      ON_SCREEN_PARTICLES OUT OF TOLERANCE(CAP-10)\n" );


	// polygons this frame
	Int polyPerFrame = static_cast<Int>((std::min)(Graphics::Get_Frame_Submission_Statistics().Last_Frame().triangles, std::uint64_t{INT_MAX}));
	Int polyPerSecond = (Int)(polyPerFrame * fps);
	fprintf( m_fp, "  Polygons: %d per frame (%d per second)\n", polyPerFrame, polyPerSecond );

	// vertices this frame
	fprintf( m_fp, "  Submitted vertices/indices: %llu\n", static_cast<unsigned long long>(Graphics::Get_Frame_Submission_Statistics().Last_Frame().vertex_invocations) );


	// terrain stats
	fprintf( m_fp, "  3-Way Blends: %d/%d, \n Shoreline Blends: %d/%d\n", TheTerrainRenderObject->getNumExtraBlendTiles(TRUE),TheTerrainRenderObject->getNumExtraBlendTiles(FALSE), TheTerrainRenderObject->getNumShoreLineTiles(TRUE),TheTerrainRenderObject->getNumShoreLineTiles(FALSE));
  if ( flagSpikes && TheTerrainRenderObject->getNumExtraBlendTiles(TRUE) > 2000 )
  	fprintf( m_fp, "                                                                      3-WAYS OUT OF TOLERANCE(2000)\n" );
  if ( flagSpikes && TheTerrainRenderObject->getNumShoreLineTiles(TRUE) > 2000 )
  	fprintf( m_fp, "                                                                      SHORELINES OUT OF TOLERANCE(2000)\n" );

	fprintf( m_fp, "\n" );

#if defined(RTS_DEBUG)
  if ( ! beBrief )
  {
    TheAudio->audioDebugDisplay( nullptr, nullptr, m_fp );
	  fprintf( m_fp, "\n" );
  }
#endif

#ifdef MEMORYPOOL_DEBUG
	//Report memory usage.
	TheMemoryPoolFactory->debugMemoryReport( REPORT_FACTORYINFO | REPORT_POOLINFO, 0, 0, m_fp );
#else
	fprintf( m_fp, "Memory Report -- unavailable \n(build doesn't have MEMORYPOOL_DEBUG defined)\n" );
#endif
	fprintf( m_fp, "\n" );

	fprintf( m_fp, "%s", TheSubsystemList->dumpTimesForAll().str());

  if ( ! beBrief )
  {
	  fprintf( m_fp, "----------------------------------------------------------------\n" );
	  fprintf( m_fp, "END -- Frame %d\n", TheGameLogic->getFrame() );
	  fprintf( m_fp, "----------------------------------------------------------------\n" );
  }
	fprintf( m_fp, "\n\n" );
	fflush(m_fp);
}

StatDumpClass TheStatDump("StatisticsDump.txt");

#endif //DUMP_PERF_STATS

//*****************************************************************************************
//**** End Statistical Dump ***************************************************************
//*****************************************************************************************
//*****************************************************************************************



///////////////////////////////////////////////////////////////////////////////
// DEFINITIONS ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
RTS3DScene *W3DDisplay::m_3DScene = nullptr;
RTS2DScene *W3DDisplay::m_2DScene = nullptr;
RTS3DInterfaceScene *W3DDisplay::m_3DInterfaceScene = nullptr;
W3DAssetManager *W3DDisplay::m_assetManager = nullptr;

//=============================================================================
	// note, can't use the ones from PerfTimer.h 'cuz they are currently
	// only valid when "-vtune" is used... (srj)
inline Int64 getPerformanceCounter()
{
	Int64 tmp;
	QueryPerformanceCounter((LARGE_INTEGER*)&tmp);
	return tmp;
}

inline Int64 getPerformanceCounterFrequency()
{
	Int64 tmp;
	QueryPerformanceFrequency((LARGE_INTEGER*)&tmp);
	return tmp;
}

// W3DDisplay::W3DDisplay =====================================================
/** */
//=============================================================================
W3DDisplay::W3DDisplay()
{
	Int i;

	m_initialized = false;
	m_assetManager = nullptr;
	m_3DScene = nullptr;
	m_2DScene = nullptr;
	m_3DInterfaceScene = nullptr;
	m_averageFPS = TheGlobalData->m_framesPerSecondLimit;
#if defined(RTS_DEBUG)
	m_timerAtCumuFPSStart = 0;
#endif
	for (i=0; i<Graphics::Material_Light_Count; i++)
		m_myLight[i] = nullptr;
	m_isClippedEnabled = FALSE;
	m_clipRegion.lo.x = 0;
	m_clipRegion.lo.y = 0;
	m_clipRegion.hi.x = 0;
	m_clipRegion.hi.y = 0;

	for (i = 0; i < DisplayStringCount; i++)
		m_displayStrings[i] = nullptr;


}

// W3DDisplay::~W3DDisplay ====================================================
/** */
//=============================================================================
W3DDisplay::~W3DDisplay()
{
	displayMovieCapture.Stop();

	// get rid of the debug display
	delete m_debugDisplay;
	m_debugDisplay = nullptr;
	m_nativeDebugDisplay = nullptr;

	// delete the display strings
	for (int i = 0; i < DisplayStringCount; i++)
		TheDisplayStringManager->freeDisplayString(m_displayStrings[i]);

	// TheSuperHackers @fix Mauller/Tomsons26 28/04/2025 Free benchmark display string
	if( m_benchmarkDisplayString ) {
		TheDisplayStringManager->freeDisplayString(m_benchmarkDisplayString);
	}

	// delete all our views now since they are W3D views and we need to
	// free them BEFORE we shutdown W3D
	//
	Display::deleteViews();

	REF_PTR_RELEASE( m_3DScene );
	REF_PTR_RELEASE( m_2DScene );
	REF_PTR_RELEASE( m_3DInterfaceScene );
	for (Int j=0; j<Graphics::Material_Light_Count; j++)
		REF_PTR_RELEASE( m_myLight[j] );


	// shutdown
	Graphics::Get_Frame_Submission_Statistics().Reset();
	if (!TheGlobalData->m_headless)
		W3DShaderManager::shutdown();
	Shutdown_Dazzle_Resources();
	m_assetManager->Catalog().Free_Assets();
	delete m_assetManager;
	graphicsRendererAvailable = false;
	shutdownGraphicsRenderer();
	W3DAssetRuntime::Shutdown();
	if (!TheGlobalData->m_headless)
		Get_W3D_Render_Services().Shutdown();
		Graphics::Graphics_Shutdown_Shared_Frame();
	WWMath::Shutdown();
	if (!TheGlobalData->m_headless)
	delete TheW3DFileSystem;
	TheW3DFileSystem = nullptr;

}

// TheSuperHackers @tweak valeronm 20/03/2025 No longer filters resolutions by a 4:3 aspect ratio.
Int W3DDisplay::getDisplayModeCount()
{
    m_displayResolutions = Graphics::Enumerate_Display_Resolutions(SDLPlatformWindow::window());
    std::erase_if(m_displayResolutions,[](const auto& resolution) {
        return resolution.width < DEFAULT_DISPLAY_WIDTH;
    });
    return static_cast<Int>(m_displayResolutions.size());
}

void W3DDisplay::getDisplayModeDescription(Int modeIndex, Int *xres, Int *yres, Int *bitDepth)
{
    if (m_displayResolutions.empty()) getDisplayModeCount();
    if (modeIndex < 0 || static_cast<std::size_t>(modeIndex) >= m_displayResolutions.size()) return;
    const auto& resolution = m_displayResolutions[modeIndex];
    *xres = resolution.width;
    *yres = resolution.height;
    *bitDepth = 32;
}

void W3DDisplay::setGamma(Real gamma, Real bright, Real contrast, Bool calibrate)
{
    // The frame pipeline currently has no display color-transform pass.
    // The removed backend hook was empty; keep the existing display contract.
}

static Bool setSDLWindowed(Bool windowed)
{
	if (SDLPlatformWindow::window() == nullptr)
		return TRUE;

	return SDLPlatformWindow::setFullscreen(windowed ? false : true) ? TRUE : FALSE;
}

static void resizeSDLWindow(UnsignedInt xres, UnsignedInt yres, Bool windowed)
{
	SDL_Window *window = static_cast<SDL_Window *>(SDLPlatformWindow::window());
	if (window == nullptr || !windowed ||
		SDLPlatformWindow::isFullscreen())
		return;

	int pixelWidth = 0;
	int pixelHeight = 0;
	if (!SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight))
		return;

	if (pixelWidth == static_cast<int>(xres) && pixelHeight == static_cast<int>(yres))
		return;

	// SDL window coordinates are logical points. The renderer resolution is in
	// drawable pixels, so convert before asking SDL to resize a high-DPI window.
	float pixelDensity = SDL_GetWindowPixelDensity(window);
	if (pixelDensity <= 0.0f)
		pixelDensity = 1.0f;
	const int windowWidth = static_cast<int>(static_cast<float>(xres) / pixelDensity + 0.5f);
	const int windowHeight = static_cast<int>(static_cast<float>(yres) / pixelDensity + 0.5f);
	if (windowWidth <= 0 || windowHeight <= 0)
		return;

	if (SDL_SetWindowSize(window, windowWidth, windowHeight))
		SDL_SyncWindow(window);
}

/** Set resolution of display */
//=============================================================================
Bool W3DDisplay::setDisplayMode( UnsignedInt xres, UnsignedInt yres, UnsignedInt bitdepth, Bool windowed )
{
	const UnsignedInt oldWidth = getWidth();
	const UnsignedInt oldHeight = getHeight();
	const UnsignedInt oldBitDepth = getBitDepth();
	const Bool oldWindowed = getWindowed();

	// SDL owns window styles and logical size; graphics owns the swap chain.
	if (!setSDLWindowed(windowed))
		return FALSE;
	resizeSDLWindow(xres, yres, windowed);
	graphicsRendererAvailable = false;
	shutdownGraphicsRenderer();
	if (Graphics::Resize_Frame_Device(xres, yres, false))
	{
		graphicsRendererAvailable = initializeGraphicsRenderer();
		if (graphicsRendererAvailable) {
			Display::setDisplayMode(xres, yres, bitdepth, windowed);
			return TRUE;
		}
	}

	//set back to the original mode.
	setSDLWindowed(oldWindowed);
	resizeSDLWindow(oldWidth, oldHeight, oldWindowed);
	Graphics::Resize_Frame_Device(oldWidth, oldHeight, false);
	graphicsRendererAvailable = initializeGraphicsRenderer();
	Display::setDisplayMode(oldWidth, oldHeight, oldBitDepth, oldWindowed);
	return FALSE;	//did not change to a new mode.
}

/** Set width of display */
//=============================================================================
void W3DDisplay::setWidth(UnsignedInt width)
{
	Display::setWidth(width);
}

// W3DDisplay::setHeight ======================================================
/** Set height of display */
//=============================================================================
void W3DDisplay::setHeight(UnsignedInt height)
{
	Display::setHeight(height);
}

void W3DDisplay::onBeginBatch()
{
}

void W3DDisplay::onEndBatch()
{
}

void W3DDisplay::onFlush()
{
}

// W3DDisplay::initAssets =====================================================
/** */
//=============================================================================
void W3DDisplay::initAssets()
{

}

// W3DDisplay::init3DScene ====================================================
/** */
//=============================================================================
void W3DDisplay::init3DScene()
{

}

// W3DDisplay::init2DScene ====================================================
/** This is the 2D scene, you can use it to draw on a 2D plane over the
	* 3D background */
//=============================================================================
void W3DDisplay::init2DScene()
{

}

// W3DDisplay::init ===========================================================
/** Initialize or re-initialize the W3D display system.  Here we need to
  * create our window, and get our 3D hardware setup and online */
//=============================================================================
void W3DDisplay::init()
{

	//
	// call our base class init, this method should be able to handle re-entry
	// with its own logic
	//
	Display::init();

	// handle re-entry for ourselves
	if( m_initialized )
	{

		/// @todo W3DDisplay needs RE-init logic!
		return;

	}
	// Override the W3D File system
	TheW3DFileSystem = NEW W3DFileSystem;

	// init the Westwood math library
	WWMath::Init();

	if (!TheGlobalData->m_headless)
	{

		// create our 3D interface scene
		m_3DInterfaceScene = NEW_REF( RTS3DInterfaceScene, () );
		m_3DInterfaceScene->Set_Ambient_Light( Vector3( 1, 1, 1 ) );

		// create our 2D scene
		m_2DScene = NEW_REF( RTS2DScene, () );
		m_2DScene->Set_Ambient_Light( Vector3( 1, 1, 1 ) );

		// create our 3D scene
		m_3DScene =NEW_REF( RTS3DScene, () );
	#if defined(RTS_DEBUG)
		if( TheGlobalData->m_wireframe )
			m_3DScene->Set_Polygon_Mode( W3DScene::LINE );
	#endif
	//============================================================================
		// m_myLight = NEW_REF
	//============================================================================
		Int lindex;
		for (lindex=0; lindex<TheGlobalData->m_numGlobalLights; lindex++)
		{	m_myLight[lindex] = NEW_REF( W3DLight, (W3DLight::DIRECTIONAL) );
		}

		setTimeOfDay( TheGlobalData->m_timeOfDay );	//set each light to correct values for given time

		for (lindex=0; lindex<TheGlobalData->m_numGlobalLights; lindex++)
		{	m_3DScene->setGlobalLight( m_myLight[lindex], lindex );
		}

	#ifdef SAMPLE_DYNAMIC_LIGHT
		theDynamicLight = NEW_REF(W3DDynamicLight, ());
		Real red = 1;
		Real green = 1;
		Real blue = 0;
		if(red==0 && blue==0 && green==0) {
			red = green = blue = 1;
		}
		theDynamicLight->Set_Ambient( Vector3( red, green, blue ) );
		theDynamicLight->Set_Diffuse( Vector3( red, green, blue) );
		theDynamicLight->Set_Position(Vector3(0, 0, 4));
		theDynamicLight->Set_Far_Attenuation_Range(1, 8);
		// Note: Don't Add_Render_Object dynamic lights.
		m_3DScene->addDynamicLight( theDynamicLight );
	#endif

	}

	// create a new asset manager
	m_assetManager = NEW W3DAssetManager;
	m_assetManager->Catalog().Install_Reserved_Model_Factory(
		std::unique_ptr<Graphics::ModelFactory<W3DRenderObject>>(
			Create_Null_Render_Object_Factory()));
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkDazzle, Load_Dazzle_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkMesh, Load_Mesh_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkNullObject, Load_Null_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkEmitter,Load_ParticleEmitter_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkAggregate,Load_Aggregate_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkHModel,Load_HModel_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkCollection,Load_Collection_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkHlod,Load_HLod_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkLodModel,Load_ModelLevels_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkBox,Load_Collision_Box_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkRing,Load_Ring_Factory);
	m_assetManager->Catalog().Register_Model_Decoder(Assets::W3D::W3DChunkSphere,Load_Sphere_Factory);
	m_assetManager->Catalog().Set_Load_On_Demand(true);

	if (!TheGlobalData->m_headless)
	{

		if (TheGlobalData->m_incrementalAGPBuf)
		{
		}
		Graphics::Get_Render_Diagnostics() = {};
		Graphics::Get_Texture_Quality_Settings().prefer_16_bits = true;
		if (!Get_W3D_Render_Services().Initialize())
			throw ERROR_INVALID_D3D;	//failed to initialize.  User probably doesn't have DX 8.1

		if (!Initialize_Dazzle_Resources()) throw ERROR_INVALID_D3D;

		Graphics::Get_Render_Settings().Set_Prelit_Mode(Graphics::RenderPrelitMode::LightmapMultiPass);
		Graphics::Set_Collision_Box_Display_Mask(0x00);	///<set to 0xff to make collision boxes visible
		Graphics::Get_Scene_Draw_Queue().Set_Enabled(true);

		setWindowed( TheGlobalData->m_windowed );

		// create a 2D renderer helper

        setWidth(TheGlobalData->m_xResolution);
        setHeight(TheGlobalData->m_yResolution);
        setBitDepth(DEFAULT_DISPLAY_BIT_DEPTH);
        bool device_ready = false;
        for (unsigned attempt = 0; attempt < 2 && !device_ready; ++attempt) {
            if (attempt) {
                Int width = DEFAULT_DISPLAY_WIDTH, height = DEFAULT_DISPLAY_HEIGHT;
                Int bits = DEFAULT_DISPLAY_BIT_DEPTH;
                const Int count = getDisplayModeCount();
                for (Int mode = 0; mode < count; ++mode) {
                    getDisplayModeDescription(mode,&width,&height,&bits);
                    if (width * height >= DEFAULT_DISPLAY_WIDTH * DEFAULT_DISPLAY_HEIGHT) break;
                }
                setWidth(width); setHeight(height);
                TheWritableGlobalData->m_xResolution = width;
                TheWritableGlobalData->m_yResolution = height;
            }
            resizeSDLWindow(getWidth(),getHeight(),getWindowed());
            Graphics::FrameDeviceOptions options;
            options.window = SDLPlatformWindow::nativeHandle();
            options.width = getWidth(); options.height = getHeight();
            options.backbuffer_format = Graphics::RHITextureFormat::BGRA8_UNorm;
            device_ready = Graphics::Initialize_Frame_Device(options);
        }
        if (!device_ready) {
            Get_W3D_Render_Services().Shutdown();
            Graphics::Graphics_Shutdown_Shared_Frame();
            WWMath::Shutdown();
            throw ERROR_INVALID_D3D;
        }
        // Preserve the serialized preference; the frame targets currently use one sample.
        const auto samples = TheWritableGlobalData->m_antiAliasLevel;
        if (samples != 2 && samples != 4 && samples != 8) TheWritableGlobalData->m_antiAliasLevel = 0;
        Graphics::Set_Texture_Sampling_Mode(TheWritableGlobalData->m_textureFilteringMode);
        TheWritableGlobalData->m_textureFilteringMode = static_cast<unsigned>(Graphics::Get_Texture_Sampling_Settings().mode);
        Graphics::Set_Texture_Anisotropy(TheWritableGlobalData->m_textureAnisotropyLevel);
        TheWritableGlobalData->m_textureAnisotropyLevel = Graphics::Get_Texture_Sampling_Settings().anisotropy;
		Graphics::Get_Texture_Quality_Settings().prefer_16_bits = getBitDepth() == 16;
		graphicsRendererAvailable = initializeGraphicsRenderer();
		if (!graphicsRendererAvailable) {
			Get_W3D_Render_Services().Shutdown();
			Graphics::Graphics_Shutdown_Shared_Frame();
			WWMath::Shutdown();
			throw ERROR_INVALID_D3D;
		}

		//Check if level was never set and default to setting most suitable for system.
		if (TheGameLODManager->getStaticLODLevel() == STATIC_GAME_LOD_UNKNOWN)
		{
			TheGameLODManager->setStaticLODLevel(TheGameLODManager->getRecommendedStaticLODLevel());
		}
		else
		{
			//Static LOD level was applied during GameLOD manager init except for texture reduction
			//which needs to be applied here.
			TheGameClient->setTextureLOD(TheWritableGlobalData->m_textureReductionFactor);
		}

		if (TheGlobalData->m_displayGamma != 1.0f)
			setGamma(TheGlobalData->m_displayGamma,0.0f,1.0f,FALSE);
	}

	initAssets();

	if (!TheGlobalData->m_headless)
	{
		init2DScene();
		init3DScene();
		W3DShaderManager::init();

		// Create and initialize the debug display
		m_nativeDebugDisplay = NEW W3DDebugDisplay();
		m_debugDisplay = m_nativeDebugDisplay;
		if ( m_nativeDebugDisplay )
		{
			m_nativeDebugDisplay->init();
			GameFont *font;

			if (TheGlobalLanguageData && TheGlobalLanguageData->m_nativeDebugDisplay.name.isNotEmpty())
			{
				font=TheFontLibrary->getFont(
					TheGlobalLanguageData->m_nativeDebugDisplay.name,
					TheGlobalLanguageData->m_nativeDebugDisplay.size,
					TheGlobalLanguageData->m_nativeDebugDisplay.bold);
			}
			else
				font=TheFontLibrary->getFont( "FixedSys", 8, FALSE );

			m_nativeDebugDisplay->setFont( font );
			m_nativeDebugDisplay->setFontHeight( 13 );
			m_nativeDebugDisplay->setFontWidth( 9 );
		}

	}

	// we're now online
	m_initialized = true;
	if( TheGlobalData->m_displayDebug )
	{
		m_debugDisplayCallback = StatDebugDisplay;
	}
}

// W3DDisplay::reset ===========================================================
/** Reset the W3D display system.  Here we need to
  * remove the objects from the previous map. */
//=============================================================================
void W3DDisplay::reset()
{

	Display::reset();

	// Remove all render objects.

	if (m_3DScene != nullptr)
	{
		W3DSceneIterator *sceneIter = m_3DScene->Create_Iterator();
		sceneIter->First();
		while(!sceneIter->Is_Done()) {
			W3DRenderObject * robj = sceneIter->Current_Item();
			robj->Add_Ref();
			m_3DScene->Remove_Render_Object(robj);
			robj->Release_Ref();
			sceneIter->Next();
		}
		m_3DScene->Destroy_Iterator(sceneIter);
	}

	m_isClippedEnabled = FALSE;

	// release any unused assets from W3D
	/// @todo really need that "scene abstraction", having this stuff in the display is icky
		m_assetManager->Catalog().Release_Unused_Assets();

	if (TheWritableGlobalData)
		TheWritableGlobalData->m_drawSkyBox =0;
}

void W3DDisplay::update()
{
	// Display::update has no playback responsibilities; video timing is serviced by GameClient.
	// playback is updated by GameClient before the display update.
}

const UnsignedInt START_CUMU_FRAME = LOGICFRAMES_PER_SECOND / 2;	// skip first half-sec

void W3DDisplay::updateAverageFPS()
{
	constexpr const Int FPS_HISTORY_SIZE = 30;

	static Int64 lastUpdateTime64 = 0;
	static Int historyOffset = 0;
	static Real fpsHistory[FPS_HISTORY_SIZE] = {0};

	const Int64 freq64 = getPerformanceCounterFrequency();
	const Int64 time64 = getPerformanceCounter();

#if defined(RTS_DEBUG)
	if (TheGameLogic->getFrame() == START_CUMU_FRAME)
	{
		m_timerAtCumuFPSStart = time64;
	}
#endif

	const Int64 timeDiff = time64 - lastUpdateTime64;

	// convert elapsed time to seconds
	Real elapsedSeconds = (Real)timeDiff/(Real)freq64;

	// append new sample to fps history.
	if (historyOffset >= FPS_HISTORY_SIZE)
		historyOffset = 0;

	m_currentFPS = 1.0f/elapsedSeconds;
	fpsHistory[historyOffset++] = m_currentFPS;

	// determine average frame rate over our past history.
	const Real sum = std::accumulate(fpsHistory, fpsHistory + FPS_HISTORY_SIZE, 0.0f);
	m_averageFPS = sum / FPS_HISTORY_SIZE;

	lastUpdateTime64 = time64;
}

#if defined(RTS_DEBUG)	//debug hack to view object under mouse stats
ICoord2D TheMousePos;
#endif

// W3DDisplay::gatherDebugStats ===================================================
/** Compute and display debug stats on screen */
//=============================================================================
void W3DDisplay::gatherDebugStats()
{
	static UnsignedInt s_framesRenderedSinceLastUpdate = 0;
	static Int64 s_lastUpdateTime64 = 0;
	static double s_timeSinceLastUpdateInSecs = 0.0;

	// allocate the display strings if needed
	if( m_displayStrings[0] == nullptr )
	{
		GameFont *font;
		if (TheGlobalLanguageData && TheGlobalLanguageData->m_nativeDebugDisplay.name.isNotEmpty())
		{
			font=TheFontLibrary->getFont(
				TheGlobalLanguageData->m_nativeDebugDisplay.name,
				TheGlobalLanguageData->m_nativeDebugDisplay.size,
				TheGlobalLanguageData->m_nativeDebugDisplay.bold);
		}
		else
			font = TheFontLibrary->getFont( "FixedSys", 8, FALSE );

		for (int i = 0; i < DisplayStringCount; i++)
		{
			if (m_displayStrings[i] == nullptr)
			{
				m_displayStrings[i] = TheDisplayStringManager->newDisplayString();
				DEBUG_ASSERTCRASH( m_displayStrings[i], ("Failed to create DisplayString") );
				m_displayStrings[i]->setFont( font );
			}
		}

	}

	if (m_benchmarkDisplayString == nullptr)
	{
		GameFont *thisFont = TheFontLibrary->getFont( "FixedSys", 8, FALSE );
		m_benchmarkDisplayString = TheDisplayStringManager->newDisplayString();
		DEBUG_ASSERTCRASH( m_benchmarkDisplayString, ("Failed to create DisplayString") );
		m_benchmarkDisplayString->setFont( thisFont );
	}

	++s_framesRenderedSinceLastUpdate;

	Int64 freq64 = getPerformanceCounterFrequency();
	Int64 time64 = getPerformanceCounter();

	s_timeSinceLastUpdateInSecs = ((double)(time64 - s_lastUpdateTime64) / (double)(freq64));

#ifdef EXTENDED_STATS
		static FILE *pListFile = nullptr;
		static Int64 lastFrameTime=0;
		static samples = 0;
		if (pListFile == nullptr) {
			pListFile = fopen("FrameRateLog.txt", "w");
		}
		samples++;
		if (pListFile && lastFrameTime && samples<100) {
			float timeSinceLastFrame = (float)((double)(time64-lastFrameTime) / (double)(freq64));
			fprintf(pListFile, "%d ", (int)(1/timeSinceLastFrame));
		}
		lastFrameTime = time64;
#endif

	// we update stats on a delay
	const Real UPDATE_RATE_SECS = 2.0;
	if( s_timeSinceLastUpdateInSecs >= UPDATE_RATE_SECS || TheGlobalData->m_constantDebugUpdate )
	{
		UnicodeString unibuffer, unibuffer2;
		UnicodeString fpsString;

		// setup texture stats

		// frames per second
		double fps = (Real)s_framesRenderedSinceLastUpdate / s_timeSinceLastUpdateInSecs;
		const double drawsPerFrame = static_cast<double>(Graphics::Get_Frame_Submission_Statistics().Last_Frame().draw_calls);

		if (fps<0.1) fps = 0.1;

		double ms = 1000.0f/fps;


#if defined(RTS_DEBUG)
		double cumuTime = ((double)(time64 - m_timerAtCumuFPSStart) / (double)(freq64));
		if (cumuTime < 0.0) cumuTime = 0.0;
		Int numFrames = (Int)TheGameLogic->getFrame() - (Int)START_CUMU_FRAME;
		double cumuFPS = (numFrames > 0 && cumuTime > 0.0) ? (numFrames / cumuTime) : 0.0;

		Int LOD = TheGlobalData->m_terrainLOD;
		if (TheGlobalData->m_useFpsLimit)
				unibuffer.format( L"%.2f/%d FPS, ", fps, TheFramePacer->getFramesPerSecondLimit());
		else
				unibuffer.format( L"%.2f FPS, ", fps);

		unibuffer2.format( L"%.2fms [cumuFPS=%.2f] draws: %.0f LOD %d", ms, cumuFPS, drawsPerFrame, LOD);
		unibuffer.concat(unibuffer2);
#else
		//Int LOD = TheGlobalData->m_terrainLOD;
		unibuffer.format( L"FPS: %.2f, %.2fms draws: %.0f", fps, ms, drawsPerFrame);
		if (TheGlobalData->m_useFpsLimit)
		{
			unibuffer2.format(L", FPSLock %d",TheGlobalData->m_framesPerSecondLimit);
			unibuffer.concat(unibuffer2);
		}
#endif

		fpsString.format( L"FPS: %.2f", fps);
		m_benchmarkDisplayString->setText( fpsString );

		Int polyPerFrame = static_cast<Int>((std::min)(Graphics::Get_Frame_Submission_Statistics().Last_Frame().triangles, std::uint64_t{INT_MAX}));

#ifdef EXTENDED_STATS
		static float gameOverheadMS = 0.0f;
		static float consoleMS = 0.0f;
		static float threeDOverheadMS = 0.0f;
		static float terrainMS = 0.0f;
		static float objectMS = 0.0f;
		static float overlapMS = 0.0f;
		static int  extendedStats = 0;
		const int SHOW_STATS_TIME=12; // show extended stats for 5 cycles == 10 seconds.
		static enum {disabled, sync, gameOverhead, console, threeDOverhead, terrain, objects, overlap, normal} statMode = disabled;

		if (statMode == sync) {
			extendedStats = SHOW_STATS_TIME;
			statMode = gameOverhead;
		} else if (statMode == gameOverhead) {
			gameOverheadMS = ms;
			statMode = console;
			Graphics::Get_Render_Diagnostics().disable_overhead = true;
			Graphics::Get_Render_Diagnostics().disable_water = true;
			Graphics::Get_Render_Diagnostics().disable_objects = true;
			Graphics::Get_Render_Diagnostics().disable_console = false;
			Graphics::Get_Render_Diagnostics().console_line_limit = 1;
		} else if (statMode == console) {
			consoleMS = ms;
			statMode = threeDOverhead;
			Graphics::Get_Render_Diagnostics().disable_overhead = true;
			Graphics::Get_Render_Diagnostics().disable_water = true;
			Graphics::Get_Render_Diagnostics().disable_objects = true;
			Graphics::Get_Render_Diagnostics().disable_console = true;
			Graphics::Get_Render_Diagnostics().console_line_limit = 1;
		} else if (statMode == threeDOverhead) {
			threeDOverheadMS = ms;
			statMode = terrain;
			Graphics::Get_Render_Diagnostics().disable_overhead = true;
			Graphics::Get_Render_Diagnostics().disable_water = true;
			Graphics::Get_Render_Diagnostics().disable_objects = true;
			Graphics::Get_Render_Diagnostics().disable_console = true;
			Graphics::Get_Render_Diagnostics().console_line_limit = 1;
		} else if (statMode == terrain) {
			terrainMS = ms;
			statMode = objects;
			Graphics::Get_Render_Diagnostics().disable_overhead = true;
			Graphics::Get_Render_Diagnostics().disable_water = true;
			Graphics::Get_Render_Diagnostics().disable_objects = false;
			Graphics::Get_Render_Diagnostics().disable_console = true;
			Graphics::Get_Render_Diagnostics().console_line_limit = 1;
		} else if (statMode == objects) {
			objectMS = ms;
			statMode = overlap;
			Graphics::Get_Render_Diagnostics().disable_overhead = false;
			Graphics::Get_Render_Diagnostics().disable_water = false;
			Graphics::Get_Render_Diagnostics().disable_objects = false;
			Graphics::Get_Render_Diagnostics().disable_console = true;
			Graphics::Get_Render_Diagnostics().console_line_limit = 1;
		} else if (statMode == overlap) {
			overlapMS = ms;
			statMode = normal;
			Graphics::Get_Render_Diagnostics().disable_overhead = false;
			Graphics::Get_Render_Diagnostics().disable_water = false;
			Graphics::Get_Render_Diagnostics().disable_objects = false;
			Graphics::Get_Render_Diagnostics().disable_console = true;
			Graphics::Get_Render_Diagnostics().console_line_limit = 1;
		} else if (statMode == normal) {
			overlapMS = (ms + ((int)terrainMS) - overlapMS );
			statMode = disabled;
			extendedStats = SHOW_STATS_TIME;

			// Done collecting stats. Re-enable stuff
			Graphics::Get_Render_Diagnostics().disable_console = false;
			Graphics::Get_Render_Diagnostics().console_line_limit = -1;
		} else if (!Graphics::Get_Render_Diagnostics().collecting_statistics) {
			// start collecting extended info.
			Graphics::Get_Render_Diagnostics().collecting_statistics = true;
			Graphics::Get_Render_Diagnostics().disable_overhead = false;
			Graphics::Get_Render_Diagnostics().disable_water = true;
			Graphics::Get_Render_Diagnostics().disable_objects = true;
			Graphics::Get_Render_Diagnostics().disable_console = true;
			Graphics::Get_Render_Diagnostics().console_line_limit = 1;
			statMode = sync;
			gameOverheadMS = 0.0f;
			threeDOverheadMS = 0.0f;
			terrainMS = 0.0f;
			objectMS = 0.0f;
		}
		if (statMode != disabled) {
			unibuffer.format(L"FPS: %.2f, %.2fms - Collecting extended stats.", fps, ms);
		} else if (extendedStats>0) {
			extendedStats--;
			unibuffer.format( L"FPS: %.2f, %.2fms - OH %.2fms, Console %.2fms, 3D OH %.2fms, Terrain %.2fms, Obs %.2fms, CPU %.2fms",
				fps, ms, gameOverheadMS, consoleMS, threeDOverheadMS, terrainMS, objectMS, overlapMS);
			if (extendedStats==SHOW_STATS_TIME-2) {
				char bufferA[ 256 ];
				sprintf( bufferA, "FPS: %.2f, %.2fms - OH %.2fms, Console %.2fms, 3D OH %.2fms, Terrain %.2fms, Obs %.2fms, CPU %.2fms\n",
					fps, ms, gameOverheadMS, consoleMS, threeDOverheadMS, terrainMS, objectMS, overlapMS);
				SDL_Log("%s", bufferA);
				if (pListFile) {
					fprintf(pListFile, "\n%s", bufferA);
				}
				sprintf( bufferA, "Polygons: per frame %d, per second %d\n", polyPerFrame,
						(Int)(polyPerFrame*fps));
				SDL_Log("%s", bufferA);
				if (pListFile) {
					fprintf(pListFile, "%s", bufferA);
					fflush(pListFile);
				}
			}
		}
 		if (pListFile) {
			fprintf(pListFile, "\nFPS: %.2f, %.2fms\n", fps, ms);
			fflush(pListFile);
		}
		if (pListFile) {
			samples = 0;
			if (statMode != disabled) {
				fprintf(pListFile, "Stat%d-", statMode);
			}
		}

#endif
#ifdef RTS_DEBUG
		unibuffer.concat(L", DEBUG app");
#endif

		m_displayStrings[FPS]->setText( unibuffer );

		// Actual GameLogic frame number
		unibuffer.format(L"Frame: %d", TheGameLogic->getFrame());
		m_displayStrings[Frame]->setText( unibuffer );

		// polygons this frame
		unibuffer.format( L"Polygons: per frame %d, per second %d", polyPerFrame,
				(Int)(polyPerFrame*fps));
		m_displayStrings[Polygons]->setText( unibuffer );

		// vertices this frame
		unibuffer.format( L"Submitted vertices/indices: %llu", static_cast<unsigned long long>(Graphics::Get_Frame_Submission_Statistics().Last_Frame().vertex_invocations) );
		m_displayStrings[Vertices]->setText( unibuffer );

		m_displayStrings[VideoRam]->setText( UnicodeString::TheEmptyString );

		s_lastUpdateTime64 = time64;
		s_timeSinceLastUpdateInSecs = 0.0f;
		s_framesRenderedSinceLastUpdate = 0;

		// terrain stats
		unibuffer.format( L"3-Way Blends: %d/%d, Shoreline Blends: %d/%d", TheTerrainRenderObject->getNumExtraBlendTiles(TRUE),
			TheTerrainRenderObject->getNumExtraBlendTiles(FALSE),
			TheTerrainRenderObject->getNumShoreLineTiles(TRUE),
			TheTerrainRenderObject->getNumShoreLineTiles(FALSE));
		m_displayStrings[TerrainStats]->setText( unibuffer );

		// misc debug info
		Coord3D camPos = TheTacticalView->getPosition();
		Real zoom = TheTacticalView->getZoom();
		Real pitch = TheTacticalView->getPitch();
		Real FXPitch = TheTacticalView->getFXPitch();
		Real angle = TheTacticalView->getAngle();
		Real FOV = TheTacticalView->getFieldOfView();
		Real terrainHeight = TheTacticalView->getTerrainHeightAtPivot();
		Real actualHeightAboveGround = TheTacticalView->getCurrentHeightAboveGround();

		unibuffer.format(
			L"Camera zoom: %.3f, pitch: %.2f, FXpitch: %.2f, yaw: %.2f, pos: (%.2f, %.2f, %.2f), FOV: %.2f\n"
			L"Height above ground: %.2f, Terrain height at camera pivot: %.2f",
			zoom,
			RAD_TO_DEGF(pitch),
			RAD_TO_DEGF(FXPitch),
			RAD_TO_DEGF(angle),
			camPos.x, camPos.y, camPos.z,
			RAD_TO_DEGF(FOV),
			actualHeightAboveGround, terrainHeight );

		m_displayStrings[DebugInfo]->setText( unibuffer );

		// display the keyboard modifier and mouse states.
		unibuffer.format( L"States: " );
		if( TheKeyboard->isShift() )
		{
			unibuffer.concat( L"Shift(" );
			if( TheKeyboard->getModifierFlags() & KEY_STATE_LSHIFT )
			{
				unibuffer.concat( L"L" );
			}
			if( TheKeyboard->getModifierFlags() & KEY_STATE_RSHIFT )
			{
				unibuffer.concat( L"R" );
			}
			unibuffer.concat( L") " );
		}
		if( TheKeyboard->isCtrl() )
		{
			unibuffer.concat( L"Ctrl(" );
			if( TheKeyboard->getModifierFlags() & KEY_STATE_LCONTROL )
			{
				unibuffer.concat( L"L" );
			}
			if( TheKeyboard->getModifierFlags() & KEY_STATE_RCONTROL )
			{
				unibuffer.concat( L"R" );
			}
			unibuffer.concat( L") " );
		}
		if( TheKeyboard->isAlt() )
		{
			unibuffer.concat( L"Alt(" );
			if( TheKeyboard->getModifierFlags() & KEY_STATE_LALT )
			{
				unibuffer.concat( L"L" );
			}
			if( TheKeyboard->getModifierFlags() & KEY_STATE_RALT )
			{
				unibuffer.concat( L"R" );
			}
			unibuffer.concat( L") " );
		}

		const MouseIO *mouseStatus = TheMouse->getMouseStatus();

		if( mouseStatus->leftState )
		{
			unibuffer.concat( L"LMB " );
		}
		if( mouseStatus->middleState )
		{
			unibuffer.concat( L"MMB " );
		}
		if( mouseStatus->rightState )
		{
			unibuffer.concat( L"RMB " );
		}

		Object *object = nullptr;
#if defined(RTS_DEBUG)	//debug hack to view object under mouse stats
		Drawable *draw = 	TheTacticalView->pickDrawable(&TheMousePos, FALSE, (PickType)0xffffffff );
#else
		Drawable *draw = TheGameClient->findDrawableByID( TheInGameUI->getMousedOverDrawableID() );
#endif
		if( draw  )
			object = draw->getObject();
		if( object )
		{
			unibuffer2.format( L"Moused over object: %S (%d) ", object->getTemplate()->getName().str(), object->getID() );
			unibuffer.concat( unibuffer2 );
		}
		else
		{
			unibuffer.concat( L"Moused over object: TERRAIN " );
		}

		m_displayStrings[ KEY_MOUSE_STATES ]->setText( unibuffer );

		//display the x and y mouse coordinates
		const MouseIO *mouseIO = TheMouse->getMouseStatus();
		Coord3D worldPos;
		if( TheTacticalView->screenToTerrain(&mouseIO->pos, &worldPos) )
		{
			unibuffer.format( L"Mouse position: screen: (%d, %d), world: (%g, %g, %g)",
				mouseIO->pos.x, mouseIO->pos.y, worldPos.x, worldPos.y, worldPos.z);
		}
		else
		{
			unibuffer.format( L"Mouse position: screen: (%d, %d), world: none",
				mouseIO->pos.x, mouseIO->pos.y);
		}
		m_displayStrings[MousePosition]->setText( unibuffer );

		//display the number of particles in the world and being displayed on screen
		Int totalParticles = TheParticleSystemManager->getParticleCount();
		Int onScreenParticleCount = TheParticleSystemManager->getOnScreenParticleCount();
		unibuffer.format( L"Particles: %d in world, %d being displayed", totalParticles, onScreenParticleCount );
		m_displayStrings[Particles]->setText( unibuffer );

		//display the number of objects in the world
		UnsignedInt objCount = TheGameLogic->getObjectCount();
		UnsignedInt objScreenCount = TheGameClient->getRenderedObjectCount();

		unibuffer.format(L"Objects: %d in world, %d being displayed", objCount, objScreenCount );
		m_displayStrings[Objects]->setText( unibuffer );

		// Network incoming bandwidth stats
		if (TheNetwork != nullptr) {
			unibuffer.format(L"IN: %.2f bytes/sec, %.2f packets/sec",
				TheNetwork->getIncomingBytesPerSecond(), TheNetwork->getIncomingPacketsPerSecond());
			m_displayStrings[NetIncoming]->setText( unibuffer );

			// Network outgoing bandwidth stats
			unibuffer.format(L"OUT: %.2f bytes/sec, %.2f packets/sec",
				TheNetwork->getOutgoingBytesPerSecond(), TheNetwork->getOutgoingPacketsPerSecond());
			m_displayStrings[NetOutgoing]->setText( unibuffer );

			// Network performance stats
			unibuffer.format(L"Run Ahead: %d, Net FPS: %d, Packet arrival cushion: %d",
				TheNetwork->getRunAhead(), TheNetwork->getFrameRate(), TheNetwork->getPacketArrivalCushion());
			m_displayStrings[NetStats]->setText( unibuffer );

			// Client frame rate averages for all players in the game.  This only works right for the packet router.
			unibuffer.clear();
			Int numPlayers = TheNetwork->getNumPlayers();
			for (Int i = 0; i < numPlayers; ++i) {
				UnicodeString tempstr;
				tempstr.format(L"%s: %d ", TheNetwork->getPlayerName(i).str(), TheNetwork->getSlotAverageFPS(i));
				unibuffer.concat(tempstr);
			}
			m_displayStrings[NetFPSAverages]->setText( unibuffer );
		} else {
//			unibuffer.format(L"IN: 0.0 bytes/sec, 0.0 packets/sec");
//			m_displayStrings[NetIncoming]->setText( unibuffer );

			// Network outgoing bandwidth stats
//			unibuffer.format(L"OUT: 0.0 bytes/sec, 0.0 packets/sec");
//			m_displayStrings[NetOutgoing]->setText( unibuffer );
			unibuffer.clear();
//			unibuffer.format(L"Network not present");
			m_displayStrings[NetOutgoing]->setText(unibuffer);
			m_displayStrings[NetIncoming]->setText(unibuffer);
			m_displayStrings[NetStats]->setText(unibuffer);
			m_displayStrings[NetFPSAverages]->setText( unibuffer );
		}

		// selected object info stats
		unibuffer.format( L"Select Info: '%d' drawables selected", TheInGameUI->getSelectCount() );



		//Sorry, guys. I need a special kluge here to get constantdebug results for angry mob.
		//Do no be cross with me.
		//if there is not exactly one drawable selected it will report on the moused-over drawable
		if (TheInGameUI->getSelectCount() == 1)
			draw = TheInGameUI->getFirstSelectedDrawable();


		if( draw )
		{
			Object *obj = draw->getObject();
			AsciiString objectName;

			objectName.set( "No-Name" );
			if( obj && obj->getName().isEmpty() == FALSE )
				objectName = obj->getName();

			unibuffer.format( L"Select Info: '%S'(%S) at (%.3f,%.3f,%.3f)",
												draw->getTemplate()->getName().str(),
												objectName.str(),
												draw->getPosition()->x,
												draw->getPosition()->y,
												draw->getPosition()->z
											);

			const PhysicsBehavior *physics = obj->getPhysics();
			PhysicsTurningType turnType = physics ? physics->getTurning() : TURN_NONE;

			const DrawableLocoInfo *locoInfo = draw->getLocoInfo();
			if( locoInfo )
			{
				unibuffer2.format( L"\nPhysics Info -- Turn: %d, Pitch(accel): %.3f(%.3f), Roll(accel): %.3f(%.3f)",
													 turnType,
													 locoInfo->m_accelerationPitch, locoInfo->m_accelerationPitchRate,
													 locoInfo->m_accelerationRoll, locoInfo->m_accelerationRollRate );
				unibuffer.concat( unibuffer2 );
			}






			// (gth) compute some stats about the rendering cost of this drawable
#if defined(RTS_DEBUG)
			RenderCost rcost;
			for (DrawModule** dm = draw->getDrawModules(); *dm; ++dm)
			{
				(*dm)->getRenderCost(rcost);
			}
			if (rcost.getDrawCallCount() > 0)
			{
				unibuffer2.format( L"\ndraw calls: %d(+%d) sort meshes: %d skins: %d  bones: %d",rcost.getDrawCallCount(),rcost.getShadowDrawCount(),rcost.getSortedMeshCount(),rcost.getSkinMeshCount(),rcost.getBoneCount());
				unibuffer.concat( unibuffer2 );
			}
#endif

			unibuffer.concat( L"\nModelStates: " );
			ModelConditionFlags mcFlags = draw->getModelConditionFlags();
			const int numEntriesPerLine = 4;
			int lineCount = 0;

			for( int i = 0; i < MODELCONDITION_COUNT; i++ )
			{
				if( mcFlags.test( i ) )
				{
					unibuffer2.format( L"%S ", ModelConditionFlags::getBitNames()[ i ] );
					unibuffer.concat( unibuffer2 );
					lineCount++;
					if( lineCount == numEntriesPerLine )
					{
						lineCount = 0;
						unibuffer.concat( L"\n" );
					}
				}
			}

			//Render ALL modelcondition statii

		}
		m_displayStrings[ SelectedInfo ]->setText( unibuffer );

	}

}

// W3DDisplay::drawDebugStats =================================================
/** Draw debug statistics */
//=============================================================================
void W3DDisplay::drawDebugStats()
{
	Int	x = 3;
	Int	y = 30;
	Color textColor = GameMakeColor( 255, 255, 255, 255 );
	Color dropColor = GameMakeColor( 0, 0, 0, 255 );

	int linesOfStrings = DisplayStringCount;
#ifdef EXTENDED_STATS
	if (Graphics::Get_Render_Diagnostics().console_line_limit > -1)
	{
		linesOfStrings = Graphics::Get_Render_Diagnostics().console_line_limit;
	}

#endif


	Int w, h;
	for (int i = 0; i < linesOfStrings; i++)
	{
		m_displayStrings[i]->draw( x, y, textColor, dropColor );
		m_displayStrings[i]->getSize(&w, &h);
		y += h;
	}

}

// W3DDisplay::drawFPSStats =================================================
/** Draw the FPS on the screen */
//=============================================================================
void W3DDisplay::drawFPSStats()
{
	Int	x = 3;
	Int	y = 20;
	Color textColor = GameMakeColor( 255, 255, 255, 255 );
	Color dropColor = GameMakeColor( 0, 0, 0, 255 );

	int linesOfStrings = 1;

	for (int i = 0; i < linesOfStrings; i++)
	{
		m_benchmarkDisplayString->draw( x, y, textColor, dropColor );
	}
}


//=============================================================================
void StatDebugDisplay( DebugDisplayInterface *, void *, FILE *fp )
{
	DEBUG_CRASH(("This should never be called directly, but is just a placeholder for drawDebugStats()"));
}

// W3DDisplay::drawCurrentDebugDisplay =================================================
/** Draw current debug display */
//=============================================================================
void W3DDisplay::drawCurrentDebugDisplay()
{
	if (m_debugDisplayCallback == StatDebugDisplay)
	{
		drawDebugStats();
	}
	else
	{
		if ( m_debugDisplay && m_debugDisplayCallback )
		{
			m_debugDisplay->reset();
			m_debugDisplayCallback( m_debugDisplay, m_debugDisplayUserData, nullptr );
		}
	}
}

// W3DDisplay::calculateTerrainLOD =================================================
/** Calculates an adequately speedy terrain Level Of Detail. */
//=============================================================================
void W3DDisplay::calculateTerrainLOD()
{
	const Int NUM_SAMPLES=20;
	const Int NUM_TO_DISCARD=5;

	Int64 freq64 = getPerformanceCounterFrequency();

	float frameTime = 0;
	float maxTimeLimit = TheGlobalData->m_terrainLODTargetTimeMS/1000.0f;
	TerrainLOD goodLOD = TERRAIN_LOD_MIN;
	TerrainLOD curLOD = TERRAIN_LOD_AUTOMATIC;
	Int count = 0;
#ifdef RTS_DEBUG
	// just go to TERRAIN_LOD_NO_WATER, mirror off.
	TheWritableGlobalData->m_terrainLOD = TERRAIN_LOD_NO_WATER;
	m_3DScene->drawTerrainOnly(false);
	TheTerrainRenderObject->adjustTerrainLOD(0);
	return;
#endif
	do {
		Int i;
		float timeForFrame=0;
		frameTime = 0;
		switch(curLOD) {
			default: curLOD = TERRAIN_LOD_DISABLE; break;
			case TERRAIN_LOD_AUTOMATIC: curLOD = TERRAIN_LOD_MAX; break;
			case TERRAIN_LOD_MAX: curLOD = TERRAIN_LOD_NO_WATER; break;
			case TERRAIN_LOD_NO_WATER: curLOD = TERRAIN_LOD_DISABLE; break;
		}
		if (curLOD == TERRAIN_LOD_DISABLE) {
			break;
		}
		TheWritableGlobalData->m_terrainLOD = curLOD;
		m_3DScene->drawTerrainOnly(true);
		TheTerrainRenderObject->adjustTerrainLOD(0);
		for (i=0; i<NUM_SAMPLES; i++) {
			Int64 startTime64 = getPerformanceCounter();
			// start render block
			updateViews();
            if (!graphicsRendererAvailable || !Graphics_Begin_Frame()) {
                Graphics_Abort_Frame();
                m_3DScene->drawTerrainOnly(false);
                return;
            }
            if (Get_W3D_Render_Services().Begin_Render(true, true, Vector3(0.0f, 0.0f, 0.0f))) {
                drawViews();
                Get_W3D_Render_Services().End_Render();
                if (!Graphics_End_Frame() || !Graphics_Present())
                    Graphics_Abort_Frame();
            } else {
                Graphics_Abort_Frame();
            }
			Int64 time64 = getPerformanceCounter();
			timeForFrame = (float)((double)(time64-startTime64) / (double)(freq64));
			if (i>=NUM_TO_DISCARD) {
				frameTime += timeForFrame;
				if (i>NUM_TO_DISCARD+1 &&
					(timeForFrame / ((i+1)-NUM_TO_DISCARD)) > 2*maxTimeLimit) {
					i++;
					break;
				}
			}
		}
		frameTime /= ((i)-NUM_TO_DISCARD);
		count++;
		if (frameTime<maxTimeLimit && goodLOD<curLOD) {
			goodLOD = curLOD;
		}
		if (frameTime < maxTimeLimit) break;
	} while (count<10);

	TheWritableGlobalData->m_terrainLOD = goodLOD;
	m_3DScene->drawTerrainOnly(false);
	TheTerrainRenderObject->adjustTerrainLOD(0);
#ifdef RTS_DEBUG
	DEBUG_ASSERTCRASH(count<10, ("calculateTerrainLOD") );
#endif

}


Real W3DDisplay::getAverageFPS()
{
	return m_averageFPS;
}

Real W3DDisplay::getCurrentFPS()
{
	return m_currentFPS;
}

Int W3DDisplay::getLastFrameDrawCalls()
{
	return static_cast<Int>((std::min)(Graphics::Get_Frame_Submission_Statistics().Last_Frame().draw_calls, std::uint64_t{INT_MAX}));
}

//=============================================================================
void W3DDisplay::step()
{
	stepViews();
}

//DECLARE_PERF_TIMER(BigAssRenderLoop)

// W3DDisplay::draw ===========================================================
/** Draw the entire W3D Display */
//=============================================================================
//DECLARE_PERF_TIMER(W3DDisplay_draw)
void W3DDisplay::draw()
{
    GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.Display.Draw");
    PROFILER_PLOT("Graphics.LogicRate", static_cast<int64_t>(TheFramePacer->getActualLogicTimeScaleFps()));
    PROFILER_PLOT("Graphics.RenderLimit", static_cast<int64_t>(TheFramePacer->getActualFramesPerSecondLimit()));
    PROFILER_PLOT("Graphics.LogicFrame", static_cast<int64_t>(TheGameLogic->getFrame()));
	//USE_PERF_TIMER(W3DDisplay_draw)

	if (SDLPlatformWindow::isMinimized()) {
		return;
	}

	if (TheGlobalData->m_headless)
		return;

	// TheSuperHackers @feature bobtista 10/07/2026 Show messages for screenshots finished by the screenshot thread.
	W3D_UpdateScreenshotMessages();

	updateAverageFPS();
	if (TheGlobalData->m_enableDynamicLOD && TheGameLogic->getShowDynamicLOD())
	{
		DynamicGameLODLevel lod=TheGameLODManager->findDynamicLODLevel(m_averageFPS);
		TheGameLODManager->setDynamicLODLevel(lod);
	}
	else
	{	//if dynamic LOD is turned off, force highest LOD
		TheGameLODManager->setDynamicLODLevel(DYNAMIC_GAME_LOD_VERY_HIGH);
	}

	if (TheGlobalData->m_terrainLOD == TERRAIN_LOD_AUTOMATIC && TheTerrainRenderObject)
	{
		calculateTerrainLOD();
	}
#ifdef EXTENDED_STATS
AGAIN:
#endif

#ifdef DUMP_PERF_STATS
	if( TheGlobalData->m_dumpPerformanceStatistics )
	{
		TheStatDump.dumpStats( FALSE, TRUE );
		TheWritableGlobalData->m_dumpPerformanceStatistics = FALSE;
	}
  //The <= GAME_REPLAY essentially means, GAME_SINGLE_PLAYER || GAME_LAN || GAME_SKIRMISH || GAME_REPLAY
  else if ( TheGlobalData->m_dumpStatsAtInterval && TheGameLogic->getGameMode() <= GAME_REPLAY )
  {
    Int interval = TheGlobalData->m_statsInterval;
    if ( TheGameLogic->getFrame() > 0 && (TheGameLogic->getFrame() % interval) == 0 )
    {
  	  TheStatDump.dumpStats( TRUE, TRUE );
    	TheInGameUI->message( L"-stats is running, at interval: %d.", TheGlobalData->m_statsInterval );
    }
  }
#endif

	// compute debug statistics for display later
	if ( m_debugDisplayCallback == StatDebugDisplay
#if defined(RTS_DEBUG)
				|| TheGlobalData->m_benchmarkTimer > 0
#endif
			)
	{
		gatherDebugStats();
	}
#ifdef EXTENDED_STATS
	else
	{
		Graphics::Get_Render_Diagnostics().collecting_statistics = false;
	}
#endif

#ifdef SAMPLE_DYNAMIC_LIGHT
	Vector3 loc;
	loc = theDynamicLight->Get_Position();
	loc.X += theLightXOffset;
	if(loc.X>128) theLightXOffset = -theLightXOffset;
	if(loc.X<0) theLightXOffset = -theLightXOffset;
	loc.Y += theLightYOffset;
	if(loc.Y>128) theLightYOffset = -theLightYOffset;
	if(loc.Y<0) theLightYOffset = -theLightYOffset;
	theDynamicLight->Set_Position(loc);
#endif


	/// @todo Make more explicit drawing layers(ground, ground UI, objects, object UI, overlay UI)

	///@todo: Ask Vegas why the LOD optimizer hangs particle system.
 	//
  	// Predictive LOD optimizer optimizes the mesh LOD levels to match
  	// the given polygon budget
  	//

	Bool freezeTime = TheFramePacer->isTimeFrozen() || TheFramePacer->isGameHalted();

	/// @todo: I'm assuming the first view is our main 3D view.
	W3DView *primaryW3DView=(W3DView *)getFirstView();

	if (!freezeTime && TheScriptEngine->isTimeFast())
	{
		primaryW3DView->updateCameraMovements();  // Update camera motion effects.
		return;
	}


	//update state of all the terrain tracks (fade, remove, etc.)
	/// @todo: Is there a better place to put per-frame updates like this?

	if(TheGlobalData->m_loadScreenRender != TRUE)
	{

		if (TheTerrainTracksRenderObjClassSystem)
			TheTerrainTracksRenderObjClassSystem->update();

		//Shroud data is needed to render all other views, so handle this first.
		if (TheTerrainRenderObject)
		{
			//update the shroud surface here since it may be needed by reflections
			if (TheTerrainRenderObject->getMap())	//make sure a valid map is loaded into terrain.
			{
				if (TheTerrainRenderObject->getShroud())
				{
					GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.Shroud.Update");
					TheTerrainRenderObject->getShroud()->render(primaryW3DView->get3DCamera());
				}
			}
		}
	}

	Graphics::Get_Render_Clock().Update_Logic_Frame_Time(TheFramePacer->getLogicTimeStepMilliseconds());

	// TheSuperHackers @info This binds the WW3D update to the logic update.
	{
		GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.Scene.Sync");
		Graphics::Get_Render_Clock().Sync(TheGameLogic->hasUpdated());
	}

	static Int now;
	now=SDL_GetTicks();

	if (TheTacticalView->getTimeMultiplier()>1)
	{
		static Int timeMultiplierCounter = 1;
		timeMultiplierCounter--;
		if (timeMultiplierCounter>1)
			return;
		timeMultiplierCounter = TheTacticalView->getTimeMultiplier();
		// limit the framerate, because while fast time is on, the game logic is running as fast as it can.
	}

	do {
		auto& submissionStatistics = Graphics::Get_Frame_Submission_Statistics();
		submissionStatistics.Cancel();
		if (auto* device = Graphics::Shared_Frame_Device()) {
			auto& commands = device->Immediate_Command_List();
			submissionStatistics.Begin(&commands, commands.Submission_Counts());
		}

		// update all views of the world - recomputes data which will affect drawing
		if (Graphics::Frame_Device_Ready())
		{	//Checking if we have the device before updating views because the heightmap crashes otherwise while
			//trying to refresh the visible terrain geometry.
//			if(TheGlobalData->m_loadScreenRender != TRUE)
			{
				GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.Views.Update");
				updateViews();
			}
			{
				GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.Particles.Update");
     		TheParticleSystemManager->update();//LORENZEN AND WILCZYNSKI MOVED THIS FROM ITS NATIVE POSITION, ABOVE
                                           //FOR THE PURPOSE OF LETTING THE PARTICLE SYSTEM LOOK UP THE RENDER OBJECT"S
                                           //TRANSFORM MATRIX, WHILE IT IS STILL VALID (HAVING DONE ITS CLIENT TRANSFORMS
                                           //BUT NOT YET RESETTING TOT HE LOGICAL TRANSFORM)
                                           //THE RESULT IS THAT PARTICLESYSTEMS LINKED TO BONES IN DRAWABLES.OBJECTS
                                           //MOVE WITH THE CLIENT TRANSFORMS, NOW.
                                           //REVOLUTIONARY!
                                           //-LORENZEN
			}


			if (TheWaterRenderSystem)
				TheWaterRenderSystem->updateRenderTargetTextures(primaryW3DView->get3DCamera());	//do a render into each texture

			//Can't render into textures while rendering to screen so these textures need to be updated
			//before we enter main rendering loop.
			if (TheW3DProjectedShadowManager)
			{
				GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.ProjectedShadows.Update");
				TheW3DProjectedShadowManager->updateRenderTargetTextures();
			}
		}



		// start render block
		#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
    if ( (TheGameLogic->getFrame() % 30 == 1) || ( ! ( !TheGameLogic->isGamePaused() && TheGlobalData->m_TiVOFastMode) ) )
		#else
	    if ( (TheGameLogic->getFrame() % 30 == 1) || ( ! (!TheGameLogic->isGamePaused() && TheGlobalData->m_TiVOFastMode && TheGameLogic->isInReplayGame())) )
    #endif
		{
			//USE_PERF_TIMER(BigAssRenderLoop)
			static Bool couldRender = true;
			bool graphicsFrame = false;
			if (graphicsRendererAvailable) {
				graphicsFrame = beginGraphicsFrame();
				if (!graphicsFrame)
					Graphics_Abort_Frame();
			}
			if (graphicsFrame && (TheGlobalData->m_breakTheMovie == FALSE) && (TheGlobalData->m_disableRender == false) && Get_W3D_Render_Services().Begin_Render( true, true, Vector3( 0.0f, 0.0f, 0.0f ), TheWaterTransparency->m_minWaterOpacity ))
			{

				if(TheGlobalData->m_loadScreenRender == TRUE)
				{
					TheInGameUI->draw();
					if( TheMouse )
						TheMouse->draw();	//keep applying the current cursor style so it remains hidden if needed.
					if (graphicsFrame)
						Submit_Videos(static_cast<std::uint32_t>(getWidth()), static_cast<std::uint32_t>(getHeight()));
					Get_W3D_Render_Services().End_Render();
					if (graphicsFrame)
						renderGraphicsScenePasses(primaryW3DView->get3DCamera());
					continue;
				}
				couldRender = true;

				// draw all views of the world
				{
					GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.Views.Draw");
					drawViews();
				}

				// draw the user interface
				{
				GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.UI.Draw");
				TheInGameUI->DRAW();

				TheGameClient->DRAW();

				// draw the mouse
				if( TheMouse )
					TheMouse->DRAW();
				}

				if (graphicsFrame)
					Submit_Videos(static_cast<std::uint32_t>(getWidth()), static_cast<std::uint32_t>(getHeight()));

				// render letter box before debug display so debug info isn't hidden
				renderLetterBox(now);

				// display cinematicText over the black
				if( m_cinematicText != AsciiString::TheEmptyString && m_cinematicTextFrames != 0)
				{
					DisplayString *displayString = TheDisplayStringManager->newDisplayString();

					// set word wrap if necessary

					Int wordWrapWidth = TheDisplay->getWidth() - 20;
					displayString->setWordWrap( wordWrapWidth );
					displayString->setWordWrapCentered( TRUE );

					UnicodeString text;
					text.translate( m_cinematicText );
					displayString->setText( text );
					Color color = GameMakeColor( 255, 255, 255, 255 );  // white
					Color backColor = GameMakeColor( 0, 0, 0, 0 );      // black
					displayString->setFont( m_cinematicFont );
					Int height = TheDisplay->getHeight() * .9;

					Int width;
					if( displayString->getWidth() > TheDisplay->getWidth() )
						width = 20;
					else
						width = ( TheDisplay->getWidth() - displayString->getWidth() ) / 2;
					displayString->draw( width, height, color, backColor );

					m_cinematicTextFrames--;
				}

				if ( m_debugDisplayCallback )
				{
					// draw the current debug display
					drawCurrentDebugDisplay();
				}

#if defined(RTS_DEBUG)
				if (TheGlobalData->m_benchmarkTimer > 0)
				{
					drawFPSStats();
				}
#endif


#if defined(RTS_DEBUG)
				if (TheGlobalData->m_debugShowGraphicalFramerate)
				{
					drawFramerateBar();
				}
#endif

#ifdef PERF_TIMERS
				TheGraphDraw->render();
				TheGraphDraw->clear();
#endif

#ifdef PROFILER_ENABLED
                if (PROFILER_IS_CONNECTED && !TheGlobalData->m_headless) {
                    GENERALS_GRAPHICS_PROFILE_SCOPE("Graphics.Profiler.FrameCapture");
                    auto* device = Graphics::Shared_Frame_Device();
                    if (device) {
                        const auto frame = Graphics::Get_Frame_Preview().Read(
                            Graphics::Shared_Frame_Targets(),
                            Graphics::RHITextureFormat::BGRA8_UNorm, PROFILER_FRAME_IMAGE_SIZE,
                            Graphics::Get_Render_Clock().Logic_Time_Milliseconds(), PROFILER_FRAME_IMAGE_INTERVAL_MS);
                        if (frame.Is_Valid())
                            PROFILER_FRAME_IMAGE(frame.pixels.data(), frame.width, frame.height, 0, false);
                    }
                }
#endif
                // render is all done!
				Get_W3D_Render_Services().End_Render();
				if (graphicsFrame)
					renderGraphicsScenePasses(primaryW3DView->get3DCamera());
			}
			else
			{
				if (graphicsFrame)
					Graphics_Abort_Frame();
				if (couldRender)
				{
					couldRender = false;
					DEBUG_LOG(("Could not prepare RenderServices::Begin_Render()!  Are we ALT-Tabbed out?"));
				}
			}
		}

		submissionStatistics.Cancel();
		if (TheScriptEngine->isTimeFrozenDebug() || TheScriptEngine->isTimeFrozenScript() || TheGameLogic->isGamePaused())
		{
			freezeTime = false; // We're frozen for debug or for pause, and need to continue out of the loop.
		}

	} while (freezeTime && !TheTacticalView->isCameraMovementFinished());

#ifdef EXTENDED_STATS
	if (Graphics::Get_Render_Diagnostics().disable_overhead) {
		goto AGAIN;
	}
#endif
}

#define LETTER_BOX_FADE_TIME	1000.0f		///1000 ms.

/** Render letter-box border at top/bottom of display
*/
void W3DDisplay::renderLetterBox(UnsignedInt currentTime)
{
		if (m_letterBoxEnabled)
		{	if (m_letterBoxFadeLevel != 1.0f)
			{
				m_letterBoxFadeLevel = (currentTime - m_letterBoxFadeStartTime)/LETTER_BOX_FADE_TIME;
				if (m_letterBoxFadeLevel > 1.0f)
					m_letterBoxFadeLevel = 1.0f;
			}

			UnsignedInt lbcolor = (Int)(m_letterBoxFadeLevel * 255.0f) << 24;

#ifdef SLIDE_LETTERBOX
			Int height = (Int)(getHeight() * 0.12f * m_letterBoxFadeLevel);
			TheTacticalView->setOrigin(0, height);
#else
			drawFillRect( 0, 0, m_width, (m_height-(9.0f/16.0f * m_width))*0.5f, lbcolor );
			drawFillRect( 0, m_height-(m_height-(9.0f/16.0f * m_width))*0.5f, m_width, m_height, lbcolor );
#endif
		}
		else
		{	//letter box is disabled, but may still be fading out
			if (m_letterBoxFadeLevel != 0.0f)
			{
				m_letterBoxFadeLevel = 1.0f - (currentTime - m_letterBoxFadeStartTime)/LETTER_BOX_FADE_TIME;
				if (m_letterBoxFadeLevel < 0.0f)
					m_letterBoxFadeLevel = 0.0f;

				UnsignedInt lbcolor = (Int)(m_letterBoxFadeLevel * 255.0f) << 24;

#ifdef SLIDE_LETTERBOX
				Int height = (Int)(getHeight() * 0.12f * m_letterBoxFadeLevel);
				TheTacticalView->setOrigin(0, height);
#else
				drawFillRect( 0, 0, m_width, (m_height-(9.0f/16.0f * m_width))*0.5f, lbcolor );
				//drawFillRect( 0, m_height-(m_height-(9.0f/16.0f * m_width))*0.5f, m_width, m_height, lbcolor );
#endif
			}
			else
			{	//box has finished fading out
#ifdef SLIDE_LETTERBOX
				TheTacticalView->setOrigin(0, 0);
#else
				m_letterBoxEnabled = FALSE;
#endif
			}
		}
}

Bool W3DDisplay::isLetterBoxFading()
{
	if (m_letterBoxEnabled && m_letterBoxFadeLevel != 1.0f)
		return TRUE;
	if (!m_letterBoxEnabled && m_letterBoxFadeLevel != 0.0f)
		return TRUE;
	return FALSE;
}

//WST 10/2/2002 added query function.  JSC Integrated 5/20/03
Bool W3DDisplay::isLetterBoxed()
{
	return (m_letterBoxEnabled);
}

// W3DDisplay::createLightPulse ===============================================
/** Create a "light pulse" which is a dynamic light that grows, decays
	* and vanishes over several frames */
//=============================================================================
void W3DDisplay::createLightPulse( const Coord3D *pos, const RGBColor *color,
																	 Real innerRadius, Real attenuationWidth,
																	 UnsignedInt increaseFrameTime,
																	 UnsignedInt decayFrameTime//, Bool donut
																	 )
{
	if (m_3DScene == nullptr)
		return;
	if (innerRadius+attenuationWidth<2.0*PATHFIND_CELL_SIZE_F + 1.0f) {
		return; // it basically won't make any visual difference.  jba.
	}
	W3DDynamicLight * theDynamicLight = m_3DScene->getADynamicLight();
	// turn it on.
	theDynamicLight->setEnabled(true);

	theDynamicLight->Set_Ambient( Vector3( color->red, color->green, color->blue ) );
	theDynamicLight->Set_Diffuse( Vector3( color->red, color->green, color->blue) );
	theDynamicLight->Set_Position(Vector3(pos->x, pos->y, pos->z));
	theDynamicLight->Set_Far_Attenuation_Range(innerRadius, innerRadius + attenuationWidth);
	theDynamicLight->setFrameFade(increaseFrameTime, decayFrameTime);
	theDynamicLight->setDecayRange();
	theDynamicLight->setDecayColor();
	//theDynamicLight->setDonut(donut);
	// (gth) CNC3 enable far attenuation.  C&C3 defaults to disabled.  Must enable to match Generals. MW 8-06-03
	theDynamicLight->Set_Flag(W3DLight::FAR_ATTENUATION,true);
}

void W3DDisplay::toggleLetterBox()
{
	m_letterBoxEnabled = !m_letterBoxEnabled;
	m_letterBoxFadeStartTime = SDL_GetTicks();

	//WST  9/18/2002 This is not a script api to prevent cheat. JSC Integrated 5/20/03
	if( TheTacticalView )
	{
		TheTacticalView->setZoomLimited( !m_letterBoxEnabled );
	}
}

void W3DDisplay::enableLetterBox(Bool enable)
{
	if (enable)
	{
		if (!m_letterBoxEnabled)
		{	//letterbox mode not previously enabled
			m_letterBoxEnabled = TRUE;
			m_letterBoxFadeStartTime = SDL_GetTicks();

			//WST  9/18/2002 - This is not a script api to prevent cheat.  JSC Integrated 5/20/03
			if( TheTacticalView )
			{
				TheTacticalView->setZoomLimited( 0 );
			}
		}
	}
	else
	{
		if (m_letterBoxEnabled)
		{	//letterbox mode no previously disabled
			m_letterBoxEnabled = FALSE;
			m_letterBoxFadeStartTime = SDL_GetTicks();

			//WST  9/18/2002. JSC Integrated 5/20/03
			if( TheTacticalView )
			{
				TheTacticalView->setZoomLimited( 1 );
			}
		}
	}
}

// W3DDisplay::setTimeOfDay ===================================================
/** */
//=============================================================================
void W3DDisplay::setTimeOfDay( TimeOfDay tod )
{
	const GlobalData::TerrainLighting *ol=&TheGlobalData->m_terrainObjectsLighting[tod][0];

	if( m_3DScene )
	{
		m_3DScene->Set_Ambient_Light( Vector3(ol->ambient.red, ol->ambient.green, ol->ambient.blue) );
	}

	for (Int i=0; i<Graphics::Material_Light_Count; i++)
	{
		if( m_myLight[i] )
		{
			ol=&TheGlobalData->m_terrainObjectsLighting[tod][i];

			m_myLight[i]->Set_Ambient( Vector3( 0.0f, 0.0f, 0.0f ) );
			m_myLight[i]->Set_Diffuse( Vector3(ol->diffuse.red, ol->diffuse.green, ol->diffuse.blue ) );
			m_myLight[i]->Set_Specular( Vector3(0,0,0) );
			Matrix3D mtx;
			mtx.Set(Vector3(1,0,0), Vector3(0,1,0), Vector3(ol->lightPos.x, ol->lightPos.y, ol->lightPos.z), Vector3(0,0,0));
			m_myLight[i]->Set_Transform(mtx);
		}
	}
	if(TheTerrainRenderObject) {
		TheTerrainRenderObject->setTimeOfDay(tod);
		TheTacticalView->forceRedraw();
	}
}

// W3DDisplay::drawLine =======================================================
/** draw a line on the display in pixel coordinates with the specified color */
//=============================================================================
void W3DDisplay::drawLine(
	Int startX, Int startY, Int endX, Int endY, Real lineWidth, UnsignedInt lineColor)
{
	if (!uiFrameActive)
		return;

	Set_UI_Clip(m_isClippedEnabled, m_clipRegion);
	if (!Graphics::Get_Renderer2D().Add_Line(
			{static_cast<float>(startX), static_cast<float>(startY)},
			{static_cast<float>(endX), static_cast<float>(endY)},
			lineWidth,
			To_UI_Color(lineColor))) {
		Graphics::Get_Renderer2D().Discard();
		uiFrameActive = false;
	}
}

// W3DDisplay::drawLine =======================================================
/** draw a line on the display in pixel coordinates with the specified color */
//=============================================================================
void W3DDisplay::drawLine(
	Int startX, Int startY, Int endX, Int endY, Real lineWidth,
	UnsignedInt lineColor1, UnsignedInt lineColor2)
{
	if (!uiFrameActive)
		return;

	Set_UI_Clip(m_isClippedEnabled, m_clipRegion);
	if (!Graphics::Get_Renderer2D().Add_Gradient_Line(
			{static_cast<float>(startX), static_cast<float>(startY)},
			{static_cast<float>(endX), static_cast<float>(endY)},
			lineWidth,
			{{To_UI_Color(lineColor1), To_UI_Color(lineColor1),
				To_UI_Color(lineColor2), To_UI_Color(lineColor2)}})) {
		Graphics::Get_Renderer2D().Discard();
		uiFrameActive = false;
	}
}

// W3DDisplay::drawOpenRect ===================================================
//=============================================================================
void W3DDisplay::drawOpenRect(
	Int startX, Int startY, Int width, Int height, Real lineWidth, UnsignedInt lineColor)
{
	if (!uiFrameActive)
		return;

	Set_UI_Clip(m_isClippedEnabled, m_clipRegion);
	if (!Graphics::Get_Renderer2D().Add_Outline(
			{static_cast<float>(startX), static_cast<float>(startY),
				static_cast<float>(startX + width), static_cast<float>(startY + height)},
			lineWidth,
			To_UI_Color(lineColor))) {
		Graphics::Get_Renderer2D().Discard();
		uiFrameActive = false;
	}
}

// W3DDisplay::drawFillRect ===================================================
//=============================================================================
void W3DDisplay::drawFillRect(
	Int startX, Int startY, Int width, Int height, UnsignedInt color)
{
	if (!uiFrameActive)
		return;

	Set_UI_Clip(m_isClippedEnabled, m_clipRegion);
	if (!Graphics::Get_Renderer2D().Add_Rect(
			{static_cast<float>(startX), static_cast<float>(startY),
				static_cast<float>(startX + width), static_cast<float>(startY + height)},
			To_UI_Color(color))) {
		Graphics::Get_Renderer2D().Discard();
		uiFrameActive = false;
	}
}

void W3DDisplay::drawRectClock(
	Int startX, Int startY, Int width, Int height, Int percent, UnsignedInt color)
{
	if (percent < 1 || percent > 100 || !uiFrameActive)
		return;

	Set_UI_Clip(m_isClippedEnabled, m_clipRegion);
	if (!Graphics::Get_Renderer2D().Add_Rect_Clock(
			{static_cast<float>(startX), static_cast<float>(startY),
				static_cast<float>(startX + width), static_cast<float>(startY + height)},
			percent,
			To_UI_Color(color),
			false)) {
		Graphics::Get_Renderer2D().Discard();
		uiFrameActive = false;
	}
}

void W3DDisplay::drawRemainingRectClock(
	Int startX, Int startY, Int width, Int height, Int percent, UnsignedInt color)
{
	if (percent < 0 || percent > 100 || !uiFrameActive)
		return;

	Set_UI_Clip(m_isClippedEnabled, m_clipRegion);
	if (!Graphics::Get_Renderer2D().Add_Rect_Clock(
			{static_cast<float>(startX), static_cast<float>(startY),
				static_cast<float>(startX + width), static_cast<float>(startY + height)},
			percent,
			To_UI_Color(color),
			true)) {
		Graphics::Get_Renderer2D().Discard();
		uiFrameActive = false;
	}
}


// W3DDisplay::drawImage ======================================================
/** Draw an image in screen coordinates. */
//=============================================================================
void W3DDisplay::drawImage(
	const Image *image, Int startX, Int startY, Int endX, Int endY,
	Color color, DrawImageMode mode)
{
	if (image == nullptr || !uiFrameActive)
		return;

	if (m_isClippedEnabled && (endX <= m_clipRegion.lo.x
		|| endY <= m_clipRegion.lo.y
		|| startX >= m_clipRegion.hi.x
		|| startY >= m_clipRegion.hi.y))
		return;

	Set_UI_Clip(m_isClippedEnabled, m_clipRegion);
	Engine::UI::WND::ImageRef image_reference =
		Engine::UI::WND::Resolve_Image_Reference(image->getFilename().str());
	image_reference.uv = {
		image->getUV()->lo.x, image->getUV()->lo.y,
		image->getUV()->hi.x, image->getUV()->hi.y};
	const Graphics::Renderer2DTexture binding = Engine::UI::WND::Resolve_Image_Texture(
		image_reference.texture, Graphics::Get_Renderer2D());
	if (!binding.index.Is_Valid()) {
		uiFrameActive = false;
		return;
	}
	const Region2D *uv = image->getUV();
	const Graphics::Rect2D screen{
		static_cast<float>(startX), static_cast<float>(startY),
		static_cast<float>(endX), static_cast<float>(endY)};
	const Graphics::Rect2D texture_uv{uv->lo.x, uv->lo.y, uv->hi.x, uv->hi.y};
	const Graphics::Color2D draw_color = To_UI_Color(color);
	const bool added = BitIsSet(image->getStatus(), IMAGE_STATUS_ROTATED_90_CLOCKWISE)
		? Graphics::Get_Renderer2D().Add_Quad(
			{{{screen.left, screen.top}, {screen.left, screen.bottom},
				{screen.right, screen.top}, {screen.right, screen.bottom}}},
			{{{texture_uv.right, texture_uv.top}, {texture_uv.left, texture_uv.top},
				{texture_uv.right, texture_uv.bottom}, {texture_uv.left, texture_uv.bottom}}},
			binding,
			draw_color,
			To_UI_Blend_Mode(mode),
			mode == DRAW_IMAGE_GRAYSCALE)
		: Graphics::Get_Renderer2D().Add_Quad(
			screen,
			texture_uv,
			binding,
			draw_color,
			To_UI_Blend_Mode(mode),
			mode == DRAW_IMAGE_GRAYSCALE);
	if (!added) {
		Graphics::Get_Renderer2D().Discard();
		uiFrameActive = false;
	}
}

void W3DDisplay::playMovie( AsciiString movieName )
{
	if (TheGlobalData->m_headless)
		return;

	stopMovie();
	if (Open_Fullscreen_Video(movieName))
		m_currentlyPlayingMovie = movieName;
}

void W3DDisplay::stopMovie()
{
	Close_Fullscreen_Video();
	m_currentlyPlayingMovie = AsciiString::TheEmptyString;
}

Bool W3DDisplay::isMoviePlaying()
{
	return Is_Fullscreen_Video_Playing();
}

// W3DDisplay::setClipRegion ============================================
/** Set the clipping region for all queued 2D draw operations. */
//=============================================================================
void W3DDisplay::setClipRegion( IRegion2D *region )
{
	if (region == nullptr) {
		enableClipping(FALSE);
		return;
	}

	m_clipRegion = *region;
	m_isClippedEnabled = TRUE;
	if (uiFrameActive)
		Set_UI_Clip(m_isClippedEnabled, m_clipRegion);

}

void W3DDisplay::enableClipping(Bool onoff)
{
	m_isClippedEnabled = onoff;
	if (uiFrameActive)
		Set_UI_Clip(m_isClippedEnabled, m_clipRegion);

}

//=============================================================================
/* we don't really need to override this call, since we will soon be called to
	update every shroud cell explicitly...
*/
void W3DDisplay::clearShroud()
{
	// nothing
}

//=============================================================================
void W3DDisplay::setBorderShroudLevel(UnsignedByte level)
{
	if (TheTerrainRenderObject && TheTerrainRenderObject->getShroud())
	{
		TheTerrainRenderObject->getShroud()->setBorderShroudLevel((W3DShroudLevel)level);
	}
}

//=============================================================================
void W3DDisplay::setShroudLevel( Int x, Int y, CellShroudStatus setting )
{
	if (TheTerrainRenderObject && TheTerrainRenderObject->getShroud())
	{
		#ifdef INTENSE_DEBUG
		TheTerrainRenderObject->getShroud()->setShroudFilter(false);
		#endif
		if( setting == CELLSHROUD_SHROUDED )
			TheTerrainRenderObject->getShroud()->setShroudLevel(x, y, (W3DShroudLevel)TheGlobalData->m_shroudAlpha );
		else if( setting == CELLSHROUD_FOGGED )
			TheTerrainRenderObject->getShroud()->setShroudLevel(x, y, (W3DShroudLevel)TheGlobalData->m_fogAlpha );///< @todo placeholder to get feedback on logic work while graphic side being decided
		else
			TheTerrainRenderObject->getShroud()->setShroudLevel(x, y, (W3DShroudLevel)TheGlobalData->m_clearAlpha );
		//Logic is saying shroud.  We can add alpha levels here in client if needed.
		// W3DShroud is a 0-255 alpha byte.  Logic shroud is a double reference count.

		TheTerrainRenderObject->notifyShroudChanged();

	}
}

/** Start/Stop capturing an AVI movie*/
void W3DDisplay::toggleMovieCapture()
{
	displayMovieCapture.Toggle("Movie",30);
}

void W3DDisplay::takeScreenShot(ScreenshotFormat format, Int jpegQuality)
{
	W3D_TakeCompressedScreenshot(format, jpegQuality);
}


#if defined(RTS_DEBUG)

static FILE *AssetDumpFile=nullptr;

void dumpMeshAssets(W3DMeshRenderObject *mesh)
{
	if (mesh)
	{
		W3DTextureHandle *texture;
		W3DMeshResource *model=mesh->Get_Model();
		for (int stage=0;stage<W3DMeshResource::MaterialDescription::MAX_TEX_STAGES;++stage)
		{
			for (int pass=0;pass<model->Get_Pass_Count();++pass)
			{
				if (model->Has_Texture_Array(pass,stage))
				{
					for (int i=0;i<model->Get_Polygon_Count();++i)
					{
						if ((texture=model->Peek_Texture(i,pass,stage)) != nullptr)
						{
							fprintf(AssetDumpFile,"\t%s\n",texture->Get_Texture_Name().str());
						}
					}
				}
				else
				{
					if ((texture=model->Peek_Single_Texture(pass,stage)) != nullptr)
					{
						fprintf(AssetDumpFile,"\t%s\n",texture->Get_Texture_Name().str());
					}
				}
			}
		}
	}
}

void dumpHLODAssets(W3DHierarchyRenderObject *hlod)
{
	if (hlod)
	{
		//model composed of multiple meshes.
		for (Int i=0; i<hlod->Get_Num_Sub_Objects(); i++)
		{
			W3DRenderObject *subObj=hlod->Get_Sub_Object(i);
			if (subObj->Class_ID() == W3DRenderObject::CLASSID_HLOD)
				dumpHLODAssets((W3DHierarchyRenderObject *)subObj);
			else
			if (subObj->Class_ID() == W3DRenderObject::CLASSID_MESH)
				dumpMeshAssets((W3DMeshRenderObject *)subObj);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**  dump all used models/textures to a file.*/
//-------------------------------------------------------------------------------------------------
void W3DDisplay::dumpModelAssets(const char *path)
{
	if (m_3DScene)
	{
		AssetDumpFile=fopen(path,"w");
		if (AssetDumpFile)
		{
			fprintf(AssetDumpFile,"Models and Textures used on %s:\n\n",TheGlobalData->m_mapName.str());
			W3DSceneIterator *sceneIter = m_3DScene->Create_Iterator();
			sceneIter->First();
			while(!sceneIter->Is_Done())
			{
				W3DRenderObject * robj = sceneIter->Current_Item();
				if (robj->Class_ID() == W3DRenderObject::CLASSID_HLOD)
				{	fprintf(AssetDumpFile,"%s.W3D:\n",robj->Get_Name());
					dumpHLODAssets((W3DHierarchyRenderObject *)robj);
				}
				else
				if (robj->Class_ID() == W3DRenderObject::CLASSID_MESH)
				{	fprintf(AssetDumpFile,"%s.W3D:\n",robj->Get_Name());
					dumpMeshAssets((W3DMeshRenderObject *)robj);
				}
				sceneIter->Next();
			}
			m_3DScene->Destroy_Iterator(sceneIter);
			fclose(AssetDumpFile);
		}
	}
}
#endif	//only include above code in debug and internal
//-------------------------------------------------------------------------------------------------
/** Preload using the W3D asset manager the model referenced by the string parameter */
//-------------------------------------------------------------------------------------------------
void W3DDisplay::preloadModelAssets( AsciiString model )
{

	if( m_assetManager )
	{
		AsciiString nameWithExtension;

		nameWithExtension.format( "%s.w3d", model.str() );
		m_assetManager->Catalog().Load_3D_Assets( nameWithExtension.str() );

	}

}

//-------------------------------------------------------------------------------------------------
/** Preload using the W3D asset manager the texture referenced by the string parameter */
//-------------------------------------------------------------------------------------------------
void W3DDisplay::preloadTextureAssets( AsciiString texture )
{

	if( m_assetManager )
	{
		W3DTextureHandle *theTexture = m_assetManager->Catalog().Get_Texture( texture.str() );
		theTexture->Release_Ref();//release reference
	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DDisplay::doSmartAssetPurgeAndPreload(const char* usageFileName)
{
	if (!m_assetManager || !usageFileName || !*usageFileName)
		return;

	std::vector<std::string> retained_names;

	// use TheFileSystem here so we can bigify these files
	File* f = TheFileSystem->openFile(usageFileName, File::READ | File::TEXT);
	if (f)
	{
		for (;;)
		{
			AsciiString tmp;
			if (f->scanString(tmp) == FALSE)
				break;

			// allow for comments in the file. Note that this doesn't allow for comments
			// with spaces! doh. oh well. better than nothing.
			if (tmp.str()[0] == ';')
				continue;

			retained_names.emplace_back(tmp.str());
		}
		f->close();
	}

	// just free everything if there's no exclusion list file (send in an empty list)
	m_assetManager->Catalog().Free_Assets_With_Exclusion_List(retained_names);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#if defined(RTS_DEBUG)
void W3DDisplay::dumpAssetUsage(const char* mapname)
{
	if (!m_assetManager || !mapname || !*mapname)
		return;

	std::vector<std::string> names;
	m_assetManager->Catalog().Create_Asset_List(names);

	const char* leafname = strrchr(mapname, '\\');
	if (leafname)
		++leafname;					// point to first character after the last backslash
	else
		leafname = mapname;		// point to the start of the filename

	char buf[256];
	int idx = 1;
	while (true)
	{
		sprintf(buf, "AssetUsage_%s_%04d.txt",leafname,idx);
		if (!std::filesystem::exists(buf))
			break;	// it exists, we're good
		++idx;
	}

	FILE *fp = fopen(buf, "w");
	if (fp)
	{
		for (const std::string &name : names)
		{
			fprintf(fp, "%s\n", name.c_str());
		}
		fclose(fp);
	}
}
#endif

//-------------------------------------------------------------------------------------------------
static void drawFramerateBar()
{
	static unsigned int prevTime = SDL_GetTicks();
	unsigned int now = SDL_GetTicks();
	Real percTime = (1000.0f / (now - prevTime) ) / (1000.0f / TheGlobalData->m_framesPerSecondLimit);

	if (percTime > 1.0f)
		percTime = 1.0f;
	else if (percTime < 0.0f)
		percTime = 0.0f;
	Int width = REAL_TO_INT(percTime * TheDisplay->getWidth());
	UnsignedInt colorToUse = GameMakeColor( REAL_TO_UNSIGNEDBYTE((1.0f - percTime) * 255),
																					REAL_TO_UNSIGNEDBYTE(percTime * 255),
																					0,
																					0x7F);

	TheDisplay->drawFillRect(1, 1, width, 15, colorToUse);
	prevTime = now;
}
