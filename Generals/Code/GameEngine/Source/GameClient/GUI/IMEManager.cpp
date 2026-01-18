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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   Ganerals
//
// Module:    IME
//
// File name: IMEManager.cpp
//
// Created:		11/14/01 TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "mbstring.h"

#include "Common/Debug.h"
#include "Common/Language.h"
#include "Common/UnicodeString.h"
#include "GameClient/Display.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/IMEManager.h"
#include "GameClient/Mouse.h"
#include "GameClient/Color.h"
#include "Common/NameKeyGenerator.h"


//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------

extern HWND ApplicationHWnd;  ///< our application window handle
extern Int	IMECandidateWindowLineSpacing;

//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------


//#define DEBUG_IME

//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------

//===============================
// IMEManager
//===============================

class IMEManager : public IMEManagerInterface
{

	public:

		IMEManager();
		~IMEManager();

		virtual void					init( void );
		virtual void					reset( void );
		virtual void					update( void );

		virtual void					attach( GameWindow *window );		///< attach IME to specified window
		virtual void					detach( void );								///< detach IME from current window
		virtual void					enable( void );									///< Enable IME
		virtual void					disable( void );								///< Disable IME
		virtual Bool					isEnabled( void );							///< Is IME enabled
		virtual Bool					isAttachedTo( GameWindow *window );	///< Is the manager attached toa window
		virtual GameWindow*		getWindow( void );							///< Returns the window we are currently attached to
		virtual Bool					isComposing( void );						///< Manager is currently composing new input string
		virtual void					getCompositionString( UnicodeString &string ); ///< Return the current composition string
		virtual Int						getCompositionCursorPosition( void );			///< Returns the composition cursor position
		virtual Int						getIndexBase( void );						///< Get index base for candidate list

		virtual Int						getCandidateCount();						///< Returns the total number of candidates
		virtual const UnicodeString* getCandidate( Int index );	///< Returns the candidate string
		virtual Int						getSelectedCandidateIndex();		///< Returns the indexed of the currently selected candidate
		virtual Int						getCandidatePageSize();					///< Returns the page size for the candidates list
		virtual Int						getCandidatePageStart();				///< Returns the index of the first visibel candidate



		/// Checks for and services IME messages. Returns TRUE if message serviced
		virtual Bool serviceIMEMessage(	void *windowsHandle,
												UnsignedInt message,
												Int wParam,
												Int lParam );
		virtual Int result( void );														///< result return value of last serviced IME message

	protected:

		enum
		{
				MAX_COMPSTRINGLEN = 2*1024
		};

		struct MessageInfo
		{
			Char *name;
			Int value;
		};

		Int										m_result;												///< last IME message's winProc return code
		GameWindow						*m_window;											///< window we are accepting input for
		HIMC									m_context;											///< Imput Manager Context
		HIMC									m_oldContext;										///< Previous IME comtext
		Int										m_disabled;											///< IME disable count 0 = enabled
		Bool									m_composing;										///< Are we currently composing a new string
		WideChar							m_compositionString[MAX_COMPSTRINGLEN+1];
		WideChar							m_resultsString[MAX_COMPSTRINGLEN+1];
		Int										m_compositionCursorPos;
		Int										m_compositionStringLength;
		Int										m_indexBase;

		Int										m_pageStart;										///< index of first visible candidate
		Int										m_pageSize;											///< Number of candidate per page
		Int										m_selectedIndex;								///< Index of the currently selected candidate
		Int										m_candidateCount;								///< Total number of candidate strings
		UnicodeString					*m_candidateString;							///< table of canidate strings
		Bool									m_unicodeIME;										///< Is this an unicode IME
		Int										m_compositionCharsDisplayed;		///< number of temporary composition characters displayed that need to be replaced with result string.

		WideChar			convertCharToWide( WPARAM mbchar );						///< Convert multibyte character to wide
		void					updateCompositionString( void );							///< Update the context of the composition string from the IMM
		void					getResultsString( void );											///< Get the final composition string result
		void					updateProperties( void );											///< Read the current IME properties
		void					openCandidateList( Int candidateFlags  );			///< open candidate window
		void					closeCandidateList( Int candidateFlags  );		///< Close candidate window
		void					updateCandidateList( Int candidateFlags  );		///< Update candidate window
		void					updateListBox( CANDIDATELIST *candidateList  );	///< Update candidate list box gadget
		void					convertToUnicode( Char *mbcs, UnicodeString &unicode );
		void					resizeCandidateWindow( Int pageSize );

		void					openStatusWindow( void );
		void					closeStatusWindow( void );
		void					updateStatusWindow( void );

		GameWindow						*m_candidateWindow;							///< IME candidate window interface
		GameWindow						*m_statusWindow;								///< IME status window interface
		GameWindow						*m_candidateTextArea;						///< list box area
		GameWindow						*m_candidateUpArrow;						///< up arrow
		GameWindow						*m_candidateDownArrow;					///< down arrow



	#ifdef DEBUG_IME
		static MessageInfo		m_mainMessageInfo[];
		static MessageInfo		m_notifyInfo[];
		static MessageInfo		m_requestInfo[];
		static MessageInfo		m_controlInfo[];
		static MessageInfo		m_setContextInfo[];
		static MessageInfo		m_setCmodeInfo[];
		static MessageInfo		m_setSmodeInfo[];
		Char*					getMessageName( MessageInfo *msgTable, Int value );
		void					buildFlagsString( IMEManager::MessageInfo *msgTable, Int value, AsciiString &string );
		void					printMessageInfo( Int message, Int wParam, Int lParam );
		void					printConversionStatus( void );
		void					printSentenceStatus( void );
	#endif
};




//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------
#ifdef DEBUG_IME

