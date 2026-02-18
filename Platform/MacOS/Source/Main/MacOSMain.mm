#include "PreRTS.h"
#include "always.h"
#include <windows.h>  // macOS Win32 type shim
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

// Engine headers for real types
#include "Common/AsciiString.h"
#include "Common/CommandLine.h"
#include "Common/CriticalSection.h"
#include "Common/FramePacer.h"
#include "Common/FrameRateLimit.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "Common/MessageStream.h"
#include "Common/Money.h"
#include "Common/SubsystemInterface.h"
#include "Common/UnicodeString.h"
#include "Common/version.h"
#include "GameClient/ClientInstance.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Mouse.h"
#include "MacOSAudioManager.h"

// The big one that includes almost everything else
#include "GameLogic/GameLogic.h"
#include "Win32Device/Common/Win32GameEngine.h"

// Remaining specific headers
#include "Common/ArchiveFileSystem.h"
#include "Common/Snapshot.h"
#include <Utility/atlbase.h>

#include "Common/ArchiveFileSystem.h"
#include "Common/FileSystem.h"
#include "Common/LocalFileSystem.h"
#import "MacOSGameClient.h"
#import "MacOSWindowManager.h"
#include "StdBIGFileSystem.h"
#include "StdLocalFileSystem.h"

// Engine globals
const char *gAppPrefix = "";

#if defined(__APPLE__) && defined(__OBJC__)
// Window logic moved back down
#endif

// Globals
// Mouse *TheMouse = nullptr;
// Keyboard *TheKeyboard = nullptr;
// Version *TheVersion = nullptr; // Duplicate
// void *TheIMEManager = nullptr; // Duplicate
class CDManagerInterface;
CDManagerInterface *TheCDManager = nullptr;
void *TheMessageTimeManager = nullptr;
void *TheGameTimer = nullptr;
// FramePacer *TheFramePacer = nullptr; // Duplicate
extern "C" unsigned int TheMessageTime = 0;

