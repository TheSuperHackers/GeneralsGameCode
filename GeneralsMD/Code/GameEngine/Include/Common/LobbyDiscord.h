/*
**	Command & Conquer Generals Zero Hour(tm)
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

class LANGameInfo;

/// Render an upscaled lobby map preview with player labels at each fixed
/// start position and POST it to the Discord webhook URL baked in at
/// build time (cmake/discordwebhook.cmake -> ZULU_DISCORD_WEBHOOK_URL).
///
/// No-op when:
///   * the build was configured without a webhook URL,
///   * `game` is null or has no map metadata,
///   * fewer than two human (non-AI) slots are occupied (per the
///     "2+ humans" requirement; AI-only and 1-human matches don't post).
///
/// Best-effort and synchronous: blocks the caller for the WinINet POST
/// (mirroring the existing balance_teams / map_summary calls). Intended
/// to be invoked by the host once at game-start, just before the start
/// signal is broadcast to the rest of the lobby.
void PostLanLobbyMapToDiscord(LANGameInfo *game);
