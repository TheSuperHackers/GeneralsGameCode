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

#ifdef _WIN32
#include <process.h>
#endif
#include <cstdlib>
#include <csignal>
#include <iostream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

//#define THREADSAFE_HEADER

#include <wstring.h>
#include <wdebug.h>
#include <filed.h>
#include <stdoutd.h>
#include <wstypes.h>
#include <xtime.h>
#include "global.h"
#include "generals.h"
#include "timezone.h"
#include <threadfac.h>

#include <tcp.h>
#include "mydebug.h"

#ifdef _UNIX
using namespace std;
#else
#define sleep(x) Sleep(1000 * (x))
#endif

static const char *Program_Usage = "A config filename can be given on the command line (default=matchbot.cfg)\n";
void logMonitor(void *);
void paranoidLogMonitor(void *);

OutputDevice * output_device = NULL;
OutputDevice * paranoid_output_device = NULL;


void Signal_Quit(int)
{
	INFMSG("Exiting due to signal.");
	exit(2);
}

void Setup_Signals(void)
{
#ifdef _UNIX
	struct sigaction act, oact;
	act.sa_handler = Signal_Quit;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGTERM, &act, &oact);
	sigaction(SIGINT, &act, &oact);
#endif
}

int VerifyFileDescriptors(int requested)
{
#ifdef _UNIX
	struct rlimit limit;
	if (!getrlimit(_SC_OPEN_MAX, &limit))
	{
		INFMSG("Hard limit on file descriptors: " << limit.rlim_max);
		if (limit.rlim_max < (unsigned int)requested)
		{
			ERRMSG("Too many file descriptors requested");
			ERRMSG("Hard Limit: " << limit.rlim_max << ", requested: " << requested);
			requested = limit.rlim_max;
		}

		limit.rlim_cur = limit.rlim_max; /* make soft limit the max */
		if (setrlimit(_SC_OPEN_MAX, &limit) == -1)
		{
			ERRMSG("Error setting max file descriptors to " << limit.rlim_cur);
			exit(-1);
		}
		INFMSG("Soft limit on file descriptors: " << limit.rlim_cur);
	}
	else
	{
		ERRMSG("Couldn't get limit for _SC_OPEN_MAX");
		exit(-1);
	}
#endif
	return requested;
}



GeneralsMatcher *s_generalsMatcher = NULL;
GeneralsClientMatcher *s_generalsClientMatcher = NULL;