IMEManager::MessageInfo IMEManager::m_mainMessageInfo[] =
{
	{ "WM_IME_SETCONTEXT"				, WM_IME_SETCONTEXT      },
	{ "WM_IME_NOTIFY"						, WM_IME_NOTIFY          },
	{ "WM_IME_CONTROL"					, WM_IME_CONTROL         },
	{ "WM_IME_COMPOSITIONFULL"	, WM_IME_COMPOSITIONFULL },
	{ "WM_IME_SELECT"						, WM_IME_SELECT          },
	{ "WM_IME_CHAR"							, WM_IME_CHAR            },
#ifdef WM_IME_REQUST
	{ "WM_IME_REQUEST"					, WM_IME_REQUEST				 },
#endif
	{ "WM_IME_KEYDOWN"					, WM_IME_KEYDOWN         },
	{ "WM_IME_KEYUP"						, WM_IME_KEYUP           },
	{ "WM_IME_STARTCOMPOSITION"	, WM_IME_STARTCOMPOSITION},
	{ "WM_IME_ENDCOMPOSITION"		, WM_IME_ENDCOMPOSITION  },
	{ "WM_IME_COMPOSITION"			, WM_IME_COMPOSITION     },
	{ "WM_IME_KEYLAST"					, WM_IME_KEYLAST         },
	{ nullptr, 0 }
};

IMEManager::MessageInfo IMEManager::m_notifyInfo[] =
{
	{ "IMN_CLOSESTATUSWINDOW"			, IMN_CLOSESTATUSWINDOW   },
	{ "IMN_OPENSTATUSWINDOW"			, IMN_OPENSTATUSWINDOW    },
	{ "IMN_CHANGECANDIDATE"				, IMN_CHANGECANDIDATE     },
	{ "IMN_CLOSECANDIDATE"				, IMN_CLOSECANDIDATE      },
	{ "IMN_OPENCANDIDATE"					, IMN_OPENCANDIDATE       },
	{ "IMN_SETCONVERSIONMODE"			, IMN_SETCONVERSIONMODE   },
	{ "IMN_SETSENTENCEMODE"				, IMN_SETSENTENCEMODE     },
	{ "IMN_SETOPENSTATUS"					, IMN_SETOPENSTATUS       },
	{ "IMN_SETCANDIDATEPOS"				, IMN_SETCANDIDATEPOS     },
	{ "IMN_SETCOMPOSITIONFONT"		, IMN_SETCOMPOSITIONFONT  },
	{ "IMN_SETCOMPOSITIONWINDOW"	, IMN_SETCOMPOSITIONWINDOW},
	{ "IMN_SETSTATUSWINDOWPOS"		, IMN_SETSTATUSWINDOWPOS  },
	{ "IMN_GUIDELINE"							, IMN_GUIDELINE           },
	{ "IMN_PRIVATE"								, IMN_PRIVATE             },
	{ nullptr, 0 }
};

IMEManager::MessageInfo IMEManager::m_requestInfo[] =
{
#ifdef WM_IME_REQUST
	{ "IMR_COMPOSITIONWINDOW"				, IMR_COMPOSITIONWINDOW     },
	{ "IMR_CANDIDATEWINDOW"					, IMR_CANDIDATEWINDOW       },
	{ "IMR_COMPOSITIONFONT"					, IMR_COMPOSITIONFONT       },
	{ "IMR_RECONVERTSTRING"					, IMR_RECONVERTSTRING       },
	{ "IMR_CONFIRMRECONVERTSTRING"	, IMR_CONFIRMRECONVERTSTRING},
#endif
	{ nullptr, 0 }
};


IMEManager::MessageInfo IMEManager::m_controlInfo[] =
{
	{ "IMC_GETCANDIDATEPOS"				, IMC_GETCANDIDATEPOS     },
	{ "IMC_SETCANDIDATEPOS "			, IMC_SETCANDIDATEPOS     },
	{ "IMC_GETCOMPOSITIONFONT"		, IMC_GETCOMPOSITIONFONT  },
	{ "IMC_SETCOMPOSITIONFONT"		, IMC_SETCOMPOSITIONFONT  },
	{ "IMC_GETCOMPOSITIONWINDOW"	, IMC_GETCOMPOSITIONWINDOW},
	{ "IMC_SETCOMPOSITIONWINDOW"	, IMC_SETCOMPOSITIONWINDOW},
	{ "IMC_GETSTATUSWINDOWPOS"		, IMC_GETSTATUSWINDOWPOS  },
	{ "IMC_SETSTATUSWINDOWPOS"		, IMC_SETSTATUSWINDOWPOS  },
	{ "IMC_CLOSESTATUSWINDOW"			, IMC_CLOSESTATUSWINDOW   },
	{ "IMC_OPENSTATUSWINDOW"			, IMC_OPENSTATUSWINDOW    },
 	{ nullptr, 0 }
};

IMEManager::MessageInfo IMEManager::m_setContextInfo[] =
{
	{ "CANDIDATEWINDOW1"			, ISC_SHOWUICANDIDATEWINDOW			},
	{ "CANDIDATEWINDOW2"			, ISC_SHOWUICANDIDATEWINDOW<<1	},
	{ "CANDIDATEWINDOW3"			, ISC_SHOWUICANDIDATEWINDOW<<2	},
	{ "CANDIDATEWINDOW4"			, ISC_SHOWUICANDIDATEWINDOW<<3	},
	{ "COMPOSITIONWINDOW"			,	ISC_SHOWUICOMPOSITIONWINDOW		},
	{ "GUIDELINE"							, ISC_SHOWUIGUIDELINE						},
	{ nullptr, 0 }
};

IMEManager::MessageInfo IMEManager::m_setCmodeInfo[] =
{
	{ "ALPHANUMERIC"	, IME_CMODE_ALPHANUMERIC},
	{ "NATIVE"				, IME_CMODE_NATIVE      },
	{ "KATAKANA"			, IME_CMODE_KATAKANA    },
	{ "LANGUAGE"			, IME_CMODE_LANGUAGE    },
	{ "FULLSHAPE"			, IME_CMODE_FULLSHAPE   },
	{ "ROMAN"					, IME_CMODE_ROMAN       },
	{ "CHARCODE"			, IME_CMODE_CHARCODE    },
	{ "HANJACONVERT"	, IME_CMODE_HANJACONVERT},
	{ "SOFTKBD"				, IME_CMODE_SOFTKBD     },
	{ "NOCONVERSION"	, IME_CMODE_NOCONVERSION},
	{ "EUDC"					, IME_CMODE_EUDC        },
	{ "SYMBOL"				, IME_CMODE_SYMBOL      },
	{ "FIXED"					, IME_CMODE_FIXED       },
	{ nullptr, 0 }
};

