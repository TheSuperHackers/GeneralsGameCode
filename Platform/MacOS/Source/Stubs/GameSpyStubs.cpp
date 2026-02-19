// GameSpyStubs.cpp — Linker stubs for GameSpy/WOL/Network/Win32 symbols on macOS
// Online multiplayer is not available on macOS. These are all no-ops.

#include "PreRTS.h"
#include <string>

// ── Safe headers ──
#include "GameNetwork/LANGameInfo.h"
#include "GameNetwork/LANAPI.h"
#include "GameNetwork/GameSpy/LadderDefs.h"
#include "GameNetwork/Transport.h"
#include "GameNetwork/IPEnumeration.h"
#include "GameNetwork/NAT.h"
#include "GameNetwork/User.h"
#include "GameNetwork/DownloadManager.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/GameSpy/StagingRoomGameInfo.h"
#include "GameNetwork/GameSpy/GameResultsThread.h"
#include "Common/WorkerProcess.h"
#include "Common/INI.h"
#include "DbgHelpGuard.h"
#include "registry.h"
#include "dx8webbrowser.h"

// ── Forward declarations for pointer-only singletons ──
class GameSpyConfigInterface;
class GameSpyInfoInterface;
class GameSpyPeerMessageQueueInterface;
class GameSpyBuddyMessageQueueInterface;
class PingerInterface;
class IMEManagerInterface;

// ══════════════════════════════════════════════════════
//  NULL SINGLETONS
// ══════════════════════════════════════════════════════

GameSpyConfigInterface *TheGameSpyConfig = nullptr;
GameSpyInfoInterface *TheGameSpyInfo = nullptr;
GameSpyPeerMessageQueueInterface *TheGameSpyPeerMessageQueue = nullptr;
GameSpyBuddyMessageQueueInterface *TheGameSpyBuddyMessageQueue = nullptr;
PingerInterface *ThePinger = nullptr;
GameResultsInterface *TheGameResultsQueue = nullptr;
NAT *TheNAT = nullptr;
LANAPI *TheLAN = nullptr;
DownloadManager *TheDownloadManager = nullptr;
IMEManagerInterface *TheIMEManager = nullptr;
LadderList *TheLadderList = nullptr;

// TheGameSpyGame is declared in StagingRoomGameInfo.h as GameSpyStagingRoom*
// Both Generals and ZH build targets link against macos_platform
GameSpyStagingRoom *TheGameSpyGame = nullptr;

// ── Colors & flags ──
#ifndef GSCOLOR_MAX
#define GSCOLOR_MAX 16
#endif
Color GameSpyColor[GSCOLOR_MAX] = {};
Color acceptTrueColor = 0x00FF00FF;
Color acceptFalseColor = 0xFF0000FF;
Color chatSystemColor = 0xFFFF00FF;
Bool isThreadHosting = FALSE;
char g_LastErrorDump[1] = {0};

// ── IME ──
IMEManagerInterface *CreateIMEManagerInterface(void) { return nullptr; }

// ══════════════════════════════════════════════════════
//  GAMESPY OVERLAY FUNCTIONS
// ══════════════════════════════════════════════════════

void GSMessageBoxOk(UnicodeString, UnicodeString, GameWinMsgBoxFunc) {}
void RaiseGSMessageBox(void) {}
void ReOpenPlayerInfo(void) {}
void CheckReOpenPlayerInfo(void) {}
void GameSpyOpenOverlay(GSOverlayType) {}
void GameSpyCloseOverlay(GSOverlayType) {}
void GameSpyCloseAllOverlays(void) {}
Bool GameSpyIsOverlayOpen(GSOverlayType) { return FALSE; }
void GameSpyToggleOverlay(GSOverlayType) {}
void GameSpyUpdateOverlays(void) {}

// ══════════════════════════════════════════════════════
//  LOBBY / GAME LIST
// ══════════════════════════════════════════════════════

GameWindow *GetGameListBox(void) { return nullptr; }
NameKeyType GetGameListBoxID(void) { return NAMEKEY_INVALID; }
GameWindow *GetGameInfoListBox(void) { return nullptr; }
void GrabWindowInfo(void) {}
void ReleaseWindowInfo(void) {}
void RefreshGameInfoListBox(GameWindow *, GameWindow *) {}
void RefreshGameListBoxes(void) {}
void ToggleGameListType(void) {}
Bool HandleSortButton(NameKeyType) { return FALSE; }

