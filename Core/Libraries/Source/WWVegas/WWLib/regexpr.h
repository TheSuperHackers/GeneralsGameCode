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

// regexpr.h

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef REGEXPR_H
#define REGEXPR_H


/*
** RegularExpressionClass - This class encapsulates regular expression
** evaluation. It provides a nice and simple wrapper around the
** unfriendliness that is the GNU regex library. So try not to use
** gnu_regex.h anywhere, cause it'll make your code rather gross looking.
**
** 10/12/2000 AJA - Initial Implementation
**
** 01/02/2001 AJA - Added a quick regular expression tutorial to the project.
** You can find it in your wwlib directory in a file named
** "Regular Expression Tutorial.html". This file comes from the web at
** http://www.living-source.com/user/matte/regexp/regexp_tutorial.html
*/
class RegularExpressionClass
{
public:
	RegularExpressionClass (const char *expression=0);
	RegularExpressionClass (const RegularExpressionClass &copy);
	~RegularExpressionClass (void);

	// Before you try to match a string against this regular expression,
	// you must first compile the expression. This has the logical effect
	// of assigning the regular expression string to this
	// RegularExpressionClass object.
	// If you passed the expression into the constructor of this object,
	// Compile() was called for you. To ensure the compilation in the
	// constructor succeeded, check the return value of Is_Valid().
	// Compile() returns true if the compilation succeeded (ie. the given
	// expression string formed a valid regular expression).
	bool Compile (const char *expression);

	// Returns true if a valid regular expression has been assigned to
	// this object. If you passed a regular expression string into this
	// object's constructor, make sure you check Is_Valid before calling
	// Match().
	bool Is_Valid (void) const;

	// Tests if 'string' matches this regular expression.
	bool Match (const char *string) const;

	// Standard operators.
	RegularExpressionClass & operator = (const RegularExpressionClass &rhs);
	bool operator == (const RegularExpressionClass &rhs) const;
	bool operator != (const RegularExpressionClass &rhs) const;

private:

	struct DataStruct;
	DataStruct	*Data;
};


#endif

