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
 *                 Project Name : WWDebug                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwdebug/wwprofile.cpp                        $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/01/02 10:30a                                              $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *   WWProfile_Get_Ticks -- Retrieves the cpu performance counter                              *
 *   WWProfileHierachyNodeClass::WWProfileHierachyNodeClass -- Constructor                     *
 *   WWProfileHierachyNodeClass::~WWProfileHierachyNodeClass -- Destructor                     *
 *   WWProfileHierachyNodeClass::Get_Sub_Node -- Searches for a child node by name (pointer)   *
 *   WWProfileHierachyNodeClass::Reset -- Reset all profiling data in the tree                 *
 *   WWProfileHierachyNodeClass::Call -- Start timing                                          *
 *   WWProfileHierachyNodeClass::Return -- Stop timing, record results                         *
 *   WWProfileManager::Start_Profile -- Begin a named profile                                  *
 *   WWProfileManager::Stop_Profile -- Stop timing and record the results.                     *
 *   WWProfileManager::Reset -- Reset the contents of the profiling system                     *
 *   WWProfileManager::Increment_Frame_Counter -- Increment the frame counter                  *
 *   WWProfileManager::Get_Time_Since_Reset -- returns the elapsed time since last reset       *
 *   WWProfileManager::Get_Iterator -- Creates an iterator for the profile tree                *
 *   WWProfileManager::Release_Iterator -- Return an iterator for the profile tree             *
 *   WWProfileManager::Get_In_Order_Iterator -- Creates an "in-order" iterator for the profile *
 *   WWProfileManager::Release_In_Order_Iterator -- Return an "in-order" iterator              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "wwprofile.h"
#include "FastAllocator.h"
#include "wwdebug.h"
//#include "systimer.h"
#include "systimer.h"
#include "RAWFILE.H"
#include "ffactory.h"
#include "simplevec.h"
#include "cpudetect.h"
#include "hashtemplate.h"
#include <stdio.h>
#include <Utility/intrin_compat.h>

static SimpleDynVecClass<WWProfileHierachyNodeClass*> ProfileCollectVector;
static double TotalFrameTimes;
static bool ProfileCollecting;

static HashTemplateClass<StringClass, unsigned> ProfileStringHash;
static unsigned ProfileStringCount;

unsigned WWProfile_Get_System_Time()
{
	return TIMEGETTIME();
}

WWINLINE double WWProfile_Get_Inv_Processor_Ticks_Per_Second(void) 
{
#ifdef WIN32
	return CPUDetectClass::Get_Inv_Processor_Ticks_Per_Second();
#elif defined (_UNIX)
	return 0.001;
#endif
}

