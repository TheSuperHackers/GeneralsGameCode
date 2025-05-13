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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: WOLLoginMenu.cpp
// Author: Chris Huybregts, November 2001
// Description: Lan Lobby Menu
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/STLTypedefs.h"

#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameEngine.h"
#include "Common/GameSpyMiscPreferences.h"
#include "Common/QuotedPrintable.h"
#include "Common/Registry.h"
#include "Common/UserPreferences.h"
#include "GameClient/AnimateWindowManager.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameText.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/MessageBox.h"
#include "GameClient/ShellHooks.h"
#include "GameClient/GameWindowTransitions.h"

#include "GameNetwork/GameSpy/GSConfig.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PingThread.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/ThreadUtils.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"

#include "GameNetwork/GameSpyOverlay.h"

#include "GameNetwork/WOLBrowser/WebBrowser.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

#ifdef ALLOW_NON_PROFILED_LOGIN
Bool GameSpyUseProfiles = false;
#endif // ALLOW_NON_PROFILED_LOGIN

static Bool webBrowserActive = FALSE;
static Bool useWebBrowserForTOS = FALSE;

static Bool isShuttingDown = false;
static Bool buttonPushed = false;
static const char *nextScreen = NULL;

static const UnsignedInt loginTimeoutInMS = 10000;
static UnsignedInt loginAttemptTime = 0;

class GameSpyLoginPreferences : public UserPreferences
{
public:
	GameSpyLoginPreferences() { m_emailPasswordMap.clear(); m_emailNickMap.clear(); }
	virtual ~GameSpyLoginPreferences() { m_emailPasswordMap.clear(); m_emailNickMap.clear(); }

	virtual Bool load(AsciiString fname);
	virtual Bool write(void);

	AsciiString getPasswordForEmail( AsciiString email );
	AsciiString getDateForEmail( AsciiString email, AsciiString &month, AsciiString &date, AsciiString &year  );
	AsciiStringList getNicksForEmail( AsciiString email );
	void addLogin( AsciiString email, AsciiString nick, AsciiString password, AsciiString date );
	void forgetLogin( AsciiString email );
	AsciiStringList getEmails( void );

private:
	typedef std::map<AsciiString, AsciiString> PassMap;
	typedef std::map<AsciiString, AsciiString> DateMap;
	typedef std::map<AsciiString, AsciiStringList> NickMap;
	PassMap m_emailPasswordMap;
	NickMap m_emailNickMap;
	DateMap m_emailDateMap;
};

static AsciiString obfuscate( AsciiString in )
{
	char *buf = NEW char[in.getLength() + 1];
	strcpy(buf, in.str());
	static const char *xorWord = "1337Munkee";
	char *c = buf;
	const char *c2 = xorWord;
	while (*c)
	{
		if (!*c2)
			c2 = xorWord;
		if (*c != *c2)
			*c = *c++ ^ *c2++;
		else
			c++, c2++;
	}
	AsciiString out = buf;
	delete buf;
	return out;
}

Bool GameSpyLoginPreferences::load( AsciiString fname )
{
	if (!UserPreferences::load(fname))
		return false;

	UserPreferences::iterator upIt = begin();
	while (upIt != end())
	{
		AsciiString key = upIt->first;
		if (key.startsWith("pass_"))
		{
			AsciiString email, pass;
			email = key.str() + 5;
			pass = upIt->second;

			AsciiString quoPass = QuotedPrintableToAsciiString(pass);
			pass = obfuscate(quoPass);

			m_emailPasswordMap[email] = pass;
		}
		if (key.startsWith("date_"))
		{
			AsciiString email, date;
			email = key.str() + 5;
			date = upIt->second;

			date = QuotedPrintableToAsciiString(date);

			m_emailDateMap[email] = date;
		}
		else if (key.startsWith("nick_"))
		{
			AsciiString email, nick, nicks;
			email = key.str() + 5;
			nicks = upIt->second;
			while (nicks.nextToken(&nick, ","))
			{
				m_emailNickMap[email].push_back(nick);
			}
		}
		++upIt;
	}

	return true;
}

Bool GameSpyLoginPreferences::write( void )
{
	if (m_filename.isEmpty())
		return false;

	FILE *fp = fopen(m_filename.str(), "w");
	if (fp)
	{
		fprintf(fp, "lastEmail = %s\n",   ((*this)["lastEmail"].str()));
		fprintf(fp, "lastName = %s\n",    ((*this)["lastName"].str()));
		fprintf(fp, "useProfiles = %s\n", ((*this)["useProfiles"].str()));
		PassMap::iterator passIt = m_emailPasswordMap.begin();
		while (passIt != m_emailPasswordMap.end())
		{
			AsciiString pass = obfuscate(passIt->second);
			AsciiString quoPass = AsciiStringToQuotedPrintable(pass);

			fprintf(fp, "pass_%s = %s\n", passIt->first.str(), quoPass.str());
			++passIt;
		}
		
		PassMap::iterator dateIt = m_emailDateMap.begin();
		while (dateIt != m_emailDateMap.end())
		{
			AsciiString date = AsciiStringToQuotedPrintable(dateIt->second);

			fprintf(fp, "date_%s = %s\n", dateIt->first.str(), date.str());
			++dateIt;
		}

		NickMap::iterator nickIt = m_emailNickMap.begin();
		while (nickIt != m_emailNickMap.end())
		{
			AsciiString nicks;
			AsciiStringListIterator listIt = nickIt->second.begin();
			while (listIt != nickIt->second.end())
			{
				nicks.concat(*listIt);
				nicks.concat(',');
				++listIt;
			}
			fprintf(fp, "nick_%s = %s\n", nickIt->first.str(), nicks.str());
			++nickIt;
		}

		fclose(fp);
		return true;
	}
	return false;
}
AsciiString GameSpyLoginPreferences::getDateForEmail( AsciiString email, AsciiString &month, AsciiString &date, AsciiString &year )
{
	if ( m_emailDateMap.find(email) == m_emailDateMap.end() )
		return AsciiString::TheEmptyString;
	AsciiString fullDate = m_emailDateMap[email];
	if(fullDate.getLength() != 8)
		return AsciiString::TheEmptyString;
	month.format("%c%c", fullDate.getCharAt(0), fullDate.getCharAt(1));
	date.format("%c%c", fullDate.getCharAt(2), fullDate.getCharAt(3));
	year.format("%c%c%c%c", fullDate.getCharAt(4), fullDate.getCharAt(5), fullDate.getCharAt(6), fullDate.getCharAt(7));
	return m_emailDateMap[email];
}

AsciiString GameSpyLoginPreferences::getPasswordForEmail( AsciiString email )
{
	if ( m_emailPasswordMap.find(email) == m_emailPasswordMap.end() )
		return AsciiString::TheEmptyString;
	return m_emailPasswordMap[email];
}

AsciiStringList GameSpyLoginPreferences::getNicksForEmail( AsciiString email )
{
	if ( m_emailNickMap.find(email) == m_emailNickMap.end() )
	{
		AsciiStringList empty;
		return empty;
	}
	return m_emailNickMap[email];
}

