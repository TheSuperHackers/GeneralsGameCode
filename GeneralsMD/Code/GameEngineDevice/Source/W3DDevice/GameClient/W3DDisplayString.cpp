/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#include "Precompiled/PreRTS.h"

#include <cstdint>

#include "GameClient/Display.h"
#include "GameClient/GameClient.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"
#include "GameClient/HotKey.h"
#include "GameClient/GameFont.h"
#include "GameClient/GlobalLanguage.h"

import Engine.UI.WND;
import Graphics.Renderer2D;

namespace
{

Graphics::Color2D To_UI_Color(Color color) noexcept
{
	return {
		static_cast<float>((color >> 16) & 0xff) / 255.0f,
		static_cast<float>((color >> 8) & 0xff) / 255.0f,
		static_cast<float>(color & 0xff) / 255.0f,
		static_cast<float>((color >> 24) & 0xff) / 255.0f};
}

}

W3DDisplayString::W3DDisplayString()
{
	m_textChanged = FALSE;
	m_fontChanged = FALSE;
	m_hotKeyFont = nullptr;
	m_useHotKey = FALSE;
	m_hotKeyColor = GameMakeColor(255, 255, 255, 255);
	m_size.x = 0;
	m_size.y = 0;
	m_clipRegion.lo.x = 0;
	m_clipRegion.lo.y = 0;
	m_clipRegion.hi.x = 0;
	m_clipRegion.hi.y = 0;
	m_wordWrap = 0;
	m_wordWrapCentered = FALSE;
	m_hasClipRegion = FALSE;
	m_lastResourceFrame = 0;
}

W3DDisplayString::~W3DDisplayString()
{
}

void W3DDisplayString::notifyTextChanged()
{
	DisplayString::notifyTextChanged();
	computeExtents();
	m_textChanged = TRUE;
}

void W3DDisplayString::draw(Int x, Int y, Color color, Color dropColor)
{
	draw(x, y, color, dropColor, 1, 1);
}

void W3DDisplayString::draw(Int x, Int y, Color color, Color dropColor, Int xDrop, Int yDrop)
{
	if (getTextLength() == 0)
		return;

	if (m_font == nullptr || sizeof(WideChar) != sizeof(std::uint16_t))
		return;

	Engine::UI::WND::FontFace *font = static_cast<Engine::UI::WND::FontFace *>(m_font->fontData);
	if (font == nullptr)
		return;

	if (m_useHotKey && TheHotKeyManager != nullptr) {
		m_hotkey.translate(TheHotKeyManager->searchHotKey(getText()));
		if (m_hotkey.isEmpty())
			m_useHotKey = FALSE;
	}

	Graphics::Renderer2D &renderer = Graphics::Get_Renderer2D();
	if (!renderer.Is_Initialized())
		return;
	const Graphics::Rect2D text_clip{
		static_cast<float>(m_clipRegion.lo.x), static_cast<float>(m_clipRegion.lo.y),
		static_cast<float>(m_clipRegion.hi.x), static_cast<float>(m_clipRegion.hi.y)};
	const Engine::UI::WND::ClipScope clip_scope(renderer, m_hasClipRegion, text_clip);
	const Engine::UI::WND::TextLayoutOptions options{
		m_wordWrap,
		m_wordWrapCentered,
		m_useHotKey != FALSE,
		static_cast<std::uint16_t>(m_hotkey.isEmpty() ? 0 : m_hotkey.str()[0]),
		TheGlobalLanguageData != nullptr && TheGlobalLanguageData->m_useHardWrap == TRUE};
	const Engine::UI::WND::TextStyle style{
		To_UI_Color(color), To_UI_Color(dropColor), To_UI_Color(m_hotKeyColor), xDrop, yDrop};
	const bool rendered = Engine::UI::WND::Get_Text_Renderer().Draw(
		renderer,
		*font,
		m_hotKeyFont,
		reinterpret_cast<const std::uint16_t *>(getText().str()),
		static_cast<float>(x),
		static_cast<float>(y),
		options,
		style);
	if (!rendered)
		return;

	m_textChanged = FALSE;
	m_fontChanged = FALSE;
	if (TheGameClient != nullptr)
		usingResources(TheGameClient->getFrame());
}

