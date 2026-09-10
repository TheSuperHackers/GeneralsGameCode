#pragma once

#include <SDL3/SDL.h>

// Portable date/time structure used by replay and
// save-game metadata.  The field names are retained because they are part of
// the on-disk replay format and existing UI code consumes them directly.
struct GameDateTime
{
	Uint16 wYear;
	Uint16 wMonth;
	Uint16 wDayOfWeek;
	Uint16 wDay;
	Uint16 wHour;
	Uint16 wMinute;
	Uint16 wSecond;
	Uint16 wMilliseconds;
};

inline void Get_Local_Game_Date_Time(GameDateTime *time)
{
	SDL_DateTime date_time{};
	SDL_Time now = 0;
	SDL_GetCurrentTime(&now);
	SDL_TimeToDateTime(now, &date_time, true);
	 time->wYear = static_cast<Uint16>(date_time.year);
	 time->wMonth = static_cast<Uint16>(date_time.month);
	 time->wDay = static_cast<Uint16>(date_time.day);
	 time->wDayOfWeek = static_cast<Uint16>(date_time.day_of_week);
	 time->wHour = static_cast<Uint16>(date_time.hour);
	 time->wMinute = static_cast<Uint16>(date_time.minute);
	 time->wSecond = static_cast<Uint16>(date_time.second);
	 time->wMilliseconds = static_cast<Uint16>(date_time.nanosecond / 1000000);
}