void GameSpyLoginPreferences::addLogin( AsciiString email, AsciiString nick, AsciiString password, AsciiString date  )
{
	if ( std::find(m_emailNickMap[email].begin(), m_emailNickMap[email].end(), nick) == m_emailNickMap[email].end() )
		m_emailNickMap[email].push_back(nick);
	m_emailPasswordMap[email] = password;
	m_emailDateMap[email] = date;
}

void GameSpyLoginPreferences::forgetLogin( AsciiString email )
{
	m_emailNickMap.erase(email);
	m_emailPasswordMap.erase(email);
	m_emailDateMap.erase(email);

}

AsciiStringList GameSpyLoginPreferences::getEmails( void )
{
	AsciiStringList theList;
	NickMap::iterator it = m_emailNickMap.begin();
	while (it != m_emailNickMap.end())
	{
		theList.push_back(it->first);
		++it;
	}
	return theList;
}

static const char *PREF_FILENAME = "GameSpyLogin.ini";
static GameSpyLoginPreferences *loginPref = NULL;

static void startPings( void )
{
	std::list<AsciiString> pingServers = TheGameSpyConfig->getPingServers();
	Int timeout = TheGameSpyConfig->getPingTimeoutInMs();
	Int reps = TheGameSpyConfig->getNumPingRepetitions();

	for (std::list<AsciiString>::const_iterator it = pingServers.begin(); it != pingServers.end(); ++it)
	{
		AsciiString pingServer = *it;
		PingRequest req;
		req.hostname = pingServer.str();
		req.repetitions = reps;
		req.timeout = timeout;
		ThePinger->addRequest(req);
	}
}

//-------------------------------------------------------------------------------------------------
/** This is called when a shutdown is complete for this menu */
//-------------------------------------------------------------------------------------------------
static void shutdownComplete( WindowLayout *layout )
{

	isShuttingDown = false;

	// hide the layout
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout, (nextScreen != NULL) );

	if (nextScreen != NULL)
	{
		if (loginPref)
		{
			loginPref->write();
			delete loginPref;
			loginPref = NULL;
		}
		TheShell->push(nextScreen);
	}
	else
	{
		DEBUG_ASSERTCRASH(loginPref != NULL, ("loginPref == NULL"));
		if (loginPref)
		{
			loginPref->write();
			delete loginPref;
			loginPref = NULL;
		}
	}

	nextScreen = NULL;

}  // end if


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
// window ids ------------------------------------------------------------------------------
static NameKeyType parentWOLLoginID =						NAMEKEY_INVALID;
static NameKeyType buttonBackID =								NAMEKEY_INVALID;	// profile, quick
static NameKeyType buttonLoginID =							NAMEKEY_INVALID;	// profile, quick
static NameKeyType buttonCreateAccountID =			NAMEKEY_INVALID;	// profile, quick
static NameKeyType buttonUseAccountID =					NAMEKEY_INVALID;	// quick
static NameKeyType buttonDontUseAccountID =			NAMEKEY_INVALID;	// profile
static NameKeyType buttonTOSID =								NAMEKEY_INVALID;	// TOS
static NameKeyType parentTOSID =								NAMEKEY_INVALID;	// TOS Parent
static NameKeyType buttonTOSOKID =							NAMEKEY_INVALID;	// TOS
static NameKeyType listboxTOSID =								NAMEKEY_INVALID;	// TOS
static NameKeyType comboBoxEmailID =						NAMEKEY_INVALID;	// profile
static NameKeyType comboBoxLoginNameID =				NAMEKEY_INVALID;	// profile
static NameKeyType textEntryLoginNameID =				NAMEKEY_INVALID;	// quick
static NameKeyType textEntryPasswordID =				NAMEKEY_INVALID;	// profile
static NameKeyType checkBoxRememberPasswordID =	NAMEKEY_INVALID;	// checkbox to remember information or not
static NameKeyType textEntryMonthID =				NAMEKEY_INVALID;	// profile
static NameKeyType textEntryDayID =				NAMEKEY_INVALID;	// profile
static NameKeyType textEntryYearID =				NAMEKEY_INVALID;	// profile

// Window Pointers ------------------------------------------------------------------------
static GameWindow *parentWOLLogin =						NULL;
static GameWindow *buttonBack =								NULL;
static GameWindow *buttonLogin =							NULL;
static GameWindow *buttonCreateAccount =			NULL;
static GameWindow *buttonUseAccount =					NULL;
static GameWindow *buttonDontUseAccount =			NULL;
static GameWindow *buttonTOS						=			NULL;
static GameWindow *parentTOS						=			NULL;
static GameWindow *buttonTOSOK					=			NULL;
static GameWindow *listboxTOS						=			NULL;
static GameWindow *comboBoxEmail =						NULL;
static GameWindow *comboBoxLoginName =				NULL;
static GameWindow *textEntryLoginName =				NULL;
static GameWindow *textEntryPassword =				NULL;
static GameWindow *checkBoxRememberPassword =	NULL;
static GameWindow *textEntryMonth =				NULL;
static GameWindow *textEntryDay =				NULL;
static GameWindow *textEntryYear =				NULL;

void EnableLoginControls( Bool state )
{
	if (buttonLogin)
		buttonLogin->winEnable(state);
	if (buttonCreateAccount)
		buttonCreateAccount->winEnable(state);
	if (buttonUseAccount)
		buttonUseAccount->winEnable(state);
	if (buttonDontUseAccount)
		buttonDontUseAccount->winEnable(state);
	if (comboBoxEmail)
		comboBoxEmail->winEnable(state);
	if (comboBoxLoginName)
		comboBoxLoginName->winEnable(state);
	if (textEntryLoginName)
		textEntryLoginName->winEnable(state);
	if (textEntryPassword)
		textEntryPassword->winEnable(state);
	if (checkBoxRememberPassword)
		checkBoxRememberPassword->winEnable(state);
	if( buttonTOS )
		buttonTOS->winEnable(state);

	if (textEntryMonth)
		textEntryMonth->winEnable(state);
	if (textEntryDay)
		textEntryDay->winEnable(state);
	if( textEntryYear )
		textEntryYear->winEnable(state);
}

