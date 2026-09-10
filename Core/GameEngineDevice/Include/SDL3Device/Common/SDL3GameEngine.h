#pragma once

#include "Common/GameEngine.h"
#include "GameClient/ParticleSys.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/NetworkInterface.h"
#include "XAudio2AudioDevice/XAudio2AudioManager.h"
#include "SDL3Device/Common/SDL3BIGFileSystem.h"
#include "SDL3Device/Common/SDL3LocalFileSystem.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DThingFactory.h"

class SDL3GameEngine : public GameEngine
{
public:
	SDL3GameEngine() = default;
	~SDL3GameEngine() override = default;
	void init() override { GameEngine::init(); }
	void reset() override { GameEngine::reset(); }
	void update() override;
	void serviceSDL3() override;

protected:
	// Called after SDL changes the drawable display size. Game-specific engines
	// can update their existing resolution-dependent objects here.
	virtual void onDisplaySizeChanged(UnsignedInt oldWidth, UnsignedInt oldHeight,
		UnsignedInt newWidth, UnsignedInt newHeight)
	{
		(void)oldWidth;
		(void)oldHeight;
		(void)newWidth;
		(void)newHeight;
	}

	GameLogic *createGameLogic() override { return NEW W3DGameLogic; }
	GameClient *createGameClient() override { return NEW W3DGameClient; }
	ModuleFactory *createModuleFactory() override { return NEW W3DModuleFactory; }
	ThingFactory *createThingFactory() override { return NEW W3DThingFactory; }
	FunctionLexicon *createFunctionLexicon() override { return NEW W3DFunctionLexicon; }
	LocalFileSystem *createLocalFileSystem() override { return NEW SDL3LocalFileSystem; }
	ArchiveFileSystem *createArchiveFileSystem() override { return NEW SDL3BIGFileSystem; }
	NetworkInterface *createNetwork() { return NetworkInterface::createNetwork(); }
	Radar *createRadar(Bool) override { return NEW W3DRadar; }
	ParticleSystemManager *createParticleSystemManager(Bool dummy) override
	{
		if (dummy)
			return NEW ParticleSystemManagerDummy;
		return NEW W3DParticleSystemManager;
	}
	AudioManager *createAudioManager(Bool) override { return NEW XAudio2AudioManager; }
};