bool W3DDisplayString::appendDrawData(
	Engine::UI::WND::DrawList &drawList,
	Int x, Int y, Color color, Color dropColor, Int xDrop, Int yDrop,
	const IRegion2D *clipRegion)
{
	if (getTextLength() == 0 || m_font == nullptr || sizeof(WideChar) != sizeof(std::uint16_t))
		return true;

	Engine::UI::WND::FontFace *font =
		static_cast<Engine::UI::WND::FontFace *>(m_font->fontData);
	if (font == nullptr)
		return false;

	if (m_useHotKey && TheHotKeyManager != nullptr) {
		m_hotkey.translate(TheHotKeyManager->searchHotKey(getText()));
		if (m_hotkey.isEmpty())
			m_useHotKey = FALSE;
	}

	const Engine::UI::WND::TextLayoutOptions options{
		m_wordWrap,
		m_wordWrapCentered,
		m_useHotKey != FALSE,
		static_cast<std::uint16_t>(m_hotkey.isEmpty() ? 0 : m_hotkey.str()[0]),
		TheGlobalLanguageData != nullptr && TheGlobalLanguageData->m_useHardWrap == TRUE};
	const Engine::UI::WND::TextStyle style{
		To_UI_Color(color), To_UI_Color(dropColor), To_UI_Color(m_hotKeyColor), xDrop, yDrop};
	const IRegion2D *effectiveClip = clipRegion != nullptr
		? clipRegion
		: (m_hasClipRegion ? &m_clipRegion : nullptr);
	Graphics::Rect2D textClip{};
	if (effectiveClip != nullptr) {
		textClip = {
			static_cast<float>(effectiveClip->lo.x), static_cast<float>(effectiveClip->lo.y),
			static_cast<float>(effectiveClip->hi.x), static_cast<float>(effectiveClip->hi.y)};
	}
	if (!drawList.Add_Text(
			font,
			m_hotKeyFont,
			reinterpret_cast<const std::uint16_t *>(m_textString.str()),
			static_cast<float>(x),
			static_cast<float>(y),
			options,
			style,
		effectiveClip != nullptr,
			textClip))
		return false;

	m_textChanged = FALSE;
	m_fontChanged = FALSE;
	if (TheGameClient != nullptr)
		usingResources(TheGameClient->getFrame());
	return true;
}

bool W3DDisplayString::appendStaticTextDrawData(
	Engine::UI::WND::DrawList &drawList,
	const Engine::UI::WND::StaticTextVisual &visual,
	Color color,
	Color dropColor)
{
	if (getTextLength() == 0 || m_font == nullptr || sizeof(WideChar) != sizeof(std::uint16_t))
		return true;

	Engine::UI::WND::FontFace *font =
		static_cast<Engine::UI::WND::FontFace *>(m_font->fontData);
	if (font == nullptr)
		return false;

	if (m_useHotKey && TheHotKeyManager != nullptr) {
		m_hotkey.translate(TheHotKeyManager->searchHotKey(getText()));
		if (m_hotkey.isEmpty())
			m_useHotKey = FALSE;
	}

	const Engine::UI::WND::TextLayoutOptions options{
		m_wordWrap,
		m_wordWrapCentered,
		m_useHotKey != FALSE,
		static_cast<std::uint16_t>(m_hotkey.isEmpty() ? 0 : m_hotkey.str()[0]),
		TheGlobalLanguageData != nullptr && TheGlobalLanguageData->m_useHardWrap == TRUE};
	const Engine::UI::WND::StaticTextContent content{
		font,
		m_hotKeyFont,
		reinterpret_cast<const std::uint16_t *>(m_textString.str()),
		options,
		{To_UI_Color(color), To_UI_Color(dropColor), To_UI_Color(m_hotKeyColor), 1, 1},
		true,
		visual.rectangle};
	if (!Engine::UI::WND::Add_Static_Text(drawList, visual, content))
		return false;

	m_textChanged = FALSE;
	m_fontChanged = FALSE;
	if (TheGameClient != nullptr)
		usingResources(TheGameClient->getFrame());
	return true;
}