//-------------------------------------------------------------------------------------------------
/** Initialize the WOL Login Menu */
//-------------------------------------------------------------------------------------------------
void WOLLoginMenuInit( WindowLayout *layout, void *userData )
{
	nextScreen = NULL;
	buttonPushed = false;
	isShuttingDown = false;
	loginAttemptTime = 0;

	if (!loginPref)
	{
		loginPref = NEW GameSpyLoginPreferences;
		loginPref->load(PREF_FILENAME);
	}
	
	// if the ESRB warning is blank (other country) hide the box
	GameWindow *esrbTitle = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("GameSpyLoginProfile.wnd:StaticTextESRBTop") );
	GameWindow *esrbParent = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("GameSpyLoginProfile.wnd:ParentESRB") );
	if (esrbTitle && esrbParent)
	{
		if ( GadgetStaticTextGetText( esrbTitle ).getLength() < 2 )
		{
			esrbParent->winHide(TRUE);
		}
	}

	parentWOLLoginID =						TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:WOLLoginMenuParent" );
	buttonBackID =								TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ButtonBack" );
	buttonLoginID =								TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ButtonLogin" );
	buttonCreateAccountID =				TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ButtonCreateAccount" );
	buttonUseAccountID =					TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ButtonUseAccount" );
	buttonDontUseAccountID =			TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ButtonDontUseAccount" );
	buttonTOSID							=			TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ButtonTOS" );
	parentTOSID							=			TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ParentTOS" );
	buttonTOSOKID						=			TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ButtonTOSOK" );
	listboxTOSID						=			TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ListboxTOS" );
	comboBoxEmailID =							TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ComboBoxEmail" );
	comboBoxLoginNameID =					TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:ComboBoxLoginName" );
	textEntryLoginNameID =				TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:TextEntryLoginName" );
	textEntryPasswordID =					TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:TextEntryPassword" );
	checkBoxRememberPasswordID =	TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:CheckBoxRememberInfo" );
	textEntryMonthID =					TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:TextEntryMonth" );
	textEntryDayID =					TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:TextEntryDay" );
	textEntryYearID =					TheNameKeyGenerator->nameToKey( "GameSpyLoginProfile.wnd:TextEntryYear" );

	parentWOLLogin =							TheWindowManager->winGetWindowFromId( NULL,  parentWOLLoginID );
	buttonBack =									TheWindowManager->winGetWindowFromId( NULL,  buttonBackID);
	buttonLogin =									TheWindowManager->winGetWindowFromId( NULL,  buttonLoginID);
	buttonCreateAccount =					TheWindowManager->winGetWindowFromId( NULL,  buttonCreateAccountID);
	buttonUseAccount =						TheWindowManager->winGetWindowFromId( NULL,  buttonUseAccountID);
	buttonDontUseAccount =				TheWindowManager->winGetWindowFromId( NULL,  buttonDontUseAccountID);
	buttonTOS =										TheWindowManager->winGetWindowFromId( NULL,  buttonTOSID);
	parentTOS =										TheWindowManager->winGetWindowFromId( NULL,  parentTOSID);
	buttonTOSOK =									TheWindowManager->winGetWindowFromId( NULL,  buttonTOSOKID);
	listboxTOS =									TheWindowManager->winGetWindowFromId( NULL,  listboxTOSID);
	comboBoxEmail =								TheWindowManager->winGetWindowFromId( NULL,  comboBoxEmailID);
	comboBoxLoginName =						TheWindowManager->winGetWindowFromId( NULL,  comboBoxLoginNameID);
	textEntryLoginName =					TheWindowManager->winGetWindowFromId( NULL,  textEntryLoginNameID);
	textEntryPassword =						TheWindowManager->winGetWindowFromId( NULL,  textEntryPasswordID);
	checkBoxRememberPassword =		TheWindowManager->winGetWindowFromId( NULL,  checkBoxRememberPasswordID);
	textEntryMonth =					TheWindowManager->winGetWindowFromId( NULL,  textEntryMonthID);
	textEntryDay =					TheWindowManager->winGetWindowFromId( NULL,  textEntryDayID);
	textEntryYear =					TheWindowManager->winGetWindowFromId( NULL,  textEntryYearID);

	GadgetTextEntrySetText(textEntryMonth, UnicodeString::TheEmptyString);
	
	GadgetTextEntrySetText(textEntryDay, UnicodeString::TheEmptyString);
	
	GadgetTextEntrySetText(textEntryYear, UnicodeString::TheEmptyString);



	GameWindowList tabList;
	tabList.push_front(comboBoxEmail);
	tabList.push_back(comboBoxLoginName);
	tabList.push_back(textEntryPassword);
	tabList.push_back(textEntryMonth);
	tabList.push_back(textEntryDay);
	tabList.push_back(textEntryYear);
	tabList.push_back(checkBoxRememberPassword);
	tabList.push_back(buttonLogin);
	tabList.push_back(buttonCreateAccount);
	tabList.push_back(buttonTOS);
	tabList.push_back(buttonBack);
	TheWindowManager->clearTabList();
	TheWindowManager->registerTabList(tabList);
	TheWindowManager->winSetFocus( comboBoxEmail );
	// short form or long form?