IMEManager::MessageInfo IMEManager::m_setSmodeInfo[] =
{
	{ "NONE"					, IME_SMODE_NONE         },
	{ "PLAURALCLAUSE"	, IME_SMODE_PLAURALCLAUSE},
	{ "SINGLECONVERT"	, IME_SMODE_SINGLECONVERT},
	{ "AUTOMATIC"			, IME_SMODE_AUTOMATIC    },
	{ "PHRASEPREDICT"	, IME_SMODE_PHRASEPREDICT},
	{ "CONVERSATION"	, IME_SMODE_CONVERSATION },
	{ nullptr, 0 }
};

#endif

//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------

IMEManagerInterface *TheIMEManager = nullptr;


//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------

#ifdef DEBUG_IME

//============================================================================
// IMEManager::getMessageName
//============================================================================

Char*		IMEManager::getMessageName( IMEManager::MessageInfo *msgTable, Int value )
{
	Char *name = nullptr;

	if ( msgTable )
	{
		while ( msgTable->name )
		{
			if ( msgTable->value == value )
			{
				name = msgTable->name;
				break;
			}
			msgTable++;
		}
	}

	return name;
}

//============================================================================
// IMEManager::buildFlagsString
//============================================================================

void		IMEManager::buildFlagsString( IMEManager::MessageInfo *msgTable, Int value, AsciiString &string )
{
	string.clear();
	Bool first = TRUE;

	if ( msgTable )
	{
		while ( msgTable->name )
		{
			if ( msgTable->value & value )
			{
				if ( !first )
				{
					string.concat( "|" );
					first = FALSE;
				}
				string.concat( msgTable->name );
			}
			msgTable++;
		}
	}
}

//============================================================================
// IMEManager::printMessageInfo
//============================================================================

void		IMEManager::printMessageInfo( Int message, Int wParam, Int lParam )
{
	Char *messageText = getMessageName( m_mainMessageInfo, message);

	switch( message )
	{
		case WM_IME_NOTIFY:
		{
			Char *notifyName = getMessageName( m_notifyInfo, wParam );
			if ( notifyName == nullptr ) notifyName = "unknown";
			DEBUG_LOG(( "IMM: %s(0x%04x) - %s(0x%04x) - 0x%08x",  messageText, message, notifyName, wParam, lParam ));
			break;
		}
		case WM_IME_CONTROL:
		{
			Char *controlName = getMessageName( m_controlInfo, wParam );
			if ( controlName == nullptr ) controlName = "unknown";

			DEBUG_LOG(( "IMM: %s(0x%04x) - %s(0x%04x) - 0x%08x",  messageText, message, controlName, wParam, lParam ));
			break;
		}
		#ifdef WM_IME_REQUEST
		case WM_IME_REQUEST:
		{
			Char *requestName = getMessageName( m_requestInfo, wParam );
			if ( requestName == nullptr ) requestName = "unknown";

			DEBUG_LOG(( "IMM: %s(0x%04x) - %s(0x%04x) - 0x%08x",  messageText, message, requestName, wParam, lParam ));
			break;
		}
		#endif
		case WM_IME_SETCONTEXT:
		{
			AsciiString flags;

			buildFlagsString( m_setContextInfo, lParam, flags );

			DEBUG_LOG(( "IMM: %s(0x%04x) - 0x%08x - %s(0x%04x)",  messageText, message, wParam, flags.str(), lParam ));
			break;
		}
		default:
			if ( messageText )
			{
				DEBUG_LOG(( "IMM: %s(0x%04x) - 0x%08x - 0x%08x",  messageText, message, wParam, lParam ));
			}
			break;
	}
}

//============================================================================
// IMEManager::printConversionStatus
//============================================================================

void IMEManager::printConversionStatus( void )
{
	DWORD mode;
	if ( m_context )
	{
		ImmGetConversionStatus( m_context, &mode, nullptr );

		AsciiString flags;

		buildFlagsString( m_setCmodeInfo, mode, flags );

		DEBUG_LOG(( "IMM: Conversion mode = (%s)", flags.str()));
	}
}

//============================================================================
// IMEManager::printSentenceStatus
//============================================================================

void IMEManager::printSentenceStatus( void )
{
	DWORD mode;
	if ( m_context )
	{
		ImmGetConversionStatus( m_context, nullptr, &mode );

		AsciiString flags;

		buildFlagsString( m_setSmodeInfo, mode, flags );

		DEBUG_LOG(( "IMM: Sentence mode = (%s)", flags.str()));
	}
}
#endif // DEBUG_IME


//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------

//============================================================================
// *CreateIMEManagerInterface
//============================================================================

IMEManagerInterface *CreateIMEManagerInterface( void )
{
	return NEW IMEManager;
}


//============================================================================
// IMEManager::IMEManager
//============================================================================

IMEManager::IMEManager()
: m_window(nullptr),
	m_context(nullptr),
	m_candidateWindow(nullptr),
	m_statusWindow(nullptr),
	m_candidateCount(0),
	m_candidateString(nullptr),
	m_compositionStringLength(0),
	m_composing(FALSE),
	m_disabled(0),
	m_result(0),
	m_indexBase(1),

	m_compositionCharsDisplayed(0),
	m_candidateDownArrow(nullptr),
	m_candidateTextArea(nullptr),
	m_candidateUpArrow(nullptr),
	m_compositionCursorPos(0),
	m_pageSize(0),
	m_pageStart(0),
	m_selectedIndex(0),
	m_unicodeIME(FALSE)
{
}

