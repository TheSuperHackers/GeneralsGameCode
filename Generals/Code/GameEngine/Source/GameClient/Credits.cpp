/*
**	Command & Conquer Generals(tm)
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Credits.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Dec 2002
//
//	Filename: 	Credits.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	This is where all the credit texts is going to be held.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/INI.h"
#include "GameClient/Credits.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/Display.h"
#include "GameClient/GameText.h"
#include "GameClient/GlobalLanguage.h"


//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
CreditsManager *TheCredits = NULL;

const FieldParse CreditsManager::m_creditsFieldParseTable[] = 
{

	{ "ScrollRate",					INI::parseInt,											NULL, offsetof( CreditsManager, m_scrollRate )	},
	{ "ScrollRateEveryFrames",	INI::parseInt,											NULL, offsetof( CreditsManager, m_scrollRatePerFrames )	},
	{ "ScrollDown",					INI::parseBool,											NULL,	offsetof( CreditsManager, m_scrollDown )  },
	{ "TitleColor",					INI::parseColorInt,									NULL,	offsetof( CreditsManager, m_titleColor )  },
	{ "MinorTitleColor",		INI::parseColorInt,									NULL,	offsetof( CreditsManager, m_positionColor )  },
	{ "NormalColor",				INI::parseColorInt,									NULL,	offsetof( CreditsManager, m_normalColor )  },
	{ "Style",							INI::parseLookupList,								CreditStyleNames,	offsetof( CreditsManager, m_currentStyle )  },
	{ "Blank",							CreditsManager::parseBlank,					NULL,	NULL  },
	{ "Text",								CreditsManager::parseText,					NULL,	NULL  },

	{ NULL,										NULL,													NULL, 0 }  // keep this last

};

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

void INI::parseCredits( INI *ini )
{
	// find existing item if present
	DEBUG_ASSERTCRASH( TheCredits, ("parseCredits: TheCredits has not been ininialized yet.\n") );
	if( !TheCredits )
		return;

	// parse the ini definition
	ini->initFromINI( TheCredits, TheCredits->getFieldParse() );

}  // end parseCommandButtonDefinition


CreditsLine::CreditsLine()
{
	m_useSecond = FALSE;
	m_done = FALSE;
	m_style = CREDIT_STYLE_BLANK;
	m_displayString = NULL;
	m_secondDisplayString = NULL;
}

CreditsLine::~CreditsLine()
{
	if(m_displayString)
		TheDisplayStringManager->freeDisplayString(m_displayString);
	if(m_secondDisplayString)
		TheDisplayStringManager->freeDisplayString(m_secondDisplayString);
	
	m_displayString = NULL;
	m_secondDisplayString = NULL;
}


CreditsManager::CreditsManager(void)
{
	m_scrollRate = 1; // in pixels
	m_scrollRatePerFrames = 1;
	m_scrollDown = TRUE;	// if TRUE text will come from the top to the bottom if False, it will go from the bottom up
	m_framesSinceStarted = 0;
	m_titleColor = m_positionColor = m_normalColor = GameMakeColor(255,255,255,255);
	
	m_currentStyle = CREDIT_STYLE_NORMAL;
	m_isFinished = FALSE;
	m_normalFontHeight = 10;
}

CreditsManager::~CreditsManager(void)
{

	m_displayedCreditLineList.clear();
	CreditsLineList::iterator it =m_creditLineList.begin();
	while (it != m_creditLineList.end())
	{
		CreditsLine *cLine = *it;
		delete cLine;
		it = m_creditLineList.erase(it);
	}
}

void CreditsManager::init(void )
{
	m_isFinished = FALSE;
	m_creditLineListIt = m_creditLineList.begin();
	m_framesSinceStarted = 0;
}

void CreditsManager::load(void )
{
	INI ini;
	// Read from INI all the ControlBarSchemes
	ini.load( AsciiString( "Data\\INI\\Credits.ini" ), INI_LOAD_OVERWRITE, NULL );

	if(m_scrollRatePerFrames <=0)
		m_scrollRatePerFrames = 1;
	if(m_scrollRate <=0)
		m_scrollRate = 1;
	
	GameFont *font = TheFontLibrary->getFont(TheGlobalLanguageData->m_creditsNormalFont.name,
														TheGlobalLanguageData->adjustFontSize(TheGlobalLanguageData->m_creditsNormalFont.size),
														TheGlobalLanguageData->m_creditsNormalFont.bold);

	m_normalFontHeight = font->height;
}

void CreditsManager::reset( void )
{
	m_displayedCreditLineList.clear();
	m_isFinished = FALSE;
	m_creditLineListIt = m_creditLineList.begin();
	m_framesSinceStarted = 0;

}

void CreditsManager::update( void )
{
	if(m_isFinished)
		return;
	m_framesSinceStarted++;
	
	if(m_framesSinceStarted%m_scrollRatePerFrames != 0)
		return;
	

	Int y = 0;
	Int yTest = 0;
	Int lastHeight = 0;
	Int start = m_scrollDown? 0:TheDisplay->getHeight();
	Int end =m_scrollDown? TheDisplay->getHeight():0;
	Int offsetStartMultiplyer = m_scrollDown? -1:0;  // if we're scrolling from the top, we need to subtract the height
	Int offsetEndMultiplyer = m_scrollDown? 0:1;
	Int directionMultiplyer = m_scrollDown? 1:-1;
	CreditsLineList::iterator drawIt = m_displayedCreditLineList.begin();
	while (drawIt != m_displayedCreditLineList.end())
	{
		CreditsLine *cLine = *drawIt;
		y = cLine->m_pos.y = cLine->m_pos.y + (m_scrollRate * directionMultiplyer);
		lastHeight = cLine->m_height;
		yTest = y + ((lastHeight + CREDIT_SPACE_OFFSET) * offsetEndMultiplyer);
		if(((m_scrollDown && (yTest > end)) || (!m_scrollDown && (yTest < end))))
		{
			TheDisplayStringManager->freeDisplayString(cLine->m_displayString);
			TheDisplayStringManager->freeDisplayString(cLine->m_secondDisplayString);
			cLine->m_displayString = NULL;
			cLine->m_secondDisplayString = NULL;
			drawIt = m_displayedCreditLineList.erase(drawIt);
		}
		else
			drawIt++;
	}
	
	y= y + ((lastHeight + CREDIT_SPACE_OFFSET) * offsetStartMultiplyer);
	
	// is it time to add a new string?
	if(!((m_scrollDown && (yTest >= start)) || (!m_scrollDown && (yTest  <= start))))
		return;
	
	if(m_displayedCreditLineList.size() == 0 && m_creditLineListIt == m_creditLineList.end())
		m_isFinished = TRUE;
	
	if(m_creditLineListIt == m_creditLineList.end())
		return;

	CreditsLine *cLine = *m_creditLineListIt;
	ICoord2D pos;
	switch (cLine->m_style) 
	{
	case CREDIT_STYLE_TITLE:
		{
			cLine->m_color = m_titleColor;
			
			if(TheGlobalLanguageData&& !cLine->m_text.isEmpty())
			{
				DisplayString *ds = TheDisplayStringManager->newDisplayString();
				if(!ds)
					return;
				ds->setFont(TheFontLibrary->getFont(TheGlobalLanguageData->m_creditsTitleFont.name,
														TheGlobalLanguageData->adjustFontSize(TheGlobalLanguageData->m_creditsTitleFont.size),
														TheGlobalLanguageData->m_creditsTitleFont.bold));
				ds->setText(cLine->m_text);
				ds->getSize(&pos.x,&pos.y);
				cLine->m_height = pos.y;
				cLine->m_pos.x = TheDisplay->getWidth()/2 - pos.x/2 ;
				cLine->m_pos.y = start + (cLine->m_height * offsetStartMultiplyer);
				cLine->m_displayString = ds;
			}
		}
		break;
	case CREDIT_STYLE_POSITION:
		{
			cLine->m_color = m_positionColor;
			
			if(TheGlobalLanguageData && !cLine->m_text.isEmpty())
			{
				DisplayString *ds = TheDisplayStringManager->newDisplayString();
				if(!ds)
					return;
				ds->setFont(TheFontLibrary->getFont(TheGlobalLanguageData->m_creditsPositionFont.name,
														TheGlobalLanguageData->adjustFontSize(TheGlobalLanguageData->m_creditsPositionFont.size),
														TheGlobalLanguageData->m_creditsPositionFont.bold));
				ds->setText(cLine->m_text);
				ds->getSize(&pos.x,&pos.y);
				cLine->m_height = pos.y;
				cLine->m_pos.x = TheDisplay->getWidth()/2 - pos.x/2 ;
				cLine->m_pos.y = start + (cLine->m_height * offsetStartMultiplyer);
				cLine->m_displayString = ds;
			}
		}
		break;
	case CREDIT_STYLE_NORMAL:
	 {
			cLine->m_color = m_normalColor;
			
			if(TheGlobalLanguageData && !cLine->m_text.isEmpty())
			{
				DisplayString *ds = TheDisplayStringManager->newDisplayString();
				if(!ds)
					return;
				ds->setFont(TheFontLibrary->getFont(TheGlobalLanguageData->m_creditsNormalFont.name,
														TheGlobalLanguageData->adjustFontSize(TheGlobalLanguageData->m_creditsNormalFont.size),
														TheGlobalLanguageData->m_creditsNormalFont.bold));
				ds->setText(cLine->m_text);
				ds->getSize(&pos.x,&pos.y);
				cLine->m_height = pos.y;
				cLine->m_pos.x = TheDisplay->getWidth()/2 - pos.x/2 ;
				cLine->m_pos.y = start + (cLine->m_height * offsetStartMultiplyer);
				cLine->m_displayString = ds;
			}
		}
		break;
	case CREDIT_STYLE_COLUMN:
		{
			cLine->m_color = m_normalColor;
			
			if(TheGlobalLanguageData && !cLine->m_text.isEmpty())
			{
				DisplayString *ds = TheDisplayStringManager->newDisplayString();
				if(!ds)
					return;
				ds->setFont(TheFontLibrary->getFont(TheGlobalLanguageData->m_creditsNormalFont.name,
														TheGlobalLanguageData->adjustFontSize(TheGlobalLanguageData->m_creditsNormalFont.size),
														TheGlobalLanguageData->m_creditsNormalFont.bold));
				ds->setText(cLine->m_text);
				ds->getSize(&pos.x,&pos.y);
				cLine->m_height = pos.y;
				cLine->m_pos.x = TheDisplay->getWidth()/2 - pos.x/2 ;
				cLine->m_pos.y = start + (cLine->m_height * offsetStartMultiplyer);
				cLine->m_displayString = ds;
			}
			if(TheGlobalLanguageData && !cLine->m_secondText.isEmpty())
			{
				DisplayString *ds = TheDisplayStringManager->newDisplayString();
				if(!ds)
					return;
				ds->setFont(TheFontLibrary->getFont(TheGlobalLanguageData->m_creditsNormalFont.name,
														TheGlobalLanguageData->adjustFontSize(TheGlobalLanguageData->m_creditsNormalFont.size),
														TheGlobalLanguageData->m_creditsNormalFont.bold));
				ds->setText(cLine->m_secondText);
				ds->getSize(&pos.x,&pos.y);
				cLine->m_height = pos.y;
				cLine->m_pos.x = TheDisplay->getWidth()/2 - pos.x/2 ;
				cLine->m_pos.y = start + (cLine->m_height * offsetStartMultiplyer);
				cLine->m_secondDisplayString = ds;
				
			}
		}
		break;
	case CREDIT_STYLE_BLANK:
		{
			cLine->m_height = m_normalFontHeight;
			cLine->m_pos.y = start + (cLine->m_height * offsetStartMultiplyer);
		}
		break;
	}

	m_displayedCreditLineList.push_back(cLine);

	if(m_creditLineListIt != m_creditLineList.end())
		m_creditLineListIt++;
	
}

void CreditsManager::draw( void )
{
	CreditsLineList::iterator drawIt = m_displayedCreditLineList.begin();
	while (drawIt != m_displayedCreditLineList.end())
	{
		CreditsLine *cLine = *drawIt;
		Int heightChunk = TheDisplay->getHeight()/3;
		Real perc = 0.0f;
		if(cLine->m_pos.y < heightChunk || cLine->m_pos.y > heightChunk * 2)
		{
			// adjust the color
			if( cLine->m_pos.y < 0 || cLine->m_pos.y > TheDisplay->getHeight())
				perc = 0.0f;
			else if( cLine->m_pos.y < heightChunk)
				perc = INT_TO_REAL(cLine->m_pos.y) / heightChunk;
			else
				perc = 1 - INT_TO_REAL(cLine->m_pos.y - 2*heightChunk) / heightChunk;
		}
		else
			perc = 1.0f;
		UnsignedByte r,g,b,a;
		GameGetColorComponents(cLine->m_color, &r, &g, &b, &a);		
		Int color = GameMakeColor( r,g,b, a * perc);
		Int bColor= GameMakeColor( 0,0,0, a * perc);

		switch (cLine->m_style) {
		case CREDIT_STYLE_TITLE:
		case CREDIT_STYLE_POSITION:
		case CREDIT_STYLE_NORMAL:
			{
				if(cLine->m_displayString)
					cLine->m_displayString->draw(cLine->m_pos.x,cLine->m_pos.y,color, bColor, 1,1 );
			}
			break;
		case CREDIT_STYLE_COLUMN:
			{
				Int chunk = TheDisplay->getWidth()/3;
				ICoord2D pos;
				if(cLine->m_displayString)
				{
					cLine->m_displayString->getSize(&pos.x, &pos.y);
					cLine->m_displayString->draw(chunk - (pos.x/2),cLine->m_pos.y,color, bColor, 1,1 );
				}
				if(cLine->m_secondDisplayString)
				{
					cLine->m_secondDisplayString->getSize(&pos.x, &pos.y);
					cLine->m_secondDisplayString->draw(2*chunk - (pos.x/2),cLine->m_pos.y,color, bColor, 1,1 );
				}
			}
			break;
		}

		drawIt++;
	}
}
void CreditsManager::addBlank( void )
{
	CreditsLine *cLine = new CreditsLine;
	cLine->m_style = CREDIT_STYLE_BLANK;
	m_creditLineList.push_back(cLine);
}



void CreditsManager::parseBlank( INI* ini, void *instance, void *store, const void *userData )
{
	CreditsManager *cManager = (CreditsManager *)instance;
	cManager->addBlank();
}

void CreditsManager::parseText( INI* ini, void *instance, void *store, const void *userData )
{
	
	AsciiString asciiString = ini->getNextQuotedAsciiString();
	CreditsManager *cManager = (CreditsManager *)instance;
	cManager->addText(asciiString);
}
void CreditsManager::addText( AsciiString text )
{
	CreditsLine *cLine = new CreditsLine;

	switch (m_currentStyle) 
	{
		case CREDIT_STYLE_TITLE:
		case CREDIT_STYLE_POSITION:
		case CREDIT_STYLE_NORMAL:
			{
				cLine->m_text = getUnicodeString(text);
				cLine->m_style = m_currentStyle;
				m_creditLineList.push_back(cLine);
			}
			break;
		case CREDIT_STYLE_COLUMN:
			{
				CreditsLineList::reverse_iterator rIt = m_creditLineList.rbegin();
				CreditsLine *rcLine = *rIt;
				if(rIt == m_creditLineList.rend() || rcLine->m_style != CREDIT_STYLE_COLUMN 
				   || (rcLine->m_style == CREDIT_STYLE_COLUMN && rcLine->m_done == TRUE))
				{
					cLine->m_text = getUnicodeString(text);
					cLine->m_style = CREDIT_STYLE_COLUMN;
					cLine->m_useSecond = TRUE;
					m_creditLineList.push_back(cLine);
				}
				else
				{
					rcLine->m_secondText = getUnicodeString(text);
					rcLine->m_done = TRUE;
					delete cLine;
				}

			}
			break;
		default:
			DEBUG_ASSERTCRASH( FALSE, ("CreditsManager::addText we tried to add a credit text with the wrong style before it.  Style is %d\n", m_currentStyle) );
			delete cLine;
	}

}

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

UnicodeString CreditsManager::getUnicodeString(AsciiString str)
{
	UnicodeString uStr;
	if(str.compare("<BLANK>") == 0)
		return UnicodeString::TheEmptyString;

	if(str.find(':'))
		uStr = TheGameText->fetch(str);
	else
		uStr.translate(str);

	return uStr;
}
