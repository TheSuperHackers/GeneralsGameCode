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

#include "cpudetect.h"
#include "wwstring.h"
#include "wwdebug.h"
#include "thread.h"
#pragma warning (disable : 4201)	// Nonstandard extension - nameless struct
#include "systimer.h"
#include <Utility/intrin_compat.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _UNIX
# include <time.h>  // for time(), localtime() and timezone variable.
#endif

struct OSInfoStruct {
	const char* Code;
	const char* SubCode;
	const char* VersionString;
	unsigned char VersionMajor;
	unsigned char VersionMinor;
	unsigned short VersionSub;
	unsigned char BuildMajor;
	unsigned char BuildMinor;
	unsigned short BuildSub;
};

static void Get_OS_Info(
	OSInfoStruct& os_info,
	unsigned OSVersionPlatformId,
	unsigned OSVersionNumberMajor,
	unsigned OSVersionNumberMinor,
	unsigned OSVersionBuildNumber);


StringClass CPUDetectClass::ProcessorLog;
StringClass CPUDetectClass::CompactLog;

int CPUDetectClass::ProcessorType;
int CPUDetectClass::ProcessorFamily;
int CPUDetectClass::ProcessorModel;
int CPUDetectClass::ProcessorRevision;
int CPUDetectClass::ProcessorSpeed;
sint64 CPUDetectClass::ProcessorTicksPerSecond;	// Ticks per second
double CPUDetectClass::InvProcessorTicksPerSecond;	// 1.0 / Ticks per second

unsigned CPUDetectClass::FeatureBits;
unsigned CPUDetectClass::ExtendedFeatureBits;

unsigned CPUDetectClass::L2CacheSize;
unsigned CPUDetectClass::L2CacheLineSize;
unsigned CPUDetectClass::L2CacheSetAssociative;
unsigned CPUDetectClass::L1DataCacheSize;
unsigned CPUDetectClass::L1DataCacheLineSize;
unsigned CPUDetectClass::L1DataCacheSetAssociative;
unsigned CPUDetectClass::L1InstructionCacheSize;
unsigned CPUDetectClass::L1InstructionCacheLineSize;
unsigned CPUDetectClass::L1InstructionCacheSetAssociative;
unsigned CPUDetectClass::L1InstructionTraceCacheSize;
unsigned CPUDetectClass::L1InstructionTraceCacheSetAssociative;

#if defined(_MSC_VER) && _MSC_VER < 1300
unsigned CPUDetectClass::TotalPhysicalMemory;
unsigned CPUDetectClass::AvailablePhysicalMemory;
unsigned CPUDetectClass::TotalPageMemory;
unsigned CPUDetectClass::AvailablePageMemory;
unsigned CPUDetectClass::TotalVirtualMemory;
unsigned CPUDetectClass::AvailableVirtualMemory;
#else
unsigned long long CPUDetectClass::TotalPhysicalMemory;
unsigned long long CPUDetectClass::AvailablePhysicalMemory;
unsigned long long CPUDetectClass::TotalPageMemory;
unsigned long long CPUDetectClass::AvailablePageMemory;
unsigned long long CPUDetectClass::TotalVirtualMemory;
unsigned long long CPUDetectClass::AvailableVirtualMemory;
#endif

unsigned CPUDetectClass::OSVersionNumberMajor;
unsigned CPUDetectClass::OSVersionNumberMinor;
unsigned CPUDetectClass::OSVersionBuildNumber;
unsigned CPUDetectClass::OSVersionPlatformId;
StringClass CPUDetectClass::OSVersionExtraInfo;

bool CPUDetectClass::HasCPUIDInstruction=false;
bool CPUDetectClass::HasRDTSCInstruction=false;
bool CPUDetectClass::HasSSESupport=false;
bool CPUDetectClass::HasSSE2Support=false;
bool CPUDetectClass::HasCMOVSupport=false;
bool CPUDetectClass::HasMMXSupport=false;
bool CPUDetectClass::Has3DNowSupport=false;
bool CPUDetectClass::HasExtended3DNowSupport=false;

CPUDetectClass::ProcessorManufacturerType CPUDetectClass::ProcessorManufacturer = CPUDetectClass::MANUFACTURER_UNKNOWN;
CPUDetectClass::IntelProcessorType CPUDetectClass::IntelProcessor;
CPUDetectClass::AMDProcessorType CPUDetectClass::AMDProcessor;
CPUDetectClass::VIAProcessorType CPUDetectClass::VIAProcessor;
CPUDetectClass::RiseProcessorType CPUDetectClass::RiseProcessor;

char CPUDetectClass::VendorID[20];
char CPUDetectClass::ProcessorString[48];

const char* CPUDetectClass::Get_Processor_Manufacturer_Name()
{
	static const char* const ManufacturerNames[] = {
		"<Unknown>",
		"Intel",
		"UMC",
		"AMD",
		"Cyrix",
		"NextGen",
		"VIA",
		"Rise",
		"Transmeta"
	};
	static_assert(ARRAY_SIZE(ManufacturerNames) == MANUFACTURER_COUNT, "Incorrect array size");

	return ManufacturerNames[ProcessorManufacturer];
}

#define ASM_RDTSC _asm _emit 0x0f _asm _emit 0x31

static unsigned Calculate_Processor_Speed(sint64& ticks_per_second)
{
	sint64 timer0=0;
	sint64 timer1=0;

	timer0=_rdtsc();

	unsigned start=TIMEGETTIME();
	unsigned elapsed;
	while ((elapsed=TIMEGETTIME()-start)<200) {
		timer1=_rdtsc();
	}

	sint64 t=timer1-timer0;
	ticks_per_second=(sint64)((1000.0/(double)elapsed)*(double)t);	// Ticks per second
	return unsigned((double)t/(double)(elapsed*1000));
}

void CPUDetectClass::Init_Processor_Speed()
{
	if (!Has_RDTSC_Instruction()) {
		ProcessorSpeed=0;
		return;
	}

	// Loop until two subsequent samples are within 5% of each other (max 5 iterations).
	unsigned speed1=Calculate_Processor_Speed(ProcessorTicksPerSecond);
	unsigned total_speed=speed1;
	for (int i=0;i<5;++i) {
		unsigned speed2=Calculate_Processor_Speed(ProcessorTicksPerSecond);
		float rel=float(speed1)/float(speed2);
		if (rel>=0.95f && rel<=1.05f) {
			ProcessorSpeed=(speed1+speed2)/2;
			InvProcessorTicksPerSecond=1.0/double(ProcessorTicksPerSecond);
			return;
		}
		speed1=speed2;
		total_speed+=speed2;
	}
	// If no two subsequent samples where close enough, use intermediate
	ProcessorSpeed=total_speed/6;
	InvProcessorTicksPerSecond=1.0/double(ProcessorTicksPerSecond);
}