//============================================================================
// IMEManager::~IMEManager
//============================================================================

IMEManager::~IMEManager()
{
	if ( m_candidateWindow )
	{
		m_candidateWindow->winSetUserData( nullptr );
		m_candidateTextArea->winSetUserData( nullptr );

		TheWindowManager->winDestroy( m_candidateWindow );
	}

	if ( m_statusWindow )
	{
		TheWindowManager->winDestroy( m_statusWindow );
	}

	delete [] m_candidateString;

	detach();
	ImmAssociateContext( ApplicationHWnd, m_oldContext );
	ImmReleaseContext( ApplicationHWnd, m_oldContext );
	if ( m_context )
	{
		ImmDestroyContext( m_context );
	}
}

//============================================================================
// IMEManager::init
//============================================================================

void IMEManager::init( void )
{
	//HWND ImeWindow = ImmGetDefaultIMEWnd(ApplicationHWnd);
  // if(ImeWindow)
	// {
  //    DestroyWindow(ImeWindow);
	//	}

	m_context = ImmCreateContext();
	m_oldContext = ImmGetContext( ApplicationHWnd );
	m_disabled = 0;
	m_candidateWindow = TheWindowManager->winCreateFromScript( "IMECandidateWindow.wnd");
	m_candidateWindow->winSetStatus(WIN_STATUS_ABOVE);

	if ( m_candidateWindow )
	{
		m_candidateWindow->winHide( TRUE );

		// find text area window
		NameKeyType id = TheNameKeyGenerator->nameToKey( "IMECandidateWindow.wnd:TextArea" );
		m_candidateTextArea = TheWindowManager->winGetWindowFromId(m_candidateWindow, id);

		// find arrows
		id = TheNameKeyGenerator->nameToKey( "IMECandidateWindow.wnd:UpArrow" );
		m_candidateUpArrow = TheWindowManager->winGetWindowFromId(m_candidateWindow, id);

		id = TheNameKeyGenerator->nameToKey( "IMECandidateWindow.wnd:DownArrow" );
		m_candidateDownArrow = TheWindowManager->winGetWindowFromId(m_candidateWindow, id);



		if ( m_candidateTextArea == nullptr )
		{
			TheWindowManager->winDestroy( m_candidateWindow );
			m_candidateWindow = nullptr;
		}
	}

	m_statusWindow = TheWindowManager->winCreateFromScript( "IMEStatusWindow.wnd");

	if ( m_statusWindow )
	{
		m_statusWindow->winHide( TRUE );
	}

	// attach IMEManager to each window
	if ( m_candidateWindow != nullptr )
	{
		m_candidateWindow->winSetUserData( TheIMEManager );
		m_candidateTextArea->winSetUserData( TheIMEManager );
	}

	detach();
	enable();
}

//============================================================================
// IMEManager::reset
//============================================================================

void IMEManager::reset( void )
{

}

//============================================================================
// IMEManager::update
//============================================================================

void IMEManager::update( void )
{

}

//============================================================================
// IMEManager::attach
//============================================================================

void IMEManager::attach( GameWindow *window )
{
	if ( m_window != window )
	{
		detach();
		if ( m_disabled == 0 )
		{
			ImmAssociateContext( ApplicationHWnd, m_context );
			updateStatusWindow();
			//openStatusWindow();
		}
		m_window = window;
	}
}

//============================================================================
// IMEManager::detach
//============================================================================

void IMEManager::detach( void )
{
	//ImmAssociateContext( ApplicationHWnd, nullptr );
	m_window = nullptr;

}

//============================================================================
// IMEManager::serviceIMEMessage
//============================================================================

