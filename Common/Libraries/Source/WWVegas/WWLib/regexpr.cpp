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

// regexpr.cpp

#include "always.h"
#include "regexpr.h"
#include "wwstring.h"
#include <assert.h>

// Pull in the gnu_regex library's definitions.
#define __STDC__ 1
extern "C" {
#include "gnu_regex.h"
}


// The regular expression syntax options that RegularExpressionClass uses.
// The dirty details of each option are described in "gnu_regex.h"
#define OUR_SYNTAX_OPTIONS																									\
	RE_CHAR_CLASSES |					/* Support character classes such as [:alpha:] and [:digit:] */	\
	RE_CONTEXT_INDEP_ANCHORS |		/* ^ and $ are always anchors (outside bracket expressions)  */	\
	RE_CONTEXT_INDEP_OPS |			/* operators such as + * ? are always considered operators   */	\
	RE_CONTEXT_INVALID_OPS |		/* operators are invalid as the first characters in a string */	\
	RE_INTERVALS |						/* { } are used to define intervals                          */	\
	RE_NO_BK_BRACES |					/* { } are interval markers and \{ \} are literals           */	\
	RE_NO_BK_PARENS |					/* ( ) are group markers and \( \) are literals              */	\
	RE_NO_BK_VBAR |					/* | is the OR operator and \| is a literal                  */	\
	RE_NO_EMPTY_RANGES				/* [z-a] is an invalid range but [a-z] is valid              */


/*
** Definition of private DataStruct for RegularExpressionClass
*/

struct RegularExpressionClass::DataStruct
{
	DataStruct (void)
	:	IsValid(false)
	{
		// Blank out the expression structure.
		memset(&CompiledExpr, 0, sizeof(CompiledExpr));
	}

	~DataStruct (void)
	{
		ClearExpression();
	}

	void ClearExpression (void)
	{
		// If the expression was valid, let the gnu_regex library
		// deallocate any memory it had allocated for it.
		if (IsValid)
			regfree(&CompiledExpr);

		// Blank out the expression structure.
		memset(&CompiledExpr, 0, sizeof(CompiledExpr));

		// Erase the expression string.
		ExprString = "";

		// No longer a valid compiled expression.
		IsValid = false;
	}


	// The regular expression that has been compiled.
	StringClass	ExprString;

	// gnu_regex compiled version of the regular expression used
	// during matching or any form of evaluation
	regex_t		CompiledExpr;

	// True if CompiledExpr is valid.
	bool			IsValid;
};



/*
** RegularExpressionClass Implementation
*/

RegularExpressionClass::RegularExpressionClass (const char *expression)
:	Data(0)
{
	// Allocate our private members.
	Data = new DataStruct;
	assert(Data);

	// Compile the expression if we were given one.
	if (expression)
		Compile(expression);
}


RegularExpressionClass::RegularExpressionClass (const RegularExpressionClass &copy)
:	Data(0)
{
	// Allocate our private members.
	Data = new DataStruct;
	assert(Data);

	// Compile the expression if the given object had one.
	if (copy.Is_Valid())
	{
		Compile(copy.Data->ExprString);
		assert(Is_Valid());
	}
}


RegularExpressionClass::~RegularExpressionClass (void)
{
	delete Data;
	Data = 0;
}


bool RegularExpressionClass::Compile (const char *expression)
{
	assert(Data);
	assert(expression);

	// Clear any existing expression data. This makes it safe to
	// call Compile() twice on one object.
	Data->ClearExpression();

	// Set the regular expression module to the syntax that we
	// would like to use.
	reg_syntax_t old_syntax = re_set_syntax(OUR_SYNTAX_OPTIONS);

	// Compile the given expression.
	const char *error_str = re_compile_pattern(expression,
		strlen(expression), &Data->CompiledExpr);

	// Restore the old syntax setting.
	re_set_syntax(old_syntax);

	// If no error string was returned, the expression was good!
	if (error_str == 0)
	{
		Data->IsValid = true;
		Data->ExprString = expression;
		return true;
	}
	return false;
}


bool RegularExpressionClass::Is_Valid (void) const
{
	assert(Data);
	return Data->IsValid;
}


bool RegularExpressionClass::Match (const char *string) const
{
	assert(Data);

	// If we have no valid compiled expression, we can't match Jack.
	if (!Data->IsValid)
		return false;

	// Set the regular expression module to the syntax that we
	// would like to use.
	reg_syntax_t old_syntax = re_set_syntax(OUR_SYNTAX_OPTIONS);

	// Try to match the given string with our regular expression.
	int retval = re_match(&Data->CompiledExpr, string, strlen(string), 0, 0);

	// Restore the old syntax setting.
	re_set_syntax(old_syntax);

	// -1 means no match, -2 means internal gnu_regex lib error, otherwise
	// re_match returned the number of characters matched. A 0 character
	// match is valid, and distinctly different than no match at all.
	if (retval < 0)
		return false;

	// The given string matched our regular expression!
	return true;
}


/*
** Operators
*/

RegularExpressionClass & RegularExpressionClass::operator = (const RegularExpressionClass &rhs)
{
	// Check for assignment to self.
	if (*this == rhs)
		return *this;

	// Assign that object to this one.
	assert(rhs.Data);
	Compile(rhs.Data->ExprString);
	assert(Is_Valid());

	// Return this object.
	return *this;
}


bool RegularExpressionClass::operator == (const RegularExpressionClass &rhs) const
{
	// Two RegularExpressionClass objects are equivalent if they both
	// have the same validity state, and if that state is 'true' both
	// of their expressions are the same.

	// Check validity states for equality.
	if (Is_Valid() != rhs.Is_Valid())
		return false;

	// If they're valid, check their expressions.
	if (Is_Valid())
	{
		// The objects are not equivalent if their expression strings
		// don't match.
		if (Data->ExprString != rhs.Data->ExprString)
			return false;
	}
	return true;
}


inline bool RegularExpressionClass::operator != (const RegularExpressionClass &rhs) const
{
	return !(*this == rhs);
}


