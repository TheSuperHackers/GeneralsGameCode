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
#include "wdebug.h"
#include "streamer.h"
#include "odevice.h"


static MsgManager         *msg_manager=nullptr;

static int                debug_enabled=0;
static ostream           *debug_ostream=nullptr;
static Streamer           debug_streamer;

static int                info_enabled=0;
static ostream           *info_ostream=nullptr;
static Streamer           info_streamer;

static int                warn_enabled=0;
static ostream           *warn_ostream=nullptr;
static Streamer           warn_streamer;

static int                error_enabled=0;
static ostream           *error_ostream=nullptr;
static Streamer           error_streamer;


// Don't dare touch this semaphore in application code!
#ifdef USE_DEBUG_SEM
Sem4                      DebugLibSemaphore;
#else
CritSec                      DebugLibSemaphore;
#endif


int MsgManager::setAllStreams(OutputDevice *device)
{
  if (device==nullptr)
    return(1);

  DEBUGLOCK;
  debug_streamer.setOutputDevice(device);
  delete(debug_ostream);
  debug_ostream=new ostream(&debug_streamer);

  info_streamer.setOutputDevice(device);
  delete(info_ostream);
  info_ostream=new ostream(&info_streamer);

  warn_streamer.setOutputDevice(device);
  delete(warn_ostream);
  warn_ostream=new ostream(&warn_streamer);

  error_streamer.setOutputDevice(device);
  delete(error_ostream);
  error_ostream=new ostream(&error_streamer);

  DEBUGUNLOCK;

  return(0);
}


int MsgManager::ReplaceAllStreams(FileD * output_device, IN char *device_filename, IN char *copy_filename)
{
	DebugLibSemaphore.Wait();

	delete(debug_ostream);
	delete(info_ostream);
	delete(warn_ostream);
	delete(error_ostream);

	if (output_device != nullptr)
	{
		delete(output_device);
		output_device = nullptr;
	}

	rename(device_filename, copy_filename);

//	FileD new_device(device_filename);
	output_device = new FileD(device_filename);

	debug_streamer.setOutputDevice(output_device);
	debug_ostream = new ostream(&debug_streamer);

	info_streamer.setOutputDevice(output_device);
	info_ostream=new ostream(&info_streamer);

	warn_streamer.setOutputDevice(output_device);
	warn_ostream = new ostream(&warn_streamer);

	error_streamer.setOutputDevice(output_device);
	error_ostream = new ostream(&error_streamer);

	DebugLibSemaphore.Post();

	return(0);
}


int MsgManager::setDebugStream(OutputDevice *device)
{
  if (device==nullptr)
    return(1);

  DEBUGLOCK;
  debug_streamer.setOutputDevice(device);
  delete(debug_ostream);
  debug_ostream=new ostream(&debug_streamer);
  DEBUGUNLOCK;
  return(0);
}

int MsgManager::setInfoStream(OutputDevice *device)
{
  if (device==nullptr)
    return(1);

  DEBUGLOCK;
  info_streamer.setOutputDevice(device);
  delete(info_ostream);
  info_ostream=new ostream(&info_streamer);
  DEBUGUNLOCK;
  return(0);
}

int MsgManager::setWarnStream(OutputDevice *device)
{
  if (device==nullptr)
    return(1);

  DEBUGLOCK;
  warn_streamer.setOutputDevice(device);
  delete(warn_ostream);
  warn_ostream=new ostream(&warn_streamer);
  DEBUGUNLOCK;
  return(0);
}

int MsgManager::setErrorStream(OutputDevice *device)
{
  if (device==nullptr)
    return(1);

  DEBUGLOCK;
  error_streamer.setOutputDevice(device);
  delete(error_ostream);
  error_ostream=new ostream(&error_streamer);
  DEBUGUNLOCK;
  return(0);
}



ostream *MsgManager::debugStream(void)
{
  return(debug_ostream);
}

ostream *MsgManager::infoStream(void)
{
  return(info_ostream);
}

ostream *MsgManager::warnStream(void)
{
  return(warn_ostream);
}

ostream *MsgManager::errorStream(void)
{
  return(error_ostream);
}