Bool IMEManager::serviceIMEMessage(	void *windowsHandle, UnsignedInt message,	Int wParam,	Int lParam )
{

	DEBUG_ASSERTCRASH( windowsHandle == ApplicationHWnd, ("Unexpected window handle for IMEManager") );
	#ifdef DEBUG_IME
	printMessageInfo( message, wParam, lParam );
	#endif

   switch(message){

			// --------------------------------------------------------------------
			case WM_IME_CHAR:
			{
				WideChar wchar = convertCharToWide(wParam);
				#ifdef DEBUG_IME
				DEBUG_LOG(("IMM: WM_IME_CHAR - '%hc'0x%04x", wchar, wchar ));
				#endif

				if ( m_window && (wchar > 32 || wchar == VK_RETURN ))
				{
					TheWindowManager->winSendInputMsg( m_window, GWM_IME_CHAR, (wParam & 0xffff), lParam );
					m_result = 0;
					return TRUE;
				}
				return FALSE;
			}

			// --------------------------------------------------------------------
			case WM_CHAR:
			{
					WideChar wchar = (WideChar) (wParam & 0xffff);

				#ifdef DEBUG_IME
				DEBUG_LOG(("IMM: WM_CHAR - '%hc'0x%04x", wchar, wchar ));
				#endif

				if ( m_window && (wchar >= 32 || wchar == VK_RETURN) )
				{
					TheWindowManager->winSendInputMsg( m_window, GWM_IME_CHAR, (wParam & 0xffff), lParam );
					m_result = 0;
					return TRUE;
				}
				return FALSE;
			}

			// --------------------------------------------------------------------
      case WM_IME_SELECT:
					DEBUG_LOG(("IMM: WM_IME_SELECT"));
				return FALSE;
      case WM_IME_STARTCOMPOSITION:
        //The WM_IME_STARTCOMPOSITION message is sent immediately before an
        //IME generates a composition string as a result of a user's keystroke.
        //
				m_composing = TRUE;
				m_compositionCharsDisplayed = 0;
				updateCompositionString();
        m_result = 1;
				return TRUE;
			// --------------------------------------------------------------------
      case WM_IME_ENDCOMPOSITION:
			{
				//First remove the composition characters
				m_composing = FALSE;
				while (m_compositionCharsDisplayed > 0)
				{	//if cursor has moved since start of composition, we need to move it back using backspace message
					TheWindowManager->winSendInputMsg( m_window, GWM_CHAR, KEY_BACKSPACE, KEY_STATE_DOWN);
					m_compositionCharsDisplayed--;
				}
				closeCandidateList(0);
				return TRUE;
				// I removed the rest of this message handler because we update
				// the strings real time inside WM_IME_COMPOSITION
				// instead of waiting for user to enter separator. -MW

        //IMEs send this message to the application when the IMEs' composition
        //windows have closed. Applications that display their own composition
        //characters should process this message. Other applications should
        //send the message to the application IME window or to DefWindowProc,
        //which will pass the message to  the default IME window.
        //

				m_composing = FALSE; // reset this flag before calling GWM_IME_CHAR

				if ( m_window )
				{
					WideChar *ch = m_resultsString;
					getResultsString();
					while ( *ch )
					{
						TheWindowManager->winSendInputMsg( m_window, GWM_IME_CHAR, *ch, 0 );
						ch++;
					}
				}
				m_result = 1;
				return TRUE;
			}
			// --------------------------------------------------------------------
			case WM_IME_COMPOSITION:
			{
				//IMEs send this message to the application when they change composition
				//status in response to a keystroke. Applications that display their own
				//composition characters should process this message by calling
				//ImmGetCompositionString. Other applications should send the message to
				//the application IME window or to DefWindowProc, which will pass the
				//message to the default IME window.
				//
				//The WM_IME_COMPOSITION message is sent to an application when the IME
				//changes composition status as a result of a keystroke. An application
				//should process this message if it displays composition characters itself.
				//Otherwise, it should send the message to the IME window. This message
				//has no return value. wParam = DBCS character. lParam = change indicator.
				//

				if (lParam & GCS_RESULTSTR)	//added to show instant updates as soon as a character is translated. -MW
				{
					if ( m_window )
					{
						m_composing = FALSE; // reset this flag before calling GWM_IME_CHAR

						while (m_compositionCharsDisplayed > 0)
						{	//if cursor has moved since start of composition, we need to move it back using backspace message
							TheWindowManager->winSendInputMsg( m_window, GWM_CHAR, KEY_BACKSPACE, KEY_STATE_DOWN);
							m_compositionCharsDisplayed--;
						}

						WideChar *ch = m_resultsString;
						getResultsString();
						while ( *ch )
						{
							TheWindowManager->winSendInputMsg( m_window, GWM_IME_CHAR, *ch, 0 );
							ch++;
						}
					}
					m_compositionCharsDisplayed = 0;
				}
				else
				if (lParam & CS_INSERTCHAR && lParam & CS_NOMOVECARET)
				{	//we are supposed to display the composition character without moving the cursor. (it's a candidate).
					if (m_window)
					{
						m_composing = FALSE; // reset this flag before calling GWM_IME_CHAR

						while (m_compositionCharsDisplayed > 0)
						{	//if cursor has moved since start of composition, we need to move it back using backspace message
							TheWindowManager->winSendInputMsg( m_window, GWM_CHAR, KEY_BACKSPACE, KEY_STATE_DOWN);
							m_compositionCharsDisplayed--;
						}

						WideChar *ch = m_compositionString;
						updateCompositionString();
						while (*ch)
						{
							TheWindowManager->winSendInputMsg(m_window, GWM_IME_CHAR, *ch, 0 );
							ch++;
							m_compositionCharsDisplayed++;
						}
						m_composing = TRUE; // reset this flag before calling GWM_IME_CHAR
					}
				}
				else
				if (lParam & GCS_COMPSTR)
				{	//we are supposed to display the composition character without moving the cursor. (it's a candidate).
					if (m_window)
					{
						m_composing = FALSE; // reset this flag before calling GWM_IME_CHAR

						while (m_compositionCharsDisplayed > 0)
						{	//if cursor has moved since start of composition, we need to move it back using backspace message
							TheWindowManager->winSendInputMsg( m_window, GWM_CHAR, KEY_BACKSPACE, KEY_STATE_DOWN);
							m_compositionCharsDisplayed--;
						}

						WideChar *ch = m_compositionString;
						updateCompositionString();
						while (*ch)
						{
							TheWindowManager->winSendInputMsg(m_window, GWM_IME_CHAR, *ch, 0 );
							ch++;
							m_compositionCharsDisplayed++;
						}
						m_composing = TRUE; // reset this flag before calling GWM_IME_CHAR
					}
				}

				m_result = 1;
				return TRUE;
			}
			// --------------------------------------------------------------------
			case WM_IME_SETCONTEXT:
			{
				//The system sends this message to an application when one of the
				//application's windows is activated. Applications should respond by
				//calling ImmGetContext.
				//
				updateProperties();
				//ignore for now
				m_result = 0;
				return FALSE;
			}


			// --------------------------------------------------------------------
			case WM_IME_NOTIFY:
			{
				//IMEs generate this message to notify the application or the IME window
				//that the IME status has changed. The wParam value is a submessage that
				//specifies the nature of the change.
				//
				switch(wParam)
				{
					case IMN_OPENCANDIDATE:
					{
						//This message is sent to the application when an IME is about to
						//open the candidate window. An application should process this
						//message if it displays candidates. The application can retrieve
						//a list of candidates to display by using theImmGetCandidateList
						//function. The application receives this notification message through
						//the WM_IME_NOTIFY message.
						//

						// open candidate window
						openCandidateList( lParam );
						m_result =  1;
						return TRUE;
					}
					case IMN_CLOSECANDIDATE:
					{
						//This message is sent to the application when an IME is about to
						//close the candidate window. An application should process this
						//message if it displays candidates. The application receives this
						//notification message through the WM_IME_NOTIFY message.
						//
						closeCandidateList( lParam );
						m_result =  1;
						return TRUE;
					}

					case IMN_CHANGECANDIDATE:
					{
						//This message is sent to the application when an IME is about to
						//change the content of the candidate window. An application should
						//process this notification message if it displays candidates itself.
						//The application receives this notification message through the
						//WM_IME_NOTIFY message.
						//
						updateCandidateList( lParam );
						m_result =  1;
				 	  return TRUE;
					}
					case IMN_GUIDELINE:              //This message is sent when an IME is about to show an error message or other data.
					{
						// display error message
						m_result = 1;
						return FALSE;
					}

					case IMN_SETCONVERSIONMODE:      //This message is sent when the conversion mode of the input context is updated.
					{
						#ifdef DEBUG_IME
						printConversionStatus();
						#endif
						return FALSE;
					}

					case IMN_SETSENTENCEMODE:        //This message is sent when the sentence mode of the input context is updated.
					{
						#ifdef DEBUG_IME
						printSentenceStatus();
						#endif
						return FALSE;
					}
/*
					// pass the reset on through to the installed IME
					case IMN_SETCANDIDATEPOS:        //This message is sent when the IME is about to move the candidate window.
					case IMN_SETSTATUSWINDOWPOS:     //This message is sent when the status window position in the input context is updated.
						return FALSE;
					case IMN_OPENSTATUSWINDOW:       //This message is sent when an IME is about to create the status window.
						DEBUG_LOG(("Open Status Window"));
						return FALSE;
					case IMN_CLOSESTATUSWINDOW:      //This message is sent to the application when an input method editor (IME) is about to close the status window.
						DEBUG_LOG(("Close Status Window"));
						return FALSE;
					case IMN_SETOPENSTATUS:          //This message is sent when the open status of the input context is updated.
					case IMN_SETCOMPOSITIONFONT:     //This message is sent when the font of the input context is updated.
					case IMN_SETCOMPOSITIONWINDOW:   //This message is sent when the style or position of the composition window is updated.
					case IMN_PRIVATE:                //This message is for your own use, it seems.
*/				default:
						m_result =  1;
						return TRUE;
				}
				break;
		}

			// --------------------------------------------------------------------
      case WM_IME_COMPOSITIONFULL:
			{
				//IMEs send this message to the application when they are unable to
        //extend the composition window to accommodate any more characters.
        //Applications should tell IMEs how to display the composition window
        //using the IMC_SETCOMPOSITIONWINDOW message.
        //
        //I'm not sure what to do here.
        m_result = 1;
				return TRUE;
			}
   }

	return FALSE;
}

