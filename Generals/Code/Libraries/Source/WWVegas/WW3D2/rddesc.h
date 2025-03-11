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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/rddesc.h                               $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/19/01 11:43a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef RDDESC_H
#define RDDESC_H

#include "Vector.H"
#include <stdlib.h>
#include <string.h>

class ResolutionDescClass
{
public:
	ResolutionDescClass(void) : Width(0), Height(0), BitDepth(0) { }
	ResolutionDescClass(int w,int h,int bits) : Width(w), Height(h), BitDepth(bits) { }
	bool operator == (const ResolutionDescClass & src) { return ((Width==src.Width) && (Height==src.Height) && (BitDepth==src.BitDepth)); }
	bool operator != (const ResolutionDescClass & src) { return ((Width!=src.Width) || (Height!=src.Height) || (BitDepth!=src.BitDepth)); }
	
	int			Width;
	int			Height;
	int			BitDepth;
	int			RefreshRate;
};


class RenderDeviceDescClass
{

public:

	RenderDeviceDescClass(void) : DeviceName(NULL), DeviceVendor(NULL), DevicePlatform(NULL), 
											DriverName(NULL), DriverVendor(NULL), DriverVersion(NULL),
											HardwareName(NULL), HardwareVendor(NULL), HardwareChipset(NULL)
	{
	}

	~RenderDeviceDescClass(void)
	{
		if (DeviceName) { free(DeviceName); }
		if (DeviceVendor) { free(DeviceVendor); }
		if (DevicePlatform) { free(DevicePlatform); }
		if (DriverName) { free(DriverName); }
		if (DriverVendor) { free(DriverVendor); }
		if (DriverVersion) { free(DriverVersion); }
		if (HardwareName) { free(HardwareName); }
		if (HardwareVendor) { free(HardwareVendor); }
		if (HardwareChipset) { free(HardwareChipset); }
	}

	RenderDeviceDescClass & operator = (const RenderDeviceDescClass & src) 
	{
		set_device_name(src.Get_Device_Name());
		set_device_vendor(src.Get_Device_Vendor());
		set_device_platform(src.Get_Device_Platform());
		set_driver_name(src.Get_Driver_Name());
		set_driver_vendor(src.Get_Driver_Vendor());
		set_driver_version(src.Get_Driver_Version());
		set_hardware_name(src.Get_Hardware_Name());
		set_hardware_vendor(src.Get_Hardware_Vendor());
		set_hardware_chipset(src.Get_Hardware_Chipset());
		ResArray = src.ResArray;
		return *this;
	}	

	bool operator == (const RenderDeviceDescClass & /*src*/) { return false; }
	bool operator != (const RenderDeviceDescClass & /*src*/) { return true; }

	const char *		Get_Device_Name() const			{ return DeviceName; }
	const char *		Get_Device_Vendor() const		{ return DeviceVendor; }
	const char *		Get_Device_Platform() const	{ return DevicePlatform; }

	const char *		Get_Driver_Name() const			{ return DriverName; }
	const char *		Get_Driver_Vendor() const		{ return DriverVendor; }
	const char *		Get_Driver_Version() const		{ return DriverVersion; }

	const char *		Get_Hardware_Name() const		{ return HardwareName; }
	const char *		Get_Hardware_Vendor() const	{ return HardwareVendor; }
	const char *		Get_Hardware_Chipset() const	{ return HardwareChipset; }

	const DynamicVectorClass<ResolutionDescClass> & Enumerate_Resolutions(void) const	{ return ResArray; }

private:

	void set_device_name(const char * name)		{ if (DeviceName) { free(DeviceName); }				DeviceName = NULL;			if (name) DeviceName = strdup(name); }
	void set_device_vendor(const char * name)		{ if (DeviceVendor) { free(DeviceVendor); }			DeviceVendor = NULL;		if (name) DeviceVendor = strdup(name); }
	void set_device_platform(const char * name)	{ if (DevicePlatform) { free(DevicePlatform); }		DevicePlatform = NULL;		if (name) DevicePlatform = strdup(name); }
	void set_driver_name(const char * name)		{ if (DriverName) { free(DriverName); }				DriverName = NULL;			if (name) DriverName = strdup(name); }
	void set_driver_vendor(const char * name)		{ if (DriverVendor) { free(DriverVendor); }			DriverVendor = NULL;			if (name) DriverVendor = strdup(name); }
	void set_driver_version(const char * name)	{ if (DriverVersion) { free(DriverVersion); }		DriverVersion = NULL;		if (name) DriverVersion = strdup(name); }
	void set_hardware_name(const char * name)		{ if (HardwareName) { free(HardwareName); }			HardwareName = NULL;			if (name) HardwareName = strdup(name); }
	void set_hardware_vendor(const char * name)	{ if (HardwareVendor) { free(HardwareVendor); }		HardwareVendor = NULL;		if (name) HardwareVendor = strdup(name); }
	void set_hardware_chipset(const char * name)	{ if (HardwareChipset) { free(HardwareChipset); }	HardwareChipset = NULL;		if (name) HardwareChipset = strdup(name); }

	void reset_resolution_list(void)					{ ResArray.Delete_All(); }
	void add_resolution(int w,int h,int bits);

	char *				DeviceName;
	char *				DeviceVendor;
	char *				DevicePlatform;

	char *				DriverName;
	char *				DriverVendor;
	char *				DriverVersion;

	char *				HardwareName;
	char *				HardwareVendor;
	char *				HardwareChipset;
	
	DynamicVectorClass<ResolutionDescClass>	ResArray;

	friend class WW3D;
	friend class DX8Wrapper;
};


inline void RenderDeviceDescClass::add_resolution(int w,int h,int bits)		
{ 
	bool found = false;
	for (int i=0; i<ResArray.Count(); i++) {
		if (	(ResArray[i].Width == w) &&
				(ResArray[i].Height == h) &&
				(ResArray[i].BitDepth == bits))
		{
			found = true;
		}
	}
	
	if (!found) {
		ResArray.Add(ResolutionDescClass(w,h,bits)); 
	}
}


#endif 

