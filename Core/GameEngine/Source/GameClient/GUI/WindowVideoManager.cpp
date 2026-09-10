#include "PreRTS.h"

#include "GameClient/WindowVideoManager.h"
#include "GameClient/GameWindow.h"
#include "GameClient/VideoRuntime.h"

import Video.Runtime;

WindowVideo::WindowVideo()
	: m_playType(WINDOW_PLAY_MOVIE_ONCE),
	  m_win(nullptr),
	  m_state(WINDOW_VIDEO_STATE_STOP),
	  m_presentationId(Invalid_Video_Presentation)
{
}

WindowVideo::~WindowVideo()
{
	if (m_presentationId != Invalid_Video_Presentation)
		Close_Window_Video(m_presentationId);
	m_presentationId = Invalid_Video_Presentation;
	m_win = nullptr;
}

void WindowVideo::init(
	GameWindow *win,
	AsciiString movieName,
	WindowVideoPlayType playType,
	std::uint64_t presentationId)
{
	m_win = win;
	m_movieName = movieName;
	m_playType = playType;
	m_state = presentationId == Invalid_Video_Presentation
		? WINDOW_VIDEO_STATE_STOP
		: WINDOW_VIDEO_STATE_PLAY;
	m_presentationId = presentationId;
}

void WindowVideo::setWindowState(WindowVideoStates state)
{
	m_state = state;
	if (m_presentationId != Invalid_Video_Presentation)
		Set_Window_Video_Visible(
			m_presentationId,
			state != WINDOW_VIDEO_STATE_STOP && state != WINDOW_VIDEO_STATE_HIDDEN);
}

WindowVideoManager::WindowVideoManager()
	: m_stopAllMovies(FALSE),
	  m_pauseAllMovies(FALSE)
{
}

WindowVideoManager::~WindowVideoManager()
{
	reset();
}

void WindowVideoManager::init()
{
	reset();
	m_stopAllMovies = FALSE;
	m_pauseAllMovies = FALSE;
}

void WindowVideoManager::reset()
{
	for (WindowVideoMap::iterator it = m_playingVideos.begin(); it != m_playingVideos.end(); ++it)
		delete it->second;
	m_playingVideos.clear();
	m_stopAllMovies = FALSE;
	m_pauseAllMovies = FALSE;
}

void WindowVideoManager::update()
{
	if (m_pauseAllMovies || m_stopAllMovies)
		return;

	for (WindowVideoMap::iterator it = m_playingVideos.begin(); it != m_playingVideos.end();) {
		WindowVideo *video = it->second;
		if (video == nullptr || video->getWin() == nullptr) {
			delete video;
			it = m_playingVideos.erase(it);
			continue;
		}

		GameWindow *window = video->getWin();
		if (video->getState() == WINDOW_VIDEO_STATE_HIDDEN && !window->winIsHidden())
			resumeMovie(window);
		if (video->getState() == WINDOW_VIDEO_STATE_PLAY && window->winIsHidden())
			hideMovie(window);

		if (video->getState() != WINDOW_VIDEO_STATE_PLAY) {
			++it;
			continue;
		}

		const Engine::Video::PlaybackState state = Get_Window_Video_State(video->getPresentationId());
		if (state == Engine::Video::PlaybackState::Finished) {
			if (video->getPlayType() == WINDOW_PLAY_MOVIE_ONCE) {
				stopAndRemoveMovie(window);
				it = m_playingVideos.begin();
				continue;
			}
			if (video->getPlayType() == WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME) {
				video->setWindowState(WINDOW_VIDEO_STATE_PAUSE);
				++it;
				continue;
			}

			stopAndRemoveMovie(window);
			it = m_playingVideos.begin();
			continue;
		} else if (state == Engine::Video::PlaybackState::Closed || state == Engine::Video::PlaybackState::Error) {
			stopAndRemoveMovie(window);
			it = m_playingVideos.begin();
			continue;
		}
		++it;
	}
}