/***********************************************************************************************
 * WWProfile_Get_Ticks -- Retrieves the cpu performance counter                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
inline void WWProfile_Get_Ticks(_int64 * ticks)
{
#ifdef _UNIX
       *ticks = TIMEGETTIME();
#else
	*ticks = _rdtsc();
#endif
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::WWProfileHierachyNodeClass -- Constructor                       *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - pointer to a static string which is the name of this profile node                    *
 * parent - parent pointer                                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The name is assumed to be a static pointer, only the pointer is stored and compared for     *
 * efficiency reasons.                                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileHierachyNodeClass::WWProfileHierachyNodeClass( const char * name, WWProfileHierachyNodeClass * parent ) :
	Name( name ),
	TotalCalls( 0 ),
	TotalTime( 0 ),
	StartTime( 0 ),
	RecursionCounter( 0 ),
	Parent( parent ),
	Child( NULL ),
	Sibling( NULL )
{
	Reset();

	if (!ProfileStringHash.Get(name, ProfileStringID)) {
		ProfileStringID=ProfileStringCount++;
		ProfileStringHash.Insert(name,ProfileStringID);
	}
}

WWProfileHierachyNodeClass::WWProfileHierachyNodeClass( unsigned id, WWProfileHierachyNodeClass * parent ) :
	Name( NULL ),
	TotalCalls( 0 ),
	TotalTime( 0 ),
	StartTime( 0 ),
	RecursionCounter( 0 ),
	Parent( parent ),
	Child( NULL ),
	Sibling( NULL ),
	ProfileStringID(id)
{
	Reset();
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::~WWProfileHierachyNodeClass -- Destructor                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileHierachyNodeClass::~WWProfileHierachyNodeClass( void )
{
	delete Child;
	delete Sibling;
}


WWProfileHierachyNodeClass* WWProfileHierachyNodeClass::Clone_Hierarchy(WWProfileHierachyNodeClass* parent)
{
	WWProfileHierachyNodeClass* node=new WWProfileHierachyNodeClass(Name,parent);
	node->TotalCalls=TotalCalls;
	node->TotalTime=TotalTime;
	node->StartTime=StartTime;
	node->RecursionCounter=RecursionCounter;
	
	if (Child) {
		node->Child=Child->Clone_Hierarchy(this);
	}
	if (Sibling) {
		node->Sibling=Sibling->Clone_Hierarchy(parent);
	}

	return node;
}

void WWProfileHierachyNodeClass::Write_To_File(FileClass* file,int recursion)
{
	if (TotalTime!=0.0f) {
		int i;
		StringClass string;
		StringClass work;
		for (i=0;i<recursion;++i) { string+="\t"; }
		work.Format("%s\t%d\t%f\r\n",Name,TotalCalls,TotalTime*1000.0f);
		string+=work;
		file->Write(string.Peek_Buffer(),string.Get_Length());
	}
	if (Child) {
		Child->Write_To_File(file,recursion+1);
	}
	if (Sibling) {
		Sibling->Write_To_File(file,recursion);
	}
}

void WWProfileHierachyNodeClass::Add_To_String_Compact(StringClass& string,int recursion)
{
	if (TotalTime!=0.0f) {
		StringClass work;
		work.Format("%d %d %2.2f;",ProfileStringID,TotalCalls,TotalTime*1000.0f);
		string+=work;
	}
	if (Child) {
		StringClass work;
		Child->Add_To_String_Compact(work,recursion+1);
		if (work.Get_Length()!=0) {
			string+="{";
			string+=work;
			string+="}";
		}
	}
	if (Sibling) {
		Sibling->Add_To_String_Compact(string,recursion);
	}
}

/***********************************************************************************************
 * WWProfileHierachyNodeClass::Get_Sub_Node -- Searches for a child node by name (pointer)     *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - static string pointer to the name of the node we are searching for                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * All profile names are assumed to be static strings so this function uses pointer compares   *
 * to find the named node.                                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileHierachyNodeClass * WWProfileHierachyNodeClass::Get_Sub_Node( const char * name )
{
	// Try to find this sub node
	WWProfileHierachyNodeClass * child = Child;
	while ( child ) {
		if ( child->Name == name ) {
			return child;
		}
		child = child->Sibling;
	}

	// We didn't find it, so add it
	WWProfileHierachyNodeClass * node = W3DNEW WWProfileHierachyNodeClass( name, this );
	node->Sibling = Child;
	Child = node;
	return node;
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::Reset -- Reset all profiling data in the tree                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileHierachyNodeClass::Reset( void )
{
	TotalCalls = 0;
	TotalTime = 0.0f;

	if ( Child ) {
		Child->Reset();
	}
	if ( Sibling ) {
		Sibling->Reset();
	}
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::Call -- Start timing                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileHierachyNodeClass::Call( void )
{
	TotalCalls++;
	if (RecursionCounter++ == 0) {
		WWProfile_Get_Ticks(&StartTime);
	}
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::Return -- Stop timing, record results                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool	WWProfileHierachyNodeClass::Return( void )
{
	if (--RecursionCounter == 0) {
		if ( TotalCalls != 0 ) {
			__int64 time;
			WWProfile_Get_Ticks(&time);
			time-=StartTime;

			TotalTime += float(double(time)*WWProfile_Get_Inv_Processor_Ticks_Per_Second());
		}
	}
	return RecursionCounter == 0;
}


/***************************************************************************************************
**
** WWProfileManager Implementation
**
***************************************************************************************************/
bool									WWProfileManager::IsProfileEnabled=false;
WWProfileHierachyNodeClass		WWProfileManager::Root( "Root", NULL );
WWProfileHierachyNodeClass	*	WWProfileManager::CurrentNode = &WWProfileManager::Root;
WWProfileHierachyNodeClass	*	WWProfileManager::CurrentRootNode = &WWProfileManager::Root;
int									WWProfileManager::FrameCounter = 0;
__int64								WWProfileManager::ResetTime = 0;