void CPUDetectClass::Init_Processor_Manufacturer()
{
	VendorID[0] = 0;
	unsigned max_cpuid;
	CPUID(
		max_cpuid,
		(unsigned&)VendorID[0],
		(unsigned&)VendorID[8],
		(unsigned&)VendorID[4],
		0);

	ProcessorManufacturer = MANUFACTURER_UNKNOWN;

	if (stricmp(VendorID, "GenuineIntel") == 0) ProcessorManufacturer = MANUFACTURER_INTEL;
	else if (stricmp(VendorID, "AuthenticAMD") == 0) ProcessorManufacturer = MANUFACTURER_AMD;
	else if (stricmp(VendorID, "AMD ISBETTER") == 0) ProcessorManufacturer = MANUFACTURER_AMD;
	else if (stricmp(VendorID, "UMC UMC UMC") == 0) ProcessorManufacturer = MANUFACTURER_UMC;
	else if (stricmp(VendorID, "CyrixInstead") == 0) ProcessorManufacturer = MANUFACTURER_CYRIX;
	else if (stricmp(VendorID, "NexGenDriven") == 0) ProcessorManufacturer = MANUFACTURER_NEXTGEN;
	else if (stricmp(VendorID, "CentaurHauls") == 0) ProcessorManufacturer = MANUFACTURER_VIA;
	else if (stricmp(VendorID, "RiseRiseRise") == 0) ProcessorManufacturer = MANUFACTURER_RISE;
	else if (stricmp(VendorID, "GenuineTMx86") == 0) ProcessorManufacturer = MANUFACTURER_TRANSMETA;
}

void CPUDetectClass::Process_Cache_Info(unsigned value)
{
	switch (value) {
	case 0x00: // Null
		break;
	case 0x01: // Instruction TLB, 4K pages, 4-way set associative, 32 entries
		break;
	case 0x02: // Instruction TLB, 4M pages, fully associative, 2 entries
		break;
	case 0x03: // Data TLB, 4K pages, 4-way set associative, 64 entries
		break;
	case 0x04: // Data TLB, 4M pages, 4-way set associative, 8 entries
		break;
	case 0x06: // Instruction cache, 8K, 4-way set associative, 32 byte line size
		L1InstructionCacheSize=8*1024;
		L1InstructionCacheLineSize=32;
		L1InstructionCacheSetAssociative=4;
		break;
	case 0x08: // Instruction cache 16K, 4-way set associative, 32 byte line size
		L1InstructionCacheSize=16*1024;
		L1InstructionCacheLineSize=32;
		L1InstructionCacheSetAssociative=4;
		break;
	case 0x0A: // Data cache, 8K, 2-way set associative, 32 byte line size
		L1DataCacheSize=8*1024;
		L1DataCacheLineSize=32;
		L1DataCacheSetAssociative=2;
		break;
	case 0x0C: // Data cache, 16K, 4-way set associative, 32 byte line size
		L1DataCacheSize=16*1024;
		L1DataCacheLineSize=32;
		L1DataCacheSetAssociative=4;
		break;
	case 0x40: // No L2 cache (P6 family), or No L3 cache (Pentium 4 processor)
		// Nice of Intel, Pentium4 has an exception and this field is defined as "no L3 cache"
		if (ProcessorManufacturer==MANUFACTURER_INTEL && ProcessorFamily==0xf) {
		}
		else {
			L2CacheSize=0;
			L2CacheLineSize=0;
			L2CacheSetAssociative=0;
		}
		break;
	case 0x41: // Unified cache, 32 byte cache line,4-way set associative, 128K
		L2CacheSize=128*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=4;
		break;
	case 0x42: // Unified cache, 32 byte cache line, 4-way set associative, 256K
		L2CacheSize=256*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=4;
		break;
	case 0x43: // Unified cache, 32 byte cache line, 4-way set associative, 512K
		L2CacheSize=512*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=4;
		break;
	case 0x44: // Unified cache, 32 byte cache line, 4-way set associative, 1M
		L2CacheSize=1024*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=4;
		break;
	case 0x45: // Unified cache, 32 byte cache line, 4-way set associative, 2M
		L2CacheSize=2048*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=4;
		break;
	case 0x50: // Instruction TLB, 4K, 2M or 4M pages, fully associative, 64 entries
		break;
	case 0x51: // Instruction TLB, 4K, 2M or 4M pages, fully associative, 128 entries
		break;
	case 0x52: // Instruction TLB, 4K, 2M or 4M pages, fully associative, 256 entries
		break;
	case 0x5B: // Data TLB, 4K or 4M pages, fully associative, 64 entries
		break;
	case 0x5C: // Data TLB, 4K or 4M pages, fully associative, 128 entries
		break;
	case 0x5D: // Data TLB, 4K or 4M pages, fully associative, 256 entries
		break;
	case 0x66: // Data cache, sectored, 64 byte cache line, 4 way set associative, 8K
		L1DataCacheSize=8*1024;
		L1DataCacheLineSize=64;
		L1DataCacheSetAssociative=4;
		break;
	case 0x67: // Data cache, sectored, 64 byte cache line, 4 way set associative, 16K
		L1DataCacheSize=16*1024;
		L1DataCacheLineSize=64;
		L1DataCacheSetAssociative=4;
		break;
	case 0x68: // Data cache, sectored, 64 byte cache line, 4 way set associative, 32K
		L1DataCacheSize=32*1024;
		L1DataCacheLineSize=64;
		L1DataCacheSetAssociative=4;
		break;
	case 0x70: // Instruction Trace cache, 8 way set associative, 12K uOps
		L1InstructionTraceCacheSize=12*1024;
		L1InstructionTraceCacheSetAssociative=8;
		break;
	case 0x71: // Instruction Trace cache, 8 way set associative, 16K uOps
		L1InstructionTraceCacheSize=16*1024;
		L1InstructionTraceCacheSetAssociative=8;
		break;
	case 0x72: // Instruction Trace cache, 8 way set associative, 32K uOps
		L1InstructionTraceCacheSize=32*1024;
		L1InstructionTraceCacheSetAssociative=8;
		break;
	case 0x79: // Unified cache, sectored, 64 byte cache line, 8 way set associative, 128K
		L2CacheSize=128*1024;
		L2CacheLineSize=64;
		L2CacheSetAssociative=8;
		break;
	case 0x7A: // Unified cache, sectored, 64 byte cache line, 8 way set associative, 256K
		L2CacheSize=256*1024;
		L2CacheLineSize=64;
		L2CacheSetAssociative=8;
		break;
	case 0x7B: // Unified cache, sectored, 64 byte cache line, 8 way set associative, 512K
		L2CacheSize=512*1024;
		L2CacheLineSize=64;
		L2CacheSetAssociative=8;
		break;
	case 0x7C: // Unified cache, sectored, 64 byte cache line, 8 way set associative, 1M
		L2CacheSize=1024*1024;
		L2CacheLineSize=64;
		L2CacheSetAssociative=8;
		break;
	case 0x82: // Unified cache, 32 byte cache line, 8 way set associative, 256K
		L2CacheSize=256*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=8;
		break;
	case 0x83: // Unified cache, 32 byte cache line, 8 way set associative, 512K
		L2CacheSize=512*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=8;
		break;
	case 0x84: // Unified cache, 32 byte cache line, 8 way set associative, 1M
		L2CacheSize=1024*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=8;
		break;
	case 0x85: // Unified cache, 32 byte cache line, 8 way set associative, 2M
		L2CacheSize=2048*1024;
		L2CacheLineSize=32;
		L2CacheSetAssociative=8;
		break;
	}
}