//============================================================================
// IMEManager::result
//============================================================================

Int IMEManager::result( void )
{
	return m_result;
}

//============================================================================
// IMEManager::enable
//============================================================================

void IMEManager::enable( void )
{
	if ( --m_disabled <= 0 )
	{
		m_disabled = 0;
		ImmAssociateContext( ApplicationHWnd, m_context );
	}
}

//============================================================================
// IMEManager::disable
//============================================================================

void IMEManager::disable( void )
{
	m_disabled++;
	ImmAssociateContext( ApplicationHWnd, nullptr );
}

//============================================================================
// IMEManager::isEnabled
//============================================================================

Bool IMEManager::isEnabled( void )
{
	return m_context != nullptr && m_disabled == 0;
}

//============================================================================
// IMEManager::isAttached
//============================================================================

Bool IMEManager::isAttachedTo( GameWindow *window )
{
	return m_window == window;
}

//============================================================================
// IMEManager::getWindow
//============================================================================

GameWindow* IMEManager::getWindow( void )
{
	return m_window;
}


//============================================================================
// IMEManager::convertChartoWide
//============================================================================

WideChar IMEManager::convertCharToWide( WPARAM wParam )
{
	char dcbsString[3];

	if ( wParam&0xff00 )
	{
		dcbsString[0] = (wParam>>8)&0xff;
		dcbsString[1] = wParam&0xff;
		dcbsString[2] = 0;
	}
	else
	{
		dcbsString[0] = wParam&0xff;
		dcbsString[1] = 0;
	}


	WideChar uniString[2];

	if ( MultiByteToWideChar( CP_ACP, 0, dcbsString, strlen( dcbsString ), uniString, 1 ) == 1 )
	{
		return uniString[0];
	}

	return 0;
}

//============================================================================
// IMEManager::isComposing
//============================================================================

Bool IMEManager::isComposing( void )
{
	return m_composing;
}


void IMEManager::getCompositionString ( UnicodeString &string )
{
	string.set(m_compositionString);
}

//============================================================================
// IMEManager::getCompositionCursorPosition
//============================================================================

Int	IMEManager::getCompositionCursorPosition( void )
{
	return 0;//m_compositionCursorPos;
}

//============================================================================
// IMEManager::updateCompositionString
//============================================================================

void IMEManager::updateCompositionString( void )
{
	char tempBuf[ (MAX_COMPSTRINGLEN+1)*2];

	m_compositionCursorPos = 0;
	m_compositionString[0] = 0;
	m_compositionStringLength = 0;

	if ( m_context )
	{
		// try reading unicode directly
		LONG result = ImmGetCompositionStringW( m_context, GCS_COMPSTR, m_compositionString, MAX_COMPSTRINGLEN );

		if ( result >= 0 )
		{
			m_compositionStringLength = result/2;
			m_compositionCursorPos = (ImmGetCompositionStringW( m_context, GCS_CURSORPOS, nullptr, 0) & 0xffff );
		}
		else
		{

			// read MBCS instead
			result = ImmGetCompositionStringA( m_context, GCS_COMPSTR, tempBuf, MAX_COMPSTRINGLEN*2);

			if ( result > 0 )
			{
				tempBuf[ result ] = '\0';

				int convRes = MultiByteToWideChar( CP_ACP, 0, tempBuf, -1, m_compositionString, MAX_COMPSTRINGLEN );
				GameArrayEnd(m_compositionString);

				if ( convRes < 0)
				{
					convRes = 0;
				}
				else
				{
					m_compositionCursorPos = (ImmGetCompositionString( m_context, GCS_CURSORPOS, nullptr, 0) & 0xffff );
					convRes = wcslen( m_compositionString );
				}

				// m_compositionCursorPos is in DBCS characters, need to convert it to Wide characters

				//msg_assert ( (int)strlen(tempBuf) >= convRes ,("bad DBCS string: DBCS = %d chars, Wide = %d chars", strlen(tempBuf), convRes));
				m_compositionCursorPos = _mbsnccnt ( (unsigned char *) tempBuf, m_compositionCursorPos );

				m_compositionString[convRes] = 0;
				m_compositionStringLength = convRes;
				if ( m_compositionCursorPos > convRes )
				{
					m_compositionCursorPos = convRes;
				}
				else if ( m_compositionCursorPos < 0 )
				{
					m_compositionCursorPos = 0;
				}
			}
		}
	}

	DEBUG_ASSERTCRASH( m_compositionStringLength < MAX_COMPSTRINGLEN, ("composition string too large"));
	m_compositionString[m_compositionStringLength] = 0;
	GameArrayEnd(m_compositionString);
}