class WinInstanceData;
void playerTemplateComboBoxTooltip(GameWindow *, WinInstanceData *, UnsignedInt) {}
void playerTemplateListBoxTooltip(GameWindow *, WinInstanceData *, UnsignedInt) {}

// ══════════════════════════════════════════════════════
//  NETWORK / PATCH
// ══════════════════════════════════════════════════════

void HTTPThinkWrapper(void) {}
void StartPatchCheck(void) {}
void CancelPatchCheckCallback(void) {}
void StartDownloadingPatches(void) {}
void StopAsyncDNSCheck(void) {}
void SetUpGameSpy(const char *, const char *) {}
void TearDownGameSpy(void) {}
AsciiString GenerateGameOptionsString(void) { return AsciiString::TheEmptyString; }

// ══════════════════════════════════════════════════════
//  REGISTRY (std::string-based, from WWDownload)
// ══════════════════════════════════════════════════════

bool GetStringFromRegistry(std::string, std::string, std::string &) { return false; }
bool GetUnsignedIntFromRegistry(std::string, std::string, unsigned int &) { return false; }
bool SetStringInRegistry(std::string, std::string, std::string) { return false; }
bool SetUnsignedIntInRegistry(std::string, std::string, unsigned int) { return false; }

// ══════════════════════════════════════════════════════
//  STRING CONVERSION
// ══════════════════════════════════════════════════════

std::wstring MultiByteToWideCharSingleLine(const char *orig) {
  if (!orig) return L"";
  std::wstring r; while (*orig) r += (wchar_t)(unsigned char)*orig++; return r;
}
std::string WideCharStringToMultiByte(const wchar_t *orig) {
  if (!orig) return "";
  std::string r; while (*orig) r += (char)*orig++; return r;
}

// ══════════════════════════════════════════════════════
//  DEBUG STUBS
// ══════════════════════════════════════════════════════

void FillStackAddresses(void **, unsigned int, unsigned int) {}
void StackDumpFromAddresses(void **, unsigned int, void (*)(const char *)) {}
DbgHelpGuard::DbgHelpGuard() : m_needsUnload(false) {}
DbgHelpGuard::~DbgHelpGuard() {}
void DbgHelpGuard::activate() {}
void DbgHelpGuard::deactivate() {}

// ══════════════════════════════════════════════════════
//  DX8 WEB BROWSER
// ══════════════════════════════════════════════════════

bool DX8WebBrowser::Initialize(const char *, const char *, const char *, const char *) { return false; }
void DX8WebBrowser::Render(int) {}
void DX8WebBrowser::Update() {}
void DX8WebBrowser::Shutdown() {}

// ══════════════════════════════════════════════════════
//  TRANSPORT / UDP
// ══════════════════════════════════════════════════════

Transport::Transport() : m_port(0), m_winsockInit(FALSE), m_udpsock(nullptr),
  m_useLatency(FALSE), m_usePacketLoss(FALSE), m_statisticsSlot(0), m_lastSecond(0) {
  memset(m_incomingBytes, 0, sizeof(m_incomingBytes));
  memset(m_unknownBytes, 0, sizeof(m_unknownBytes));
  memset(m_outgoingBytes, 0, sizeof(m_outgoingBytes));
  memset(m_incomingPackets, 0, sizeof(m_incomingPackets));
  memset(m_unknownPackets, 0, sizeof(m_unknownPackets));
  memset(m_outgoingPackets, 0, sizeof(m_outgoingPackets));
}
Transport::~Transport() {}
Bool Transport::init(AsciiString, UnsignedShort) { return FALSE; }
Bool Transport::init(UnsignedInt, UnsignedShort) { return FALSE; }
void Transport::reset() {}
Bool Transport::update() { return FALSE; }
Bool Transport::doRecv() { return FALSE; }
Bool Transport::doSend() { return FALSE; }
Bool Transport::queueSend(UnsignedInt, UnsignedShort, const UnsignedByte *, Int) { return FALSE; }
Real Transport::getIncomingBytesPerSecond() { return 0; }
Real Transport::getIncomingPacketsPerSecond() { return 0; }
Real Transport::getOutgoingBytesPerSecond() { return 0; }
Real Transport::getOutgoingPacketsPerSecond() { return 0; }
Real Transport::getUnknownBytesPerSecond() { return 0; }
Real Transport::getUnknownPacketsPerSecond() { return 0; }