void CPUDetectClass::Process_Extended_Cache_Info()
{
	CPUIDStruct max_ext(0x80000000);
	if (max_ext.Eax>=0x80000006) {
		CPUIDStruct l1info(0x80000005);
		CPUIDStruct l2info(0x80000006);

		// L1 data cache
		L1DataCacheLineSize=l1info.Ecx&0xff;
		L1DataCacheSetAssociative=(l1info.Ecx>>16)&0xff;
		L1DataCacheSize=((l1info.Ecx>>24)&0xff)*1024;

		// L1 instruction cache
		L1InstructionCacheLineSize=l1info.Edx&0xff;
		L1InstructionCacheSetAssociative=(l1info.Edx>>16)&0xff;
		L1InstructionCacheSize=((l1info.Edx>>24)&0xff)*1024;

		// L2 cache
		L2CacheSize=((l2info.Ecx>>16)&0xffff)*1024;
		L2CacheLineSize=l2info.Ecx&0xff;
		switch ((l2info.Ecx>>12)&0xf) {
		case 0: L2CacheSetAssociative=0; break;
		case 1: L2CacheSetAssociative=1; break;
		case 2: L2CacheSetAssociative=2; break;
		case 4: L2CacheSetAssociative=4; break;
		case 6: L2CacheSetAssociative=8; break;
		case 8: L2CacheSetAssociative=16; break;
		case 15: L2CacheSetAssociative=0xff; break;
		}

	}
}

void CPUDetectClass::Init_Cache()
{
	CPUIDStruct cache_id(2);
	// This code can only determine cache sizes for cpuid count 1 (see Intel manuals for more info on the subject)
	if ((cache_id.Eax&0xff)==1) {
		if (!(cache_id.Eax&0x80000000)) {
			Process_Cache_Info((cache_id.Eax>>24)&0xff);
			Process_Cache_Info((cache_id.Eax>>16)&0xff);
			Process_Cache_Info((cache_id.Eax>>8)&0xff);
		}
		if (!(cache_id.Ebx&0x80000000)) {
			Process_Cache_Info((cache_id.Ebx>>24)&0xff);
			Process_Cache_Info((cache_id.Ebx>>16)&0xff);
			Process_Cache_Info((cache_id.Ebx>>8)&0xff);
			Process_Cache_Info((cache_id.Ebx)&0xff);
		}
		if (!(cache_id.Ecx&0x80000000)) {
			Process_Cache_Info((cache_id.Ecx>>24)&0xff);
			Process_Cache_Info((cache_id.Ecx>>16)&0xff);
			Process_Cache_Info((cache_id.Ecx>>8)&0xff);
			Process_Cache_Info((cache_id.Ecx)&0xff);
		}
		if (!(cache_id.Edx&0x80000000)) {
			Process_Cache_Info((cache_id.Edx>>24)&0xff);
			Process_Cache_Info((cache_id.Edx>>16)&0xff);
			Process_Cache_Info((cache_id.Edx>>8)&0xff);
			Process_Cache_Info((cache_id.Edx)&0xff);
		}
	}
	else {
		// If standard (Intel) cache information not found, try extended info.
		Process_Extended_Cache_Info();
	}
}

void CPUDetectClass::Init_Intel_Processor_Type()
{
	switch (ProcessorFamily) {
	case 3:
		IntelProcessor=INTEL_PROCESSOR_80386;
		break;
	case 4:
		switch (ProcessorModel) {
		default:
		case 0:
		case 1:
			IntelProcessor=INTEL_PROCESSOR_80486DX;
			break;
		case 2:
			IntelProcessor=INTEL_PROCESSOR_80486SX;
			break;
		case 3:
			IntelProcessor=INTEL_PROCESSOR_80486DX2;
			break;
		case 4:
			IntelProcessor=INTEL_PROCESSOR_80486SL;
			break;
		case 5:
			IntelProcessor=INTEL_PROCESSOR_80486SX2;
			break;
		case 7:
			IntelProcessor=INTEL_PROCESSOR_80486DX2_WB;
			break;
		case 8:
			IntelProcessor=INTEL_PROCESSOR_80486DX4;
			break;
		case 9:
			IntelProcessor=INTEL_PROCESSOR_80486DX4_WB;
			break;
		}
		break;
	case 5:
		switch (ProcessorModel) {
		default:
		case 0:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM; // Early model
			break;
		case 1:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM; // 60/66, 5V
			break;
		case 2:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM; // 75+, 3.xV
			break;
		case 3:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_OVERDRIVE;
			break;
		case 4:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_MMX;
			break;
		case 5:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_OVERDRIVE;
			break;
		case 6:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_OVERDRIVE;
			break;
		case 7:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM;
			break;
		case 8:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_MMX;
			break;
		}
	case 6:
		switch (ProcessorModel) {
		default:
		case 0:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_PRO_SAMPLE;
			break;
		case 1:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_PRO;
			break;
		case 3:
			if (ProcessorType==0) {
				IntelProcessor=INTEL_PROCESSOR_PENTIUM_II_MODEL_3;
			}
			else {
				IntelProcessor=INTEL_PROCESSOR_PENTIUM_II_OVERDRIVE;
			}
			break;
		case 4:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_II_MODEL_4;
			break;
		case 5:
			// This one is either PII, PIIXeon or Celeron with no L2 cache!
			{
				CPUIDStruct cache_id(2);
				// TODO: Check for cache to determine processor type!
				if (L2CacheSize==0) {
					IntelProcessor=INTEL_PROCESSOR_CELERON_MODEL_5;
				}
				else if (L2CacheSize<=512*1024) {	// If Xeon has 512k L2 cache we will assume a PII - tough luck!
					IntelProcessor=INTEL_PROCESSOR_PENTIUM_II_MODEL_5;
				}
				else {
					IntelProcessor=INTEL_PROCESSOR_PENTIUM_II_XEON_MODEL_5;
				}
			}
			break;
		case 6:
			IntelProcessor=INTEL_PROCESSOR_CELERON_MODEL_6;
			break;
		case 7:
			if (L2CacheSize<=512*1024) {	// If Xeon has 512k L2 cache we will assume a PIII - tough luck!
				IntelProcessor=INTEL_PROCESSOR_PENTIUM_III_MODEL_7;
			}
			else {
				IntelProcessor=INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_7;
			}
			break;
		case 8:
			// This could be PentiumIII Coppermine or Celeron with SSE, or a Xeon
			{
				CPUIDStruct brand(1);
				switch (brand.Ebx&0xff) {
				case 0x1: IntelProcessor=INTEL_PROCESSOR_CELERON_MODEL_8; break;
				case 0x2:
				case 0x4: IntelProcessor=INTEL_PROCESSOR_PENTIUM_III_MODEL_8; break;
				case 0x3:
				case 0xe: IntelProcessor=INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_8; break;
				}
			}
			break;
		case 0xa:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_A;
			break;
		case 0xb:
			IntelProcessor=INTEL_PROCESSOR_PENTIUM_III_MODEL_B;
			break;
		}
		break;
	case 0xf:
		IntelProcessor=INTEL_PROCESSOR_PENTIUM4;
		break;
	}
}

