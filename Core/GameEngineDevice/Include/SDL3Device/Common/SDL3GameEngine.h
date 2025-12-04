#pragma once

#include "Common/GameEngine.h"

class SDL3GameEngine : public GameEngine
{
public:
	SDL3GameEngine();
	~SDL3GameEngine() override;

	void init(void) override;
	void reset(void) override;
	void update(void) override;
	void serviceWindowsOS(void) override;

protected:
	GameLogic *createGameLogic(void) override;
	GameClient *createGameClient(void) override;
	ModuleFactory *createModuleFactory(void) override;
	ThingFactory *createThingFactory(void) override;
	FunctionLexicon *createFunctionLexicon(void) override;
	LocalFileSystem *createLocalFileSystem(void) override;
	ArchiveFileSystem *createArchiveFileSystem(void) override;
	Radar *createRadar(void) override;
	WebBrowser *createWebBrowser(void) override;
	ParticleSystemManager *createParticleSystemManager(void) override;
	AudioManager *createAudioManager(void) override;
};