static unsigned int				ThreadID = static_cast<unsigned int>(-1);


/***********************************************************************************************
 * WWProfileManager::Start_Profile -- Begin a named profile                                    *
 *                                                                                             *
 * Steps one level deeper into the tree, if a child already exists with the specified name     *
 * then it accumulates the profiling; otherwise a new child node is added to the profile tree. *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - name of this profiling record                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The string used is assumed to be a static string; pointer compares are used throughout      *
 * the profiling code for efficiency.                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Start_Profile( const char * name )
{
	if (::GetCurrentThreadId() != ThreadID) {
		return;
	}

//	int current_thread = ::GetCurrentThreadId();
	if (name != CurrentNode->Get_Name()) {
		CurrentNode = CurrentNode->Get_Sub_Node( name );
	}

	CurrentNode->Call();
}

void	WWProfileManager::Start_Root_Profile( const char * name )
{
	if (::GetCurrentThreadId() != ThreadID) {
		return;
	}

	if (name != CurrentRootNode->Get_Name()) {
		CurrentRootNode = CurrentRootNode->Get_Sub_Node( name );
	}

	CurrentRootNode->Call();
}


/***********************************************************************************************
 * WWProfileManager::Stop_Profile -- Stop timing and record the results.                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Stop_Profile( void )
{
	if (::GetCurrentThreadId() != ThreadID) {
		return;
	}

	// Return will indicate whether we should back up to our parent (we may
	// be profiling a recursive function)
	if (CurrentNode->Return()) {
		CurrentNode = CurrentNode->Get_Parent();
	}
}

void	WWProfileManager::Stop_Root_Profile( void )
{
	if (::GetCurrentThreadId() != ThreadID) {
		return;
	}

	// Return will indicate whether we should back up to our parent (we may
	// be profiling a recursive function)
	if (CurrentRootNode->Return()) {
		CurrentRootNode = CurrentRootNode->Get_Parent();
	}
}


/***********************************************************************************************
 * WWProfileManager::Reset -- Reset the contents of the profiling system                       *
 *                                                                                             *
 *    This resets everything except for the tree structure.  All of the timing data is reset.  *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Reset( void )
{
	ThreadID = ::GetCurrentThreadId();

	Root.Reset();
	FrameCounter = 0;
	WWProfile_Get_Ticks(&ResetTime);
}


/***********************************************************************************************
 * WWProfileManager::Increment_Frame_Counter -- Increment the frame counter                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void WWProfileManager::Increment_Frame_Counter( void )
{
	if (ProfileCollecting) {
		float time=Get_Time_Since_Reset();
		TotalFrameTimes+=time;
		WWProfileHierachyNodeClass* new_root=Root.Clone_Hierarchy(NULL);
		new_root->Set_Total_Time(time);
		new_root->Set_Total_Calls(1);
		ProfileCollectVector.Add(new_root);
		Reset();
	}

	FrameCounter++;

}


/***********************************************************************************************
 * WWProfileManager::Get_Time_Since_Reset -- returns the elapsed time since last reset         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
float WWProfileManager::Get_Time_Since_Reset( void )
{
	__int64 time;
	WWProfile_Get_Ticks(&time);
	time -= ResetTime;

	return float(double(time) * WWProfile_Get_Inv_Processor_Ticks_Per_Second());
}


/***********************************************************************************************
 * WWProfileManager::Get_Iterator -- Creates an iterator for the profile tree                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileIterator *	WWProfileManager::Get_Iterator( void )
{
	return W3DNEW WWProfileIterator( &Root );
}


/***********************************************************************************************
 * WWProfileManager::Release_Iterator -- Return an iterator for the profile tree               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Release_Iterator( WWProfileIterator * iterator )
{
	delete iterator;
}


void	WWProfileManager::Begin_Collecting()
{
	Reset();
	ProfileCollecting=true;
	TotalFrameTimes=0.0;
}

void	WWProfileManager::End_Collecting(const char* filename)
{
	int i;
	if (filename && ProfileCollectVector.Count()!=0) {
		FileClass * file= _TheWritingFileFactory->Get_File(filename);	
		if (file != NULL) {
			//
			//	Open or create the file
			//
			file->Open (FileClass::WRITE);

			StringClass str;
			float avg_frame_time=TotalFrameTimes/float(ProfileCollectVector.Count());
			str.Format(
				"Total frames: %d, average frame time: %fms\r\n"
				"All frames taking more than twice the average frame time are marked with keyword SPIKE.\r\n\r\n",
				ProfileCollectVector.Count(),avg_frame_time*1000.0f);
			file->Write(str.Peek_Buffer(),str.Get_Length());

			HashTemplateIterator<StringClass,unsigned> ite(ProfileStringHash);
			for (ite.First();!ite.Is_Done();ite.Next()) {
				StringClass name=ite.Peek_Key();
				// Remove spaces, tabs and commas from the name as it would confuse the profile browser
				int i;
				for (i=0;i<name.Get_Length();++i) {
					if (name[i]=='\t') name[i]='-';
					if (name[i]==' ') name[i]='_';
					if (name[i]==',') name[i]='.';
					if (name[i]==';') name[i]=':';
				}
				str.Format("ID: %d %s\r\n",ite.Peek_Value(),name);
				file->Write(str.Peek_Buffer(),str.Get_Length());
			}

			str.Format("\r\n\r\n");
			file->Write(str.Peek_Buffer(),str.Get_Length());

			for (i=0;i<ProfileCollectVector.Count();++i) {
				float frame_time=ProfileCollectVector[i]->Get_Total_Time();
				str.Format("FRAME: %d %2.2f %s ",i,frame_time*1000.0f,frame_time>avg_frame_time*2.0f ? "SPIKE" : "OK");
				ProfileCollectVector[i]->Add_To_String_Compact(str,0);
				str+="\r\n";
				file->Write(str.Peek_Buffer(),str.Get_Length());
			}
		
			//
			//	Close the file
			//
			file->Close ();
			_TheWritingFileFactory->Return_File (file);
		}
	}

	for (i=0;i<ProfileCollectVector.Count();++i) {
		delete ProfileCollectVector[i];
		ProfileCollectVector[i]=0;
	}
	ProfileCollectVector.Delete_All();
	ProfileCollecting=false;
}

static unsigned Read_Line(char* memory,unsigned pos,unsigned maxpos)
{
	while (pos<maxpos) {
		if (memory[pos++]=='\n') {
			return pos;
		}
	}
	return pos;
}

static unsigned Seek_Char(char* memory,unsigned pos,unsigned maxpos,char c)
{
	while (pos<maxpos) {
		if (memory[pos++]==c) {
			return pos;
		}
	}
	return pos;
}

unsigned Read_Int(char* memory,unsigned pos,unsigned maxpos,unsigned& number)
{
	number=0;
	while (pos<maxpos) {
		char c=memory[pos++];
		if (isdigit(c)) {
			number*=10;
			number+=c-'0';
		}
		else break;
	}
	return pos;
}

unsigned Read_String(char* memory,unsigned pos,unsigned maxpos,char* string)
{
	int ccount=0;
	while (pos<maxpos) {
		char c=memory[pos++];
		if (c>' ') {
			string[ccount++]=c;
		}
		else break;
	}
	string[ccount]='\0';
	return pos;
}

unsigned Read_Float(char* memory,unsigned pos,unsigned maxpos,float& fnumber)
{
	unsigned ftint=0;
	float ftfloat=0.0f;
	while (pos<maxpos) {
		char c=memory[pos++];
		if (isdigit(c)) {
			ftint*=10;
			ftint+=c-'0';
		}
		else {
			if (c=='.') {
				float divider=1.0f;
				while (pos<maxpos) {
					char c=memory[pos++];
					if (isdigit(c)) {
						ftfloat*=10.0f;
						ftfloat+=float(c-'0');
						divider*=10.0f;
					}
					else break;
				}
				ftfloat/=divider;
			}
			break;
		}
	}
	ftfloat+=float(ftint);
	fnumber=ftfloat;

	return pos;
}

static unsigned Read_ID(char* memory,unsigned pos,unsigned maxpos,StringClass& string,unsigned& id)
{
	char idstring[256];
	unsigned idnumber=0;
	pos+=4;	// "ID: "

	// Read idnumber
	pos=Read_Int(memory,pos,maxpos,idnumber);
	// Read name
	pos=Read_String(memory,pos,maxpos,idstring);

	string=idstring;
	id=idnumber;

	return Read_Line(memory,pos,maxpos);
}

static unsigned Read_Frame(char* memory,unsigned pos,unsigned maxpos,WWProfileHierachyInfoClass*& root,HashTemplateClass<unsigned, StringClass>& id_hash)
{
	char statusstring[256];
	unsigned framenumber=0;
	float frametime;
	root=NULL;
	WWProfileHierachyInfoClass* parent=NULL;
	WWProfileHierachyInfoClass* latest=NULL;

	pos+=7;	// "FRAME: "

	// Read frame number
	pos=Read_Int(memory,pos,maxpos,framenumber);

	// Read frametime
	pos=Read_Float(memory,pos,maxpos,frametime);

	// Read statusstring
	pos=Read_String(memory,pos,maxpos,statusstring);

	// Done with the header, read the fram data

	unsigned lineend=Read_Line(memory,pos,maxpos);
	while (pos<lineend) {
		// A Digit?
		if (isdigit(memory[pos])) {
			unsigned id;
			unsigned count;
			float time;
			// Read id
			pos=Read_Int(memory,pos,maxpos,id);

			// Read count
			pos=Read_Int(memory,pos,maxpos,count);

			// Read time
			pos=Read_Float(memory,pos,maxpos,time);

			StringClass name="Unknown";
			id_hash.Get(id,name);
			WWProfileHierachyInfoClass* new_node=new WWProfileHierachyInfoClass(name,parent);
			if (parent) {
				new_node->Set_Sibling(parent->Get_Child());
				parent->Set_Child(new_node);
			}
			new_node->Set_Total_Time(time);
			new_node->Set_Total_Calls(count);
			latest=new_node;
			if (root==NULL) root=new_node;
		}
		else if (memory[pos]=='{') {
			parent=latest;
			pos++;
		}
		else if (memory[pos]=='}') {
			if (parent) {
				parent=parent->Get_Parent();
			}
			pos++;
		}
		else {
			pos++;
		}
	}

	return Read_Line(memory,pos,maxpos);
}

void WWProfileManager::Load_Profile_Log(const char* filename, WWProfileHierachyInfoClass**& array, unsigned& count)
{
	array=NULL;
	count=0;

	unsigned i;
	FileClass * file= _TheFileFactory->Get_File(filename);	
	if (file != NULL && file->Is_Available()) {
		HashTemplateClass<StringClass, unsigned> string_hash;
		HashTemplateClass<unsigned, StringClass> id_hash;
		SimpleDynVecClass<WWProfileHierachyInfoClass*> vec;

		//
		//	Open or create the file
		//
		file->Open (FileClass::READ);
		char tmp[256];

		unsigned size=file->Size();
		char* memory=new char[size];
		file->Read(memory,size);
		file->Close();
		unsigned pos=0;

		while (pos<size) {
			Read_String(memory,pos,size,tmp);
			if (tmp[0]=='I' && tmp[1]=='D' && tmp[2]==':') {
				StringClass string;
				unsigned id;
				pos=Read_ID(memory,pos,size,string,id);
				string_hash.Insert(string,id);
				id_hash.Insert(id,string);
			}
			else if (tmp[0]=='F' && tmp[1]=='R' && tmp[2]=='A' && tmp[3]=='M' && tmp[4]=='E' && tmp[5]==':') {
				WWProfileHierachyInfoClass* node=NULL;
				pos=Read_Frame(memory,pos,size,node,id_hash);
				if (node) {
					vec.Add(node);
				}
			}
			else {
				pos=Read_Line(memory,pos,size);
			}
		}
		delete[] memory;

		if (vec.Count()) {
			count=vec.Count();
			array=new WWProfileHierachyInfoClass*[count];
			for (i=0;i<count;++i) {
				array[i]=vec[i];
			}
		}
	}
}


/***********************************************************************************************
 * WWProfileManager::Get_In_Order_Iterator -- Creates an "in-order" iterator for the profile t *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileInOrderIterator * WWProfileManager::Get_In_Order_Iterator( void )
{
	return W3DNEW WWProfileInOrderIterator;
}


/***********************************************************************************************
 * WWProfileManager::Release_In_Order_Iterator -- Return an "in-order" iterator                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Release_In_Order_Iterator( WWProfileInOrderIterator * iterator )
{
	delete iterator;
}


/***************************************************************************************************
**
** WWProfileIterator Implementation
**
***************************************************************************************************/
WWProfileIterator::WWProfileIterator( WWProfileHierachyNodeClass * start )
{
	CurrentParent = start;
	CurrentChild = CurrentParent->Get_Child();
}