void CPUDetectClass::Init_AMD_Processor_Type()
{
	switch (ProcessorFamily) {
	case 4:
		switch (ProcessorModel) {
		case 3:
			AMDProcessor=AMD_PROCESSOR_486DX2;
			break;
		case 7:
			AMDProcessor=AMD_PROCESSOR_486DX4;
			break;
		case 8:
			AMDProcessor=AMD_PROCESSOR_486DX4;
			break;
		case 9:
			AMDProcessor=AMD_PROCESSOR_486DX4;
			break;
		case 0xe:
			AMDProcessor=AMD_PROCESSOR_5x86;
			break;
		case 0xf:
			AMDProcessor=AMD_PROCESSOR_5x86;
			break;
		default:
			AMDProcessor=AMD_PROCESSOR_486;
		}
		break;
	case 5:
		switch (ProcessorModel) {
		case 0:
			AMDProcessor=AMD_PROCESSOR_K5_MODEL0;
			break;
		case 1:
			AMDProcessor=AMD_PROCESSOR_K5_MODEL1;
			break;
		case 2:
			AMDProcessor=AMD_PROCESSOR_K5_MODEL2;
			break;
		case 3:
			AMDProcessor=AMD_PROCESSOR_K5_MODEL3;
			break;
		case 6:
			AMDProcessor=AMD_PROCESSOR_K6_MODEL6;
			break;
		case 7:
			AMDProcessor=AMD_PROCESSOR_K6_MODEL7;
			break;
		case 8:
			AMDProcessor=AMD_PROCESSOR_K6_2_3DNOW_MODEL8;
			break;
		case 9:
			AMDProcessor=AMD_PROCESSOR_K6_3_3DNOW_MODEL9;
			break;
		case 0xd:
			AMDProcessor=AMD_PROCESSOR_K6_3_PLUS;
			break;
		default:
			AMDProcessor=AMD_PROCESSOR_K6;
		}
	case 6:
		switch (ProcessorModel) {
		case 1:
			AMDProcessor=AMD_PROCESSOR_ATHLON_025;
			break;
		case 2:
			AMDProcessor=AMD_PROCESSOR_ATHLON_018;
			break;
		case 3:
			AMDProcessor=AMD_PROCESSOR_DURON;
			break;
		case 4:
			AMDProcessor=AMD_PROCESSOR_ATHLON_018_IL2;
			break;
		default:
			AMDProcessor=AMD_PROCESSOR_ATHLON;
			break;
		}
	}
}

void CPUDetectClass::Init_VIA_Processor_Type()
{
	switch (ProcessorFamily) {
	case 5:
		switch (ProcessorModel) {
		case 4:
			VIAProcessor=VIA_PROCESSOR_IDT_C6_WINCHIP;
			break;
		case 8:
			VIAProcessor=VIA_PROCESSOR_IDT_C6_WINCHIP2;
			break;
		case 9:
			VIAProcessor=VIA_PROCESSOR_IDT_C6_WINCHIP3;
			break;
		}
	case 6:
		switch (ProcessorModel) {
		case 4:
			VIAProcessor=VIA_PROCESSOR_CYRIX_III_SAMUEL;
			break;
		}
	}
}

void CPUDetectClass::Init_Rise_Processor_Type()
{
	switch (ProcessorFamily) {
	case 5:
		switch (ProcessorModel) {
		case 0:
			RiseProcessor=RISE_PROCESSOR_DRAGON_025;
			break;
		case 2:
			RiseProcessor=RISE_PROCESSOR_DRAGON_018;
			break;
		case 8:
			RiseProcessor=RISE_PROCESSOR_DRAGON2_025;
			break;
		case 9:
			RiseProcessor=RISE_PROCESSOR_DRAGON2_018;
			break;
		}
	}
}

void CPUDetectClass::Init_Processor_Family()
{
	unsigned signature=0;
	unsigned notneeded1;
	unsigned notneeded2;
	unsigned features;
	CPUID(
		signature,
		notneeded1,
		notneeded2,
		features,
		1);

	ProcessorType=(signature>>12)&0x0f;
	ProcessorFamily=(signature>>8)&0x0f;
	ProcessorModel=(signature>>4)&0x0f;
	ProcessorRevision=(signature)&0x0f;

	IntelProcessor=INTEL_PROCESSOR_UNKNOWN;
	AMDProcessor=AMD_PROCESSOR_UNKNOWN;
	VIAProcessor=VIA_PROCESSOR_UNKNOWN;
	RiseProcessor=RISE_PROCESSOR_UNKNOWN;

	Init_Cache();

	switch (ProcessorManufacturer) {
	case MANUFACTURER_INTEL:
		Init_Intel_Processor_Type();
		break;
	case MANUFACTURER_AMD:
		Init_AMD_Processor_Type();
		break;
	case MANUFACTURER_VIA:
		Init_VIA_Processor_Type();
		break;
	case MANUFACTURER_RISE:
		Init_Rise_Processor_Type();
		break;
	}
}

void CPUDetectClass::Init_Processor_String()
{
	if (!Has_CPUID_Instruction()) {
		ProcessorString[0]='\0';
	}

	CPUIDStruct ext_id(0x80000000);	// Check maximum extended cpuid level
//	if (ProcessorManufacturer==MANUFACTURER_INTEL && IntelProcessor<INTEL_PROCESSOR_PENTIUM4) {
//		ext_id.Eax=0;	// No support for extended cpuid in Intel cpus prior to Pentium4
//	}
	// If extended cpuid level's highest bit is set, we'll get the cpu string from 2/3/4
	if (ext_id.Eax&0x80000000) {
		CPUDetectClass::CPUID(
			(unsigned&)ProcessorString[0],
			(unsigned&)ProcessorString[4],
			(unsigned&)ProcessorString[8],
			(unsigned&)ProcessorString[12],
			0x80000002);
		CPUDetectClass::CPUID(
			(unsigned&)ProcessorString[16],
			(unsigned&)ProcessorString[20],
			(unsigned&)ProcessorString[24],
			(unsigned&)ProcessorString[28],
			0x80000003);
		CPUDetectClass::CPUID(
			(unsigned&)ProcessorString[32],
			(unsigned&)ProcessorString[36],
			(unsigned&)ProcessorString[40],
			(unsigned&)ProcessorString[44],
			0x80000004);
	}
	// If no extended cpuid available (Intel processors prior to P4), compose the string
	else {
		StringClass str(0,true);
		str=Get_Processor_Manufacturer_Name();
		if (ProcessorManufacturer==MANUFACTURER_INTEL) {
			str+=" ";
			switch (IntelProcessor) {
			default:
			case INTEL_PROCESSOR_UNKNOWN:							str+="<UNKNOWN>"; break;
			case INTEL_PROCESSOR_80386:							str+="80386"; break;
			case INTEL_PROCESSOR_80486DX:							str+="80486DX"; break;
			case INTEL_PROCESSOR_80486SX:							str+="80486SX"; break;
			case INTEL_PROCESSOR_80486DX2:						str+="80486DX2"; break;
			case INTEL_PROCESSOR_80486SL:							str+="80486SL"; break;
			case INTEL_PROCESSOR_80486SX2:						str+="80486SX2"; break;
			case INTEL_PROCESSOR_80486DX2_WB:					str+="80486DX2(WB)"; break;
			case INTEL_PROCESSOR_80486DX4:						str+="80486DX4"; break;
			case INTEL_PROCESSOR_80486DX4_WB:					str+="80486DX4(WB)"; break;
			case INTEL_PROCESSOR_PENTIUM:							str+="Pentium"; break;
			case INTEL_PROCESSOR_PENTIUM_OVERDRIVE:			str+="Pentium Overdrive"; break;
			case INTEL_PROCESSOR_PENTIUM_MMX:					str+="Pentium MMX"; break;
			case INTEL_PROCESSOR_PENTIUM_PRO_SAMPLE:			str+="Pentium Pro (Engineering Sample)"; break;
			case INTEL_PROCESSOR_PENTIUM_PRO:					str+="Pentium Pro"; break;
			case INTEL_PROCESSOR_PENTIUM_II_OVERDRIVE:		str+="Pentium II Overdrive"; break;
			case INTEL_PROCESSOR_PENTIUM_II_MODEL_3:			str+="Pentium II, model 3"; break;
			case INTEL_PROCESSOR_PENTIUM_II_MODEL_4:			str+="Pentium II, model 4"; break;
			case INTEL_PROCESSOR_CELERON_MODEL_5:				str+="Celeron, model 5"; break;
			case INTEL_PROCESSOR_PENTIUM_II_MODEL_5:			str+="Pentium II, model 5"; break;
			case INTEL_PROCESSOR_PENTIUM_II_XEON_MODEL_5:	str+="Pentium II Xeon, model 5"; break;
			case INTEL_PROCESSOR_CELERON_MODEL_6:				str+="Celeron, model 6"; break;
			case INTEL_PROCESSOR_PENTIUM_III_MODEL_7:			str+="Pentium III, model 7"; break;
			case INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_7:	str+="Pentium III Xeon, model 7"; break;
			case INTEL_PROCESSOR_CELERON_MODEL_8:				str+="Celeron, model 8"; break;
			case INTEL_PROCESSOR_PENTIUM_III_MODEL_8:			str+="Pentium III, model 8"; break;
			case INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_8:	str+="Pentium III Xeon, model 8"; break;
			case INTEL_PROCESSOR_PENTIUM_III_XEON_MODEL_A:	str+="Pentium III Xeon, model A"; break;
			case INTEL_PROCESSOR_PENTIUM_III_MODEL_B:			str+="Pentium III, model B"; break;
			case INTEL_PROCESSOR_PENTIUM4:						str+="Pentium4"; break;
			}
		}
		strlcpy(ProcessorString,str.str(),sizeof(ProcessorString));
	}

}

