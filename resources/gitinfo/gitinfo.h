/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include <time.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

extern const char GitSHA1[];
extern const char GitShortSHA1[];
extern const char GitCommitDate[];
extern const char GitCommitAuthorName[];
extern const char GitTag[];
extern time_t GitCommitTimeStamp;
extern bool GitUncommittedChanges;
extern bool GitHaveInfo;
extern int GitRevision;

#ifdef __cplusplus
} // extern "C"
#endif
