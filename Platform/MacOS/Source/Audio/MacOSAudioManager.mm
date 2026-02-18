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

MacOSAudioManager::MacOSAudioManager()
    : m_engine(nullptr), m_mainMixer(nullptr) {
  fprintf(stderr, "MACOS AUDIO: Constructor started\n");
  fflush(stderr);
}

MacOSAudioManager::~MacOSAudioManager() {
  fprintf(stderr, "MACOS AUDIO: Destructor started\n");
  fflush(stderr);
  if (m_engine) {
    AVAudioEngine *engine = (__bridge_transfer AVAudioEngine *)m_engine;
    [engine stop];
    // ARC handles engine release now due to bridge_transfer
    for (auto &playing : m_playingAudio) {
      if (playing.playerNode) {
        // Use bridge_transfer to let ARC handle the release of the retained
        // node
        (void)(__bridge_transfer AVAudioPlayerNode *)playing.playerNode;
      }
      if (playing.event) {
        delete playing.event;
      }
    }
    m_playingAudio.clear();
  }
}

void MacOSAudioManager::init() {
  fprintf(stderr, "MACOS AUDIO: init() started\n");
  fflush(stderr);

  if (!m_engine) {
    m_engine = (__bridge_retained void *)[[AVAudioEngine alloc] init];
    AVAudioEngine *engine = (__bridge AVAudioEngine *)m_engine;
    m_mainMixer = (__bridge void *)[engine mainMixerNode];
    fprintf(stderr, "MACOS AUDIO: Engine allocated: %p, Mixer: %p\n", m_engine,
            m_mainMixer);
    fflush(stderr);
  }

  AudioManager::init();

  NSError *error = nil;
  AVAudioEngine *engine = (__bridge AVAudioEngine *)m_engine;
  if (![engine startAndReturnError:&error]) {
    fprintf(stderr, "MACOS AUDIO: Failed to start engine: %s\n",
            [[error localizedDescription] UTF8String]);
    fflush(stderr);
  } else {
    fprintf(stderr, "MACOS AUDIO: Engine started, mainMixer=%p\n", m_mainMixer);
    fflush(stderr);
  }
}

void MacOSAudioManager::reset() {
  AudioManager::reset();
  // Stop all playing audio
  for (auto &playing : m_playingAudio) {
    AVAudioPlayerNode *node = (__bridge AVAudioPlayerNode *)playing.playerNode;
    [node stop];
    AVAudioEngine *engine = (__bridge AVAudioEngine *)m_engine;
    [engine detachNode:node];
    (void)(__bridge_transfer AVAudioPlayerNode *)playing.playerNode;
    if (playing.event) {
      delete playing.event;
    }
  }
  m_playingAudio.clear();
}

void MacOSAudioManager::update() {
  AudioManager::update();
  processRequestList();

  AVAudioEngine *engine = (__bridge AVAudioEngine *)m_engine;
  if (!engine)
    return;

  // Cleanup finished audio
  for (auto it = m_playingAudio.begin(); it != m_playingAudio.end();) {
    AVAudioPlayerNode *node = (__bridge AVAudioPlayerNode *)it->playerNode;
    if (!node.isPlaying) {
      [engine detachNode:node];
      void *toRelease = it->playerNode;
      AudioEventRTS *event = it->event;
      it = m_playingAudio.erase(it);
      (void)(__bridge_transfer AVAudioPlayerNode *)toRelease;
      if (event) {
        delete event;
      }
    } else {
      ++it;
    }
  }
}