extern "C" const GUID IID_IUnknown = {
    0x00000000,
    0x0000,
    0x0000,
    {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

char g_GeneralsCommandLine[2048] = {0};

GameEngine *CreateGameEngine(void) {
  Win32GameEngine *engine = NEW Win32GameEngine();
  engine->setIsActive(true);
  return engine;
}

class File;
const char *g_csfFile = "Data/English/Generals.csf";
const char *g_strFile = "Data/Generals.str";

// More engine globals
// void *TheLAN = nullptr; // Duplicate
// void *TheProjectedShadowManager = nullptr; // Duplicate
// void *TheGameSpyBuddyMessageQueue = nullptr; // Duplicate
// void *TheGameSpyGame = nullptr; // Duplicate
// void *TheGameSpyInfo = nullptr; // Duplicate
void *TheGameSpyChat = nullptr;
void *ApplicationHInstance = nullptr;
extern "C" void GetMacOSHardwareName(char *buffer, size_t size) {
  strncpy(buffer, "Apple Metal Graphics Support", size);
}
void *ApplicationHWnd = nullptr;

// D3DX stubs moved to D3DXStubs.cpp
#include <Utility/atlbase.h>
CComModule _Module;

// Memory manager will be handled by
// Core/GameEngine/Source/Common/System/GameMemory.cpp

// SubsystemInterface stubs
#if 0
// Snapshot stubs
Snapshot::Snapshot() {}
Snapshot::~Snapshot() {}
#endif

// Version stubs removed

#if 0
// Mouse stubs
Mouse::Mouse() {}
Mouse::~Mouse() {}
void Mouse::init() {}
void Mouse::onCursorMovedOutside() {}
void Mouse::onCursorMovedInside() {}
void Mouse::refreshCursorCapture() {}
Bool Mouse::isCursorInside() const { return true; }
void Mouse::regainFocus() {}
void Mouse::loseFocus() {}
void Mouse::setPosition(int, int) {}
void Mouse::setMouseLimits() {}
void Mouse::createStreamMessages() {}
void Mouse::draw() {}
void Mouse::parseIni() {}
void Mouse::reset() {}
void Mouse::update() {}

// Keyboard stubs
Keyboard::Keyboard() {}
Keyboard::~Keyboard() {}
void Keyboard::init() {}
void Keyboard::resetKeys() {}
void Keyboard::createStreamMessages() {}
void Keyboard::reset() {}
void Keyboard::update() {}

// CursorInfo stubs
CursorInfo::CursorInfo() {}

// Win32Mouse stubs
void Win32Mouse::addWin32Event(unsigned int, unsigned long, long,
                               unsigned int) {}

// CommandLine stubs
void CommandLine::parseCommandLineForStartup() {
  if (TheWritableGlobalData == nullptr) {
    TheWritableGlobalData = new GlobalData();
  }
}

// GlobalData stubs
GlobalData::GlobalData() {
  m_headless = false;
  m_windowed = true;
  m_xResolution = 800;
  m_yResolution = 600;

#ifdef __APPLE__
  // On Mac, we use ~/Library/Application Support/Generals
  NSString *appName = @"Generals Zero Hour";
  NSArray *paths = NSSearchPathForDirectoriesInDomains(
      NSApplicationSupportDirectory, NSUserDomainMask, YES);
  NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : @"./";
  NSString *userDataPath = [basePath stringByAppendingPathComponent:appName];

  // Create directory if it doesn't exist
  [[NSFileManager defaultManager] createDirectoryAtPath:userDataPath
                            withIntermediateDirectories:YES
                                             attributes:nil
                                                  error:nil];

  m_userDataDir = [userDataPath UTF8String];
  m_userDataDir.concat("/");
#else
  m_userDataDir = "./";
#endif

  fprintf(stderr, "USER DATA DIRECTORY: %s\n", m_userDataDir.str());
}
GlobalData::~GlobalData() {}
void GlobalData::init() {}
void GlobalData::reset() {}
Bool GlobalData::setTimeOfDay(TimeOfDay tod) { return false; }

// Stubs removed for Version, Money, FramePacer, INI.

// AudioManager stubs
AudioManager::AudioManager() {}
AudioManager::~AudioManager() {}
void AudioManager::muteAudio(AudioManager::MuteAudioReason) {}
void AudioManager::unmuteAudio(AudioManager::MuteAudioReason) {}
Bool AudioManager::isValidAudioEvent(const AudioEventRTS *) const {
  return false;
}
Bool AudioManager::isValidAudioEvent(AudioEventRTS *) const { return false; }
AudioEventInfo *AudioManager::findAudioEventInfo(AsciiString) const {
  return nullptr;
}
const Coord3D *AudioManager::getListenerPosition() const { return nullptr; }
void AudioManager::getInfoForAudioEvent(const AudioEventRTS *) const {}
Bool AudioManager::isMusicAlreadyLoaded() const { return false; }
Bool AudioManager::isOn(AudioAffect) const { return false; }
void AudioManager::findAllAudioEventsOfType(AudioType,
                                            std::vector<AudioEventInfo *> &) {}
void AudioManager::setAudioEventVolumeOverride(AsciiString, Real) {}
Bool AudioManager::isCurrentSpeakerTypeSurroundSound() { return false; }
UnsignedInt
AudioManager::translateSpeakerTypeToUnsignedInt(const AsciiString &) {
  return 0;
}
AsciiString AudioManager::translateUnsignedIntToSpeakerType(UnsignedInt) {
  return "";
}
void AudioManager::removeLevelSpecificAudioEventInfos() {}
Bool AudioManager::isCurrentProviderHardwareAccelerated() { return false; }
void AudioManager::init() {}
void AudioManager::reset() {}
void AudioManager::setOn(Bool, AudioAffect) {}
void AudioManager::update() {}
Real AudioManager::getVolume(AudioAffect) { return 1.0f; }
void AudioManager::setVolume(Real, AudioAffect) {}
AudioHandle AudioManager::addAudioEvent(const AudioEventRTS *) { return 0; }
void AudioManager::postProcessLoad() {}
Real AudioManager::getAudioLengthMS(const AudioEventRTS *) { return 0.0f; }
void AudioManager::removeAudioEvent(AsciiString) {}
void AudioManager::removeAudioEvent(UnsignedInt) {}
void AudioManager::addAudioEventInfo(AudioEventInfo *) {}
AudioHandle AudioManager::allocateNewHandle() { return 0; }
AudioEventInfo *AudioManager::newAudioEventInfo(AsciiString) { return nullptr; }
Bool AudioManager::shouldPlayLocally(const AudioEventRTS *) { return true; }
// void AudioManager::appendAudioRequest(AudioRequest *) {}
Bool AudioManager::isCurrentlyPlaying(AudioHandle) { return false; }
void AudioManager::processRequestList() {}
void AudioManager::releaseAudioRequest(AudioRequest *) {}
void AudioManager::setListenerPosition(const Coord3D *, const Coord3D *) {}
AudioRequest *AudioManager::allocateAudioRequest(Bool) { return nullptr; }
void AudioManager::releaseAudioEventRTS(AudioEventRTS *&) {}
void AudioManager::removeDisabledEvents() {}
void AudioManager::setAudioEventEnabled(AsciiString, Bool) {}
void AudioManager::set3DVolumeAdjustment(Real) {}
void AudioManager::refreshCachedVariables() {}
#endif

// GameEngine stubs
// GameEngine implementations moved to GameEngine.cpp

// Win32GameEngine stubs
Win32GameEngine::Win32GameEngine() {}
Win32GameEngine::~Win32GameEngine() {}
void Win32GameEngine::init() {
  printf("DEBUG: Win32GameEngine::init entering\n");
  fflush(stdout);

  if (!TheWritableGlobalData) {
    TheWritableGlobalData = new GlobalData();
  }

  // Ensure we use our factory for the client
  if (!TheGameClient) {
    printf("DEBUG: Creating MacOSGameClient via factory...\n");
    fflush(stdout);
    TheGameClient = createGameClient();
  }

  GameEngine::init();

  if (!TheSubsystemList) {
    TheSubsystemList = new SubsystemInterfaceList();
  }

  printf("DEBUG: Win32GameEngine::init exiting. TheGameClient=%p TheMouse=%p\n",
         TheGameClient, TheMouse);
  fflush(stdout);
}

void Win32GameEngine::reset() { GameEngine::reset(); }
void Win32GameEngine::update() {
  // THE HEARTBEAT: W3D now owns the Metal frame lifecycle
  // (BeginScene/Clear/Present happen inside WW3D::Begin_Render/End_Render)
  // MacOSRenderDevice BeginScene/EndScene removed to avoid stealing the
  // drawable.

  // The engine relies on TheMessageTime for GUI transitions and animations.
  TheMessageTime = timeGetTime();

  GameEngine::update();

  serviceWindowsOS();
}
void Win32GameEngine::serviceWindowsOS() {
  // First, pump native macOS events
#if defined(__APPLE__) && defined(__OBJC__)
  MacOS_PumpEvents();

  if (MacOS_ShouldQuit() && TheGameEngine) {
    TheGameEngine->setQuitting(true);
  }
#endif

  // Then pump the Windows-compatible message queue for any internal game
  // messages
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TheMessageTime = msg.time;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Ensure TheMessageTime is updated even if there are no messages,
  // to prevent UI freezing.
  TheMessageTime = timeGetTime();
}

// Factory methods now as stubs to avoid linking W3D classes
GameLogic *Win32GameEngine::createGameLogic(void) {
  fprintf(stderr, "FACTORY: createGameLogic\n");
  return new GameLogic();
}

// Headers for factory return types
#include "Common/FunctionLexicon.h"
#include "Common/ModuleFactory.h"
#include "Common/Radar.h"
#include "Common/ThingFactory.h"
#include "GameClient/ParticleSys.h" // Defines ParticleSystemManager
#include "GameNetwork/NetworkInterface.h"
// WebBrowser is in GameNetwork/WOLBrowser/WebBrowser.h but let's check exact
// include path validness If it fails, we will fix it. Based on find output:
// Core/GameEngine/Include/GameNetwork/WOLBrowser/WebBrowser.h
#include "GameNetwork/WOLBrowser/WebBrowser.h"

// Generic stub subsystems

// Stub subsystems for interfaces
class StubNetwork : public NetworkInterface {
public:
  void init() override {}
  void reset() override {}
  void update() override {}

  // Stub implementation of pure virtual methods
  virtual Bool initNetwork(void) { return true; }
  virtual void shutdownNetwork(void) {}
  virtual Bool updateNetwork(void) { return true; }
  virtual Real getIncomingBytesPerSecond(void) { return 0; }
  virtual Real getIncomingPacketsPerSecond(void) { return 0; }
  virtual Real getOutgoingBytesPerSecond(void) { return 0; }
  virtual Real getOutgoingPacketsPerSecond(void) { return 0; }
  virtual Real getUnknownBytesPerSecond(void) { return 0; }
  virtual Real getUnknownPacketsPerSecond(void) { return 0; }
  virtual void updateLoadProgress(Int percent) {}
  virtual void loadProgressComplete(void) {}
  virtual void sendTimeOutGameStart(void) {}
  virtual UnsignedInt getLocalPlayerID() { return 0; }
  virtual UnicodeString getPlayerName(Int playerNum) { return UnicodeString(); }
  virtual Int getNumPlayers() { return 1; }
  virtual Int getAverageFPS() { return 30; }
  virtual Int getSlotAverageFPS(Int slot) { return 30; }
  virtual void attachTransport(Transport *transport) {}
  virtual void initTransport() {}
  virtual Bool sawCRCMismatch() { return false; }
  virtual void setSawCRCMismatch() {}
  virtual Bool isPlayerConnected(Int playerID) { return true; }
  virtual void notifyOthersOfCurrentFrame() {}
  virtual void notifyOthersOfNewFrame(UnsignedInt frame) {}
  virtual Int getExecutionFrame() { return 0; }
  virtual UnsignedInt getPingFrame() { return 0; }
  virtual Int getPingsSent() { return 0; }
  virtual Int getPingsReceived() { return 0; }
  virtual void liteupdate(void) {}
  virtual void setLocalAddress(UnsignedInt ip, UnsignedInt port) {}
  virtual Bool isFrameDataReady(void) { return false; }
  virtual Bool isStalling() { return false; }
  virtual void parseUserList(const GameInfo *game) {}
  virtual void startGame(void) {}
  virtual UnsignedInt getRunAhead(void) { return 0; }
  virtual UnsignedInt getFrameRate(void) { return 30; }
  virtual UnsignedInt getPacketArrivalCushion(void) { return 0; }
  virtual void sendChat(UnicodeString text, Int playerMask) {}
  virtual void sendDisconnectChat(UnicodeString text) {}
  virtual void sendFile(AsciiString path, UnsignedByte playerMask,
                        UnsignedShort commandID) {}
  virtual UnsignedShort sendFileAnnounce(AsciiString path,
                                         UnsignedByte playerMask) {
    return 0;
  }
  virtual Int getFileTransferProgress(Int playerID, AsciiString path) {
    return 0;
  }
  virtual Bool areAllQueuesEmpty(void) { return true; }
  virtual void quitGame() {}
  virtual void selfDestructPlayer(Int index) {}
  virtual void voteForPlayerDisconnect(Int slot) {}
  virtual Bool isPacketRouter(void) { return false; }
  virtual void toggleNetworkOn(void) override {}
};

class StubParticleSystemManager : public ParticleSystemManager {
public:
  void init() override {}
  void reset() override {}
  void update() override {}

  // Pure virtuals from ParticleSystemManager
  virtual Int getOnScreenParticleCount(void) { return 0; }
  virtual void doParticles(RenderInfoClass &rinfo) {}
  virtual void queueParticleRender() {}
};

class StubWebBrowser : public WebBrowser {
public:
  void init() override {}
  void reset() override {}
  void update() override {}

  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
                                                   void **ppvObject) {
    return E_NOINTERFACE;
  }
  virtual ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
  virtual ULONG STDMETHODCALLTYPE Release(void) { return 1; }

  virtual Bool createBrowserWindow(const char *tag, GameWindow *win) {
    return false;
  }
  virtual void closeBrowserWindow(GameWindow *win) {}
};

// FACTORY METHODS
GameClient *Win32GameEngine::createGameClient(void) {
  fprintf(stderr, "FACTORY: createGameClient\n");
  return new MacOSGameClient();
}

LocalFileSystem *Win32GameEngine::createLocalFileSystem(void) {
  fprintf(stderr, "FACTORY: createLocalFileSystem\n");
  return new StdLocalFileSystem;
}

ArchiveFileSystem *Win32GameEngine::createArchiveFileSystem(void) {
  fprintf(stderr, "FACTORY: createArchiveFileSystem\n");
  return new StdBIGFileSystem;
}

ModuleFactory *Win32GameEngine::createModuleFactory(void) {
  fprintf(stderr, "FACTORY: createModuleFactory\n");
  return new ModuleFactory();
}
ThingFactory *Win32GameEngine::createThingFactory(void) {
  fprintf(stderr, "FACTORY: createThingFactory\n");
  return new ThingFactory();
}
FunctionLexicon *Win32GameEngine::createFunctionLexicon(void) {
  fprintf(stderr, "FACTORY: createFunctionLexicon (W3D)\n");
  return new W3DFunctionLexicon();
}
ParticleSystemManager *Win32GameEngine::createParticleSystemManager(void) {
  fprintf(stderr, "FACTORY: createParticleSystemManager\n");
  return new StubParticleSystemManager();
}
NetworkInterface *Win32GameEngine::createNetwork(void) {
  fprintf(stderr, "FACTORY: createNetwork\n");
  return new StubNetwork();
}
Radar *Win32GameEngine::createRadar(void) {
  fprintf(stderr, "FACTORY: createRadar\n");
  return new RadarDummy();
}
WebBrowser *Win32GameEngine::createWebBrowser(void) {
  fprintf(stderr, "FACTORY: createWebBrowser\n");
  return new StubWebBrowser();
}
AudioManager *Win32GameEngine::createAudioManager(void) {
  fprintf(stderr, "!!!! MACOS AUDIOMANAGER CREATED !!!!\n");
  return ::new MacOSAudioManager();
}

#if 0
// ClientInstance stubs
namespace rts {
Bool ClientInstance::initialize() { return true; }
const char *ClientInstance::getFirstInstanceName() { return "Generals"; }
} // namespace rts
#endif

#if 0
// GameLogic stubs
GameLogic::GameLogic() {}
GameLogic::~GameLogic() {}
void GameLogic::init() {}
void GameLogic::reset() {}
void GameLogic::update() {}
void GameLogic::loadPostProcess() {}
void GameLogic::crc(Xfer *) {}
void GameLogic::xfer(Xfer *) {}
TerrainLogic *GameLogic::createTerrainLogic(void) { return nullptr; }
GhostObjectManager *GameLogic::createGhostObjectManager(void) {
  return nullptr;
}

// WebBrowser stubs
WebBrowser::WebBrowser() {}
WebBrowser::~WebBrowser() {}
void WebBrowser::init() {}
void WebBrowser::reset() {}
void WebBrowser::update() {}
STDMETHODIMP WebBrowser::TestMethod(Int) { return S_OK; }
#endif

extern "C" const GUID IID_IBrowserDispatch = {
    0x12345678,
    0x1234,
    0x1234,
    {0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56}};

#if 0
// ModuleFactory stubs
ModuleFactory::ModuleFactory() {}
ModuleFactory::~ModuleFactory() {}
void ModuleFactory::init() {}
void ModuleFactory::crc(Xfer *) {}
void ModuleFactory::xfer(Xfer *) {}
void ModuleFactory::loadPostProcess() {}
#endif

// Other base classes stubs
// GameClient methods moved to real implementation

// GameClient methods
// GameClient::GameClient() {}
// GameClient::~GameClient() {}
// void GameClient::init() {}
// void GameClient::reset() {}
// // void GameClient::update() {}
// void GameClient::loadPostProcess() {}
// void GameClient::crc(Xfer *) {}
// void GameClient::xfer(Xfer *) {}
#if 0
void GameClient::setTimeOfDay(TimeOfDay) {}
void GameClient::preloadAssets(TimeOfDay) {}
void GameClient::releaseShadows() {}
void GameClient::allocateShadows() {}
void GameClient::destroyDrawable(Drawable *) {}
void GameClient::getRayEffectData(Drawable *, RayEffectData *) {}
void GameClient::registerDrawable(Drawable *) {}
void GameClient::removeFromRayEffects(Drawable *) {}
GameMessage::Type
GameClient::evaluateContextCommand(Drawable *, const Coord3D *,
                                   CommandTranslator::CommandEvaluateType) {
  return GameMessage::MSG_INVALID;
}
void GameClient::selectDrawablesInGroup(Int) {}
void GameClient::iterateDrawablesInRegion(Region3D *, GameClientFuncPtr,
                                          void *) {}
void GameClient::assignSelectedDrawablesToGroup(Int) {}
Bool GameClient::loadMap(AsciiString) { return true; }
void GameClient::unloadMap(AsciiString) {}
#endif

#if 0
TerrainLogic::TerrainLogic() {}
TerrainLogic::~TerrainLogic() {}
void TerrainLogic::init() {}
void TerrainLogic::reset() {}
void TerrainLogic::update() {}
void TerrainLogic::loadPostProcess() {}
void TerrainLogic::crc(Xfer *) {}
void TerrainLogic::xfer(Xfer *) {}
Bool TerrainLogic::isCliffCell(Real, Real) const { return false; }
Bridge *TerrainLogic::findBridgeAt(Coord3D const *) const { return nullptr; }
Real TerrainLogic::getLayerHeight(Real, Real, PathfindLayerEnum, Coord3D *,
                                  Bool) const {
  return 0;
}
Real TerrainLogic::getGroundHeight(Real, Real, Coord3D *) const { return 0; }
Bridge *TerrainLogic::findBridgeLayerAt(Coord3D const *, PathfindLayerEnum,
                                        Bool) const {
  return nullptr;
}
Bool TerrainLogic::isClearLineOfSight(Coord3D const &, Coord3D const &) const {
  return true;
}
Coord3D TerrainLogic::findClosestEdgePoint(Coord3D const *) const {
  return Coord3D();
}
Coord3D TerrainLogic::findFarthestEdgePoint(Coord3D const *) const {
  return Coord3D();
}
Bool TerrainLogic::objectInteractsWithBridgeEnd(Object *, Int) const {
  return false;
}
Bool TerrainLogic::objectInteractsWithBridgeLayer(Object *, Int, Bool) const {
  return false;
}
void TerrainLogic::newMap(Bool) {}
Bool TerrainLogic::loadMap(AsciiString, Bool) { return true; }
Drawable *TerrainLogic::pickBridge(const Vector3 &, const Vector3 &,
                                   Vector3 *) {
  return nullptr;
}
void TerrainLogic::deleteBridge(Bridge *) {}
Bool TerrainLogic::isUnderwater(Real, Real, Real *, Real *) { return false; }
PathfindLayerEnum TerrainLogic::alignOnTerrain(Real, const Coord3D &, Bool,
                                               Matrix3D &) {
  return LAYER_GROUND;
}
const WaterHandle *TerrainLogic::getWaterHandle(Real, Real) { return nullptr; }
Real TerrainLogic::getWaterHeight(const WaterHandle *) { return 0.0f; }
void TerrainLogic::setWaterHeight(const WaterHandle *, Real, Real, Bool) {}
Waypoint *TerrainLogic::getWaypointByID(UnsignedInt) { return nullptr; }
Bool TerrainLogic::isPurposeOfPath(Waypoint *, AsciiString) { return false; }
void TerrainLogic::addBridgeToLogic(BridgeInfo *, Dict *, AsciiString) {}
Waypoint *TerrainLogic::getWaypointByName(AsciiString) { return nullptr; }
PolygonTrigger *TerrainLogic::getTriggerAreaByName(AsciiString) {
  return nullptr;
}
const WaterHandle *TerrainLogic::getWaterHandleByName(AsciiString) {
  return nullptr;
}
void TerrainLogic::addLandmarkBridgeToLogic(Object *) {}
Waypoint *TerrainLogic::getClosestWaypointOnPath(const Coord3D *, AsciiString) {
  return nullptr;
}
void TerrainLogic::updateBridgeDamageStates() {}
void TerrainLogic::changeWaterHeightOverTime(const WaterHandle *, Real, Real,
                                             Real) {}

ThingFactory::ThingFactory() {}
ThingFactory::~ThingFactory() {}
void ThingFactory::init() {}
void ThingFactory::reset() {}
void ThingFactory::update() {}
void ThingFactory::postProcessLoad() {}

FunctionLexicon::FunctionLexicon() {}
FunctionLexicon::~FunctionLexicon() {}
void FunctionLexicon::init() {}
void FunctionLexicon::reset() {}
void FunctionLexicon::update() {}

// Stubs for remaining classes

GhostObjectManager::GhostObjectManager() {}
GhostObjectManager::~GhostObjectManager() {}
void GhostObjectManager::removeGhostObject(GhostObject *) {}
void GhostObjectManager::releasePartitionData() {}
void GhostObjectManager::restorePartitionData() {}
void GhostObjectManager::updateOrphanedObjects(Int *, Int) {}
GhostObject *GhostObjectManager::addGhostObject(Object *, PartitionData *) {
  return nullptr;
}
void GhostObjectManager::loadPostProcess() {}
void GhostObjectManager::crc(Xfer *) {}
void GhostObjectManager::xfer(Xfer *) {}
void GhostObjectManager::reset() {}

ParticleSystemManager::ParticleSystemManager() {}
ParticleSystemManager::~ParticleSystemManager() {}
void ParticleSystemManager::preloadAssets(TimeOfDay) {}
void ParticleSystemManager::setOnScreenParticleCount(Int) {}
void ParticleSystemManager::loadPostProcess() {}
void ParticleSystemManager::crc(Xfer *) {}
void ParticleSystemManager::init() {}
void ParticleSystemManager::xfer(Xfer *) {}
void ParticleSystemManager::reset() {}
void ParticleSystemManager::update() {}

Radar::Radar() {}
Radar::~Radar() {}
void Radar::loadPostProcess() {}
void Radar::crc(Xfer *) {}
void Radar::xfer(Xfer *) {}
Bool Radar::removeObject(Object *) { return false; }
void Radar::refreshTerrain(TerrainLogic *) {}
void Radar::queueTerrainRefresh() {}
void Radar::newMap(TerrainLogic *) {}
void Radar::update() {}
Bool Radar::addObject(Object *) { return false; }
void Radar::reset() {}
#endif

// Memory critical sections and allocator will be handled by GameMemory.cpp

// operator new/delete handled by GameMemory.cpp

#include "Common/OSDisplay.h"
OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m,
                                        UnsignedInt buttonFlags,
                                        UnsignedInt otherFlags) {
  fprintf(stderr, "OSDisplayWarningBox: %s - %s\n", p.str(), m.str());
  return OSDBT_OK;
}

