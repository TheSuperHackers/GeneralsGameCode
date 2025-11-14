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

/******************************************************************************
*
* NAME
*     $Archive: /Commando/Code/wwlib/WWCOMUtil.h $
*
* DESCRIPTION
*     COM utility functions and macros
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 3 $
*     $Modtime: 8/02/01 3:00p $
*
******************************************************************************/

#pragma once

#include <oaidl.h>

//! Invoke PropertyGet on IDispatch interface.
HRESULT STDMETHODCALLTYPE Dispatch_GetProperty(IDispatch* object,
		const OLECHAR* propName, VARIANT* result);

//! Invoke PropertyPut on IDispatch interface.
HRESULT STDMETHODCALLTYPE Dispatch_PutProperty(IDispatch* object,
		const OLECHAR* propName, VARIANT* propValue);

//! Invoke Method on IDispatch interface.
HRESULT STDMETHODCALLTYPE Dispatch_InvokeMethod(IDispatch* object,
		const OLECHAR* methodName, DISPPARAMS* params, VARIANT* result);

//! Register COM in-process DLL server
bool RegisterCOMServer(const char* dllName);

//! Unregister COM in-process DLL server
bool UnregisterCOMServer(const char* dllName);