void MacOSAudioManager::processRequestList() {
  if (!m_audioRequests.empty()) {
    fprintf(stderr, "MACOS AUDIO: Processing %lu requests\n",
            (unsigned long)m_audioRequests.size());
    fflush(stderr);
  }
  for (auto it = m_audioRequests.begin(); it != m_audioRequests.end();) {
    AudioRequest *req = *it;
    if (req) {
      if (req->m_request == AR_Play) {
        fprintf(stderr, "MACOS AUDIO: Request Play\n");
        fflush(stderr);
        playAudioEvent(req->m_pendingEvent);
      } else if (req->m_request == AR_Stop) {
        fprintf(stderr, "MACOS AUDIO: Request Stop\n");
        fflush(stderr);
        // Stop matching events
        for (auto &playing : m_playingAudio) {
          if (playing.event == req->m_pendingEvent ||
              (req->m_pendingEvent && playing.event &&
               playing.event->getEventName() ==
                   req->m_pendingEvent->getEventName())) {
            AVAudioPlayerNode *node =
                (__bridge AVAudioPlayerNode *)playing.playerNode;
            [node stop];
          }
        }
      }
      MemoryPoolObject::deleteInstanceInternal(req);
    }
    it = m_audioRequests.erase(it);
  }
}

void MacOSAudioManager::playAudioEvent(AudioEventRTS *event) {
  if (!event)
    return;
  fprintf(stderr, "MACOS AUDIO: playAudioEvent(%p)\n", event);
  fflush(stderr);
  // For now, use the same logic as force play but we own the event now
  friend_forcePlayAudioEventRTS(event);
}

void MacOSAudioManager::stopAudio(AudioAffect which) {
  // TODO: Implement global stop
}
void MacOSAudioManager::pauseAudio(AudioAffect which) {
  [(__bridge AVAudioEngine *)m_engine pause];
}
void MacOSAudioManager::resumeAudio(AudioAffect which) {
  NSError *error = nil;
  [(__bridge AVAudioEngine *)m_engine startAndReturnError:&error];
}
void MacOSAudioManager::pauseAmbient(Bool shouldPause) {}
void MacOSAudioManager::killAudioEventImmediately(AudioHandle audioEvent) {}

void MacOSAudioManager::nextMusicTrack() {}
void MacOSAudioManager::prevMusicTrack() {}
Bool MacOSAudioManager::isMusicPlaying() const { return false; }
Bool MacOSAudioManager::hasMusicTrackCompleted(const AsciiString &trackName,
                                               Int numberOfTimes) const {
  return false;
}
AsciiString MacOSAudioManager::getMusicTrackName() const { return ""; }

void MacOSAudioManager::openDevice() {}
void MacOSAudioManager::closeDevice() {}
void *MacOSAudioManager::getDevice() { return m_engine; }

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
  return false;
}
Bool MacOSAudioManager::isPlayingLowerPriority(AudioEventRTS *event) const {
  return false;
}
Bool MacOSAudioManager::isPlayingAlready(AudioEventRTS *event) const {
  return false;
}
Bool MacOSAudioManager::isObjectPlayingVoice(UnsignedInt objID) const {
  return false;
}

void MacOSAudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName,
                                                   Real newVolume) {}
void MacOSAudioManager::removePlayingAudio(AsciiString eventName) {}
void MacOSAudioManager::removeAllDisabledAudio() {}

Bool MacOSAudioManager::has3DSensitiveStreamsPlaying() const { return false; }
void *MacOSAudioManager::getHandleForBink() { return nullptr; }
void MacOSAudioManager::releaseHandleForBink() {}