int main(int argc, char ** argv)
{
	Wstring config_fname = "matchbot.cfg";

	// You can specify the config file on the command line
	if (argc == 2)
		config_fname = argv[1];

	// Read the config file
	FILE *fp;
	if ((fp = fopen(config_fname.get(), "r")) == NULL)
	{
		cerr << "\nCan't open the config file '" << config_fname.get() << "'\n\n";
		cerr << Program_Usage << endl;
		exit( -1);
	}
	fclose(fp);

	Global.ReadFile(config_fname.get());

	// Setup debugging & logging output
	Wstring output_file;
	output_file.set("matchbot.log");
	Global.config.getString("OUTPUTFILE", output_file);
	if (output_file != "STDOUT")
	{
		int append = 1;
		Global.config.getInt("APPENDTOLOG", append);
		if (append)
			output_device = new FileD(output_file.get(), "a");
		else
			output_device = new FileD(output_file.get(), "w");
		if (!output_device)
		{
			cerr << "Could not open " << output_file.get() << " for writing!" << endl;
			exit( -1);
		}
	}
	else
	{
		output_device = new StdoutD;
	}
	MsgManager::setAllStreams(output_device);
	DBGMSG("Matching bot started!");
	INFMSG("Matching bot " << argv[0] << " started!");

	// Setup logging of suspicious activity
	Wstring paranoid_output_file;
	paranoid_output_file.set("hacks.log");
	Global.config.getString("PARANOIDFILE", paranoid_output_file);
	if (paranoid_output_file != "STDOUT")
	{
		paranoid_output_device = new FileD(paranoid_output_file.get(), "a");
		if (!paranoid_output_device)
		{
			cerr << "Could not open " << paranoid_output_file.get() << " for writing!" << endl;
			exit( -1);
		}
	}
	else
	{
		paranoid_output_device = new StdoutD;
	}
	MyMsgManager::setParanoidStream(paranoid_output_device);
	DBGMSG("Hack log started!");
	PARANOIDMSG("Hack log started!");

	Setup_Signals();

#ifdef _WINDOWS
	// ----- Initialize Winsock -----
	WORD verReq = MAKEWORD(2, 2);
	WSADATA wsadata;

	int err = WSAStartup(verReq, &wsadata);
	if (err != 0)
	{
		ERRMSG("Winsock Init failed.");
		return 1;
	}

	if ((LOBYTE(wsadata.wVersion) != 2) || (HIBYTE(wsadata.wVersion) !=2))
	{
		ERRMSG("Winsock DLL is not 2.2");
		WSACleanup();
		ERRMSG("Winsock Init failed.");
		return 1;
	}
	INFMSG("Winsock Init done.");
#endif

	// Check game type & start matcher
	Wstring gametype = "unknown";
	Global.config.getString("GAME", gametype);

	// Command-line override for gamemode.
	// This is for test suites, so they can use
	// the same config file as the corresponding
	// matchbot.
	const char *s = argv[0] + strlen(argv[0]);
	while (s > argv[0] && *s != '/')
		--s;
	if (*s == '/')
		++s;
	Wstring exe = s;
	exe.toLower();
	DBGMSG("Executable file is [" << exe.get() << "]");

	if (gametype == "Generals")
	{
		DBGMSG("Generals matching behavior");
		s_generalsMatcher = new GeneralsMatcher;
		s_generalsMatcher->connectAndLoop();
	}
	else if (gametype == "GeneralsClient")
	{
		DBGMSG("Generals TEST client matching behavior");
		s_generalsClientMatcher = new GeneralsClientMatcher;
		s_generalsClientMatcher->connectAndLoop();
	}
	else
	{
		cerr << "\nNo valid GAME entry found!" << endl;
		exit( -1);
	}

	if (s_generalsMatcher)
		delete s_generalsMatcher;
	if (s_generalsClientMatcher)
		delete s_generalsClientMatcher;

	return 0;
}

/*----------------------------------------------------------------------+
| THREAD: logMonitor                                                    |
| This thread is spawned once per execution.  It will activate after    |
| midnight and create a new log file.  The old one gets put into the    |
| logfiles directory.                                                   |
`----------------------------------------------------------------------*/
void logMonitor(void *)
{
#ifdef _UNIX
	Xtime xtime;
	time_t curtime;
	//char timebuf[40];
	char filenamebuf[128];
	int delay = -1;
	Global.config.getInt("ROTATEDELAY", delay);
	DBGMSG("ROTATEDELAY: " << delay);
	if (delay == -1)
		return ;
	while (1)
	{
		curtime = time(NULL);
		// get the number of seconds that have passed since midnight
		// of the current day.
		curtime -= TimezoneOffset();
		time_t timeofday = curtime % (delay);
		if ((timeofday > 0) && (timeofday <= 300))
		{
			// We're within 5 minutes of midnight, switch the files.

			DBGMSG("about to switch.");
			Wstring logfilename = "matchbot.log";
			Global.config.getString("OUTPUTFILE", logfilename);
			Wstring newfilename = "tmp.log";
			Global.config.getString("ROTATEFILE", newfilename);
			MsgManager::ReplaceAllStreams((FileD*)output_device, logfilename.get(), newfilename.get());
			Wstring logpath = "logs";
			Global.config.getString("LOGPATH", logpath);
			xtime.update();
			sprintf(filenamebuf, "%s/%02d%02d%04d_%02d%02d%02d_log", logpath.get(), xtime.getMonth(),
			        xtime.getMDay(), xtime.getYear(), xtime.getHour(), xtime.getMinute(), xtime.getSecond());
			rename(newfilename.get(), filenamebuf);
			DBGMSG("Normal: Just been switched.  " << logfilename.get() << ", " << newfilename.get());
			sleep(60*60*23); // sleep the next 23 hours
		}
		sleep(300);
	}
#endif
}