void CPUDetectClass::Init_CPUID_Instruction()
{
	unsigned long cpuid_available=0;

   // The pushfd/popfd commands are done using emits
   // because CodeWarrior seems to have problems with
   // the command (huh?)

#if defined(_MSC_VER) && _MSC_VER < 1300
#ifdef WIN32
   __asm
   {
      mov cpuid_available, 0	// clear flag
      push ebx
      pushfd
      pop eax
      mov ebx, eax
      xor eax, 0x00200000
      push eax
      popfd
      pushfd
      pop eax
      xor eax, ebx
      je done
      mov cpuid_available, 1
   done:
      push ebx
      popfd
      pop ebx
   }
#elif defined(_UNIX)
     __asm__(" mov $0, __cpuid_available");  // clear flag
     __asm__(" push %ebx");
     __asm__(" pushfd");
     __asm__(" pop %eax");
     __asm__(" mov %eax, %ebx");
     __asm__(" xor 0x00200000, %eax");
     __asm__(" push %eax");
     __asm__(" popfd");
     __asm__(" pushfd");
     __asm__(" pop %eax");
     __asm__(" xor %ebx, %eax");
     __asm__(" je done");
     __asm__(" mov $1, __cpuid_available");
     goto done;  // just to shut the compiler up
   done:
     __asm__(" push %ebx");
     __asm__(" popfd");
     __asm__(" pop %ebx");
#endif
	HasCPUIDInstruction=!!cpuid_available;
#else
	// TheSuperHackers @info Mauller 30/3/2020 All modern CPUs have the CPUID instruction, VS22 code will not run on a cpu that doesn't.
	HasCPUIDInstruction = true;
#endif	// defined(_MSC_VER) && _MSC_VER < 1300
}

void CPUDetectClass::Init_Processor_Features()
{
	if (!CPUDetectClass::Has_CPUID_Instruction()) return;

	CPUIDStruct id(1);
	FeatureBits=id.Edx;
	HasRDTSCInstruction=(!!(FeatureBits&(1<<4)));
	HasCMOVSupport=(!!(FeatureBits&(1<<15)));
	HasMMXSupport=(!!(FeatureBits&(1<<23)));
	HasSSESupport=!!(FeatureBits&(1<<25));
	HasSSE2Support=!!(FeatureBits&(1<<26));

	Has3DNowSupport=false;
	ExtendedFeatureBits=0;
	if (ProcessorManufacturer==MANUFACTURER_AMD) {
		if (Has_CPUID_Instruction()) {
			CPUIDStruct max_ext_id(0x80000000);
			if (max_ext_id.Eax>=0x80000001) {	// Signature & features field available?
				CPUIDStruct ext_signature(0x80000001);
				ExtendedFeatureBits=ext_signature.Edx;
				Has3DNowSupport=!!(ExtendedFeatureBits&0x80000000);
				HasExtended3DNowSupport=!!(ExtendedFeatureBits&0x40000000);
			}
		}
	}
}

void CPUDetectClass::Init_Memory()
{
#ifdef WIN32

#if defined(_MSC_VER) && _MSC_VER < 1300
	MEMORYSTATUS mem;
   GlobalMemoryStatus(&mem);

   TotalPhysicalMemory     = mem.dwTotalPhys;
   AvailablePhysicalMemory = mem.dwAvailPhys;
   TotalPageMemory         = mem.dwTotalPageFile;
   AvailablePageMemory     = mem.dwAvailPageFile;
   TotalVirtualMemory      = mem.dwTotalVirtual;
   AvailableVirtualMemory  = mem.dwAvailVirtual;
#else
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
   GlobalMemoryStatusEx(&mem);

   TotalPhysicalMemory     = mem.ullTotalPhys;
   AvailablePhysicalMemory = mem.ullAvailPhys;
   TotalPageMemory         = mem.ullTotalPageFile;
   AvailablePageMemory     = mem.ullAvailPageFile;
   TotalVirtualMemory      = mem.ullTotalVirtual;
   AvailableVirtualMemory  = mem.ullAvailVirtual;
#endif // defined(_MSC_VER) && _MSC_VER < 1300

#else
#warning FIX Init_Memory()
#endif // WIN32
}