void OSDisplaySetBusyState(Bool busyDisplay, Bool busySystem) {}

// Memory manager init handled by GameMemory.cpp

// Window logic moved to MacOSWindowManager.mm

#if 0
Int GameMain() {
  fprintf(stderr, "GameMain entering\n");

  void *window =
      MacOS_CreateWindow(800, 600, "Command & Conquer Generals (macOS)");
  MacOS_InitRenderer(window);

  TheFramePacer = new FramePacer();
  TheGameEngine = CreateGameEngine();

  // Initialize basic subsystems
  CommandLine::parseCommandLineForStartup();

  TheGameEngine->init();

  // Ensure we can find our data files by showing where we are
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    fprintf(stderr, "Game running in: %s\n", cwd);
  }

  // Final check - we want to see SUCCESS here if we run from the project root
  if (TheFileSystem && TheFileSystem->doesFileExist("build_macos.sh")) {
    fprintf(stderr, "✅ FILESYSTEM READY: Data access verified.\n");
  } else {
    fprintf(stderr, "⚠️ FILESYSTEM WARNING: build_macos.sh not found. Check "
                    "Working Directory!\n");
  }

  TheGameEngine->execute();

  delete TheGameEngine;
  TheGameEngine = nullptr;
  delete TheFramePacer;
  TheFramePacer = nullptr;
  fprintf(stderr, "GameMain exiting\n");
  return 0;
}
#endif

