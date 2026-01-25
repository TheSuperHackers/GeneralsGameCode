/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

//
// threadsafe.h
//
// If you include this file and call non-threadsafe functions, it'll
//   prevent your program from compiling.  It's meant to be a slap
//   on the wrist in case you forget and call an unsafe function
//   from a threadsafe program.
//

//
// Reminder to self - use sigwait, not signal handlers!
//

#ifdef _REENTRANT

#pragma once

#define strtok		("strtok() is not MT-SAFE!")
#define ascctime	("asctime() is not MT-SAFE!")
// Can't just do ctime, as Windows' objidl.h uses it as a FILETIME thingie
#define ctime(x)	("ctime() is not MT-SAFE!")
#define gmtime		("gmtime() is not MT-SAFE!")
#define localtime	("localtime() is not MT-SAFE!")
#define tzset		("tzset() is not MT-SAFE!")
#define tzsetwall	("tzsetwall() is not MT-SAFE!")
#define readdir		("readdir() is not MT-SAFE!")

#define rand		("rand() is not MT-SAFE!")
#define srand		("srand() is not MT-SAFE!")
#define random		("random() is not MT-SAFE!")
#define srandom		("srandom() is not MT-SAFE!")

#define tmpnam		("tmpnam() is not MT-SAFE!")
#define vfork		("vfork() is not MT-SAFE!")

#define system          ("system() is not MT-SAFE!")
#define popen           ("popen() is not MT-SAFE!")
#define pclose          ("pclose() is not MT-SAFE!")
#define ctermid         ("ctermid() is not MT-SAFE!")
#define getlogin        ("getlogin() is not MT-SAFE!");
#endif  // _REENTRANT