void MacOSAudioManager::friend_forcePlayAudioEventRTS(
    const AudioEventRTS *eventToPlay) {
  if (!eventToPlay)
    return;

  AudioEventRTS *mutableEvent = const_cast<AudioEventRTS *>(eventToPlay);
  mutableEvent->generateFilename();
  AsciiString filename = mutableEvent->getFilename();

  if (filename.isEmpty())
    return;

  // Convert \ to /
  std::string pathStr = filename.str();
  for (size_t i = 0; i < pathStr.length(); ++i) {
    if (pathStr[i] == '\\')
      pathStr[i] = '/';
  }

  NSString *nsPath = [NSString stringWithUTF8String:pathStr.c_str()];
  NSURL *fileURL = [NSURL fileURLWithPath:nsPath];

  // Try relative to current directory if not absolute
  if (![nsPath isAbsolutePath]) {
    NSString *cwd = [[NSFileManager defaultManager] currentDirectoryPath];
    fileURL =
        [NSURL fileURLWithPath:[cwd stringByAppendingPathComponent:nsPath]];
  }

  // Check if file exists on disk
  if (![[NSFileManager defaultManager] fileExistsAtPath:[fileURL path]]) {
    // If not, try to load from TheFileSystem (BIG files)
    NSString *tempPath = [NSTemporaryDirectory()
        stringByAppendingPathComponent:[nsPath lastPathComponent]];
    if ([[NSFileManager defaultManager] fileExistsAtPath:tempPath]) {
      fileURL = [NSURL fileURLWithPath:tempPath];
    } else if (TheFileSystem && TheFileSystem->doesFileExist(pathStr.c_str())) {
      File *f = TheFileSystem->openFile(pathStr.c_str(), File::READ);
      if (f) {
        size_t fileSize = f->size();
        char *buffer = f->readEntireAndClose();
        if (buffer) {
          NSURL *tempURL = [NSURL fileURLWithPath:tempPath];
          NSData *data = [NSData dataWithBytesNoCopy:buffer
                                              length:fileSize
                                        freeWhenDone:YES];
          [data writeToURL:tempURL atomically:YES];
          fileURL = tempURL;
          fprintf(stderr, "MACOS AUDIO: Extracted %s to cache\n",
                  pathStr.c_str());
          fflush(stderr);
        }
      }
    }
  }

  fprintf(stderr, "MACOS AUDIO: Attempting to load URL: %s\n",
          [[fileURL description] UTF8String]);
  fflush(stderr);

  NSError *error = nil;
  AVAudioFile *audioFile = nil;
  @try {
    audioFile = [[AVAudioFile alloc] initForReading:fileURL error:&error];
  } @catch (NSException *e) {
    fprintf(stderr, "MACOS AUDIO: Exception loading file: %s\n",
            [[e reason] UTF8String]);
    fflush(stderr);
  }

  if (!audioFile) {
    fprintf(stderr, "MACOS AUDIO: Failed to load %s: %s\n", pathStr.c_str(),
            error ? [[error localizedDescription] UTF8String]
                  : "Unknown error");
    fflush(stderr);
    return;
  }

  fprintf(stderr, "MACOS AUDIO: Playing %s\n", pathStr.c_str());
  fflush(stderr);

  AVAudioEngine *engine = (__bridge AVAudioEngine *)m_engine;
  AVAudioPlayerNode *playerNode = [[AVAudioPlayerNode alloc] init];
  [engine attachNode:playerNode];
  [engine connect:playerNode
               to:(__bridge AVAudioMixerNode *)m_mainMixer
           format:audioFile.processingFormat];

  // Volume
  float baseVol = this->getVolume(AudioAffect_Sound);
  playerNode.volume = eventToPlay->getVolume() * baseVol;

  [playerNode scheduleFile:audioFile atTime:nil completionHandler:nil];
  [playerNode play];

  ApplePlayingAudio playing;
  playing.playerNode = (__bridge_retained void *)playerNode;
  // Since we take ownership in our tracking list, we must be careful.
  // In the real engine, addAudioEvent creates a NEW event anyway.
  playing.event = mutableEvent;
  m_playingAudio.push_back(playing);

  // fprintf(stderr, "MACOS AUDIO: Playing %s (vol: %.2f)\n", pathStr.c_str(),
  // playerNode.volume);
}

void MacOSAudioManager::setPreferredProvider(AsciiString providerNdx) {}
void MacOSAudioManager::setPreferredSpeaker(AsciiString speakerType) {}

Real MacOSAudioManager::getFileLengthMS(AsciiString strToLoad) const {
  return 0.0f;
}
void MacOSAudioManager::closeAnySamplesUsingFile(const void *fileToClose) {}

void MacOSAudioManager::setDeviceListenerPosition() {
  const Coord3D *pos = getListenerPosition();
  if (pos) {
    // Map to AVAudioEngine listener if needed
  }
}

#if defined(RTS_DEBUG)
void MacOSAudioManager::audioDebugDisplay(DebugDisplayInterface *dd,
                                          void *userData, FILE *fp) {}
#endif