void W3DDisplayString::getSize(Int *width, Int *height)
{
	if (width != nullptr)
		*width = m_size.x;
	if (height != nullptr)
		*height = m_size.y;
}

Int W3DDisplayString::getWidth(Int charPos)
{
	const Engine::UI::WND::FontFace *font = static_cast<const Engine::UI::WND::FontFace *>(m_font != nullptr ? m_font->fontData : nullptr);
	if (font == nullptr)
		return 0;

	Int width = 0;
	Int count = 0;
	for (const WideChar *text = m_textString.str(); *text != 0
		&& (charPos == -1 || count < charPos); ++text, ++count) {
		if (*text != static_cast<WideChar>('\n'))
		width += font->Get_Char_Spacing(static_cast<std::uint16_t>(*text));
	}
	return width;
}

void W3DDisplayString::setFont(GameFont *font)
{
	if (font == nullptr || m_font == font)
		return;

	DisplayString::setFont(font);
	m_hotKeyFont = static_cast<Engine::UI::WND::FontFace *>(font->fontData);
	if (TheFontLibrary != nullptr) {
		if (GameFont *boldFont = TheFontLibrary->getFont(font->nameString, font->pointSize, TRUE))
			m_hotKeyFont = static_cast<Engine::UI::WND::FontFace *>(boldFont->fontData);
	}
	computeExtents();
	m_fontChanged = TRUE;
}

void W3DDisplayString::setClipRegion(IRegion2D *region)
{
	DisplayString::setClipRegion(region);
	m_hasClipRegion = region != nullptr;
	if (region != nullptr)
		m_clipRegion = *region;
}

void W3DDisplayString::computeExtents()
{
	if (getTextLength() == 0 || m_font == nullptr) {
		m_size.x = 0;
		m_size.y = 0;
		return;
	}
	if (sizeof(WideChar) != sizeof(std::uint16_t)) {
		m_size.x = 0;
		m_size.y = 0;
		return;
	}

	const Engine::UI::WND::FontFace *font = static_cast<const Engine::UI::WND::FontFace *>(m_font->fontData);
	if (font == nullptr) {
		m_size.x = 0;
		m_size.y = 0;
		return;
	}

	std::uint32_t width = 0;
	std::uint32_t height = 0;
	const Engine::UI::WND::TextLayoutOptions options{
		m_wordWrap,
		m_wordWrapCentered,
		false,
		0,
		TheGlobalLanguageData != nullptr && TheGlobalLanguageData->m_useHardWrap == TRUE};
	if (!Engine::UI::WND::Get_Text_Renderer().Measure(
			*font,
			reinterpret_cast<const std::uint16_t *>(getText().str()),
			options,
			width,
			height)) {
		m_size.x = 0;
		m_size.y = 0;
		return;
	}
	m_size.x = static_cast<Int>(width);
	m_size.y = static_cast<Int>(height);
}

void W3DDisplayString::setWordWrap(Int wordWrap)
{
	if (m_wordWrap == wordWrap)
		return;
	m_wordWrap = wordWrap;
	notifyTextChanged();
}

void W3DDisplayString::setUseHotkey(Bool useHotkey, Color hotKeyColor)
{
	if (m_useHotKey == useHotkey && m_hotKeyColor == hotKeyColor)
		return;
	m_useHotKey = useHotkey;
	m_hotKeyColor = hotKeyColor;
	notifyTextChanged();
}

void W3DDisplayString::setWordWrapCentered(Bool isCentered)
{
	if (m_wordWrapCentered == isCentered)
		return;
	m_wordWrapCentered = isCentered;
	notifyTextChanged();
}