void CPUDetectClass::Init_OS()
{
#ifdef WIN32

// TheSuperHackers @fix OmniBlade 30/07/2025
// GetVersionEx only returns the version of Windows it was manifested for since Windows 8.
// RtlGetVersion returns the correct information at least at the time of writing.
#if defined(_MSC_VER) && _MSC_VER < 1300
	OSVERSIONINFO os;
   os.dwOSVersionInfoSize = sizeof(os);
	GetVersionEx(&os);

   OSVersionNumberMajor = os.dwMajorVersion;
   OSVersionNumberMinor = os.dwMinorVersion;
   OSVersionBuildNumber = os.dwBuildNumber;
   OSVersionPlatformId  = os.dwPlatformId;
   OSVersionExtraInfo   = os.szCSDVersion;
#else
	typedef LONG(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = LoadLibraryExA("ntdll", NULL, 0);
    if (ntdll != nullptr) {
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)::GetProcAddress(ntdll, "RtlGetVersion");

        if (RtlGetVersion != nullptr) {
            RTL_OSVERSIONINFOW os = {0};
            os.dwOSVersionInfoSize = sizeof(os);
            RtlGetVersion(&os);
            OSVersionNumberMajor = os.dwMajorVersion;
            OSVersionNumberMinor = os.dwMinorVersion;
            OSVersionBuildNumber = os.dwBuildNumber;
            OSVersionPlatformId = os.dwPlatformId;
            OSVersionExtraInfo = os.szCSDVersion;
            return;
        }
    }

	// GetVersionEx will return this info if no manifest is present so seems a safe fallback.
    OSVersionNumberMajor = 6;
    OSVersionNumberMinor = 2;
    OSVersionBuildNumber = 0;
    OSVersionPlatformId = 2;
    OSVersionExtraInfo = "";
#endif // defined(_MSC_VER) && _MSC_VER < 1300
#else
#warning FIX Init_OS()
#endif
}

bool CPUDetectClass::CPUID(
	unsigned& u_eax_,
	unsigned& u_ebx_,
	unsigned& u_ecx_,
	unsigned& u_edx_,
	unsigned cpuid_type)
{
	if (!Has_CPUID_Instruction()) return false;	// Most processors since 486 have CPUID...

	unsigned int regs[4];
	cpuid(regs, cpuid_type);
	u_eax_ = regs[0];
	u_ebx_ = regs[1];
	u_ecx_ = regs[2];
	u_edx_ = regs[3];

	return true;
}

#define SYSLOG(n) work.Format n ; CPUDetectClass::ProcessorLog+=work;

void CPUDetectClass::Init_Processor_Log()
{
	StringClass work(0,true);

	SYSLOG(("Operating System: "));
	switch (OSVersionPlatformId) {
#ifdef _WIN32
	case VER_PLATFORM_WIN32s: SYSLOG(("Windows 3.1")); break;
	case VER_PLATFORM_WIN32_WINDOWS: SYSLOG(("Windows 9x")); break;
	case VER_PLATFORM_WIN32_NT: SYSLOG(("Windows NT")); break;
#endif
	}
	SYSLOG(("\r\n"));

	SYSLOG(("Operating system version %d.%d\r\n",OSVersionNumberMajor,OSVersionNumberMinor));
	SYSLOG(("Operating system build: %d.%d.%d\r\n",
		(OSVersionBuildNumber&0xff000000)>>24,
		(OSVersionBuildNumber&0xff0000)>>16,
		(OSVersionBuildNumber&0xffff)));
	SYSLOG(("OS-Info: %s\r\n", OSVersionExtraInfo.str()));

	SYSLOG(("Processor: %s\r\n",CPUDetectClass::Get_Processor_String()));
	SYSLOG(("Clock speed: ~%dMHz\r\n",CPUDetectClass::Get_Processor_Speed()));
	StringClass cpu_type(0,true);
	switch (CPUDetectClass::Get_Processor_Type()) {
	case 0: cpu_type="Original OEM"; break;
	case 1: cpu_type="Overdrive"; break;
	case 2: cpu_type="Dual"; break;
	case 3: cpu_type="*Intel Reserved*"; break;
	}
	SYSLOG(("Processor type: %s\r\n", cpu_type.str()));

	SYSLOG(("\r\n"));

	SYSLOG(("Total physical memory: %dMb\r\n",Get_Total_Physical_Memory()/(1024*1024)));
	SYSLOG(("Available physical memory: %dMb\r\n",Get_Available_Physical_Memory()/(1024*1024)));
	SYSLOG(("Total page file size: %dMb\r\n",Get_Total_Page_File_Size()/(1024*1024)));
	SYSLOG(("Total available page file size: %dMb\r\n",Get_Available_Page_File_Size()/(1024*1024)));
	SYSLOG(("Total virtual memory: %dMb\r\n",Get_Total_Virtual_Memory()/(1024*1024)));
	SYSLOG(("Available virtual memory: %dMb\r\n",Get_Available_Virtual_Memory()/(1024*1024)));

	SYSLOG(("\r\n"));

	SYSLOG(("CPUID: %s\r\n",CPUDetectClass::Has_CPUID_Instruction() ? "Yes" : "No"));
	SYSLOG(("RDTSC: %s\r\n",CPUDetectClass::Has_RDTSC_Instruction() ? "Yes" : "No"));
	SYSLOG(("CMOV: %s\r\n",CPUDetectClass::Has_CMOV_Instruction() ? "Yes" : "No"));
	SYSLOG(("MMX: %s\r\n",CPUDetectClass::Has_MMX_Instruction_Set() ? "Yes" : "No"));
	SYSLOG(("SSE: %s\r\n",CPUDetectClass::Has_SSE_Instruction_Set() ? "Yes" : "No"));
	SYSLOG(("SSE2: %s\r\n",CPUDetectClass::Has_SSE2_Instruction_Set() ? "Yes" : "No"));
	SYSLOG(("3DNow!: %s\r\n",CPUDetectClass::Has_3DNow_Instruction_Set() ? "Yes" : "No"));
	SYSLOG(("Extended 3DNow!: %s\r\n",CPUDetectClass::Has_Extended_3DNow_Instruction_Set() ? "Yes" : "No"));
	SYSLOG(("CPU Feature bits: 0x%x\r\n",CPUDetectClass::Get_Feature_Bits()));
	SYSLOG(("Ext. CPU Feature bits: 0x%x\r\n",CPUDetectClass::Get_Extended_Feature_Bits()));

	SYSLOG(("\r\n"));

	if (CPUDetectClass::Get_L1_Data_Cache_Size()) {
		SYSLOG(("L1 Data Cache: %d byte cache lines, %d way set associative, %dk\r\n",
			CPUDetectClass::Get_L1_Data_Cache_Line_Size(),
			CPUDetectClass::Get_L1_Data_Cache_Set_Associative(),
			CPUDetectClass::Get_L1_Data_Cache_Size()/1024));
	}
	else {
		SYSLOG(("L1 Data Cache: None\r\n"));
	}

	if (CPUDetectClass::Get_L1_Instruction_Cache_Size()) {
		SYSLOG(("L1 Instruction Cache: %d byte cache lines, %d way set associative, %dk\r\n",
			CPUDetectClass::Get_L1_Instruction_Cache_Line_Size(),
			CPUDetectClass::Get_L1_Instruction_Cache_Set_Associative(),
			CPUDetectClass::Get_L1_Instruction_Cache_Size()/1024));
	}
	else {
		SYSLOG(("L1 Instruction Cache: None\r\n"));
	}

	if (CPUDetectClass::Get_L1_Instruction_Trace_Cache_Size()) {
		SYSLOG(("L1 Instruction Trace Cache: %d way set associative, %dk micro-OPs\r\n",
			CPUDetectClass::Get_L1_Instruction_Cache_Set_Associative(),
			CPUDetectClass::Get_L1_Instruction_Cache_Size()/1024));
	}
	else {
		SYSLOG(("L1 Instruction Trace Cache: None\r\n"));
	}


	if (CPUDetectClass::Get_L2_Cache_Size()) {
		SYSLOG(("L2 Cache: %d byte cache lines, %d way set associative, %dk\r\n",
			CPUDetectClass::Get_L2_Cache_Line_Size(),
			CPUDetectClass::Get_L2_Cache_Set_Associative(),
			CPUDetectClass::Get_L2_Cache_Size()/1024));
	}
	else {
		SYSLOG(("L2 cache: None\r\n"));
	}
	SYSLOG(("\r\n"));


}

