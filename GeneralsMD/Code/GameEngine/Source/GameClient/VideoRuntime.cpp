#include "PreRTS.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <span>
#include <string_view>

import Video.Decoder;
import Video.Runtime;
import Graphics.Video.Renderer;

#include "Common/File.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "Common/RuntimeConfig.h"
#include "XAudio2AudioDevice/AudioEngine.h"
#include "XAudio2AudioDevice/core/AudioStream.h"
#include "XAudio2AudioDevice/core/AudioSystem.h"
#include "XAudio2AudioDevice/core/AudioBus.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/VideoRuntime.h"
#include "GameClient/VideoPlayer.h"

namespace
{

bool Window_Rect(void *context, Engine::Video::PresentationRect &rect) noexcept
{
	GameWindow *window = static_cast<GameWindow *>(context);
	if (window == nullptr || window->winIsHidden())
		return false;

	ICoord2D position;
	ICoord2D size;
	window->winGetScreenPosition(&position.x, &position.y);
	window->winGetSize(&size.x, &size.y);
	if (size.x <= 0 || size.y <= 0)
		return false;

	rect = {position.x, position.y, static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y)};
	return true;
}

bool Cameo_Rect(void *, Engine::Video::PresentationRect &rect) noexcept
{
	if (TheWindowManager == nullptr || TheNameKeyGenerator == nullptr)
		return false;

	GameWindow *window = TheWindowManager->winGetWindowFromId(
		nullptr,
		TheNameKeyGenerator->nameToKey("ControlBar.wnd:RightHUD"));
	return Window_Rect(window, rect);
}

class GameVideoSource final : public Engine::Video::Source
{
public:
	explicit GameVideoSource(File *file) noexcept
		: m_file(file)
	{
	}

	~GameVideoSource() noexcept override
	{
		if (m_file != nullptr)
			m_file->close();
	}

	std::size_t Read(std::span<std::byte> destination) noexcept override
	{
		if (m_file == nullptr || destination.empty())
			return 0;

		const std::size_t read_size = std::min<std::size_t>(
			destination.size(), static_cast<std::size_t>(std::numeric_limits<Int>::max()));
		const Int bytes_read = m_file->read(destination.data(), static_cast<Int>(read_size));
		return bytes_read > 0 ? static_cast<std::size_t>(bytes_read) : 0;
	}

	bool Seek(std::int64_t offset, Engine::Video::SourceSeekOrigin origin) noexcept override
	{
		if (m_file == nullptr || offset < std::numeric_limits<Int>::min()
			|| offset > std::numeric_limits<Int>::max())
			return false;

		File::seekMode mode = File::START;
		switch (origin) {
		case Engine::Video::SourceSeekOrigin::Begin:
			mode = File::START;
			break;
		case Engine::Video::SourceSeekOrigin::Current:
			mode = File::CURRENT;
			break;
		case Engine::Video::SourceSeekOrigin::End:
			mode = File::END;
			break;
		}
		return m_file->seek(static_cast<Int>(offset), mode) >= 0;
	}

	std::int64_t Tell() const noexcept override
	{
		return m_file != nullptr ? m_file->position() : -1;
	}

	std::int64_t Size() const noexcept override
	{
		return m_file != nullptr ? m_file->size() : -1;
	}

private:
	File *m_file = nullptr;
};

class GameVideoAudioSink final : public Engine::Video::AudioSink
{
public:
	~GameVideoAudioSink() noexcept override
	{
		Shutdown();
	}

	bool Submit(const Engine::Video::DecodedAudioChunk &chunk) override
	{
		if (!chunk.Is_Valid() || chunk.samples.size() > std::numeric_limits<std::uint32_t>::max())
			return false;

		AudioEngine &audio_engine = AudioEngine::instance();
		AudioSystem *backend = audio_engine.getBackend();
		if (backend == nullptr)
			return false;
		if (backend != m_backend) {
			m_stream.reset();
			m_backend = backend;
		}
		if (m_stream == nullptr)
			m_stream = backend->createAudioStream(AudioBus::Speech);
		if (m_stream == nullptr)
			return false;

		const bool queued = m_stream->Queue(
			chunk.samples.data(),
			static_cast<std::uint32_t>(chunk.samples.size()),
			{chunk.sample_rate, chunk.channels, 16});
		if (queued && m_started)
			m_stream->Start();
		return queued;
	}