//============================================================================
// IMEManager::getResultsString
//============================================================================

void IMEManager::getResultsString ( void )
{
	Int stringLen = 0;
	m_resultsString[0] = 0;

	if ( m_context )
	{
		// try reading unicode directly
		LONG result = ImmGetCompositionStringW( m_context,  GCS_RESULTSTR, m_resultsString, MAX_COMPSTRINGLEN );

		if ( result >= 0 )
		{
		 stringLen = result/2;
		}
		else
		{
			char tempBuf[ (MAX_COMPSTRINGLEN+1)*2];

			// read MBCS instead
			result = ImmGetCompositionStringA( m_context, GCS_RESULTSTR, tempBuf, MAX_COMPSTRINGLEN*2);

			if ( result > 0 )
			{
				tempBuf[ result ] = '\0';

				int convRes = MultiByteToWideChar( CP_ACP, 0, tempBuf, strlen(tempBuf), m_resultsString, MAX_COMPSTRINGLEN );

				if ( convRes < 0)
				{
					convRes = 0;
				}
				stringLen = convRes;
			}
		}
	}

	DEBUG_ASSERTCRASH( stringLen < MAX_COMPSTRINGLEN, ("results string too large"));
	m_resultsString[stringLen] = 0;
	GameArrayEnd(m_resultsString);
}

//============================================================================
// IMEManager::convertToUnicode
//============================================================================

void IMEManager::convertToUnicode ( Char *mbcs, UnicodeString &unicode )
{
 	int size = MultiByteToWideChar( CP_ACP, 0, mbcs, strlen(mbcs), nullptr, 0 );

	unicode.clear();

	if ( size <= 0 )
	{
		return;
	}

	WideChar *buffer = NEW WideChar[ size + 1 ];

	if ( buffer )
	{
		size = MultiByteToWideChar( CP_ACP, 0, mbcs, strlen(mbcs), buffer, size );

		if ( size <= 0 )
		{
			unicode.clear();
		}
		else
		{
			buffer[size] = 0;
			unicode = buffer;
		}

		delete [] buffer;
	}
}

//============================================================================
// IMEManager::openCandidateList
//============================================================================

void IMEManager::openCandidateList( Int candidateFlags )
{
	if ( m_candidateWindow == nullptr )
	{
		return;
	}
	// first get latest candidate list info
	updateCandidateList( candidateFlags );
  resizeCandidateWindow( m_pageSize );

	m_candidateWindow->winHide( FALSE );
	m_candidateWindow->winBringToTop();
	TheWindowManager->winSetModal( m_candidateWindow );

	Int wx, wy, wwidth, wheight, wcursorx, wcursory;
	Int cx, cy, cwidth, cheight;
	Int	textHeight = 20;


	if ( m_window )
	{
		m_window->winGetScreenPosition( &wx, &wy);
		m_window->winGetSize( &wwidth, &wheight );
		m_window->winGetCursorPosition( &wcursorx, &wcursory );
		GameFont *font = m_window->winGetFont();
		if ( font )
		{
			textHeight = font->height;
		}
	}
	else
	{
		wx = wy = 0;
		wwidth = 10;
		wheight = 10;
		wcursorx = 0;
		wcursory = 0;
	}

	m_candidateWindow->winGetSize( &cwidth, &cheight );

//	// try putting the candidate list above the text
//	cx = wx + wcursorx;
//	cy = wy  - cheight;
//
//	if ( cx + cwidth > (Int) TheDisplay->getWidth())
//	{
//		cx = cx - cwidth;
//	}
//	if( cx < 0 )
//		cx = 0;
//	if ( cy < 0 )
//	{
//		// place list below text
//		cy = wy + textHeight + textHeight/2;
//	}
//	if( cy + cheight > TheDisplay->getHeight())
//		cy = 0;
	cx = TheDisplay->getWidth() - cwidth;
	cy = 0;
	updateProperties();
	m_candidateWindow->winSetPosition( cx, cy );

}

//============================================================================
// IMEManager::closeCandidateList
//============================================================================

void IMEManager::closeCandidateList( Int candidateFlags  )
{
	if ( m_candidateWindow != nullptr )
	{
		m_candidateWindow->winHide( TRUE );
		TheWindowManager->winUnsetModal( m_candidateWindow );
	}

	delete [] m_candidateString;
	m_candidateString = nullptr;

	m_candidateCount = 0;

}

//============================================================================
// IMEManager::updateCandidateList
//============================================================================