UDP::UDP() {}
UDP::~UDP() {}
Int UDP::Bind(UnsignedInt, UnsignedShort) { return -1; }
Int UDP::Bind(const char *, UnsignedShort) { return -1; }
Int UDP::Read(unsigned char *, UnsignedInt, sockaddr_in *) { return 0; }
Int UDP::Write(const unsigned char *, UnsignedInt, UnsignedInt, UnsignedShort) { return 0; }

// ══════════════════════════════════════════════════════
//  IP ENUMERATION
// ══════════════════════════════════════════════════════

IPEnumeration::IPEnumeration() {}
IPEnumeration::~IPEnumeration() {}
AsciiString IPEnumeration::getMachineName() { return AsciiString::TheEmptyString; }
EnumeratedIP *IPEnumeration::getAddresses() { return nullptr; }

// ══════════════════════════════════════════════════════
//  LAN API (all virtual methods must have implementations)
// ══════════════════════════════════════════════════════

LANAPI::LANAPI() {}
LANAPI::~LANAPI() {}
void LANAPI::init() {}
void LANAPI::reset() {}
void LANAPI::update() {}
void LANAPI::setIsActive(Bool) {}
void LANAPI::RequestLocations() {}
void LANAPI::RequestGameJoin(LANGameInfo *, UnsignedInt) {}
void LANAPI::RequestGameJoinDirectConnect(UnsignedInt) {}
void LANAPI::RequestGameLeave() {}
void LANAPI::RequestAccept() {}
void LANAPI::RequestHasMap() {}
void LANAPI::RequestChat(UnicodeString, ChatType) {}
void LANAPI::RequestGameStart() {}
void LANAPI::RequestGameStartTimer(Int) {}
void LANAPI::RequestGameOptions(AsciiString, Bool, UnsignedInt) {}
void LANAPI::RequestGameCreate(UnicodeString, Bool) {}
void LANAPI::RequestGameAnnounce() {}
void LANAPI::RequestSetName(UnicodeString) {}
void LANAPI::RequestLobbyLeave(Bool) {}
void LANAPI::ResetGameStartTimer() {}
void LANAPI::OnGameList(LANGameInfo *) {}
void LANAPI::OnPlayerList(LANPlayer *) {}
void LANAPI::OnGameJoin(ReturnType, LANGameInfo *) {}
void LANAPI::OnPlayerJoin(Int, UnicodeString) {}
void LANAPI::OnHostLeave() {}
void LANAPI::OnPlayerLeave(UnicodeString) {}
void LANAPI::OnAccept(UnsignedInt, Bool) {}
void LANAPI::OnHasMap(UnsignedInt, Bool) {}
void LANAPI::OnChat(UnicodeString, UnsignedInt, UnicodeString, ChatType) {}
void LANAPI::OnGameStart() {}
void LANAPI::OnGameStartTimer(Int) {}
void LANAPI::OnGameOptions(UnsignedInt, Int, AsciiString) {}
void LANAPI::OnGameCreate(ReturnType) {}
void LANAPI::OnNameChange(UnsignedInt, UnicodeString) {}
void LANAPI::OnInActive(UnsignedInt) {}
LANGameInfo *LANAPI::LookupGame(UnicodeString) { return nullptr; }
LANGameInfo *LANAPI::LookupGameByListOffset(Int) { return nullptr; }
LANGameInfo *LANAPI::LookupGameByHost(UnsignedInt) { return nullptr; }
LANPlayer *LANAPI::LookupPlayer(UnsignedInt) { return nullptr; }
Bool LANAPI::SetLocalIP(UnsignedInt) { return FALSE; }
void LANAPI::SetLocalIP(AsciiString) {}
Bool LANAPI::AmIHost() { return FALSE; }
void LANAPI::fillInLANMessage(LANMessage *) {}
void LANAPI::checkMOTD() {}

// ══════════════════════════════════════════════════════
//  LAN GAME INFO / SLOT
// ══════════════════════════════════════════════════════

LANGameSlot *LANGameInfo::getLANSlot(Int) { return nullptr; }
Bool LANGameInfo::amIHost(void) { return FALSE; }
void LANGameInfo::setMap(AsciiString) {}
LANPlayer *LANGameSlot::getUser(void) { return nullptr; }
Bool LANGameSlot::isLocalPlayer() const { return FALSE; }