void rotateOutput(void)
{
	Xtime xtime;
	char filenamebuf[128];
	Wstring logfilename = "matchbot.log";
	Wstring newfilename = "tmp.log";
	Wstring logpath = "logs";

	DBGMSG("About to switch.");

	Global.config.getString("OUTPUTFILE", logfilename);
	Global.config.getString("ROTATEFILE", newfilename);
	Global.config.getString("LOGPATH", logpath);

	// This grabs the semaphore, renames the file, and switches the output device
	MsgManager::ReplaceAllStreams((FileD*)output_device, logfilename.get(),
	                              newfilename.get());

	// clean up the tmp filename and move it to the log dir.
	sprintf(filenamebuf, "%s/%02d%02d%04d_%02d%02d%02d_log", logpath.get(),
	        xtime.getMonth(), xtime.getMDay(), xtime.getYear(), xtime.getHour(),
	        xtime.getMinute(), xtime.getSecond());
#ifdef _WINDOWS
	mkdir(logpath.get());
#else
	mkdir(logpath.get(), 00666);
#endif
	rename(newfilename.get(), filenamebuf);

	DBGMSG("Normal: Just been switched.  " << logfilename.get() << ", " <<
	       newfilename.get());
}

void paranoidLogMonitor(void *)
{
#ifdef _UNIX
	Xtime xtime;
	time_t curtime;
	//char timebuf[40];
	char filenamebuf[128];
	int delay = -1;
	Global.config.getInt("ROTATEDELAY", delay);
	PARANOIDMSG("ROTATEDELAY: " << delay);
	if (delay == -1)
		return ;
	while (1)
	{
		curtime = time(NULL);
		// get the number of seconds that have passed since midnight
		// of the current day.
		curtime -= TimezoneOffset();
		time_t timeofday = curtime % (delay);
		if ((timeofday > 0) && (timeofday <= 300))
		{
			// We're within 5 minutes of midnight, switch the files.

			PARANOIDMSG("about to switch.");
			Wstring logfilename = "matchbot.log";
			Global.config.getString("PARANOIDFILE", logfilename);
			Wstring newfilename = "tmp.log";
			Global.config.getString("ROTATEPARANOIDFILE", newfilename);
			MyMsgManager::ReplaceAllStreams((FileD*)paranoid_output_device, logfilename.get(), newfilename.get());
			Wstring logpath = "logs";
			Global.config.getString("PARANOIDLOGPATH", logpath);
			xtime.update();
			sprintf(filenamebuf, "%s/%02d%02d%04d_%02d%02d%02d_log", logpath.get(), xtime.getMonth(),
			        xtime.getMDay(), xtime.getYear(), xtime.getHour(), xtime.getMinute(), xtime.getSecond());
			rename(newfilename.get(), filenamebuf);
			PARANOIDMSG("Paranoid: Just been switched.  " << logfilename.get() << ", " << newfilename.get());
			sleep(60*60*23); // sleep the next 23 hours
		}
		sleep(300);
	}
#endif
}

void rotateParanoid(void)
{
	Xtime xtime;
	char filenamebuf[128];
	Wstring logfilename = "matchbot.log";
	Wstring newfilename = "tmp.log";
	Wstring logpath = "logs";

	PARANOIDMSG("About to switch.");

	Global.config.getString("PARANOIDFILE", logfilename);
	Global.config.getString("ROTATEPARANOIDFILE", newfilename);
	Global.config.getString("PARANOIDLOGPATH", logpath);

	// This grabs the semaphore, renames the file, and switches the output device
	MyMsgManager::ReplaceAllStreams((FileD*)output_device, logfilename.get(),
	                                newfilename.get());

	// clean up the tmp filename and move it to the log dir.
	sprintf(filenamebuf, "%s/%02d%02d%04d_%02d%02d%02d_log", logpath.get(),
	        xtime.getMonth(), xtime.getMDay(), xtime.getYear(), xtime.getHour(),
	        xtime.getMinute(), xtime.getSecond());
#ifdef _WINDOWS
	mkdir(logpath.get());
#else
	mkdir(logpath.get(), 00666);
#endif
	rename(newfilename.get(), filenamebuf);

	PARANOIDMSG("Paranoid: Just been switched.  " << logfilename.get() << ", " <<
	            newfilename.get());
}