// OSCODE OSSUBCODE CPUMANUFACTURER CPUSPEED MEMORY CPUBITS EXTCPUBITS
#define COMPACTLOG(n) work.Format n ; CPUDetectClass::CompactLog+=work;

void CPUDetectClass::Init_Compact_Log()
{
	StringClass work(0,true);

#ifdef WIN32
   TIME_ZONE_INFORMATION time_zone;
   GetTimeZoneInformation(&time_zone);
   COMPACTLOG(("%d\t", time_zone.Bias));  // get diff between local time and UTC
#elif defined(_UNIX)
   time_t t = time(NULL);
   localtime(&t);
   COMPACTLOG(("%d\t", timezone));
#endif

	OSInfoStruct os_info;
	Get_OS_Info(os_info,OSVersionPlatformId,OSVersionNumberMajor,OSVersionNumberMinor,OSVersionBuildNumber);
	COMPACTLOG(("%s\t",os_info.Code));

	if (!stricmp(os_info.SubCode,"UNKNOWN")) {
		COMPACTLOG(("%d\t",OSVersionBuildNumber&0xffff));
	}
	else {
		COMPACTLOG(("%s\t",os_info.SubCode));
	}

	COMPACTLOG(("%s\t%d\t",Get_Processor_Manufacturer_Name(),Get_Processor_Speed()));

	COMPACTLOG(("%d\t",Get_Total_Physical_Memory()/(1024*1024)+1));

	COMPACTLOG(("%x\t%x\t",Get_Feature_Bits(),Get_Extended_Feature_Bits()));
}

static class CPUDetectInitClass
{
public:
	CPUDetectInitClass()
	{
		CPUDetectClass::Init_CPUID_Instruction();
		// We pretty much need CPUID, but let's not crash if it doesn't exist.
		// Every processor our games run should have CPUID so it would be extremely unlikely for it not to be present.
		// One can never be sure about the clones though...
		if (CPUDetectClass::Has_CPUID_Instruction()) {
			CPUDetectClass::Init_Processor_Manufacturer();
			CPUDetectClass::Init_Processor_Family();
			CPUDetectClass::Init_Processor_String();
			CPUDetectClass::Init_Processor_Features();
			CPUDetectClass::Init_Memory();
			CPUDetectClass::Init_OS();
		}
		CPUDetectClass::Init_Processor_Speed();

		CPUDetectClass::Init_Processor_Log();
		CPUDetectClass::Init_Compact_Log();
	}
} _CPU_Detect_Init;


OSInfoStruct Windows9xVersionTable[]={
	{"WIN95",	"FINAL",		"Windows 95",								4,0,950,			4,0,950		},
	{"WIN95",	"A",			"Windows 95a OSR1 final Update",		4,0,950,			4,0,951		},
	{"WIN95",	"B20OEM",	"Windows 95B OSR 2.0 final OEM",		4,0,950,			4,0,1111		},
	{"WIN95",	"B20UPD",	"Windows 95B OSR 2.1 final Update",	4,0,950,			4,3,1212		},
	{"WIN95",	"B21OEM",	"Windows 95B OSR 2.1 final OEM",		4,1,971,			4,1,971		},
	{"WIN95",	"C25OEM",	"Windows 95C OSR 2.5 final OEM",		4,0,950,			4,3,1214		},
	{"WIN98",	"BETAPRD",	"Windows 98 Beta pre-DR",				4,10,1351,		4,10,1351 	},
	{"WIN98",	"BETADR",	"Windows 98 Beta DR",					4,10,1358,		4,10,1358 	},
	{"WIN98",	"BETAE",		"Windows 98 early Beta",				4,10,1378,		4,10,1378 	},
	{"WIN98",	"BETAE",		"Windows 98 early Beta",				4,10,1410,		4,10,1410 	},
	{"WIN98",	"BETAE",		"Windows 98 early Beta",				4,10,1423,		4,10,1423 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1500,		4,10,1500 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1508,		4,10,1508	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1511,		4,10,1511 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1525,		4,10,1525 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1535,		4,10,1535 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1538,		4,10,1538 	},
	{"WIN98",	"BETA1",		"Windows 98 Beta 1",						4,10,1543,		4,10,1543 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1544,		4,10,1544 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1546,		4,10,1546 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1550,		4,10,1550 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1559,		4,10,1559 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1564,		4,10,1564 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1569,		4,10,1569 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1577,		4,10,1577 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1581,		4,10,1581 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1593,		4,10,1593 	},
	{"WIN98",	"BETA2",		"Windows 98 Beta 2",						4,10,1599,		4,10,1599 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1602,		4,10,1602 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1605,		4,10,1605 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1614,		4,10,1614 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1619,		4,10,1619 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1624,		4,10,1624 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1629,		4,10,1629 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1633,		4,10,1633 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1650,		4,10,1650 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1650,/*,3*/4,10,1650/*,3*/	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1650,/*,8*/4,10,1650/*,8*/	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1666,		4,10,1666 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1671,		4,10,1671 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1677,		4,10,1677 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1681,		4,10,1681 	},
	{"WIN98",	"BETA3",		"Windows 98 Beta 3",						4,10,1687,		4,10,1687 	},
	{"WIN98",	"RC0",		"Windows 98 RC0",							4,10,1691,		4,10,1691 	},
	{"WIN98",	"RC0",		"Windows 98 RC0",							4,10,1702,		4,10,1702 	},
	{"WIN98",	"RC0",		"Windows 98 RC0",							4,10,1708,		4,10,1708 	},
	{"WIN98",	"RC0",		"Windows 98 RC0",							4,10,1713,		4,10,1713 	},
	{"WIN98",	"RC1",		"Windows 98 RC1",							4,10,1721,/*,3*/4,10,1721/*,3*/	},
	{"WIN98",	"RC2",		"Windows 98 RC2",							4,10,1723,/*,4*/4,10,1723/*,4*/	},
	{"WIN98",	"RC2",		"Windows 98 RC2",							4,10,1726,		4,10,1726	},
	{"WIN98",	"RC3",		"Windows 98 RC3",							4,10,1900,/*,5*/4,10,1900/*,5*/	},
	{"WIN98",	"RC4",		"Windows 98 RC4",							4,10,1900,/*,8*/4,10,1900/*,8*/	},
	{"WIN98",	"RC5",		"Windows 98 RC5",							4,10,1998,		4,10,1998	},
	{"WIN98",	"FINAL",		"Windows 98",								4,10,1998,/*,6*/4,10,1998/*,6*/	},
	{"WIN98",	"SP1B1",		"Windows 98 SP1 Beta 1",				4,10,2088,		4,10,2088	},
	{"WIN98",	"OSR1B1",	"Windows 98 OSR1 Beta 1",				4,10,2106,		4,10,2106 	},
	{"WIN98",	"OSR1B1",	"Windows 98 OSR1 Beta 1",				4,10,2120,		4,10,2120 	},
	{"WIN98",	"OSR1B1",	"Windows 98 OSR1 Beta 1",				4,10,2126,		4,10,2126 	},
	{"WIN98",	"OSR1B1",	"Windows 98 OSR1 Beta 1",				4,10,2131,		4,10,2131 	},
	{"WIN98",	"SP1B2",		"Windows 98 SP1 Beta 2",				4,10,2150,/*,0*/4,10,2150/*,0*/	},
	{"WIN98",	"SP1B2",		"Windows 98 SP1 Beta 2",				4,10,2150,/*,4*/4,10,2150/*,4*/	},
	{"WIN98",	"SP1",		"Windows 98 SP1 final Update",		4,10,2000,		4,10,2000 	},
	{"WIN98",	"OSR1B2",	"Windows 98 OSR1 Beta 2",				4,10,2174,		4,10,2174 	},
	{"WIN98",	"SERC1",		"Windows 98 SE RC1",						4,10,2183,		4,10,2183 	},
	{"WIN98",	"SERC2",		"Windows 98 SE RC2",						4,10,2185,		4,10,2185 	},
	{"WIN98",	"SE",			"Windows 98 SE",							4,10,2222,		4,10,2222/*,3*/	},
	{"WINME",	"MEBDR1",	"Windows ME Beta DR1",					4,90,2332,		4,90,2332 	},
	{"WINME",	"MEBDR2",	"Windows ME Beta DR2",					4,90,2348,		4,90,2348 	},
	{"WINME",	"MEBDR3",	"Windows ME Beta DR3",					4,90,2358,		4,90,2358 	},
	{"WINME",	"MEBDR4",	"Windows ME Beta DR4",					4,90,2363,		4,90,2363 	},
	{"WINME",	"MEEB",		"Windows ME early Beta",				4,90,2368,		4,90,2368 	},
	{"WINME",	"MEEB",		"Windows ME early Beta",				4,90,2374,		4,90,2374 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2380,		4,90,2380 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2394,		4,90,2394 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2399,		4,90,2399 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2404,		4,90,2404 	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2410,		4,90,2410	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2416,		4,90,2416	},
	{"WINME",	"MEB1",		"Windows ME Beta 1",						4,90,2419,/*,4*/4,90,2419/*,4*/	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2429,		4,90,2429 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2434,		4,90,2434 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2443,		4,90,2443 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2447,		4,90,2447 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2455,		4,90,2455 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2460,		4,90,2460 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2465,		4,90,2465 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2470,		4,90,2470 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2474,		4,90,2474 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2481,		4,90,2481 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2487,		4,90,2487 	},
	{"WINME",	"MEB2",		"Windows ME Beta 2",						4,90,2491,		4,90,2491 	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2499,		4,90,2499 	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2499,/*,3*/4,90,2499/*,3*/	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2509,		4,90,2509 	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2513,		4,90,2513 	},
	{"WINME",	"MEB3",		"Windows ME Beta 3",						4,90,2516,		4,90,2516 	},
	{"WINME",	"RC0",		"Windows ME RC0",							4,90,2525,		4,90,2525 	},
	{"WINME",	"RC1",		"Windows ME RC1",							4,90,2525,/*,6*/4,90,2525/*,6*/	},
	{"WINME",	"RC2",		"Windows ME RC2",							4,90,2535,		4,90,2535	},
	{"WINME",	"FINAL",		"Windows ME",								4,90,3000,/*,2*/4,90,3000/*,2*/	},
};