void WindowVideoManager::playMovie(GameWindow *win, AsciiString movieName, WindowVideoPlayType playType)
{
	if (win == nullptr)
		return;

	stopAndRemoveMovie(win);
	WindowVideoMode mode = WindowVideoMode::Once;
	if (playType == WINDOW_PLAY_MOVIE_LOOP)
		mode = WindowVideoMode::Loop;
	else if (playType == WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME)
		mode = WindowVideoMode::ShowLastFrame;

	const VideoPresentationId id = Open_Window_Video(win, movieName, mode);
	if (id == Invalid_Video_Presentation)
		return;

	WindowVideo *video = NEW WindowVideo;
	video->init(win, movieName, playType, id);
	m_playingVideos[win] = video;
	m_pauseAllMovies = FALSE;
	m_stopAllMovies = FALSE;
}

void WindowVideoManager::pauseMovie(GameWindow *win)
{
	WindowVideoMap::iterator it = m_playingVideos.find(win);
	if (it == m_playingVideos.end() || it->second == nullptr)
		return;
	Engine::Video::PlaybackState state = Get_Window_Video_State(it->second->getPresentationId());
	if (state == Engine::Video::PlaybackState::Playing)
		Pause_Window_Video(it->second->getPresentationId());
	it->second->setWindowState(WINDOW_VIDEO_STATE_PAUSE);
}

void WindowVideoManager::hideMovie(GameWindow *win)
{
	WindowVideoMap::iterator it = m_playingVideos.find(win);
	if (it == m_playingVideos.end() || it->second == nullptr)
		return;
	Pause_Window_Video(it->second->getPresentationId());
	it->second->setWindowState(WINDOW_VIDEO_STATE_HIDDEN);
}

void WindowVideoManager::resumeMovie(GameWindow *win)
{
	WindowVideoMap::iterator it = m_playingVideos.find(win);
	if (it != m_playingVideos.end() && it->second != nullptr) {
		Play_Window_Video(it->second->getPresentationId());
		it->second->setWindowState(WINDOW_VIDEO_STATE_PLAY);
	}
	m_pauseAllMovies = FALSE;
	m_stopAllMovies = FALSE;
}

void WindowVideoManager::stopMovie(GameWindow *win)
{
	WindowVideoMap::iterator it = m_playingVideos.find(win);
	if (it == m_playingVideos.end() || it->second == nullptr)
		return;
	Pause_Window_Video(it->second->getPresentationId());
	it->second->setWindowState(WINDOW_VIDEO_STATE_STOP);
}

void WindowVideoManager::stopAndRemoveMovie(GameWindow *win)
{
	WindowVideoMap::iterator it = m_playingVideos.find(win);
	if (it == m_playingVideos.end())
		return;
	delete it->second;
	m_playingVideos.erase(it);
}

void WindowVideoManager::stopAllMovies()
{
	for (WindowVideoMap::iterator it = m_playingVideos.begin(); it != m_playingVideos.end(); ++it) {
		if (it->second != nullptr) {
			Pause_Window_Video(it->second->getPresentationId());
			it->second->setWindowState(WINDOW_VIDEO_STATE_STOP);
		}
	}
	m_stopAllMovies = TRUE;
	m_pauseAllMovies = FALSE;
}

void WindowVideoManager::pauseAllMovies()
{
	for (WindowVideoMap::iterator it = m_playingVideos.begin(); it != m_playingVideos.end(); ++it) {
		if (it->second != nullptr) {
			Pause_Window_Video(it->second->getPresentationId());
			it->second->setWindowState(WINDOW_VIDEO_STATE_PAUSE);
		}
	}
	m_pauseAllMovies = TRUE;
	m_stopAllMovies = FALSE;
}

void WindowVideoManager::resumeAllMovies()
{
	for (WindowVideoMap::iterator it = m_playingVideos.begin(); it != m_playingVideos.end(); ++it) {
		if (it->second != nullptr) {
			Play_Window_Video(it->second->getPresentationId());
			it->second->setWindowState(WINDOW_VIDEO_STATE_PLAY);
		}
	}
	m_stopAllMovies = FALSE;
	m_pauseAllMovies = FALSE;
}

Int WindowVideoManager::getWinState(GameWindow *win)
{
	WindowVideoMap::iterator it = m_playingVideos.find(win);
	if (it != m_playingVideos.end() && it->second != nullptr)
		return it->second->getState();
	return WINDOW_VIDEO_STATE_STOP;
}