// ══════════════════════════════════════════════════════
//  NAT
// ══════════════════════════════════════════════════════

void NAT::processGlobalMessage(Int, const char *) {}
NATStateType NAT::update() { return NATSTATE_IDLE; }

// ══════════════════════════════════════════════════════
//  USER
// ══════════════════════════════════════════════════════

User::User(UnicodeString, unsigned int, unsigned int) {}
void User::setName(UnicodeString) {}

// ══════════════════════════════════════════════════════
//  DOWNLOAD MANAGER
// ══════════════════════════════════════════════════════

DownloadManager::DownloadManager() {}
DownloadManager::~DownloadManager() {}
HRESULT DownloadManager::update() { return S_OK; }
HRESULT DownloadManager::downloadFile(AsciiString, AsciiString, AsciiString, AsciiString, AsciiString, AsciiString, Bool) { return E_FAIL; }
HRESULT DownloadManager::downloadNextQueuedFile() { return E_FAIL; }
HRESULT DownloadManager::OnEnd() { return S_OK; }
HRESULT DownloadManager::OnError(Int) { return S_OK; }
HRESULT DownloadManager::OnProgressUpdate(Int, Int, Int, Int) { return S_OK; }
HRESULT DownloadManager::OnQueryResume() { return S_OK; }
HRESULT DownloadManager::OnStatusUpdate(Int) { return S_OK; }

// ══════════════════════════════════════════════════════
//  LADDER LIST
// ══════════════════════════════════════════════════════

const LadderInfo *LadderList::findLadder(const AsciiString &, UnsignedShort) { return nullptr; }
const LadderInfo *LadderList::findLadderByIndex(Int) { return nullptr; }
const LadderInfoList *LadderList::getLocalLadders() { return nullptr; }
const LadderInfoList *LadderList::getSpecialLadders() { return nullptr; }
const LadderInfoList *LadderList::getStandardLadders() { return nullptr; }

// ══════════════════════════════════════════════════════
//  GAMESPY STAGING ROOM / GAME SLOT
// ══════════════════════════════════════════════════════

GameSpyGameSlot::GameSpyGameSlot() : m_profileID(0), m_pingInt(0), m_wins(0), m_losses(0), m_rankPoints(0), m_favoriteSide(0) {}
void GameSpyGameSlot::setPingString(AsciiString) {}

GameSpyStagingRoom::GameSpyStagingRoom() : m_id(0), m_transport(nullptr),
  m_requiresPassword(FALSE), m_allowObservers(FALSE), m_version(0),
  m_exeCRC(0), m_iniCRC(0), m_isQM(FALSE), m_ladderPort(0), m_pingInt(0),
  m_reportedNumPlayers(0), m_reportedMaxPlayers(0), m_reportedNumObservers(0) {}
void GameSpyStagingRoom::reset() {}
Bool GameSpyStagingRoom::amIHost() const { return FALSE; }
void GameSpyStagingRoom::init() {}
void GameSpyStagingRoom::resetAccepted() {}
void GameSpyStagingRoom::startGame(Int) {}
Int GameSpyStagingRoom::getLocalSlotNum() const { return -1; }
void GameSpyStagingRoom::cleanUpSlotPointers() {}
GameSpyGameSlot *GameSpyStagingRoom::getGameSpySlot(Int) { return nullptr; }
AsciiString GameSpyStagingRoom::generateGameSpyGameResultsPacket() { return AsciiString::TheEmptyString; }
AsciiString GameSpyStagingRoom::generateLadderGameResultsPacket() { return AsciiString::TheEmptyString; }
void GameSpyStagingRoom::launchGame() {}
void GameSpyStagingRoom::setPingString(AsciiString) {}

// ══════════════════════════════════════════════════════
//  WORKER PROCESS
// ══════════════════════════════════════════════════════

WorkerProcess::WorkerProcess() : m_isDone(false) {}
bool WorkerProcess::startProcess(UnicodeString) { return false; }
void WorkerProcess::update() {}
bool WorkerProcess::isRunning() const { return false; }
bool WorkerProcess::isDone() const { return true; }
DWORD WorkerProcess::getExitCode() const { return 0; }
AsciiString WorkerProcess::getStdOutput() const { return AsciiString::TheEmptyString; }

// ══════════════════════════════════════════════════════
//  GAME RESULTS
// ══════════════════════════════════════════════════════