void *gLoadScreenBitmap = nullptr;
class InGameUI;
// InGameUI *TheInGameUI = nullptr; // Duplicate
class ControlBar;
// ControlBar *TheControlBar = nullptr; // Duplicate

// Registry stubs
#if 0
Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key,
                                   AsciiString &val) {
  return FALSE;
}
Bool GetStringFromRegistry(AsciiString path, AsciiString key,
                           AsciiString &val) {
  return FALSE;
}
#endif
// ... other registry stubs were actually needed if not linked
// Wait! GetStringFromRegistry was a duplicate.
// So I keep Registry stubs disabled.

#if 0
Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key,
                                UnsignedInt &val) {
  return FALSE;
}
Bool SetStringInRegistry(AsciiString path, AsciiString key, AsciiString val) {
  return TRUE;
}
Bool SetUnsignedIntInRegistry(AsciiString path, AsciiString key,
                              UnsignedInt val) {
  return TRUE;
}

bool SetStringInRegistry(std::string path, std::string key, std::string val) {
  return true;
}
bool SetUnsignedIntInRegistry(std::string path, std::string key,
                              unsigned int val) {
  return true;
}
bool GetStringFromRegistry(std::string path, std::string key,
                           std::string &val) {
  return false;
}
bool GetUnsignedIntFromRegistry(std::string path, std::string key,
                                unsigned int &val) {
  return false;
}

