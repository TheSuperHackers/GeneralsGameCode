/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#include <vector>

class DisplayString;
class Image;

class Intro
{
	enum IntroState
	{
		IntroState_Start,
		IntroState_EALogoMovie,
		IntroState_TheSuperHackersWait,
		IntroState_TheSuperHackers,
		IntroState_SizzleMovieWait,
		IntroState_SizzleMovie,
		IntroState_Done,
	};

	struct DisplayEntity
	{
		DisplayEntity()
			: displayString(nullptr)
			, image(nullptr)
			, sizeX(10)
			, sizeY(10)
			, centerOffsetY(0)
		{}
		~DisplayEntity();

		DisplayString* displayString; // is owner
		const Image* image;
		Int sizeX;
		Int sizeY;
		Int centerOffsetY;
	};

public:

	Intro();

	void update();
	void draw();

	Bool skipCurrentIntroStage();
	Bool isDone() const { return m_currentState == IntroState_Done; }

private:

	void enterNextState();

	void doEALogoMovie();
	void doTheSuperHackers();
	void doSizzleMovie();
	void doPostIntro();
	void doAsyncWait(UnsignedInt milliseconds);

	void drawDisplayEntities();

private:

	IntroState m_currentState;
	UnsignedInt m_allowedStateFlags;
	UnsignedInt m_waitUntilMs;
	UnicodeString m_unicodeStrings[1];
	std::vector<DisplayEntity> m_displayEntities;
	Real m_fadeValue;
};