class StubGameResultsInterface : public GameResultsInterface {
public:
	void init() override {}
	void reset() override {}
	void update() override {}
	void startThreads() override {}
	void endThreads() override {}
	Bool areThreadsRunning() override { return false; }
	void addRequest(const GameResultsRequest&) override {}
	Bool getRequest(GameResultsRequest&) override { return false; }
	void addResponse(const GameResultsResponse&) override {}
	Bool getResponse(GameResultsResponse&) override { return false; }
	Bool areGameResultsBeingSent() override { return false; }
};

GameResultsInterface *GameResultsInterface::createNewGameResultsInterface() { return new StubGameResultsInterface(); }

// ══════════════════════════════════════════════════════
//  INI PARSE
// ══════════════════════════════════════════════════════

void INI::parseOnlineChatColorDefinition(INI *) {}

// ══════════════════════════════════════════════════════
//  PERSISTENT STORAGE (can't include PersistentStorageThread.h — GameSpy dep)
// ══════════════════════════════════════════════════════

// Redeclare what we need from PersistentStorageThread.h
class PSPlayerStats {
public:
  PSPlayerStats();
  PSPlayerStats(const PSPlayerStats &other);
  Int wins, losses, score, disconnects;
  Int lastLadder;
  AsciiString kvString;
};
PSPlayerStats::PSPlayerStats() : wins(0), losses(0), score(0), disconnects(0), lastLadder(0) {}
PSPlayerStats::PSPlayerStats(const PSPlayerStats &other) = default;

class PSRequest {
public:
  PSRequest();
  Int type;
  Int profileID;
  PSPlayerStats player;
};
PSRequest::PSRequest() : type(0), profileID(0) {}

// Redeclare GameSpyPSMessageQueueInterface with proper static methods
class GameSpyPSMessageQueueInterface {
public:
  virtual ~GameSpyPSMessageQueueInterface() {}
  static std::string formatPlayerKVPairs(PSPlayerStats stats);
  static PSPlayerStats parsePlayerKVPairs(std::string kvPairs);
};
std::string GameSpyPSMessageQueueInterface::formatPlayerKVPairs(PSPlayerStats) { return ""; }
PSPlayerStats GameSpyPSMessageQueueInterface::parsePlayerKVPairs(std::string) { return PSPlayerStats(); }

GameSpyPSMessageQueueInterface *TheGameSpyPSMessageQueue = nullptr;

// ══════════════════════════════════════════════════════
//  PLAYER INFO (PeerDefs.h — GameSpy SDK dep)
// ══════════════════════════════════════════════════════

class PlayerInfo {
public:
  PlayerInfo();
  AsciiString m_name, m_locale;
  Int m_wins, m_losses, m_profileID, m_flags, m_rankPoints, m_side, m_preorder;
  Bool isIgnored(void);
};
Bool PlayerInfo::isIgnored(void) { return FALSE; }

// ══════════════════════════════════════════════════════
//  GAMESPY QR2 / GHTTP
// ══════════════════════════════════════════════════════

extern "C" {
  Bool getQR2HostingStatus() { return FALSE; }
  void ghttpStartup(void) {}
  void ghttpCleanup(void) {}
  void ghttpSetProxy(const char *) {}
}

// ══════════════════════════════════════════════════════
//  REGISTRY CLASS (WWVegas)
// ══════════════════════════════════════════════════════

RegistryClass::RegistryClass(const char *, bool) {}
RegistryClass::~RegistryClass() {}
int RegistryClass::Get_Int(const char *, int defaultValue) { return defaultValue; }
char *RegistryClass::Get_String(const char *, char *dest, int maxLen, const char *defaultValue) {
  if (dest && maxLen > 0) {
    if (defaultValue) { strncpy(dest, defaultValue, maxLen - 1); dest[maxLen - 1] = '\0'; }
    else dest[0] = '\0';
  }
  return dest;
}
void RegistryClass::Set_Int(const char *, int) {}
void RegistryClass::Set_String(const char *, const char *) {}

// ══════════════════════════════════════════════════════
//  ASCII COMPARATOR
// ══════════════════════════════════════════════════════

struct AsciiComparator {
  bool operator()(AsciiString a, AsciiString b) const;
};
bool AsciiComparator::operator()(AsciiString a, AsciiString b) const {
  return a.compareNoCase(b) < 0;
}