#ifdef ALLOW_NON_PROFILED_LOGIN
	if (parentWOLLogin)
	{
		GameSpyUseProfiles = true;
#endif // ALLOW_NON_PROFILED_LOGIN

		DEBUG_ASSERTCRASH(buttonBack,						("buttonBack missing!"));
		DEBUG_ASSERTCRASH(buttonLogin,					("buttonLogin missing!"));
		DEBUG_ASSERTCRASH(buttonCreateAccount,	("buttonCreateAccount missing!"));
		//DEBUG_ASSERTCRASH(buttonDontUseAccount,	("buttonDontUseAccount missing!"));
		DEBUG_ASSERTCRASH(comboBoxEmail,				("comboBoxEmail missing!"));
		DEBUG_ASSERTCRASH(comboBoxLoginName,		("comboBoxLoginName missing!"));
		DEBUG_ASSERTCRASH(textEntryPassword,		("textEntryPassword missing!"));

		//TheShell->registerWithAnimateManager(parentWOLLogin, WIN_ANIMATION_SLIDE_TOP, TRUE);
		/**/
//		TheShell->registerWithAnimateManager(buttonTOS, WIN_ANIMATION_SLIDE_BOTTOM, TRUE);
		//TheShell->registerWithAnimateManager(buttonCreateAccount, WIN_ANIMATION_SLIDE_LEFT, TRUE);
		//TheShell->registerWithAnimateManager(buttonDontUseAccount, WIN_ANIMATION_SLIDE_LEFT, TRUE);
//		TheShell->registerWithAnimateManager(buttonBack, WIN_ANIMATION_SLIDE_BOTTOM, TRUE);
		/**/
#ifdef ALLOW_NON_PROFILED_LOGIN
	}
	else
	{
		GameSpyUseProfiles = false;

		parentWOLLoginID =						TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:WOLLoginMenuParent" );
		buttonBackID =								TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ButtonBack" );
		buttonLoginID =								TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ButtonLogin" );
		buttonCreateAccountID =				TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ButtonCreateAccount" );
		buttonUseAccountID =					TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ButtonUseAccount" );
		buttonDontUseAccountID =			TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ButtonDontUseAccount" );
		buttonTOSID							=			TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ButtonTOS" );
		parentTOSID							=			TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ParentTOS" );
		buttonTOSOKID						=			TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ButtonTOSOK" );
		listboxTOSID						=			TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ListboxTOS" );
		comboBoxEmailID =							TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:ComboBoxEmail" );
		textEntryLoginNameID =				TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:TextEntryLoginName" );
		textEntryPasswordID =					TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:TextEntryPassword" );
		checkBoxRememberPasswordID =	TheNameKeyGenerator->nameToKey( "GameSpyLoginQuick.wnd:CheckBoxRememberPassword" );

		parentWOLLogin =							TheWindowManager->winGetWindowFromId( NULL,  parentWOLLoginID );
		buttonBack =									TheWindowManager->winGetWindowFromId( NULL,  buttonBackID);
		buttonLogin =									TheWindowManager->winGetWindowFromId( NULL,  buttonLoginID);
		buttonCreateAccount =					TheWindowManager->winGetWindowFromId( NULL,  buttonCreateAccountID);
		buttonUseAccount =						TheWindowManager->winGetWindowFromId( NULL,  buttonUseAccountID);
		buttonDontUseAccount =				TheWindowManager->winGetWindowFromId( NULL,  buttonDontUseAccountID);
		comboBoxEmail =								TheWindowManager->winGetWindowFromId( NULL,  comboBoxEmailID);
		buttonTOS =										TheWindowManager->winGetWindowFromId( NULL,  buttonTOSID);
		parentTOS =										TheWindowManager->winGetWindowFromId( NULL,  parentTOSID);
		buttonTOSOK =									TheWindowManager->winGetWindowFromId( NULL,  buttonTOSOKID);
		listboxTOS =									TheWindowManager->winGetWindowFromId( NULL,  listboxTOSID);
		textEntryLoginName =					TheWindowManager->winGetWindowFromId( NULL,  textEntryLoginNameID);
		textEntryPassword =						TheWindowManager->winGetWindowFromId( NULL,  textEntryPasswordID);
		checkBoxRememberPassword =		TheWindowManager->winGetWindowFromId( NULL,  checkBoxRememberPasswordID);

		DEBUG_ASSERTCRASH(buttonBack,						("buttonBack missing!"));
		DEBUG_ASSERTCRASH(buttonLogin,					("buttonLogin missing!"));
		DEBUG_ASSERTCRASH(buttonCreateAccount,	("buttonCreateAccount missing!"));
		DEBUG_ASSERTCRASH(buttonUseAccount,			("buttonUseAccount missing!"));
		DEBUG_ASSERTCRASH(textEntryLoginName,		("textEntryLoginName missing!"));
		TheWindowManager->winSetFocus( textEntryLoginName );
		//TheShell->registerWithAnimateManager(parentWOLLogin, WIN_ANIMATION_SLIDE_TOP, TRUE);
		
//		TheShell->registerWithAnimateManager(buttonTOS, WIN_ANIMATION_SLIDE_LEFT, TRUE);
//		TheShell->registerWithAnimateManager(buttonCreateAccount, WIN_ANIMATION_SLIDE_LEFT, TRUE);
//		TheShell->registerWithAnimateManager(buttonUseAccount, WIN_ANIMATION_SLIDE_LEFT, TRUE);
//		TheShell->registerWithAnimateManager(buttonBack, WIN_ANIMATION_SLIDE_RIGHT, TRUE);
		
	}
#endif // ALLOW_NON_PROFILED_LOGIN


#ifdef ALLOW_NON_PROFILED_LOGIN
	if (GameSpyUseProfiles)
	{
#endif // ALLOW_NON_PROFILED_LOGIN
		// Read login names from registry...
		GadgetComboBoxReset(comboBoxEmail);
		GadgetTextEntrySetText(textEntryPassword, UnicodeString::TheEmptyString);

		// look for cached nicks to add
		AsciiString lastName;
		AsciiString lastEmail;
		Bool markCheckBox = FALSE;
		UserPreferences::const_iterator it = loginPref->find("lastName");
		if (it != loginPref->end())
		{
			lastName = it->second;
		}
		it = loginPref->find("lastEmail");
		if (it != loginPref->end())
		{
			lastEmail = it->second;
		}

		// fill in list of Emails, and select the most recent
		AsciiStringList cachedEmails = loginPref->getEmails();
		AsciiStringListIterator eIt = cachedEmails.begin();
		Int selectedPos = -1;
		while (eIt != cachedEmails.end())
		{
			UnicodeString uniEmail;
			uniEmail.translate(*eIt);
			Int pos = GadgetComboBoxAddEntry(comboBoxEmail, uniEmail, GameSpyColor[GSCOLOR_DEFAULT]);
			if (*eIt == lastEmail)
				selectedPos = pos;

			++eIt;
		}
		if (selectedPos >= 0)
		{
			GadgetComboBoxSetSelectedPos(comboBoxEmail, selectedPos);

			// fill in the password for the selected email
			UnicodeString pass;
			pass.translate(loginPref->getPasswordForEmail(lastEmail));
			GadgetTextEntrySetText(textEntryPassword, pass);
			
			AsciiString month,day,year;
			loginPref->getDateForEmail(lastEmail, month, day, year);
			pass.translate(month);
			GadgetTextEntrySetText(textEntryMonth, pass);
			pass.translate(day);
			GadgetTextEntrySetText(textEntryDay, pass);
			pass.translate(year);
			GadgetTextEntrySetText(textEntryYear, pass);

			markCheckBox = TRUE;
		}

		// fill in list of nicks for selected email, selecting the most recent
		GadgetComboBoxReset(comboBoxLoginName);
		AsciiStringList cachedNicks = loginPref->getNicksForEmail(lastEmail);
		AsciiStringListIterator nIt = cachedNicks.begin();
		selectedPos = -1;
		while (nIt != cachedNicks.end())
		{
			UnicodeString uniNick;
			uniNick.translate(*nIt);
			Int pos = GadgetComboBoxAddEntry(comboBoxLoginName, uniNick, GameSpyColor[GSCOLOR_DEFAULT]);
			if (*nIt == lastName)
				selectedPos = pos;

			++nIt;
		}
		if (selectedPos >= 0)
		{
			GadgetComboBoxSetSelectedPos(comboBoxLoginName, selectedPos);
			markCheckBox = TRUE;
		}
		// always start with not storing information
		if( markCheckBox)
			GadgetCheckBoxSetChecked(checkBoxRememberPassword, TRUE);
		else
			GadgetCheckBoxSetChecked(checkBoxRememberPassword, FALSE);
#ifdef ALLOW_NON_PROFILED_LOGIN
	}
	else
	{
		// Read login names from registry...
		GadgetComboBoxReset(comboBoxLoginName);
		UnicodeString nick;

		UserPreferences::const_iterator it = loginPref->find("lastName");
		if (it != loginPref->end())
		{
			nick.translate(it->second);
		}
		else
		{
			char userBuf[32] = "";
			unsigned long bufSize = 32;
			GetUserName(userBuf, &bufSize);
			nick.translate(userBuf);
		}

		GadgetTextEntrySetText(textEntryLoginName, nick);
	}
#endif // ALLOW_NON_PROFILED_LOGIN

	EnableLoginControls(TRUE);

	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	
	RaiseGSMessageBox();

	OptionPreferences optionPref;
	if (!optionPref.getBool("SawTOS", TRUE))
	{
		TheWindowManager->winSendSystemMsg( parentWOLLogin, GBM_SELECTED, 
																			(WindowMsgData)buttonTOS, buttonTOSID );
	}
	TheTransitionHandler->setGroup("GameSpyLoginProfileFade");

} // WOLLoginMenuInit

