#include "PreRTS.h"

#include <cstddef>

#include "GameClient/VideoPlayer.h"

VideoPlayerInterface *TheVideoPlayer = nullptr;

const FieldParse VideoPlayer::m_videoFieldParseTable[] =
{
	{ "Filename", INI::parseAsciiString, nullptr, offsetof(Video, m_filename) },
	{ "InternalName", INI::parseAsciiString, nullptr, offsetof(Video, m_internalName) },
	{ "Comment", INI::parseAsciiString, nullptr, offsetof(Video, m_commentForWB) },
	{ nullptr, nullptr, nullptr, 0 }
};

VideoPlayer::~VideoPlayer()
{
	deinit();
	if (this == TheVideoPlayer)
		TheVideoPlayer = nullptr;
}

void VideoPlayer::init()
{
	INI ini;
	ini.loadFileDirectory("Data\\INI\\Default\\Video", INI_LOAD_OVERWRITE, nullptr);
	ini.loadFileDirectory("Data\\INI\\Video", INI_LOAD_OVERWRITE, nullptr);
}

void VideoPlayer::reset()
{
	// Movie definitions are loaded once by init and remain valid across game
	// resets. Playback sessions are closed separately by GameClient::reset.
}

void VideoPlayer::update()
{
}

void VideoPlayer::deinit()
{
	mVideosAvailableForPlay.clear();
}

void VideoPlayer::loseFocus()
{
}

void VideoPlayer::regainFocus()
{
}

void VideoPlayer::addVideo(Video *videoToAdd)
{
	if (videoToAdd == nullptr)
		return;

	for (VecVideoIt it = mVideosAvailableForPlay.begin(); it != mVideosAvailableForPlay.end(); ++it) {
		if (it->m_internalName == videoToAdd->m_internalName) {
			*it = *videoToAdd;
			return;
		}
	}
	mVideosAvailableForPlay.push_back(*videoToAdd);
}

void VideoPlayer::removeVideo(Video *videoToRemove)
{
	if (videoToRemove == nullptr)
		return;

	for (VecVideoIt it = mVideosAvailableForPlay.begin(); it != mVideosAvailableForPlay.end(); ++it) {
		if (it->m_internalName == videoToRemove->m_internalName) {
			mVideosAvailableForPlay.erase(it);
			return;
		}
	}
}

Int VideoPlayer::getNumVideos()
{
	return static_cast<Int>(mVideosAvailableForPlay.size());
}

const Video *VideoPlayer::getVideo(AsciiString movieTitle)
{
	for (VecVideoIt it = mVideosAvailableForPlay.begin(); it != mVideosAvailableForPlay.end(); ++it) {
		if (it->m_internalName == movieTitle)
			return &*it;
	}
	return nullptr;
}

const Video *VideoPlayer::getVideo(Int index)
{
	if (index < 0 || index >= static_cast<Int>(mVideosAvailableForPlay.size()))
		return nullptr;
	return &mVideosAvailableForPlay[static_cast<std::size_t>(index)];
}

void VideoPlayer::notifyVideoPlayerOfNewProvider(Bool)
{
}
