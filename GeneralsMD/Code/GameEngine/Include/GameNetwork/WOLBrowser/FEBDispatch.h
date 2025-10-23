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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//
// FEBDispatch class is a template class which, when inherited from, can implement the
// IDispatch for a COM object with a type library.
//

#pragma once

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <comutil.h>    // For _bstr_t.

#include "oleauto.h"

template <class T, class C, const IID *I>
class FEBDispatch :
public CComObjectRootEx<CComSingleThreadModel>,
public CComCoClass<T>,
public C
{
public:

	BEGIN_COM_MAP(T)
		COM_INTERFACE_ENTRY(C)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IDispatch, m_dispatch)
	END_COM_MAP()

		FEBDispatch()
	{
		m_ptinfo = NULL;
		m_dispatch = NULL;

		ITypeLib *ptlib;
		HRESULT hr;
		HRESULT TypeLibraryLoadResult;
		char filename[256];

		GetModuleFileName(NULL, filename, sizeof(filename));
		_bstr_t bstr(filename);

		TypeLibraryLoadResult = LoadTypeLib(bstr, &ptlib);

		DEBUG_ASSERTCRASH(TypeLibraryLoadResult == 0, ("Can't load type library for Embedded Browser"));

		if (TypeLibraryLoadResult == S_OK)
		{
			hr = ptlib->GetTypeInfoOfGuid(*I, &m_ptinfo);
			ptlib->Release();

			if (hr == S_OK)
			{
				hr = CreateStdDispatch(static_cast<IUnknown*>(this), static_cast<C*>(this), m_ptinfo, &m_dispatch);

				m_dispatch->AddRef();
				// Don't release the IUnknown from CreateStdDispatch without calling AddRef.
				// It looks like CreateStdDispatch doesn't call AddRef on the IUnknown it returns.
			}
		}

		if ( m_dispatch == NULL )
		{
			DEBUG_LOG(("Error creating Dispatch for Web interface"));
		}
	}

	virtual ~FEBDispatch()
	{
		if (m_ptinfo)
			m_ptinfo->Release();

		if (m_dispatch)
			m_dispatch->Release();
	}

	IUnknown *m_dispatch;

private:
	ITypeInfo *m_ptinfo;
};