//-------------------------------------------------------------------------------------------------
/** WOL Login Menu shutdown method */
//-------------------------------------------------------------------------------------------------
static Bool loggedInOK = false;
void WOLLoginMenuShutdown( WindowLayout *layout, void *userData )
{
	isShuttingDown = true;
	loggedInOK = false;
	TheWindowManager->clearTabList();
	if (webBrowserActive)
	{
		if (TheWebBrowser != NULL)
		{
			TheWebBrowser->closeBrowserWindow(listboxTOS);
		}
		webBrowserActive = FALSE;
	}

	// if we are shutting down for an immediate pop, skip the animations
	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{

		shutdownComplete( layout );
		return;

	}  //end if

	TheShell->reverseAnimatewindow();
	TheTransitionHandler->reverse("GameSpyLoginProfileFade");

}  // WOLLoginMenuShutdown


// this is used to check if we've got all the pings
static void checkLogin( void )
{
	if (loggedInOK && ThePinger && !ThePinger->arePingsInProgress())
	{
		// save off our ping string, and end those threads
		AsciiString pingStr = ThePinger->getPingString( 1000 );
		DEBUG_LOG(("Ping string is %s\n", pingStr.str()));
		TheGameSpyInfo->setPingString(pingStr);
		//delete ThePinger;
		//ThePinger = NULL;

		buttonPushed = true;
		loggedInOK = false; // don't try this again

		loginAttemptTime = 0;

		// start looking for group rooms
		TheGameSpyInfo->clearGroupRoomList();

		SignalUIInteraction(SHELL_SCRIPT_HOOK_GENERALS_ONLINE_LOGIN);
		nextScreen = "Menus/WOLWelcomeMenu.wnd";
		TheShell->pop();
		
		// read in some cached data
		GameSpyMiscPreferences mPref;
		PSPlayerStats localPSStats = GameSpyPSMessageQueueInterface::parsePlayerKVPairs(mPref.getCachedStats().str());
		localPSStats.id = TheGameSpyInfo->getLocalProfileID();
		TheGameSpyInfo->setCachedLocalPlayerStats(localPSStats);
//		TheGameSpyPSMessageQueue->trackPlayerStats(localPSStats);

		// and push the info around to other players
//		PSResponse newResp;
//		newResp.responseType = PSResponse::PSRESPONSE_PLAYERSTATS;
//		newResp.player = localPSStats;
//		TheGameSpyPSMessageQueue->addResponse(newResp);
	}
}

//-------------------------------------------------------------------------------------------------
/** WOL Login Menu update method */
//-------------------------------------------------------------------------------------------------
void WOLLoginMenuUpdate( WindowLayout * layout, void *userData)
{

	// We'll only be successful if we've requested to 
	if(isShuttingDown && TheShell->isAnimFinished() && TheTransitionHandler->isFinished())
		shutdownComplete(layout);

	if (TheShell->isAnimFinished() && !buttonPushed && TheGameSpyPeerMessageQueue)
	{
		PingResponse pingResp;
		if (ThePinger && ThePinger->getResponse(pingResp))
		{
			checkLogin();
		}

		PeerResponse resp;
		if (!loggedInOK && TheGameSpyPeerMessageQueue->getResponse( resp ))
		{
			switch (resp.peerResponseType)
			{
			case PeerResponse::PEERRESPONSE_GROUPROOM:
				{
					GameSpyGroupRoom room;
					room.m_groupID = resp.groupRoom.id;
					room.m_maxWaiting = resp.groupRoom.maxWaiting;
					room.m_name = resp.groupRoomName.c_str();
					room.m_translatedName = UnicodeString(L"TEST");
					room.m_numGames = resp.groupRoom.numGames;
					room.m_numPlaying = resp.groupRoom.numPlaying;
					room.m_numWaiting = resp.groupRoom.numWaiting;
					TheGameSpyInfo->addGroupRoom( room );
				}
				break;
			case PeerResponse::PEERRESPONSE_LOGIN:
				{
					loggedInOK = true;

					// fetch our player info
					TheGameSpyInfo->setLocalName( resp.nick.c_str() );
					TheGameSpyInfo->setLocalProfileID( resp.player.profileID );
					TheGameSpyInfo->loadSavedIgnoreList();
					TheGameSpyInfo->setLocalIPs(resp.player.internalIP, resp.player.externalIP);
					TheGameSpyInfo->readAdditionalDisconnects();
					//TheGameSpyInfo->setLocalEmail( resp.player.email );
					//TheGameSpyInfo->setLocalPassword( resp)

					GameSpyMiscPreferences miscPref;
					TheGameSpyInfo->setMaxMessagesPerUpdate(miscPref.getMaxMessagesPerUpdate());
				}
				break;
			case PeerResponse::PEERRESPONSE_DISCONNECT:
				{
					loginAttemptTime = 0;
					UnicodeString title, body;
					AsciiString disconMunkee;
					disconMunkee.format("GUI:GSDisconReason%d", resp.discon.reason);
					title = TheGameText->fetch( "GUI:GSErrorTitle" );
					body = TheGameText->fetch( disconMunkee );
					GSMessageBoxOk( title, body );
					EnableLoginControls( TRUE );

					// kill & restart the threads
					AsciiString motd = TheGameSpyInfo->getMOTD();
					AsciiString config = TheGameSpyInfo->getConfig();
					DEBUG_LOG(("Tearing down GameSpy from WOLLoginMenuUpdate(PEERRESPONSE_DISCONNECT)\n"));
					TearDownGameSpy();
					SetUpGameSpy( motd.str(), config.str() );
				}
				break;
			}
		}

		checkLogin();
	}

	if (TheGameSpyInfo && !buttonPushed && loginAttemptTime && (loginAttemptTime + loginTimeoutInMS < timeGetTime()))
	{
		// timed out a login attempt, so say so
		loginAttemptTime = 0;
		UnicodeString title, body;
		AsciiString disconMunkee;
		disconMunkee.format("GUI:GSDisconReason4");	// ("could not connect to server")
		title = TheGameText->fetch( "GUI:GSErrorTitle" );
		body = TheGameText->fetch( disconMunkee );
		GSMessageBoxOk( title, body );
		EnableLoginControls( TRUE );

		// kill & restart the threads
		AsciiString motd = TheGameSpyInfo->getMOTD();
		AsciiString config = TheGameSpyInfo->getConfig();
		DEBUG_LOG(("Tearing down GameSpy from WOLLoginMenuUpdate(login timeout)\n"));
		TearDownGameSpy();
		SetUpGameSpy( motd.str(), config.str() );
	}

}// WOLLoginMenuUpdate