	void Start() noexcept override
	{
		m_started = true;
		if (m_stream != nullptr)
			m_stream->Start();
	}

	void Pause() noexcept override
	{
		m_started = false;
		if (m_stream != nullptr)
			m_stream->Pause();
	}

	void Resume() noexcept override
	{
		m_started = true;
		if (m_stream != nullptr)
			m_stream->Resume();
	}

	void Stop() noexcept override
	{
		m_started = false;
		if (m_stream != nullptr)
			m_stream->Stop();
	}

	void Reset() noexcept override
	{
		m_started = false;
		if (m_stream != nullptr)
			m_stream->Reset();
	}

	void Shutdown() noexcept
	{
		Stop();
		m_stream.reset();
		m_backend = nullptr;
	}

private:
	AudioSystem *m_backend = nullptr;
	std::unique_ptr<AudioStream> m_stream;
	bool m_started = false;
};

GameVideoAudioSink &Video_Audio_Sink()
{
	static GameVideoAudioSink sink;
	return sink;
}

std::string Movie_File_Name(const AsciiString &movie_title)
{
	const Video *video = TheVideoPlayer != nullptr ? TheVideoPlayer->getVideo(movie_title) : nullptr;
	std::string file_name = video != nullptr && !video->m_filename.isEmpty()
		? video->m_filename.str()
		: movie_title.str();
	const bool has_bik_extension = file_name.size() >= 4
		&& file_name[file_name.size() - 4] == '.'
		&& (file_name[file_name.size() - 3] == 'b' || file_name[file_name.size() - 3] == 'B')
		&& (file_name[file_name.size() - 2] == 'i' || file_name[file_name.size() - 2] == 'I')
		&& (file_name[file_name.size() - 1] == 'k' || file_name[file_name.size() - 1] == 'K');
	if (!has_bik_extension)
		file_name += ".bik";
	return file_name;
}

File *Open_Movie_File(const AsciiString &movie_title)
{
	if (TheFileSystem == nullptr || movie_title.isEmpty())
		return nullptr;

	const std::string file_name = Movie_File_Name(movie_title);
	const std::string localized_path = std::string("Data/") + GetGameLanguage().str()
		+ "/Movies/" + file_name;
	const std::string game_path = std::string("Data\\Movies\\") + file_name;
	const std::string mod_path = TheGlobalData != nullptr && TheGlobalData->m_modDir.isNotEmpty()
		? std::string(TheGlobalData->m_modDir.str()) + "Data\\Movies\\" + file_name
		: std::string();

	if (!mod_path.empty()) {
		if (File *file = TheFileSystem->openFile(mod_path.c_str(), File::READ | File::BINARY | File::STREAMING))
			return file;
	}
	if (File *file = TheFileSystem->openFile(localized_path.c_str(), File::READ | File::BINARY | File::STREAMING))
		return file;
	return TheFileSystem->openFile(game_path.c_str(), File::READ | File::BINARY | File::STREAMING);
}

}

bool Open_Video(Engine::Video::PlaybackSlot slot, const AsciiString &movie_title)
{
	Engine::Video::Get_Runtime().Set_Audio_Sink(&Video_Audio_Sink());
	std::unique_ptr<GameVideoSource> source(new GameVideoSource(Open_Movie_File(movie_title)));
	if (source == nullptr || source->Size() <= 0)
		return false;
	if (!Engine::Video::Get_Runtime().Open(slot, std::move(source)))
		return false;

	if (slot == Engine::Video::PlaybackSlot::Cameo) {
		Engine::Video::PresentationTarget target;
		target.rect_provider = &Cameo_Rect;
		if (!Engine::Video::Get_Runtime().Configure(slot, target)) {
			Engine::Video::Get_Runtime().Close(slot);
			return false;
		}
	}
	return true;
}

bool Open_Fullscreen_Video(const AsciiString &movie_title)
{
	return Open_Video(Engine::Video::PlaybackSlot::Fullscreen, movie_title);
}

void Close_Video(Engine::Video::PlaybackSlot slot) noexcept
{
	Engine::Video::Get_Runtime().Close(slot);
}

