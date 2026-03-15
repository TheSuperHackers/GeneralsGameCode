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

#include <stdlib.h>
#include "mydebug.h"
#include "streamer.h"
#include "odevice.h"


// static MyMsgManager         *msg_manager=nullptr;

// static int                paranoid_enabled=0;
static ostream           *paranoid_ostream=nullptr;
static Streamer           paranoid_streamer;

// Don't dare touch this semaphore in application code!
#ifdef USE_SEM
Sem4                      MyDebugLibSemaphore;
#else
CritSec                      MyDebugLibSemaphore;
#endif


int MyMsgManager::setAllStreams(OutputDevice *device)
{
	if (device==nullptr)
		return(1);

	MYDEBUGLOCK;
	paranoid_streamer.setOutputDevice(device);
	delete(paranoid_ostream);
	paranoid_ostream=new ostream(&paranoid_streamer);

	MYDEBUGUNLOCK;

	return(0);
}


int MyMsgManager::setParanoidStream(OutputDevice *device)
{
	if (device==nullptr)
		return(1);

	MYDEBUGLOCK;
	paranoid_streamer.setOutputDevice(device);
	delete(paranoid_ostream);
	paranoid_ostream=new ostream(&paranoid_streamer);
	MYDEBUGUNLOCK;

	return(0);
}




ostream *MyMsgManager::paranoidStream(void)
{
	return(paranoid_ostream);
}


int MyMsgManager::ReplaceAllStreams(FileD * output_device, const char *device_filename, const char *copy_filename)
{
	MYDEBUGLOCK;

	delete(paranoid_ostream);
	delete(output_device);

	rename(device_filename, copy_filename);

	//      FileD new_device(device_filename);
	output_device = new FileD(device_filename);

	paranoid_streamer.setOutputDevice(output_device);
	paranoid_ostream = new ostream(&paranoid_streamer);

	MYDEBUGUNLOCK;

	return(0);
}
