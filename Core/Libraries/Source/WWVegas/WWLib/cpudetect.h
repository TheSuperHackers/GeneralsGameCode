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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : wwlib                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/cpudetect.h                            $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/29/02 4:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef WWLIB_CPU_DETECT_H__
#define WWLIB_CPU_DETECT_H__

#include "always.h"
#include "wwstring.h"

#ifdef WIN32
typedef signed __int64 sint64;
#elif defined (_UNIX)
typedef signed long long sint64;
#endif

class CPUDetectInitClass;

class CPUDetectClass
{
public:
	typedef enum
	{
		MANUFACTURER_UNKNOWN = 0,
		MANUFACTURER_INTEL,
		MANUFACTURER_UMC,
		MANUFACTURER_AMD,
		MANUFACTURER_CYRIX,
		MANUFACTURER_NEXTGEN,
		MANUFACTURER_VIA,
		MANUFACTURER_RISE,
		MANUFACTURER_TRANSMETA
	} ProcessorManufacturerType;

	typedef enum
	{
		INTEL_PROCESSOR_UNKNOWN,
		INTEL_PROCESSOR_80386,
		INTEL_PROCESSOR_80486DX,
		INTEL_PROCESSOR_80486SX,
		INTEL_PROCESSOR_80486DX2,
		INTEL_PROCESSOR_80486SL,
		INTEL_PROCESSOR_80486SX2,
		INTEL_PROCESSOR_80486DX2_WB,
		INTEL_PROCESSOR_80486DX4,
		INTEL_PROCESSOR_80486DX4_WB,
		INTEL_PROCESSOR_PENTIUM,
		INTEL_PROCESSOR_PENTIUM_OVERDRIVE,
		INTEL_PROCESSOR_PENTIUM_MMX,
		INTEL_PROCESSOR_PENTIUM_PRO_SAMPLE,
		INTEL_PROCESSOR_PENTIUM_PRO,
		INTEL_PROCESSOR_PENTIUM_II_OVERDRIVE,
		INTEL_PROCESSOR_PENTIUM_II_MODEL_3,
		INTEL_PROCESSOR_PENTIUM_II_MODEL_4,
		INTEL_PROCESSOR_CELERON_MODEL_5,
		INTEL_PROCESSOR_PENTIUM_II_MODEL_5,
		INTEL_PROCESSOR_PENTIUM_II_XEON_MODEL_5,
		INTEL_PROCESSOR_CELERON_MODEL_6,
		INTEL_PROCESSOR_PENTIUM_III_MODEL_7,
		INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_7,
		INTEL_PROCESSOR_CELERON_MODEL_8,
		INTEL_PROCESSOR_PENTIUM_III_MODEL_8,
		INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_8,
		INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_A,
		INTEL_PROCESSOR_PENTIUM_III_MODEL_B,
		INTEL_PROCESSOR_PENTIUM4
	} IntelProcessorType;

	typedef enum
	{
		AMD_PROCESSOR_UNKNOWN,
		AMD_PROCESSOR_486DX2,
		AMD_PROCESSOR_486DX4,
		AMD_PROCESSOR_5x86,
		AMD_PROCESSOR_486,
		AMD_PROCESSOR_K5_MODEL0,
		AMD_PROCESSOR_K5_MODEL1,
		AMD_PROCESSOR_K5_MODEL2,
		AMD_PROCESSOR_K5_MODEL3,
		AMD_PROCESSOR_K6_MODEL6,
		AMD_PROCESSOR_K6_MODEL7,
		AMD_PROCESSOR_K6_2_3DNOW_MODEL8,
		AMD_PROCESSOR_K6_3_3DNOW_MODEL9,
		AMD_PROCESSOR_K6_3_PLUS,
		AMD_PROCESSOR_K6,
		AMD_PROCESSOR_ATHLON_025,
		AMD_PROCESSOR_ATHLON_018,
		AMD_PROCESSOR_DURON,
		AMD_PROCESSOR_ATHLON_018_IL2,
		AMD_PROCESSOR_ATHLON
	} AMDProcessorType;

	typedef enum
	{
		VIA_PROCESSOR_UNKNOWN,
		VIA_PROCESSOR_IDT_C6_WINCHIP,
		VIA_PROCESSOR_IDT_C6_WINCHIP2,
		VIA_PROCESSOR_IDT_C6_WINCHIP3,
		VIA_PROCESSOR_CYRIX_III_SAMUEL
	} VIAProcessorType;

