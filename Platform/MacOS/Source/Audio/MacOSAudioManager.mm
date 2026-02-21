#include "MacOSAudioManager.h"
#include "Common/AudioAffect.h"
#include "Common/AudioEventRTS.h"
#include "Common/AudioRequest.h"
#include "Common/Debug.h"
#include "Common/FileSystem.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/file.h"
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

extern FileSystem *TheFileSystem;

// ─────────────────────────────────────────────────────
//  Simple audio playback using AVAudioPlayer
//  AVAudioEngine was crashing because it couldn't find
//  audio input/output nodes in our process context.
//  AVAudioPlayer is simpler and more reliable for our needs.
// ─────────────────────────────────────────────────────

MacOSAudioManager::MacOSAudioManager()
    : m_engine(nullptr), m_mainMixer(nullptr) {
  fprintf(stderr, "MACOS AUDIO: Constructor started\n");
  fflush(stderr);
}

MacOSAudioManager::~MacOSAudioManager() {
  fprintf(stderr, "MACOS AUDIO: Destructor started\n");
  fflush(stderr);
  // Stop and release all playing audio
  for (auto &playing : m_playingAudio) {
    if (playing.playerNode) {
      AVAudioPlayer *player = (__bridge_transfer AVAudioPlayer *)playing.playerNode;
      [player stop];
    }
  }
  m_playingAudio.clear();
  
  // Release the music player
  if (m_engine) {
    AVAudioPlayer *musicPlayer = (__bridge_transfer AVAudioPlayer *)m_engine;
    [musicPlayer stop];
    m_engine = nullptr;
  }
}

void MacOSAudioManager::init() {
  fprintf(stderr, "MACOS AUDIO: init() started\n");
  fflush(stderr);

  AudioManager::init();

  // No engine initialization needed — AVAudioPlayer creates its own
  // audio session automatically. Just mark ourselves as ready.
  fprintf(stderr, "MACOS AUDIO: init() complete (AVAudioPlayer mode)\n");
  fflush(stderr);
}

void MacOSAudioManager::reset() {
  AudioManager::reset();
  // Stop all playing audio
  for (auto &playing : m_playingAudio) {
    if (playing.playerNode) {
      AVAudioPlayer *player = (__bridge AVAudioPlayer *)playing.playerNode;
      [player stop];
      (void)(__bridge_transfer AVAudioPlayer *)playing.playerNode;
    }
  }
  m_playingAudio.clear();
}

void MacOSAudioManager::update() {
  AudioManager::update();
  processRequestList();

  // Cleanup finished audio
  for (auto it = m_playingAudio.begin(); it != m_playingAudio.end();) {
    AVAudioPlayer *player = (__bridge AVAudioPlayer *)it->playerNode;
    if (!player.isPlaying) {
      void *toRelease = it->playerNode;
      it = m_playingAudio.erase(it);
      (void)(__bridge_transfer AVAudioPlayer *)toRelease;
    } else {
      ++it;
    }
  }
}

void MacOSAudioManager::processRequestList() {
  // TheSuperHackers @fix macOS: The audio request list contains AudioEventRTS
  // pointers that may be corrupted or dangling, causing SIGSEGV in
  // AsciiString::str(). Since our audio is largely stubbed, just release
  // the requests without accessing their event data.
  for (auto it = m_audioRequests.begin(); it != m_audioRequests.end();) {
    AudioRequest *req = *it;
    if (req) {
      MemoryPoolObject::deleteInstanceInternal(req);
    }
    it = m_audioRequests.erase(it);
  }
}

void MacOSAudioManager::playAudioEvent(AudioEventRTS *event) {
  // Stubbed for macOS
}

void MacOSAudioManager::stopAudio(AudioAffect which) {
  for (auto &playing : m_playingAudio) {
    if (playing.playerNode) {
      AVAudioPlayer *player = (__bridge AVAudioPlayer *)playing.playerNode;
      [player stop];
    }
  }
}

void MacOSAudioManager::pauseAudio(AudioAffect which) {
  for (auto &playing : m_playingAudio) {
    if (playing.playerNode) {
      AVAudioPlayer *player = (__bridge AVAudioPlayer *)playing.playerNode;
      [player pause];
    }
  }
  // Pause music too
  if (m_engine) {
    AVAudioPlayer *musicPlayer = (__bridge AVAudioPlayer *)m_engine;
    [musicPlayer pause];
  }
}

