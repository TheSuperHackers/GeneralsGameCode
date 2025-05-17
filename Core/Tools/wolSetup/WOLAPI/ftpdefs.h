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

#ifndef __FTPDEFS_H_INCLUDED__
#define __FTPDEFS_H_INCLUDED__


// CFtp return codes.

#define FTP_SUCCEEDED		S_OK
#define	FTP_FAILED			MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 1 )
#define FTP_TRYING			MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 2 )


#endif