	typedef enum
	{
		RISE_PROCESSOR_UNKNOWN,
		RISE_PROCESSOR_DRAGON_025,
		RISE_PROCESSOR_DRAGON_018,
		RISE_PROCESSOR_DRAGON2_025,
		RISE_PROCESSOR_DRAGON2_018
	} RiseProcessorType;

	inline static ProcessorManufacturerType Get_Processor_Manufacturer() {return ProcessorManufacturer;}
	static const char* Get_Processor_Manufacturer_Name();

	inline static bool Has_CPUID_Instruction() { return HasCPUIDInstruction; }
	inline static bool Has_RDTSC_Instruction() { return HasRDTSCInstruction; }
	inline static bool Has_CMOV_Instruction() { return HasCMOVSupport; }
	inline static bool Has_MMX_Instruction_Set() { return HasMMXSupport; }
	inline static bool Has_SSE_Instruction_Set() { return HasSSESupport; }
	inline static bool Has_SSE2_Instruction_Set() { return HasSSE2Support; }
	inline static bool Has_3DNow_Instruction_Set() { return Has3DNowSupport; }
	inline static bool Has_Extended_3DNow_Instruction_Set() { return HasExtended3DNowSupport; }

	// Call these functions after determining the manufacturer to find out which of the manufacturers processors
	// the system has.
	inline static IntelProcessorType Get_Intel_Processor() { return IntelProcessor; }
	inline static AMDProcessorType Get_AMD_Processor() { return AMDProcessor; }
	inline static VIAProcessorType Get_VIA_Processor() { return VIAProcessor; }
	inline static RiseProcessorType Get_Rise_Processor() { return RiseProcessor; }

	// Note that processor speed is only calculated at start and could change during execution, so
	// this number is not to be relied on!
	inline static int Get_Processor_Speed() { return ProcessorSpeed; }
	inline static sint64 Get_Processor_Ticks_Per_Second() { return ProcessorTicksPerSecond; }	// Ticks per second
	inline static double Get_Inv_Processor_Ticks_Per_Second() { return InvProcessorTicksPerSecond; }	// 1.0 / Ticks per second

	static unsigned Get_Feature_Bits() { return FeatureBits; }
	static unsigned Get_Extended_Feature_Bits() { return ExtendedFeatureBits; }

	// L2 cache
	static unsigned Get_L2_Cache_Size() { return L2CacheSize; }
	static unsigned Get_L2_Cache_Line_Size() { return L2CacheLineSize; }
	static unsigned Get_L2_Cache_Set_Associative() { return L2CacheSetAssociative; }

	// L1 data cache
	static unsigned Get_L1_Data_Cache_Size() { return L1DataCacheSize; }
	static unsigned Get_L1_Data_Cache_Line_Size() { return L1DataCacheLineSize; }
	static unsigned Get_L1_Data_Cache_Set_Associative() { return L1DataCacheSetAssociative; }

	// L1 instruction cache
	static unsigned Get_L1_Instruction_Cache_Size() { return L1InstructionCacheSize; }
	static unsigned Get_L1_Instruction_Cache_Line_Size() { return L1InstructionCacheLineSize; }
	static unsigned Get_L1_Instruction_Cache_Set_Associative() { return L1InstructionCacheSetAssociative; }

	// L1 instruction trace cache
	static unsigned Get_L1_Instruction_Trace_Cache_Size() { return L1InstructionTraceCacheSize; }
	static unsigned Get_L1_Instruction_Trace_Cache_Set_Associative() { return L1InstructionTraceCacheSetAssociative; }

	// System memory
	
#if defined(_MSC_VER) && _MSC_VER < 1300
	static unsigned Get_Total_Physical_Memory() { return TotalPhysicalMemory; }
	static unsigned Get_Available_Physical_Memory() { return AvailablePhysicalMemory; }
	static unsigned Get_Total_Page_File_Size() { return TotalPageMemory; }
	static unsigned Get_Available_Page_File_Size() { return AvailablePageMemory; }
	static unsigned Get_Total_Virtual_Memory() { return TotalVirtualMemory; }
	static unsigned Get_Available_Virtual_Memory() { return AvailableVirtualMemory; }
#else
	static unsigned long long Get_Total_Physical_Memory() { return TotalPhysicalMemory; }
	static unsigned long long Get_Available_Physical_Memory() { return AvailablePhysicalMemory; }
	static unsigned long long Get_Total_Page_File_Size() { return TotalPageMemory; }
	static unsigned long long Get_Available_Page_File_Size() { return AvailablePageMemory; }
	static unsigned long long Get_Total_Virtual_Memory() { return TotalVirtualMemory; }
	static unsigned long long Get_Available_Virtual_Memory() { return AvailableVirtualMemory; }
#endif