void Get_OS_Info(
	OSInfoStruct& os_info,
	unsigned OSVersionPlatformId,
	unsigned OSVersionNumberMajor,
	unsigned OSVersionNumberMinor,
	unsigned OSVersionBuildNumber)
{
	unsigned build_major=(OSVersionBuildNumber&0xff000000)>>24;
	unsigned build_minor=(OSVersionBuildNumber&0xff0000)>>16;
	unsigned build_sub=(OSVersionBuildNumber&0xffff);

	// TheSuperHackers @bugfix JAJames 17/03/2025 Fix uninitialized memory access and add more Windows versions.
	memset(&os_info,0,sizeof(os_info));
	os_info.Code="UNKNOWN";
	os_info.SubCode="UNKNOWN";
	os_info.VersionString="UNKNOWN";

	switch (OSVersionPlatformId) {
	default:
		break;
#ifdef _WIN32
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			for(int i=0;i<sizeof(Windows9xVersionTable)/sizeof(os_info);++i) {
				if (
					Windows9xVersionTable[i].VersionMajor==OSVersionNumberMajor &&
					Windows9xVersionTable[i].VersionMinor==OSVersionNumberMinor &&
					Windows9xVersionTable[i].BuildMajor==build_major &&
					Windows9xVersionTable[i].BuildMinor==build_minor &&
					Windows9xVersionTable[i].BuildSub==build_sub) {
					os_info=Windows9xVersionTable[i];
					return;
				}
			}

			os_info.BuildMajor=build_major;
			os_info.BuildMinor=build_minor;
			os_info.BuildSub=build_sub;
			if (OSVersionNumberMajor==4) {
//				os_info.SubCode.Format("%d",build_sub);
				os_info.SubCode="UNKNOWN";
				if (OSVersionNumberMinor==0) {
					os_info.Code="WIN95";
					return;
				}
				if (OSVersionNumberMinor==10) {
					os_info.Code="WIN98";
					return;
				}
				if (OSVersionNumberMinor==90) {
					os_info.Code="WINME";
					return;
				}
				os_info.Code="WIN9X";
				return;
			}
		}
		break;
	case VER_PLATFORM_WIN32_NT:
//		os_info.SubCode.Format("%d",build_sub);
		os_info.SubCode="UNKNOWN";
		if (OSVersionNumberMajor==4) {
			os_info.Code="WINNT";
			return;
		}
		if (OSVersionNumberMajor==5) {
			if (OSVersionNumberMinor==0) {
				os_info.Code="WIN2K";
				return;
			}
			if (OSVersionNumberMinor==1) {
				os_info.Code="WINXP";
				return;
			}
		}
		if (OSVersionNumberMajor==6) {
			if (OSVersionNumberMinor==0) {
				os_info.Code="WINVS"; // Vista
				return;
			}
			if (OSVersionNumberMinor==1) {
				os_info.Code="WIN70"; // Win 7
				return;
			}
			if (OSVersionNumberMinor==2) {
				os_info.Code="WIN80"; // Win 8.0
				return;
			}
			if (OSVersionNumberMinor==3) {
				os_info.Code="WIN81"; // Win 8.1
				return;
			}
		}
		if (OSVersionNumberMinor==10) {
			os_info.Code="WIN1X"; // Win 10, Win 11, Server 2016, Server 2019, Server 2022
			return;
		}
		// Reference 1: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-osversioninfoa#remarks
		// Reference 2: https://learn.microsoft.com/en-us/windows/win32/sysinfo/operating-system-version

		// No more-specific version detected; fallback to XX
		os_info.Code="WINXX";
#endif
	}
}