void	WWProfileIterator::First(void)
{
	CurrentChild = CurrentParent->Get_Child();
}


void	WWProfileIterator::Next(void)
{
	CurrentChild = CurrentChild->Get_Sibling();
}

bool	WWProfileIterator::Is_Done(void)
{
	return CurrentChild == NULL;
}

void	WWProfileIterator::Enter_Child( void )
{
	CurrentParent = CurrentChild;
	CurrentChild = CurrentParent->Get_Child();
}

void	WWProfileIterator::Enter_Child( int index )
{
	CurrentChild = CurrentParent->Get_Child();
	while ( (CurrentChild != NULL) && (index != 0) ) {
		index--;
		CurrentChild = CurrentChild->Get_Sibling();
	}

	if ( CurrentChild != NULL ) {
		CurrentParent = CurrentChild;
		CurrentChild = CurrentParent->Get_Child();
	}
}

void	WWProfileIterator::Enter_Parent( void )
{
	if ( CurrentParent->Get_Parent() != NULL ) {
		CurrentParent = CurrentParent->Get_Parent();
	}
	CurrentChild = CurrentParent->Get_Child();
}

/***************************************************************************************************
**
** WWProfileInOrderIterator Implementation
**
***************************************************************************************************/

WWProfileInOrderIterator::WWProfileInOrderIterator( void )
{
	CurrentNode = &WWProfileManager::Root;
}

