#pragma once

#include <cstdint>
#include <filesystem>

#include "Common/AsciiString.h"

import Video.Decoder;
import Video.Runtime;
import Graphics.FrameTargets;

class GameWindow;

enum class WindowVideoMode : std::uint8_t
{
	Once,
	Loop,
	ShowLastFrame
};

using VideoPresentationId = std::uint64_t;
constexpr VideoPresentationId Invalid_Video_Presentation = 0;

bool Open_Video(Engine::Video::PlaybackSlot slot, const AsciiString &movie_title);
bool Open_Fullscreen_Video(const AsciiString &movie_title);
void Close_Video(Engine::Video::PlaybackSlot slot) noexcept;
void Close_Fullscreen_Video() noexcept;
void Close_All_Videos() noexcept;
bool Update_Videos(double delta_seconds);
bool Is_Fullscreen_Video_Playing() noexcept;
Engine::Video::PlaybackState Get_Video_State(Engine::Video::PlaybackSlot slot) noexcept;
Engine::Video::StreamInfo Get_Video_Info(Engine::Video::PlaybackSlot slot) noexcept;
const Engine::Video::DecodedVideoFrame *Get_Video_Frame(Engine::Video::PlaybackSlot slot) noexcept;

bool Initialize_Video_Presentation(Graphics::Device &device, const std::filesystem::path &shader_directory);
void Begin_Video_Frame() noexcept;
void Submit_Videos(std::uint32_t output_width, std::uint32_t output_height);
bool Render_Videos(Graphics::CommandList &command_list, const Graphics::FrameTargets &targets) noexcept;
void Shutdown_Video_Presentation() noexcept;

VideoPresentationId Open_Window_Video(GameWindow *window, const AsciiString &movie_title, WindowVideoMode mode);
void Close_Window_Video(VideoPresentationId presentation_id) noexcept;
void Set_Window_Video_Visible(VideoPresentationId presentation_id, bool visible) noexcept;
bool Pause_Window_Video(VideoPresentationId presentation_id) noexcept;
bool Play_Window_Video(VideoPresentationId presentation_id) noexcept;
Engine::Video::PlaybackState Get_Window_Video_State(VideoPresentationId presentation_id) noexcept;
