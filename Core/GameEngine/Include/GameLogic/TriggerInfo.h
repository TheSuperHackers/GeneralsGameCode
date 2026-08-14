/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#include "Lib/BaseType.h"

#include <vector>


class PolygonTrigger;

struct TTriggerInfo
{
	const PolygonTrigger *pTrigger; ///< The trigger area that the object is inside.
	Byte entered; ///< True if the object entered this trigger area this frame.
	Byte exited; ///< True if the object exited this trigger area this frame.
	Byte isInside; ///< True if the object is inside this trigger area this frame.
	Byte padding; ///< Unused.

	TTriggerInfo() : pTrigger(nullptr), entered(false), exited(false), isInside(false), padding(false) {}
};

class TriggerInfoStorage
{
public:
	enum
	{
		InlineCapacity = 5,
		MaxCapacity = 127,
	};

	Bool resize(Int size)
	{
		if (size < 0 || size > MaxCapacity)
			return false;

		const Int overflowSize = size - InlineCapacity;
		if (overflowSize > 0)
			m_overflow.resize(overflowSize);
		else
			m_overflow.clear();

		return true;
	}

	TTriggerInfo &operator[](Int index)
	{
		return index < InlineCapacity ? m_inline[index] : m_overflow[index - InlineCapacity];
	}

	const TTriggerInfo &operator[](Int index) const
	{
		return index < InlineCapacity ? m_inline[index] : m_overflow[index - InlineCapacity];
	}

	Int clearTransitionsAndRemoveExited(Int size)
	{
		Int destination = 0;
		for (Int source = 0; source < size; ++source)
		{
			if (!(*this)[source].isInside)
				continue;

			if (destination != source)
				(*this)[destination] = (*this)[source];

			(*this)[destination].entered = false;
			(*this)[destination].exited = false;
			++destination;
		}

		resize(destination);
		return destination;
	}

private:
	TTriggerInfo m_inline[InlineCapacity];
	std::vector<TTriggerInfo> m_overflow;
};