void Close_Fullscreen_Video() noexcept
{
	Close_Video(Engine::Video::PlaybackSlot::Fullscreen);
}

void Close_All_Videos() noexcept
{
	Engine::Video::Get_Runtime().Close_All();
	Video_Audio_Sink().Shutdown();
}

bool Update_Videos(double delta_seconds)
{
	return Engine::Video::Get_Runtime().Update(delta_seconds);
}

bool Is_Fullscreen_Video_Playing() noexcept
{
	const Engine::Video::PlaybackState state =
		Engine::Video::Get_Runtime().State(Engine::Video::PlaybackSlot::Fullscreen);
	return state == Engine::Video::PlaybackState::Playing || state == Engine::Video::PlaybackState::Paused;
}

Engine::Video::PlaybackState Get_Video_State(Engine::Video::PlaybackSlot slot) noexcept
{
	return Engine::Video::Get_Runtime().State(slot);
}

Engine::Video::StreamInfo Get_Video_Info(Engine::Video::PlaybackSlot slot) noexcept
{
	return Engine::Video::Get_Runtime().Info(slot);
}

const Engine::Video::DecodedVideoFrame *Get_Video_Frame(Engine::Video::PlaybackSlot slot) noexcept
{
	return Engine::Video::Get_Runtime().Current_Frame(slot);
}

bool Initialize_Video_Presentation(Graphics::Device &device, const std::filesystem::path &shader_directory)
{
	Graphics::VideoRenderer &renderer = Graphics::GetVideoRenderer();
	return renderer.Is_Initialized() || renderer.Initialize(device, shader_directory);
}

void Begin_Video_Frame() noexcept
{
	Graphics::GetVideoRenderer().Begin_Frame();
}

void Submit_Videos(std::uint32_t output_width, std::uint32_t output_height)
{
	Graphics::VideoRenderer &renderer = Graphics::GetVideoRenderer();
	if (renderer.Accepting_Submissions())
		Engine::Video::Get_Runtime().Present(renderer, {output_width, output_height});
}

bool Render_Videos(Graphics::CommandList &command_list, const Graphics::FrameTargets &targets) noexcept
{
	return Graphics::GetVideoRenderer().Render(command_list, targets);
}

void Shutdown_Video_Presentation() noexcept
{
	Graphics::GetVideoRenderer().Shutdown();
	Video_Audio_Sink().Shutdown();
}

VideoPresentationId Open_Window_Video(GameWindow *window, const AsciiString &movie_title, WindowVideoMode mode)
{
	if (window == nullptr)
		return Invalid_Video_Presentation;
	Engine::Video::Get_Runtime().Set_Audio_Sink(&Video_Audio_Sink());

	File *file = Open_Movie_File(movie_title);
	std::unique_ptr<GameVideoSource> source(new GameVideoSource(file));
	if (source == nullptr || source->Size() <= 0)
		return Invalid_Video_Presentation;

	Engine::Video::PresentationTarget target;
	target.rect_provider = &Window_Rect;
	target.context = window;
	Engine::Video::PlaybackMode playback_mode = Engine::Video::PlaybackMode::Once;
	if (mode == WindowVideoMode::Loop)
		playback_mode = Engine::Video::PlaybackMode::Loop;
	else if (mode == WindowVideoMode::ShowLastFrame)
		playback_mode = Engine::Video::PlaybackMode::Hold_Last_Frame;
	return Engine::Video::Get_Runtime().Open(std::move(source), target, playback_mode);
}

void Close_Window_Video(VideoPresentationId presentation_id) noexcept
{
	Engine::Video::Get_Runtime().Close(presentation_id);
}

void Set_Window_Video_Visible(VideoPresentationId presentation_id, bool visible) noexcept
{
	Engine::Video::Get_Runtime().Set_Visible(presentation_id, visible);
}

bool Pause_Window_Video(VideoPresentationId presentation_id) noexcept
{
	return Engine::Video::Get_Runtime().Pause(presentation_id);
}

bool Play_Window_Video(VideoPresentationId presentation_id) noexcept
{
	return Engine::Video::Get_Runtime().Play(presentation_id);
}

Engine::Video::PlaybackState Get_Window_Video_State(VideoPresentationId presentation_id) noexcept
{
	return Engine::Video::Get_Runtime().State(presentation_id);
}