void	WWProfileInOrderIterator::First(void)
{
	CurrentNode = &WWProfileManager::Root;
}

void	WWProfileInOrderIterator::Next(void)
{
	if ( CurrentNode->Get_Child() ) {				// If I have a child, go to child
		CurrentNode = CurrentNode->Get_Child();
	} else if ( CurrentNode->Get_Sibling() ) {	// If I have a sibling, go to sibling
		CurrentNode = CurrentNode->Get_Sibling();
	} else {											//	if not, go to my parent's sibling, or his.......
		// Find a parent with a sibling....
		bool done = false;
		while ( CurrentNode != NULL && !done ) {

			// go to my parent
			CurrentNode = CurrentNode->Get_Parent();

			// If I have a sibling, go there
			if ( CurrentNode != NULL && CurrentNode->Get_Sibling() != NULL ) {
				CurrentNode = CurrentNode->Get_Sibling();
				done = true;
			}
		}
	}
}

bool	WWProfileInOrderIterator::Is_Done(void)
{
	return CurrentNode == NULL;
}

/*
**
*/
WWTimeItClass::WWTimeItClass( const char * name )
{
	Name = name;
	WWProfile_Get_Ticks( &Time );
}

WWTimeItClass::~WWTimeItClass( void )
{
	__int64 End;
	WWProfile_Get_Ticks( &End );
	End -= Time;
#ifdef WWDEBUG
	float time = End * WWProfile_Get_Inv_Processor_Ticks_Per_Second();
	WWDEBUG_SAY(( "*** WWTIMEIT *** %s took %1.9f\n", Name, time ));
#endif
}


