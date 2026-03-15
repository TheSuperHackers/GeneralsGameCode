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



int MsgManager::setAllStreams(OutputDevice *device)
{
  if (device==nullptr)
    return(1);


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

  return(0);
}


int MsgManager::setDebugStream(OutputDevice *device)
{
  if (device==nullptr)
    return(1);


  debug_streamer.setOutputDevice(device);
  delete(debug_ostream);
  debug_ostream=new ostream(&debug_streamer);

  return(0);
}

int MsgManager::setInfoStream(OutputDevice *device)
{
  if (device==nullptr)
    return(1);


  info_streamer.setOutputDevice(device);
  delete(info_ostream);
  info_ostream=new ostream(&info_streamer);

  return(0);
}

int MsgManager::setWarnStream(OutputDevice *device)
{
  if (device==nullptr)
    return(1);


  warn_streamer.setOutputDevice(device);
  delete(warn_ostream);
  warn_ostream=new ostream(&warn_streamer);

  return(0);
}

int MsgManager::setErrorStream(OutputDevice *device)
{
  if (device==nullptr)
    return(1);


  error_streamer.setOutputDevice(device);
  delete(error_ostream);
  error_ostream=new ostream(&error_streamer);

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