void IMEManager::updateCandidateList( Int candidateFlags  )
{

	delete [] m_candidateString;
	m_candidateString = nullptr;

	m_pageSize = 10;
	m_candidateCount = 0;
	m_pageStart = 0;
	m_selectedIndex = 0;

	if (	m_candidateWindow == nullptr ||
				m_context == nullptr ||
				candidateFlags == 0)
	{
		return;
	}

	for( Int i = 0, candidate = 1; i < 32; i++, candidate = candidate << 1 )
	{
		if ( candidateFlags & candidate )
		{
			Bool unicode = TRUE;
			unsigned long listCount = 0;

			Int size = ImmGetCandidateListCountW( m_context, &listCount );

			if ( size <= 0 )
			{
				unicode = FALSE;
				size = ImmGetCandidateListCountA( m_context, &listCount );
				if ( size <= 0 )
				{
					return;
				}
			}

			// create a temporary buffer for reading the candidate list
			Char *buffer = NEW Char[size];

			if ( buffer == nullptr )
			{
				return;
			}

			memset( buffer, 0, size );

			CANDIDATELIST *clist = (CANDIDATELIST*) buffer;

			Bool ok = TRUE ;
			Int bytesCopied;

			if ( unicode )
			{
				bytesCopied = ImmGetCandidateListW( m_context, i, (CANDIDATELIST*) clist, size );
			}
			else
			{
				bytesCopied = ImmGetCandidateListA( m_context, i, (CANDIDATELIST*) clist, size );
			}

			if ( bytesCopied == 0 || bytesCopied > size )
			{
				DEBUG_ASSERTCRASH(bytesCopied < size,("IME candidate buffer overrun"));
				ok = FALSE;
			}

			if ( ok && clist->dwStyle != IME_CAND_UNKNOWN && clist->dwStyle != IME_CAND_CODE  )
			{
		    //Apparently there is an "IME98 bug" (IME bug under Windows 98?) that
		    //causes you to have to execute the following code.
		    if(( clist->dwPageStart >  clist->dwSelection) ||
		       (clist->dwSelection >= clist->dwPageStart + clist->dwPageSize))
		    {
		       clist->dwPageStart = (clist->dwSelection / clist->dwPageSize) * clist->dwPageSize;
		    }

				m_pageSize = clist->dwPageSize;
				m_candidateCount = clist->dwCount;
				m_pageStart = clist->dwPageStart;
				m_selectedIndex = clist->dwSelection;

				#ifdef DEBUG_IME
				DEBUG_LOG(("IME: Candidate Update: Candidates = %d, pageSize = %d pageStart = %d, selected = %d", m_candidateCount, m_pageStart, m_pageSize, m_selectedIndex ));
				#endif

				if ( m_candidateUpArrow )
				{
					m_candidateUpArrow->winHide( m_pageStart == 0 );
				}

				if ( m_candidateDownArrow )
				{
					m_candidateDownArrow->winHide( m_candidateCount - m_pageStart <= m_pageSize );
				}

				if ( m_candidateCount > 0 )
				{
					m_candidateString = NEW UnicodeString[m_candidateCount];
					if ( m_candidateString )
					{
						Int i;

						for( i=0; i < m_candidateCount; i++ )
						{
							Char *string = (Char*) ((UnsignedInt) clist + (UnsignedInt) clist->dwOffset[i]);
							if ( unicode )
							{
								m_candidateString[i].set( (WideChar *) string);
							}
							else
							{
								convertToUnicode( string, m_candidateString[i] );
							}
						}
					}
				}

			}
			delete [] buffer;
			return;
		}
	}
}

//============================================================================
// IMEManager::updateProperties
//============================================================================

void IMEManager::updateProperties( void )
{
	HKL kb = GetKeyboardLayout( 0 );
	Int prop = ImmGetProperty( kb, IGP_PROPERTY );
	m_indexBase = prop & IME_PROP_CANDLIST_START_FROM_1 ? 1 : 0 ;
	m_unicodeIME = (prop & IME_PROP_UNICODE) != 0 ;
}

//============================================================================
// IMEManager::getIndexBase
//============================================================================

Int IMEManager::getIndexBase( void )
{
	return m_indexBase;
}

//============================================================================
// IMEManager::resizeCandidateWindow
//============================================================================

void IMEManager::resizeCandidateWindow( Int pageSize )
{
	if ( m_candidateWindow == nullptr )
	{
		return;
	}

	GameFont *font = m_candidateTextArea->winGetFont();

	if ( font == nullptr )
	{
		return;
	}

	Int newh = pageSize * (font->height + IMECandidateWindowLineSpacing);
	Int w, h;
	m_candidateTextArea->winGetSize( &w, &h );

	Int dif = newh - h;

	m_candidateTextArea->winSetSize( w, newh );

	m_candidateWindow->winGetSize( &w, &h );
	h += dif;
	m_candidateWindow->winSetSize( w, h );

	if ( m_candidateDownArrow )
	{
		Int x, y;
		m_candidateDownArrow->winGetPosition( &x, &y );
		y += dif;
		m_candidateDownArrow->winSetPosition( x, y );
	}

}

//============================================================================
// IMEManager::getCandidateCount
//============================================================================

Int	IMEManager::getCandidateCount()
{
	return m_candidateCount;
}

//============================================================================
// IMEManager::getCandidate
//============================================================================

const UnicodeString* IMEManager::getCandidate( Int index )
{
	if ( m_candidateString != nullptr && index >=0 && index < m_candidateCount )
	{
		return &m_candidateString[index];
	}

	return &UnicodeString::TheEmptyString;
}

//============================================================================
// IMEManager::getSelectedCandidateIndex
//============================================================================

Int	IMEManager::getSelectedCandidateIndex()
{
	return m_selectedIndex;
}

//============================================================================
// IMEManager::getCandidatePageSize
//============================================================================

Int	IMEManager::getCandidatePageSize()
{
	return m_pageSize;
}

//============================================================================
// IMEManager::getStartCandidateIndex
//============================================================================

Int IMEManager::getCandidatePageStart()
{
	return m_pageStart;
}

//============================================================================
// IMEManager::openStatusWindow
//============================================================================

void IMEManager::openStatusWindow( void )
{
	if ( m_statusWindow == nullptr )
	{
		return;
	}
	m_statusWindow->winHide( FALSE );
}

//============================================================================
// IMEManager::closeStatusWindow
//============================================================================

void IMEManager::closeStatusWindow( void )
{
	if ( m_statusWindow == nullptr )
	{
		return;
	}

	m_statusWindow->winHide( TRUE );
}

//============================================================================
// IMEManager::updateStatusWindow
//============================================================================

void IMEManager::updateStatusWindow( void )
{

}

