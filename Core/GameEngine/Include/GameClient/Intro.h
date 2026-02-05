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

class Intro
{
	enum IntroState
	{
		IntroState_Start,
		IntroState_EALogoMovie,
		IntroState_SizzleMovie,
		IntroState_LegalPage,
		IntroState_Done,
	};

public:

	Intro();

	void update();

	void enterNextState();

	Bool isDone() const { return m_currentState == IntroState_Done; }

private:

	void doEALogoMovie();
	void doSizzleMovie();
	void doLegalPage();
	void doPostIntro();

	IntroState m_currentState;
	UnsignedInt m_allowedStateFlags;
};