	static unsigned Get_Processor_Type() { return ProcessorType; }

	inline static const char* Get_Processor_String() { return ProcessorString; }
	inline static const StringClass& Get_Processor_Log() { return ProcessorLog; }
	inline static const StringClass& Get_Compact_Log() { return CompactLog; }

	static bool CPUID(
		unsigned& u_eax_,
		unsigned& u_ebx_,
		unsigned& u_ecx_,
		unsigned& u_edx_,
		unsigned cpuid_type);

private:
//	static void Detect_CPU_Type();
	static void Init_CPUID_Instruction();
	static void Init_Processor_Speed();
	static void Init_Processor_String();
	static void Init_Processor_Manufacturer();
	static void Init_Processor_Family();
	static void Init_Processor_Features();
	static void Init_Memory();
	static void Init_OS();

	static void Init_Intel_Processor_Type();
	static void Init_AMD_Processor_Type();
	static void Init_VIA_Processor_Type();
	static void Init_Rise_Processor_Type();

	static void Init_Cache();

	static void Init_Processor_Log();
	static void Init_Compact_Log();

	static void Process_Cache_Info(unsigned value);
	static void Process_Extended_Cache_Info();

	friend class CPUDetectInitClass;

	static StringClass ProcessorLog;
	static StringClass CompactLog;

	static int ProcessorType;
	static int ProcessorFamily;
	static int ProcessorModel;
	static int ProcessorRevision;
	static int ProcessorSpeed;
	static sint64 ProcessorTicksPerSecond;	// Ticks per second
	static double InvProcessorTicksPerSecond;	// 1.0 / Ticks per second

	static ProcessorManufacturerType ProcessorManufacturer;
	static IntelProcessorType IntelProcessor;
	static AMDProcessorType AMDProcessor;
	static VIAProcessorType VIAProcessor;
	static RiseProcessorType RiseProcessor;

	static unsigned FeatureBits;
	static unsigned ExtendedFeatureBits;

	// L2 cache information
	static unsigned L2CacheSize;
	static unsigned L2CacheLineSize;
	static unsigned L2CacheSetAssociative;

	// L1 data cache information
	static unsigned L1DataCacheSize;
	static unsigned L1DataCacheLineSize;
	static unsigned L1DataCacheSetAssociative;

	// L1 instruction cache information
	static unsigned L1InstructionCacheSize;
	static unsigned L1InstructionCacheLineSize;
	static unsigned L1InstructionCacheSetAssociative;

	// L1 instruction trace cache information
	static unsigned L1InstructionTraceCacheSize;
	static unsigned L1InstructionTraceCacheSetAssociative;
#if defined(_MSC_VER) && _MSC_VER < 1300
	static unsigned TotalPhysicalMemory;
	static unsigned AvailablePhysicalMemory;
	static unsigned TotalPageMemory;
	static unsigned AvailablePageMemory;
	static unsigned TotalVirtualMemory;
	static unsigned AvailableVirtualMemory;
#else
	static unsigned long long TotalPhysicalMemory;
	static unsigned long long AvailablePhysicalMemory;
	static unsigned long long TotalPageMemory;
	static unsigned long long AvailablePageMemory;
	static unsigned long long TotalVirtualMemory;
	static unsigned long long AvailableVirtualMemory;
#endif
	static unsigned OSVersionNumberMajor;
	static unsigned OSVersionNumberMinor;
	static unsigned OSVersionBuildNumber;
	static unsigned OSVersionPlatformId;
	static StringClass OSVersionExtraInfo;

	static bool HasCPUIDInstruction;
	static bool HasRDTSCInstruction;
	static bool HasSSESupport;
	static bool HasSSE2Support;
	static bool HasCMOVSupport;
	static bool HasMMXSupport;
	static bool Has3DNowSupport;
	static bool HasExtended3DNowSupport;
	static char VendorID[20];
	static char ProcessorString[48];

};

struct CPUIDStruct
{
	unsigned Eax;
	unsigned Ebx;
	unsigned Ecx;
	unsigned Edx;
	CPUIDStruct(unsigned cpuid_type)
	{
		if (!CPUDetectClass::Has_CPUID_Instruction()) {
			Eax=Ebx=Ecx=Edx=0;
			return;
		}
		CPUDetectClass::CPUID(Eax,Ebx,Ecx,Edx,cpuid_type);
	}
};


#endif // WWLIB_CPU_DETECT_H__
