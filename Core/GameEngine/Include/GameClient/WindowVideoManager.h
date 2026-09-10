#pragma once

#include <cstdint>

#include "Common/AsciiString.h"
#include "Common/STLTypedefs.h"
#include "Common/SubsystemInterface.h"
#include "Lib/BaseType.h"

class GameWindow;

enum WindowVideoPlayType CPP_11(: Int)
{
	WINDOW_PLAY_MOVIE_ONCE = 0,
	WINDOW_PLAY_MOVIE_LOOP,
	WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME,
	WINDOW_PLAY_MOVIE_COUNT
};

enum WindowVideoStates CPP_11(: Int)
{
	WINDOW_VIDEO_STATE_START = 0,
	WINDOW_VIDEO_STATE_STOP,
	WINDOW_VIDEO_STATE_PAUSE,
	WINDOW_VIDEO_STATE_PLAY,
	WINDOW_VIDEO_STATE_HIDDEN,
	WINDOW_VIDEO_STATE_COUNT
};

class WindowVideo
{
public:
	WindowVideo();
	~WindowVideo();

	GameWindow *getWin() const;
	AsciiString getMovieName() const;
	WindowVideoPlayType getPlayType() const;
	WindowVideoStates getState() const;
	std::uint64_t getPresentationId() const;

	void setPlayType(WindowVideoPlayType playType);
	void setWindowState(WindowVideoStates state);
	void init(GameWindow *win, AsciiString movieName, WindowVideoPlayType playType, std::uint64_t presentationId);

private:
	WindowVideoPlayType m_playType;
	GameWindow *m_win;
	AsciiString m_movieName;
	WindowVideoStates m_state;
	std::uint64_t m_presentationId;
};

class WindowVideoManager : public SubsystemInterface
{
public:
	WindowVideoManager();
	~WindowVideoManager() override;

	void init() override;
	void reset() override;
	void update() override;

	void playMovie(GameWindow *win, AsciiString movieName, WindowVideoPlayType playType);
	void hideMovie(GameWindow *win);
	void pauseMovie(GameWindow *win);
	void resumeMovie(GameWindow *win);
	void stopMovie(GameWindow *win);
	void stopAndRemoveMovie(GameWindow *win);
	void stopAllMovies();
	void pauseAllMovies();
	void resumeAllMovies();
	Int getWinState(GameWindow *win);

private:
	typedef const GameWindow *ConstGameWindowPtr;
	struct hashConstGameWindowPtr
	{
		size_t operator()(ConstGameWindowPtr p) const
		{
			std::hash<UnsignedInt> hasher;
			return hasher(static_cast<UnsignedInt>(reinterpret_cast<std::uintptr_t>(p)));
		}
	};
	typedef std::hash_map<ConstGameWindowPtr, WindowVideo *, hashConstGameWindowPtr,
		std::equal_to<ConstGameWindowPtr>> WindowVideoMap;

	WindowVideoMap m_playingVideos;
	Bool m_stopAllMovies;
	Bool m_pauseAllMovies;
};

inline GameWindow *WindowVideo::getWin() const { return m_win; }
inline AsciiString WindowVideo::getMovieName() const { return m_movieName; }
inline WindowVideoPlayType WindowVideo::getPlayType() const { return m_playType; }
inline WindowVideoStates WindowVideo::getState() const { return m_state; }
inline std::uint64_t WindowVideo::getPresentationId() const { return m_presentationId; }
inline void WindowVideo::setPlayType(WindowVideoPlayType playType) { m_playType = playType; }