AsciiString GetRegistryLanguage(void) { return "English"; }
AsciiString GetRegistryGameName(void) { return "Generals"; }
UnsignedInt GetRegistryVersion(void) { return 0; }
UnsignedInt GetRegistryMapPackVersion(void) { return 0; }
#endif

#include "Common/CDManager.h"
class CDManagerStub : public CDManagerInterface {
public:
  void init() override {}
  void reset() override {}
  void update() override {}
  Int driveCount() override { return 0; }
  CDDriveInterface *getDrive(Int index) override { return nullptr; }
  CDDriveInterface *newDrive(const char *path) override { return nullptr; }
  void refreshDrives() override {}
  void destroyAllDrives() override {}

protected:
  CDDriveInterface *createDrive() override { return nullptr; }
};

CDManagerInterface *CreateCDManager() {
  static CDManagerStub stub;
  printf("CreateCDManager: returning %p\n", (void *)&stub);
  fflush(stdout);
  return &stub;
}

#include "Common/Debug.h"
#include "Common/GameMemory.h"

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void signal_handler(int sig) {
  fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  fprintf(stderr, "Signal received: %d\n", sig);
  void *array[50];
  size_t size = backtrace(array, 50);
  char **strings = backtrace_symbols(array, size);
  if (strings) {
    for (size_t i = 0; i < size; i++) {
      fprintf(stderr, "  %s\n", strings[i]);
    }
    free(strings);
  }
  fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  fflush(stderr);
  exit(sig);
}

void on_exit_handler() {
  fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  fprintf(stderr, "atexit handler triggered! Stack trace follows:\n");
  void *array[50];
  size_t size = backtrace(array, 50);
  char **strings = backtrace_symbols(array, size);
  if (strings) {
    for (size_t i = 0; i < size; i++) {
      fprintf(stderr, "  %s\n", strings[i]);
    }
    free(strings);
  }
  fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  fflush(stderr);
}

int main(int argc, char *argv[]) {
  signal(SIGSEGV, signal_handler);
  signal(SIGBUS, signal_handler);
  signal(SIGILL, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  atexit(on_exit_handler);
  // Memory manager initialization will happen automatically on first allocation

  initMemoryManager();
  DEBUG_INIT(0);

  // Fill g_GeneralsCommandLine
  int pos = 0;
  for (int i = 0; i < argc; ++i) {
    int len = strlen(argv[i]);
    if (pos + len + 2 >= 2048)
      break;
    if (i > 0)
      g_GeneralsCommandLine[pos++] = ' ';
    strcpy(&g_GeneralsCommandLine[pos], argv[i]);
    pos += len;
  }

  return MacOS_Main(argc, argv);
}