//-------------------------------------------------------------------------------------------------
/** WOL Login Menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLLoginMenuInput( GameWindow *window, UnsignedInt msg,
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg ) 
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;
			if (buttonPushed)
				break;

			switch( key )
			{

				// ----------------------------------------------------------------------------------------
				case KEY_ESC:
				{
					
					//
					// send a simulated selected event to the parent window of the
					// back/exit button
					//
					if( BitIsSet( state, KEY_STATE_UP ) )
					{
						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED, 
																							(WindowMsgData)buttonBack, buttonBackID );

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;
}// WOLLoginMenuInput

static Bool isNickOkay(UnicodeString nick)
{
	static const WideChar * legalIRCChars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]`_^{|}-";

	Int len = nick.getLength();
	if (len == 0)
		return TRUE;

	if (len == 1 && nick.getCharAt(0) == L'-')
		return FALSE;

	WideChar newChar = nick.getCharAt(len-1);
	if (wcschr(legalIRCChars, newChar) == NULL)
		return FALSE;

	return TRUE;
}

static Bool isAgeOkay(AsciiString &month, AsciiString &day, AsciiString year)
{
	if(month.isEmpty() || day.isEmpty() || year.isEmpty() || year.getLength() != 4)
		return FALSE;

	Int monthInt = atoi(month.str());
	Int dayInt = atoi(day.str());

	if(monthInt > 12 || dayInt > 31)
		return FALSE;
		// setup date buffer for local region date format
	month.format("%02.2d",monthInt);
	day.format("%02.2d",dayInt);

	// test the year first
	#define DATE_BUFFER_SIZE 256
	char dateBuffer[ DATE_BUFFER_SIZE ];
	GetDateFormat( LOCALE_SYSTEM_DEFAULT,
								 0, NULL,
								 "yyyy",
								 dateBuffer, DATE_BUFFER_SIZE );
	Int sysVal = atoi(dateBuffer);
	Int userVal = atoi(year.str());
	if(sysVal - userVal >= 14)
		return TRUE;
	else if( sysVal - userVal <= 12)
		return FALSE;

	GetDateFormat( LOCALE_SYSTEM_DEFAULT,
								 0, NULL,
								 "MM",
								 dateBuffer, DATE_BUFFER_SIZE );
	sysVal = atoi(dateBuffer);
	userVal = atoi(month.str());
	if(sysVal - userVal >0 )
		return TRUE;
	else if( sysVal -userVal < 0 )
		return FALSE;
//	month.format("%02.2d",userVal);
	GetDateFormat( LOCALE_SYSTEM_DEFAULT,
								 0, NULL,
								 "dd",
								 dateBuffer, DATE_BUFFER_SIZE );
	sysVal = atoi(dateBuffer);
	userVal = atoi(day.str());
	if(sysVal - userVal< 0)
		return FALSE;
//	day.format("%02.2d",userVal);
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
/** WOL Login Menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLLoginMenuSystem( GameWindow *window, UnsignedInt msg, 
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	UnicodeString txtInput;

	switch( msg )
	{
		
		
		case GWM_CREATE:
			{
				break;
			} // case GWM_DESTROY:

		case GWM_DESTROY:
			{
				break;
			} // case GWM_DESTROY:

		case GWM_INPUT_FOCUS:
			{	
				// if we're givin the opportunity to take the keyboard focus we must say we want it
				if( mData1 == TRUE )
					*(Bool *)mData2 = TRUE;

				return MSG_HANDLED;
			}//case GWM_INPUT_FOCUS:

		// someone typed in a combo box.  Clear password (or fill it in if the typed name matches a known login name)
		case GCM_UPDATE_TEXT:
			{
				UnicodeString uNick = GadgetComboBoxGetText(comboBoxLoginName);
				UnicodeString uEmail = GadgetComboBoxGetText(comboBoxEmail);
				AsciiString nick, email;
				nick.translate(uNick);
				email.translate(uEmail);
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				UnicodeString trimmedNick = uNick, trimmedEmail = uEmail;
				trimmedNick.trim();
				trimmedEmail.trim();
				if (!trimmedNick.isEmpty())
				{
					if (trimmedNick.getCharAt(trimmedNick.getLength()-1) == L'\\')
						trimmedNick.removeLastChar();
					if (trimmedNick.getCharAt(trimmedNick.getLength()-1) == L'/')
						trimmedNick.removeLastChar();
				}
				if (!trimmedEmail.isEmpty())
				{
					if (trimmedEmail.getCharAt(trimmedEmail.getLength()-1) == L'\\')
						trimmedEmail.removeLastChar();
					if (trimmedEmail.getCharAt(trimmedEmail.getLength()-1) == L'/')
						trimmedEmail.removeLastChar();
				}
				if (trimmedEmail.getLength() != uEmail.getLength())
				{
					// we just trimmed a space.  set the text back and bail
					GadgetComboBoxSetText(comboBoxEmail, trimmedEmail);
					break;
				}
				if (trimmedNick.getLength() != nick.getLength())
				{
					// we just trimmed a space.  set the text back and bail
					GadgetComboBoxSetText(comboBoxLoginName, trimmedNick);
					break;
				}

				if (controlID == comboBoxEmailID)
				{
					// email changed.  look up password, and choose new login names

					// fill in the password for the selected email
					UnicodeString pass;
					pass.translate(loginPref->getPasswordForEmail(email));
					GadgetTextEntrySetText(textEntryPassword, pass);

					// fill in list of nicks for selected email, selecting the first
					AsciiStringList cachedNicks = loginPref->getNicksForEmail(email);
					AsciiStringListIterator nIt = cachedNicks.begin();
					Int selectedPos = -1;
					GadgetComboBoxReset(comboBoxLoginName);
					while (nIt != cachedNicks.end())
					{
						UnicodeString uniNick;
						uniNick.translate(*nIt);
						GadgetComboBoxAddEntry(comboBoxLoginName, uniNick, GameSpyColor[GSCOLOR_DEFAULT]);
						selectedPos = 0;
						++nIt;
					}
					if (selectedPos >= 0)
					{
						GadgetComboBoxSetSelectedPos(comboBoxLoginName, selectedPos);
						GadgetCheckBoxSetChecked(checkBoxRememberPassword, true);
						AsciiString month,day,year;
						loginPref->getDateForEmail(email, month, day, year);
						pass.translate(month);
						GadgetTextEntrySetText(textEntryMonth, pass);
						pass.translate(day);
						GadgetTextEntrySetText(textEntryDay, pass);
						pass.translate(year);
						GadgetTextEntrySetText(textEntryYear, pass);

					}
					else
					{
						GadgetCheckBoxSetChecked(checkBoxRememberPassword, false);						
						GadgetTextEntrySetText(textEntryMonth, UnicodeString::TheEmptyString);
						GadgetTextEntrySetText(textEntryDay, UnicodeString::TheEmptyString);
						GadgetTextEntrySetText(textEntryYear, UnicodeString::TheEmptyString);

					}
				}
				else if (controlID == comboBoxLoginNameID)
				{
					// they typed a new login name.  Email & pass shouldn't change
				}

				break;
			}

		case GCM_SELECTED:
			{
				if (buttonPushed)
					break;
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if (controlID == comboBoxEmailID)
				{
					// email changed.  look up password, and choose new login names
					UnicodeString uEmail = GadgetComboBoxGetText(comboBoxEmail);
					AsciiString email;
					email.translate(uEmail);

					// fill in the password for the selected email
					UnicodeString pass;
					pass.translate(loginPref->getPasswordForEmail(email));
					GadgetTextEntrySetText(textEntryPassword, pass);

					// fill in list of nicks for selected email, selecting the first
					AsciiStringList cachedNicks = loginPref->getNicksForEmail(email);
					AsciiStringListIterator nIt = cachedNicks.begin();
					Int selectedPos = -1;
					GadgetComboBoxReset(comboBoxLoginName);
					while (nIt != cachedNicks.end())
					{
						UnicodeString uniNick;
						uniNick.translate(*nIt);
						GadgetComboBoxAddEntry(comboBoxLoginName, uniNick, GameSpyColor[GSCOLOR_DEFAULT]);
						selectedPos = 0;
						++nIt;
					}
					if (selectedPos >= 0)
					{
						GadgetComboBoxSetSelectedPos(comboBoxLoginName, selectedPos);
						GadgetCheckBoxSetChecked(checkBoxRememberPassword, true);
						AsciiString month,day,year;
						loginPref->getDateForEmail(email, month, day, year);
						pass.translate(month);
						GadgetTextEntrySetText(textEntryMonth, pass);
						pass.translate(day);
						GadgetTextEntrySetText(textEntryDay, pass);
						pass.translate(year);
						GadgetTextEntrySetText(textEntryYear, pass);

					}
					else
					{
						GadgetCheckBoxSetChecked(checkBoxRememberPassword, false);						
						GadgetTextEntrySetText(textEntryMonth, UnicodeString::TheEmptyString);
						GadgetTextEntrySetText(textEntryDay, UnicodeString::TheEmptyString);
						GadgetTextEntrySetText(textEntryYear, UnicodeString::TheEmptyString);
					}

				}
				else if (controlID == comboBoxLoginNameID)
				{
					// they typed a new login name.  Email & pass shouldn't change
				}
				break;
			}

		case GBM_SELECTED:
			{
				if (buttonPushed)
					break;
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				// If we back out, just bail - we haven't gotten far enough to need to log out
				if ( controlID == buttonBackID )
				{
					buttonPushed = true;
					TearDownGameSpy();
					TheShell->pop();
				} //if ( controlID == buttonBack )
#ifdef ALLOW_NON_PROFILED_LOGIN
				else if ( controlID == buttonUseAccountID )
				{
					buttonPushed = true;
					nextScreen = "Menus/GameSpyLoginProfile.wnd";
					TheShell->pop();
					//TheShell->push( "Menus/GameSpyLoginProfile.wnd" );
				} //if ( controlID == buttonUseAccount )
				else if ( controlID == buttonDontUseAccountID )
				{
					buttonPushed = true;
					nextScreen = "Menus/GameSpyLoginQuick.wnd";
					TheShell->pop();
					//TheShell->push( "Menus/GameSpyLoginQuick.wnd" );
				} //if ( controlID == buttonDontUseAccount )
#endif // ALLOW_NON_PROFILED_LOGIN
				else if ( controlID == buttonCreateAccountID )
				{
#ifdef ALLOW_NON_PROFILED_LOGIN
					if (GameSpyUseProfiles)
					{
#endif // ALLOW_NON_PROFILED_LOGIN
						// actually attempt to create an account based on info entered
						AsciiString month, day, year;
						month.translate( GadgetTextEntryGetText(textEntryMonth) );
						day.translate( GadgetTextEntryGetText(textEntryDay) );
						year.translate( GadgetTextEntryGetText(textEntryYear) );
						
						if(!isAgeOkay(month, day, year))
						{
							GSMessageBoxOk(TheGameText->fetch("GUI:AgeFailedTitle"), TheGameText->fetch("GUI:AgeFailed"));
							break;
						}
						
						AsciiString login, password, email;
						email.translate( GadgetComboBoxGetText(comboBoxEmail) );
						login.translate( GadgetComboBoxGetText(comboBoxLoginName) );
						password.translate( GadgetTextEntryGetText(textEntryPassword) );

						if ( !email.isEmpty() && !login.isEmpty() && !password.isEmpty() )
						{
							loginAttemptTime = timeGetTime();
							BuddyRequest req;
							req.buddyRequestType = BuddyRequest::BUDDYREQUEST_LOGINNEW;
							strcpy(req.arg.login.nick, login.str());
							strcpy(req.arg.login.email, email.str());
							strcpy(req.arg.login.password, password.str());
							req.arg.login.hasFirewall = TRUE;
							
							TheGameSpyInfo->setLocalBaseName( login );
							//TheGameSpyInfo->setLocalProfileID( resp.player.profileID );
							TheGameSpyInfo->setLocalEmail( email );
							TheGameSpyInfo->setLocalPassword( password );
							DEBUG_LOG(("before create: TheGameSpyInfo->stuff(%s/%s/%s)\n", TheGameSpyInfo->getLocalBaseName().str(), TheGameSpyInfo->getLocalEmail().str(), TheGameSpyInfo->getLocalPassword().str()));

							TheGameSpyBuddyMessageQueue->addRequest( req );
							if(checkBoxRememberPassword && GadgetCheckBoxIsChecked(checkBoxRememberPassword))
							{	
								(*loginPref)["lastName"] = login;
								(*loginPref)["lastEmail"] = email;
								(*loginPref)["useProfiles"] = "yes";
								AsciiString date;
								date = month;
								date.concat(day);
								date.concat(year);

								loginPref->addLogin(email, login, password, date);
							}

							EnableLoginControls( FALSE );

							// fire off some pings
							startPings();
						}
						else
						{
							// user didn't fill in all info.  prompt him.
							if(email.isEmpty() && login.isEmpty() && password.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSNoLoginInfoAll"));
							else if( email.isEmpty() && login.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSNoLoginInfoEmailNickname"));
							else if( email.isEmpty() && password.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSNoLoginInfoEmailPassword"));
							else if( login.isEmpty() && password.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSNoLoginInfoNicknamePassword"));
							else if( email.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSNoLoginInfoEmail"));
							else if( password.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSNoLoginInfoPassword"));
							else if( login.isEmpty() )
								GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSNoLoginInfoNickname"));
							else
								GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:GSNoLoginInfoAll"));
						}
#ifdef ALLOW_NON_PROFILED_LOGIN
					}
					else
					{
						// not the profile screen - switch to it
						buttonPushed = TRUE;
						nextScreen = "Menus/GameSpyLoginProfile.wnd";
						TheShell->pop();
					}
#endif // ALLOW_NON_PROFILED_LOGIN
				} //if ( controlID == buttonCreateAccount )
				else if ( controlID == buttonLoginID )
				{
					AsciiString login, password, email;

#ifdef ALLOW_NON_PROFILED_LOGIN
					if (GameSpyUseProfiles)
					{
#endif // ALLOW_NON_PROFILED_LOGIN
						AsciiString month, day, year;
						month.translate( GadgetTextEntryGetText(textEntryMonth) );
						day.translate( GadgetTextEntryGetText(textEntryDay) );
						year.translate( GadgetTextEntryGetText(textEntryYear) );
						
						if(!isAgeOkay(month, day, year))
						{
							GSMessageBoxOk(TheGameText->fetch("GUI:AgeFailedTitle"), TheGameText->fetch("GUI:AgeFailed"));
							break;
						}

						email.translate( GadgetComboBoxGetText(comboBoxEmail) );
						login.translate( GadgetComboBoxGetText(comboBoxLoginName) );
						password.translate( GadgetTextEntryGetText(textEntryPassword) );

						if ( !email.isEmpty() && !login.isEmpty() && !password.isEmpty() )
						{
							loginAttemptTime = timeGetTime();
							BuddyRequest req;
							req.buddyRequestType = BuddyRequest::BUDDYREQUEST_LOGIN;
							strcpy(req.arg.login.nick, login.str());
							strcpy(req.arg.login.email, email.str());
							strcpy(req.arg.login.password, password.str());
							req.arg.login.hasFirewall = true;
							
							TheGameSpyInfo->setLocalBaseName( login );
							//TheGameSpyInfo->setLocalProfileID( resp.player.profileID );
							TheGameSpyInfo->setLocalEmail( email );
							TheGameSpyInfo->setLocalPassword( password );
							DEBUG_LOG(("before login: TheGameSpyInfo->stuff(%s/%s/%s)\n", TheGameSpyInfo->getLocalBaseName().str(), TheGameSpyInfo->getLocalEmail().str(), TheGameSpyInfo->getLocalPassword().str()));

							TheGameSpyBuddyMessageQueue->addRequest( req );
							if(checkBoxRememberPassword && GadgetCheckBoxIsChecked(checkBoxRememberPassword))
							{	
								(*loginPref)["lastName"] = login;
								(*loginPref)["lastEmail"] = email;
								(*loginPref)["useProfiles"] = "yes";
								AsciiString date;
								date = month;
								date.concat(day);
								date.concat(year);

								loginPref->addLogin(email, login, password,date);
							}
							else
							{
								loginPref->forgetLogin(email);
							}
							EnableLoginControls( FALSE );

							// fire off some pings
							startPings();
						}
						else
						{
							// user didn't fill in all info.  prompt him.
							if(email.isEmpty() && login.isEmpty() && password.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSNoLoginInfoAll"));
							else if( email.isEmpty() && login.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSNoLoginInfoEmailNickname"));
							else if( email.isEmpty() && password.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSNoLoginInfoEmailPassword"));
							else if( login.isEmpty() && password.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSNoLoginInfoNicknamePassword"));
							else if( email.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSNoLoginInfoEmail"));
							else if( password.isEmpty())
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSNoLoginInfoPassword"));
							else if( login.isEmpty() )
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSNoLoginInfoNickname"));
							else
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSNoLoginInfoAll"));
						}
#ifdef ALLOW_NON_PROFILED_LOGIN
					}
					else
					{
						login.translate( GadgetTextEntryGetText(textEntryLoginName) );

						if ( !login.isEmpty() )
						{
							loginAttemptTime = timeGetTime();
							PeerRequest req;
							req.peerRequestType = PeerRequest::PEERREQUEST_LOGIN;
							req.nick = login.str();
							req.login.profileID = 0;
							TheGameSpyPeerMessageQueue->addRequest( req );

							(*loginPref)["lastName"] = login;
							loginPref->erase("lastEmail");
							(*loginPref)["useProfiles"] = "no";
							EnableLoginControls( FALSE );

							// fire off some pings
							startPings();
						}
					}
#endif // ALLOW_NON_PROFILED_LOGIN

				} //if ( controlID == buttonLogin )
				else if ( controlID == buttonTOSID )
				{
					parentTOS->winHide(FALSE);
					useWebBrowserForTOS = FALSE;//loginPref->getBool("UseTOSBrowser", TRUE);
					if (useWebBrowserForTOS && (TheWebBrowser != NULL))
					{
						TheWebBrowser->createBrowserWindow("TermsOfService", listboxTOS);
						webBrowserActive = TRUE;
					}
					else
					{
						// Okay, no web browser.  This means we're looking at a UTF-8 text file.
						GadgetListBoxReset(listboxTOS);
						AsciiString fileName;
						fileName.format("Data\\%s\\TOS.txt", GetRegistryLanguage().str());
						File *theFile = TheFileSystem->openFile(fileName.str(), File::READ);
						if (theFile)
						{
							Int size = theFile->size();

							char *fileBuf = new char[size];
							Color tosColor = GameMakeColor(255, 255, 255, 255);

							Int bytesRead = theFile->read(fileBuf, size);
							if (bytesRead == size && size > 2)
							{
								fileBuf[size-1] = 0; // just to be safe
								AsciiString asciiBuf = fileBuf+2;
								AsciiString asciiLine;
								while (asciiBuf.nextToken(&asciiLine, "\r\n"))
								{
									UnicodeString uniLine;
									uniLine = UnicodeString(MultiByteToWideCharSingleLine(asciiLine.str()).c_str());
									int len = uniLine.getLength();
									for (int index = len-1; index >= 0; index--)
									{
										if (iswspace(uniLine.getCharAt(index)))
										{
											uniLine.removeLastChar();
										}
										else
										{
											break;
										}
									}
									//uniLine.trim();
									DEBUG_LOG(("adding TOS line: [%ls]\n", uniLine.str()));
									GadgetListBoxAddEntryText(listboxTOS, uniLine, tosColor, -1);
								}

							}

							delete fileBuf;
							fileBuf = NULL;

							theFile->close();
							theFile = NULL;
						}
					}
					EnableLoginControls( FALSE );
					buttonBack->winEnable(FALSE);
					
				}
				else if ( controlID == buttonTOSOKID )
				{
					EnableLoginControls( TRUE );

					parentTOS->winHide(TRUE);
					if (useWebBrowserForTOS && (TheWebBrowser != NULL))
					{
						if (listboxTOS != NULL)
						{
							TheWebBrowser->closeBrowserWindow(listboxTOS);
						}
					}

					OptionPreferences optionPref;
					optionPref["SawTOS"] = "yes";
					optionPref.write();
					webBrowserActive = FALSE;
					buttonBack->winEnable(TRUE);
				}
				break;
			}// case GBM_SELECTED:
	
		case GEM_EDIT_DONE:
			{
				break;
			}
			/*
		case GEM_UPDATE_TEXT:
			{
				if (buttonPushed)
					break;
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if ( controlID == textEntryLoginNameID )
				{
					UnicodeString munkee = GadgetTextEntryGetText( textEntryLoginName );
					if ( !isNickOkay( munkee ) )
					{
						munkee.removeLastChar();
						GadgetTextEntrySetText( textEntryLoginName, munkee );
					}
				}// if ( controlID == textEntryLoginNameID )
				break;
			}//case GEM_UPDATE_TEXT:
			*/
		default:
			return MSG_IGNORED;

	}//Switch

	return MSG_HANDLED;
}// WOLLoginMenuSystem


