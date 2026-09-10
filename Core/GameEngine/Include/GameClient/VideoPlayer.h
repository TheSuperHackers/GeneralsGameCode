/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
*/

#pragma once

#include <Lib/BaseType.h>
#include "Common/AsciiString.h"
#include "Common/INI.h"
#include "Common/STLTypedefs.h"
#include "Common/SubsystemInterface.h"

struct Video
{
	AsciiString m_filename;
	AsciiString m_internalName;
	AsciiString m_commentForWB;
};

typedef std::vector<Video> VecVideo;
typedef std::vector<Video>::iterator VecVideoIt;

class VideoPlayerInterface : public SubsystemInterface
{
public:
	virtual void deinit() = 0;
	virtual ~VideoPlayerInterface() override = default;

	virtual void loseFocus() = 0;
	virtual void regainFocus() = 0;
	virtual void addVideo(Video *videoToAdd) = 0;
	virtual void removeVideo(Video *videoToRemove) = 0;
	virtual Int getNumVideos() = 0;
	virtual const Video *getVideo(AsciiString movieTitle) = 0;
	virtual const Video *getVideo(Int index) = 0;
	virtual const FieldParse *getFieldParse() const = 0;
	virtual void notifyVideoPlayerOfNewProvider(Bool nowHasValid) = 0;
};

class VideoPlayer final : public VideoPlayerInterface
{
protected:
	VecVideo mVideosAvailableForPlay;
	static const FieldParse m_videoFieldParseTable[];

public:
	VideoPlayer() = default;
	~VideoPlayer() override;

	void init() override;
	void reset() override;
	void update() override;
	void deinit() override;
	void loseFocus() override;
	void regainFocus() override;
	void addVideo(Video *videoToAdd) override;
	void removeVideo(Video *videoToRemove) override;
	Int getNumVideos() override;
	const Video *getVideo(AsciiString movieTitle) override;
	const Video *getVideo(Int index) override;
	const FieldParse *getFieldParse() const override { return m_videoFieldParseTable; }
	void notifyVideoPlayerOfNewProvider(Bool nowHasValid) override;
};

extern VideoPlayerInterface *TheVideoPlayer;
