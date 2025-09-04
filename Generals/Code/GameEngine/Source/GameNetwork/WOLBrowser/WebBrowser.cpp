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

/******************************************************************************
*
* NAME
*     $Archive:  $
*
* DESCRIPTION
*
* PROGRAMMER
*     Bryan Cleveland
*     $Author:  $
*
* VERSION INFO
*     $Revision:  $
*     $Modtime:  $
*
******************************************************************************/

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

//#include "WinMain.h"
#include "GameNetwork/WOLBrowser/WebBrowser.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Display.h"


/**
	* OLEInitializer class - Init and shutdown OLE & COM as a global
	* object.  Scary, nasty stuff, COM.  /me shivers.
	*/
class OLEInitializer
{
public:
	OLEInitializer()
	{
		// Initialize this instance
		OleInitialize(NULL);
	 }
	~OLEInitializer()
	{
		OleUninitialize();
	}
};
OLEInitializer g_OLEInitializer;
CComModule _Module;

CComObject<WebBrowser> * TheWebBrowser = NULL;


/******************************************************************************
*
* NAME
*     WebBrowser::WebBrowser
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WebBrowser::WebBrowser() :
		mRefCount(1)
{
	DEBUG_LOG(("Instantiating embedded WebBrowser"));
	m_urlList = NULL;
}


/******************************************************************************
*
* NAME
*     WebBrowser::~WebBrowser
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WebBrowser::~WebBrowser()
{
	DEBUG_LOG(("Destructing embedded WebBrowser"));
	if (this == TheWebBrowser) {
		DEBUG_LOG(("WebBrowser::~WebBrowser - setting TheWebBrowser to NULL"));
		TheWebBrowser = NULL;
	}
	WebBrowserURL *url = m_urlList;
	while (url != NULL) {
		WebBrowserURL *temp = url;
		url = url->m_next;
		deleteInstance(temp);
		temp = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
/** The INI data fields for Webpage URL's */
//-------------------------------------------------------------------------------------------------
const FieldParse WebBrowserURL::m_URLFieldParseTable[] =
{

	{ "URL",										INI::parseAsciiString,							NULL, offsetof( WebBrowserURL, m_url ) },
	{ NULL,											NULL,																NULL, 0 },

};

WebBrowserURL::WebBrowserURL()
{
	m_next = NULL;
	m_tag.clear();
	m_url.clear();
}

WebBrowserURL::~WebBrowserURL()
{
}
/******************************************************************************
*
* NAME
*     WebBrowser::init
*
* DESCRIPTION
*     Perform post creation initialization.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WebBrowser::init()
{
	m_urlList = NULL;
	INI ini;
	ini.loadFileDirectory( AsciiString( "Data\\INI\\Webpages" ), INI_LOAD_OVERWRITE, NULL );
}

/******************************************************************************
*
* NAME
*     WebBrowser::reset
*
* DESCRIPTION
*     Perform post creation initialization.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WebBrowser::reset()
{
}

void WebBrowser::update( void )
{
}

WebBrowserURL * WebBrowser::findURL(AsciiString tag)
{
	WebBrowserURL *retval = m_urlList;

	while ((retval != NULL) && tag.compareNoCase(retval->m_tag.str()))
	{
		retval = retval->m_next;
	}

	return retval;
}

WebBrowserURL * WebBrowser::makeNewURL(AsciiString tag)
{
	WebBrowserURL *newURL = newInstance(WebBrowserURL);

	newURL->m_tag = tag;

	newURL->m_next = m_urlList;
	m_urlList = newURL;

	return newURL;
}

/******************************************************************************
*
* NAME
*     IUnknown::QueryInterface
*
* DESCRIPTION
*
* INPUTS
*     IID - Interface ID
*
* RESULT
*
******************************************************************************/

STDMETHODIMP WebBrowser::QueryInterface(REFIID iid, void** ppv) IUNKNOWN_NOEXCEPT
{
	*ppv = NULL;

	if ((iid == IID_IUnknown) || (iid == IID_IBrowserDispatch))
	{
		*ppv = static_cast<IBrowserDispatch*>(this);
	}
	else
	{
		return E_NOINTERFACE;
	}

	static_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}


/******************************************************************************
*
* NAME
*     IUnknown::AddRef
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

ULONG STDMETHODCALLTYPE WebBrowser::AddRef(void) IUNKNOWN_NOEXCEPT
{
	return ++mRefCount;
}


/******************************************************************************
*
* NAME
*     IUnknown::Release
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

ULONG STDMETHODCALLTYPE WebBrowser::Release(void) IUNKNOWN_NOEXCEPT
{
	DEBUG_ASSERTCRASH(mRefCount > 0, ("Negative reference count"));
	--mRefCount;

	if (mRefCount == 0)
	{
		DEBUG_LOG(("WebBrowser::Release - all references released, deleting the object."));
		if (this == TheWebBrowser) {
			TheWebBrowser = NULL;
		}
		delete this;
		return 0;
	}

	return mRefCount;
}

STDMETHODIMP WebBrowser::TestMethod(Int num1)
{
	DEBUG_LOG(("WebBrowser::TestMethod - num1 = %d", num1));
	return S_OK;
}