void MacOSAudioManager::resumeAudio(AudioAffect which) {
  for (auto &playing : m_playingAudio) {
    if (playing.playerNode) {
      AVAudioPlayer *player = (__bridge AVAudioPlayer *)playing.playerNode;
      [player play];
    }
  }
  if (m_engine) {
    AVAudioPlayer *musicPlayer = (__bridge AVAudioPlayer *)m_engine;
    [musicPlayer play];
  }
}

void MacOSAudioManager::pauseAmbient(Bool shouldPause) {}
void MacOSAudioManager::killAudioEventImmediately(AudioHandle audioEvent) {}

void MacOSAudioManager::nextMusicTrack() {
  fprintf(stderr, "MACOS AUDIO: nextMusicTrack() called\n");
  fflush(stderr);
}

void MacOSAudioManager::prevMusicTrack() {
  fprintf(stderr, "MACOS AUDIO: prevMusicTrack() called\n");
  fflush(stderr);
}

Bool MacOSAudioManager::isMusicPlaying() const {
  if (m_engine) {
    AVAudioPlayer *musicPlayer = (__bridge AVAudioPlayer *)m_engine;
    return musicPlayer.isPlaying ? TRUE : FALSE;
  }
  return FALSE;
}

// TheSuperHackers @fix macOS: The base class checks if a music file exists
// on disk. If not found, GameEngine::init() calls setQuitting(TRUE), causing
// the game to exit immediately after loading the main menu. Since our audio
// subsystem works without pre-loaded music files, always return TRUE.
Bool MacOSAudioManager::isMusicAlreadyLoaded() const {
  return TRUE;
}
Bool MacOSAudioManager::hasMusicTrackCompleted(const AsciiString &trackName,
                                               Int numberOfTimes) const {
  return FALSE;
}
AsciiString MacOSAudioManager::getMusicTrackName() const { return ""; }

void MacOSAudioManager::openDevice() {}
void MacOSAudioManager::closeDevice() {}
void *MacOSAudioManager::getDevice() { return nullptr; }

void MacOSAudioManager::notifyOfAudioCompletion(UnsignedInt audioCompleted,
                                                UnsignedInt flags) {}

UnsignedInt MacOSAudioManager::getProviderCount() const { return 1; }
AsciiString MacOSAudioManager::getProviderName(UnsignedInt providerNum) const {
  return "MacOS CoreAudio";
}
UnsignedInt
MacOSAudioManager::getProviderIndex(AsciiString providerName) const {
  return 0;
}
void MacOSAudioManager::selectProvider(UnsignedInt providerNdx) {}
void MacOSAudioManager::unselectProvider() {}
UnsignedInt MacOSAudioManager::getSelectedProvider() const { return 0; }

void MacOSAudioManager::setSpeakerType(UnsignedInt speakerType) {}
UnsignedInt MacOSAudioManager::getSpeakerType() { return 0; }

UnsignedInt MacOSAudioManager::getNum2DSamples() const { return 64; }
UnsignedInt MacOSAudioManager::getNum3DSamples() const { return 64; }
UnsignedInt MacOSAudioManager::getNumStreams() const { return 8; }

Bool MacOSAudioManager::doesViolateLimit(AudioEventRTS *event) const {
  return FALSE;
}
Bool MacOSAudioManager::isPlayingLowerPriority(AudioEventRTS *event) const {
  return FALSE;
}
Bool MacOSAudioManager::isPlayingAlready(AudioEventRTS *event) const {
  return FALSE;
}
Bool MacOSAudioManager::isObjectPlayingVoice(UnsignedInt objID) const {
  return FALSE;
}

void MacOSAudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName,
                                                   Real newVolume) {}
void MacOSAudioManager::removePlayingAudio(AsciiString eventName) {}
void MacOSAudioManager::removeAllDisabledAudio() {}

Bool MacOSAudioManager::has3DSensitiveStreamsPlaying() const { return FALSE; }
void *MacOSAudioManager::getHandleForBink() { return nullptr; }
void MacOSAudioManager::releaseHandleForBink() {}