/*
**
*/
WWMeasureItClass::WWMeasureItClass( float * p_result )
{
	WWASSERT(p_result != NULL);
	PResult = p_result;
	WWProfile_Get_Ticks( &Time );
}

WWMeasureItClass::~WWMeasureItClass( void )
{
	__int64 End;
	WWProfile_Get_Ticks( &End );
	End -= Time;
	WWASSERT(PResult != NULL);
	*PResult = End  * WWProfile_Get_Inv_Processor_Ticks_Per_Second();
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

unsigned WWMemoryAndTimeLog::TabCount;

WWMemoryAndTimeLog::WWMemoryAndTimeLog(const char* name)
	:
	Name(name),
	TimeStart(WWProfile_Get_System_Time()),
	AllocCountStart(FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocation_Count()),
	AllocSizeStart(FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocated_Size())
{
	IntermediateTimeStart=TimeStart;
	IntermediateAllocCountStart=AllocCountStart;
	IntermediateAllocSizeStart=AllocSizeStart;
	StringClass tmp(0,true);
	for (unsigned i=0;i<TabCount;++i) tmp+="\t";
	WWRELEASE_SAY(("%s%s {\n",tmp,name));
	TabCount++;
}

WWMemoryAndTimeLog::~WWMemoryAndTimeLog()
{
	if (TabCount>0) TabCount--;
	StringClass tmp(0,true);
	for (unsigned i=0;i<TabCount;++i) tmp+="\t";
	WWRELEASE_SAY(("%s} ",tmp));

	unsigned current_time=WWProfile_Get_System_Time();
	int current_alloc_count=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocation_Count();
	int current_alloc_size=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocated_Size();
	WWRELEASE_SAY(("IN TOTAL %s took %d.%3.3d s, did %d memory allocations of %d bytes\n",
		Name,
		(current_time - TimeStart)/1000, (current_time - TimeStart)%1000,
		current_alloc_count - AllocCountStart,
		current_alloc_size - AllocSizeStart));
	WWRELEASE_SAY(("\n"));

}


void WWMemoryAndTimeLog::Log_Intermediate(const char* text)
{
	unsigned current_time=WWProfile_Get_System_Time();
	int current_alloc_count=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocation_Count();
	int current_alloc_size=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocated_Size();
	StringClass tmp(0,true);
	for (unsigned i=0;i<TabCount;++i) tmp+="\t";
	WWRELEASE_SAY(("%s%s took %d.%3.3d s, did %d memory allocations of %d bytes\n",
		tmp,
		text,
		(current_time - IntermediateTimeStart)/1000, (current_time - IntermediateTimeStart)%1000,
		current_alloc_count - IntermediateAllocCountStart,
		current_alloc_size - IntermediateAllocSizeStart));
	IntermediateTimeStart=current_time;
	IntermediateAllocCountStart=current_alloc_count;
	IntermediateAllocSizeStart=current_alloc_size;
}

/***********************************************************************************************
 * WWProfileHierachyInfoClass::WWProfileHierachyInfoClass -- Constructor                       *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - pointer to a static string which is the name of this profile node                    *
 * parent - parent pointer                                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
WWProfileHierachyInfoClass::WWProfileHierachyInfoClass( const char * name, WWProfileHierachyInfoClass * parent ) :
	Name( name ),
	TotalCalls( 0 ),
	TotalTime( 0 ),
	Parent( parent ),
	Child( NULL ),
	Sibling( NULL )
{
}

/***********************************************************************************************
 * WWProfileHierachyNodeClass::~WWProfileHierachyNodeClass -- Destructor                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileHierachyInfoClass::~WWProfileHierachyInfoClass( void )
{
	delete Child;
	delete Sibling;
}

