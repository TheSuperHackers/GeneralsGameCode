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
*     $Archive: /Commando/Code/wwlib/WWCOMUtil.cpp $
*
* DESCRIPTION
*     COM utility functions and macros
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 2 $
*     $Modtime: 8/02/01 3:00p $
*
******************************************************************************/

#include "WWCOMUtil.h"

/******************************************************************************
*
* NAME
*     Dispatch_GetProperty
*
* DESCRIPTION
*
* INPUTS
*     Dispatch - Dispatch interface pointer.
*     Property - Name of property to get.
*     Value    - Property value.
*
* RESULT
*
******************************************************************************/

STDMETHODIMP Dispatch_GetProperty(IDispatch* object, const OLECHAR* propName,
		VARIANT* result)
	{
	result->vt = VT_EMPTY;
	result->lVal = 0;

	// Get the dispid for the named property
	OLECHAR* member = const_cast<OLECHAR*>(propName);
	DISPID dispid;
	HRESULT hr = object->GetIDsOfNames(IID_NULL, &member, 1,
		LOCALE_SYSTEM_DEFAULT, &dispid);

	if (SUCCEEDED(hr))
		{
		// Get the property
		DISPPARAMS params = {nullptr, nullptr, 0, 0};
		UINT argErr = 0;
		hr = object->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT,
			DISPATCH_PROPERTYGET, &params, result, nullptr, &argErr);
		}

	return hr;
	}



/******************************************************************************
*
* NAME
*     Dispatch_PutProperty
*
* DESCRIPTION
*
* INPUTS
*     Dispatch - Dispatch interface pointer.
*     Property - Name of property to put.
*     Value    - Property value.
*
* RESULT
*
******************************************************************************/

STDMETHODIMP Dispatch_PutProperty(IDispatch* object, const OLECHAR* propName,
		VARIANT* propValue)
	{
	// Get the dispid for the named property
	OLECHAR* member = const_cast<OLECHAR*>(propName);
	DISPID dispid;

	HRESULT hr = object->GetIDsOfNames(IID_NULL, &member, 1,
		LOCALE_SYSTEM_DEFAULT, &dispid);

	if (SUCCEEDED(hr))
		{
		// Get the property
		DISPPARAMS params = {nullptr, nullptr, 0, 0};
		params.cArgs = 1;
		params.rgvarg = propValue;

		VARIANT result;
		UINT argErr = 0;

		hr = object->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT,
			DISPATCH_PROPERTYPUT, &params, &result, nullptr, &argErr);
		}

	return hr;
	}


/******************************************************************************
*
* NAME
*     Dispatch_InvokeMethod
*
* DESCRIPTION
*
* INPUTS
*     Dispatch - Dispatch interface pointer.
*     Method   - Method name
*     Params   - Parameters
*     Result   - On return; result of method call
*
* RESULT
*
******************************************************************************/

STDMETHODIMP Dispatch_InvokeMethod(IDispatch* object, const OLECHAR* methodName,
		DISPPARAMS* params, VARIANT* result)
	{
	// Get the dispid for the named property
	OLECHAR* member = const_cast<OLECHAR*>(methodName);
	DISPID dispid;

	HRESULT hr = object->GetIDsOfNames(IID_NULL, &member, 1,
		LOCALE_SYSTEM_DEFAULT, &dispid);

	if (SUCCEEDED(hr))
		{
		UINT argErr = 0;
		hr = object->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT,
			DISPATCH_METHOD, params, result, nullptr, &argErr);
		}

	return hr;
	}


/******************************************************************************
*
* NAME
*     RegisterCOMServer
*
* DESCRIPTION
*     Register an in-process COM server DLL.
*
* INPUTS
*     DLLName - Name of DLL to register.
*
* RESULT
*     Success - True if operation successful.
*
******************************************************************************/

bool RegisterCOMServer(const char* dllName)
	{
	bool success = false;

	HINSTANCE hInst = LoadLibrary(dllName);

	if (hInst != nullptr)
		{
		FARPROC regServerProc = GetProcAddress(hInst, "DllRegisterServer");

		if (regServerProc != nullptr)
			{
			HRESULT hr = regServerProc();
			success = SUCCEEDED(hr);
			}

		FreeLibrary(hInst);
		}

	return success;
	}


/******************************************************************************
*
* NAME
*     UnregisterCOMServer
*
* DESCRIPTION
*     Unregister a in-process COM server DLL.
*
* INPUTS
*     DLLName - Name of DLL to unregister.
*
* RESULT
*     Success - True if operation successful.
*
******************************************************************************/

bool UnregisterCOMServer(const char* dllName)
	{
	bool success = false;

	HINSTANCE hInst = LoadLibrary(dllName);

	if (hInst != nullptr)
		{
		FARPROC unregServerProc = GetProcAddress(hInst, "DllUnregisterServer");

		if (unregServerProc != nullptr)
			{
			HRESULT hr = unregServerProc();
			success = SUCCEEDED(hr);
			}

		FreeLibrary(hInst);
		}

	return success;
	}