// ─────────────────────────────────────────────────────
//  Helper: resolve a game audio path to a file URL
//  Handles BIG archive extraction to temp cache
// ─────────────────────────────────────────────────────
static NSURL *resolveAudioFileURL(const std::string &pathStr) {
  NSString *nsPath = [NSString stringWithUTF8String:pathStr.c_str()];
  NSURL *fileURL = nil;

  // Try absolute path first
  if ([nsPath isAbsolutePath]) {
    fileURL = [NSURL fileURLWithPath:nsPath];
    if ([[NSFileManager defaultManager] fileExistsAtPath:[fileURL path]]) {
      return fileURL;
    }
  }

  // Try relative to current directory
  NSString *cwd = [[NSFileManager defaultManager] currentDirectoryPath];
  NSString *fullPath = [cwd stringByAppendingPathComponent:nsPath];
  if ([[NSFileManager defaultManager] fileExistsAtPath:fullPath]) {
    return [NSURL fileURLWithPath:fullPath];
  }

  // Try temp cache (previously extracted from BIG)
  NSString *tempPath = [NSTemporaryDirectory()
      stringByAppendingPathComponent:[nsPath lastPathComponent]];
  if ([[NSFileManager defaultManager] fileExistsAtPath:tempPath]) {
    return [NSURL fileURLWithPath:tempPath];
  }

  // Extract from BIG archive via TheFileSystem
  if (TheFileSystem && TheFileSystem->doesFileExist(pathStr.c_str())) {
    File *f = TheFileSystem->openFile(pathStr.c_str(), File::READ);
    if (f) {
      size_t fileSize = f->size();
      char *buffer = f->readEntireAndClose();
      if (buffer && fileSize > 0) {
        NSURL *tempURL = [NSURL fileURLWithPath:tempPath];
        NSData *data = [NSData dataWithBytesNoCopy:buffer
                                            length:fileSize
                                      freeWhenDone:NO];
        [data writeToURL:tempURL atomically:YES];
        delete[] buffer;
        fprintf(stderr, "MACOS AUDIO: Extracted '%s' -> cache (%zu bytes)\n",
                pathStr.c_str(), fileSize);
        fflush(stderr);
        return tempURL;
      }
    }
  }

  return nil;
}

void MacOSAudioManager::friend_forcePlayAudioEventRTS(
    const AudioEventRTS *eventToPlay) {
  if (!eventToPlay)
    return;

  AudioEventRTS *mutableEvent = const_cast<AudioEventRTS *>(eventToPlay);
  mutableEvent->generateFilename();
  AsciiString filename = mutableEvent->getFilename();

  if (filename.isEmpty())
    return;

  // Convert backslashes to forward slashes
  std::string pathStr = filename.str();
  for (size_t i = 0; i < pathStr.length(); ++i) {
    if (pathStr[i] == '\\')
      pathStr[i] = '/';
  }

  fprintf(stderr, "MACOS AUDIO: Playing '%s'\n", pathStr.c_str());
  fflush(stderr);

  NSURL *fileURL = resolveAudioFileURL(pathStr);
  if (!fileURL) {
    fprintf(stderr, "MACOS AUDIO: File not found: '%s'\n", pathStr.c_str());
    fflush(stderr);
    return;
  }

  @try {
    NSError *error = nil;
    AVAudioPlayer *player =
        [[AVAudioPlayer alloc] initWithContentsOfURL:fileURL error:&error];
    if (!player) {
      fprintf(stderr, "MACOS AUDIO: Failed to create player for '%s': %s\n",
              pathStr.c_str(),
              error ? [[error localizedDescription] UTF8String]
                    : "Unknown error");
      fflush(stderr);
      return;
    }

    // Volume
    float baseVol = this->getVolume(AudioAffect_Sound);
    player.volume = eventToPlay->getVolume() * baseVol;

    // Play the audio
    [player prepareToPlay];
    [player play];

    ApplePlayingAudio playing;
    playing.playerNode = (__bridge_retained void *)player;
    playing.eventName = eventToPlay->getEventName().str();
    m_playingAudio.push_back(playing);

    fprintf(stderr, "MACOS AUDIO: 🔊 Playing: '%s' vol=%.2f\n",
            pathStr.c_str(), player.volume);
  } @catch (NSException *e) {
    fprintf(stderr, "MACOS AUDIO: Exception playing '%s': %s\n",
            pathStr.c_str(), [[e reason] UTF8String]);
    fflush(stderr);
  }
}

void MacOSAudioManager::setPreferredProvider(AsciiString providerNdx) {}
void MacOSAudioManager::setPreferredSpeaker(AsciiString speakerType) {}

Real MacOSAudioManager::getFileLengthMS(AsciiString strToLoad) const {
  return 0.0f;
}
void MacOSAudioManager::closeAnySamplesUsingFile(const void *fileToClose) {}

void MacOSAudioManager::setDeviceListenerPosition() {}

#if defined(RTS_DEBUG)
void MacOSAudioManager::audioDebugDisplay(DebugDisplayInterface *dd,
                                          void *userData, FILE *fp) {}
#endif
