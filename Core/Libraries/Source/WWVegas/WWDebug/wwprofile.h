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
 *                     $Archive:: /Commando/Code/wwdebug/wwprofile.h                          $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/25/02 2:05p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

//#define ENABLE_TIME_AND_MEMORY_LOG
#include "wwstring.h"

#ifdef _UNIX
typedef signed long long __int64;
typedef signed long long _int64;
#endif

// enable profiling by default in debug mode.
#ifdef WWDEBUG
#define ENABLE_WWPROFILE
#endif

extern unsigned WWProfile_Get_System_Time();	// timeGetTime() wrapper
class FileClass;

/*
** A node in the WWProfile Hierarchy Tree
*/
class	WWProfileHierarchyNodeClass {

public:
	WWProfileHierarchyNodeClass( const char * name, WWProfileHierarchyNodeClass * parent );
	WWProfileHierarchyNodeClass( unsigned id, WWProfileHierarchyNodeClass * parent );
	~WWProfileHierarchyNodeClass();

	WWProfileHierarchyNodeClass * Get_Sub_Node( const char * name );

	WWProfileHierarchyNodeClass * Get_Parent()			{ return Parent; }
	WWProfileHierarchyNodeClass * Get_Sibling()		{ return Sibling; }
	WWProfileHierarchyNodeClass * Get_Child()			{ return Child; }

	void Set_Parent( WWProfileHierarchyNodeClass *node )			{ Parent=node; }
	void Set_Sibling( WWProfileHierarchyNodeClass *node )			{ Sibling=node; }
	void Set_Child( WWProfileHierarchyNodeClass *node )			{ Child=node; }

	void								Reset();
	void								Call();
	bool								Return();

	const char *					Get_Name()				{ return Name; }
	int								Get_Total_Calls()		{ return TotalCalls; }
	float								Get_Total_Time()		{ return TotalTime; }
	void								Set_Total_Calls(int calls) { TotalCalls=calls; }
	void								Set_Total_Time(float time) { TotalTime=time; }

	WWProfileHierarchyNodeClass* Clone_Hierarchy(WWProfileHierarchyNodeClass* parent);
	void								Write_To_File(FileClass* file,int recursion);
	void								Add_To_String_Compact(StringClass& string,int recursion);


protected:

	const char *					Name;
	int								TotalCalls;
	float								TotalTime;
	__int64							StartTime;
	int								RecursionCounter;
	unsigned						ProfileStringID;

	WWProfileHierarchyNodeClass *	Parent;
	WWProfileHierarchyNodeClass *	Child;
	WWProfileHierarchyNodeClass *	Sibling;
};

class	WWProfileHierarchyInfoClass {
public:
	WWProfileHierarchyInfoClass( const char * name, WWProfileHierarchyInfoClass * parent );
	~WWProfileHierarchyInfoClass();

	WWProfileHierarchyInfoClass * Get_Parent()			{ return Parent; }
	WWProfileHierarchyInfoClass * Get_Sibling()		{ return Sibling; }
	WWProfileHierarchyInfoClass * Get_Child()			{ return Child; }

	void Set_Parent( WWProfileHierarchyInfoClass *node )			{ Parent=node; }
	void Set_Sibling( WWProfileHierarchyInfoClass *node )			{ Sibling=node; }
	void Set_Child( WWProfileHierarchyInfoClass *node )			{ Child=node; }

	const char *						Get_Name()				{ return Name; }
	void								Set_Name( const char* name )	{ Name=name; }
	int									Get_Total_Calls()		{ return TotalCalls; }
	float								Get_Total_Time()		{ return TotalTime; }
	void								Set_Total_Calls(int calls) { TotalCalls=calls; }
	void								Set_Total_Time(float time) { TotalTime=time; }

protected:

	StringClass							Name;
	int								TotalCalls;
	float								TotalTime;

	WWProfileHierarchyInfoClass *	Parent;
	WWProfileHierarchyInfoClass *	Child;
	WWProfileHierarchyInfoClass *	Sibling;
};

/*
** An iterator to navigate through the tree
*/
class WWProfileIterator
{
public:
	// Access all the children of the current parent
	void				First();
	void				Next();
	bool				Is_Done();

	void				Enter_Child();			// Make the current child the new parent
	void				Enter_Child( int index );	// Make the given child the new parent
	void				Enter_Parent();		// Make the current parent's parent the new parent

	// Access the current child
	const char *	Get_Current_Name()			{ return CurrentChild->Get_Name(); }
	int				Get_Current_Total_Calls()	{ return CurrentChild->Get_Total_Calls(); }
	float				Get_Current_Total_Time()	{ return CurrentChild->Get_Total_Time(); }

	// Access the current parent
	const char *	Get_Current_Parent_Name()			{ return CurrentParent->Get_Name(); }
	int				Get_Current_Parent_Total_Calls()	{ return CurrentParent->Get_Total_Calls(); }
	float				Get_Current_Parent_Total_Time()	{ return CurrentParent->Get_Total_Time(); }

protected:
	WWProfileHierarchyNodeClass *	CurrentParent;
	WWProfileHierarchyNodeClass *	CurrentChild;

	WWProfileIterator( WWProfileHierarchyNodeClass * start );
	friend	class		WWProfileManager;
};


/*
** An iterator to walk through the tree in depth first order
*/
class WWProfileInOrderIterator
{
public:
	void				First();
	void				Next();
	bool				Is_Done();

	// Access the current node
	const char *	Get_Current_Name()			{ return CurrentNode->Get_Name(); }
	int				Get_Current_Total_Calls()	{ return CurrentNode->Get_Total_Calls(); }
	float				Get_Current_Total_Time()	{ return CurrentNode->Get_Total_Time(); }

protected:
	WWProfileHierarchyNodeClass *	CurrentNode;

	WWProfileInOrderIterator();
	friend	class		WWProfileManager;
};


/*
** The Manager for the WWProfile system
*/
class	WWProfileManager {
public:
	WWINLINE static	void					Enable_Profile(bool enable) { IsProfileEnabled=enable; }
	WWINLINE static	bool					Is_Profile_Enabled() { return IsProfileEnabled; }

	static	void								Start_Profile( const char * name );
	static	void								Stop_Profile();

	static	void								Start_Root_Profile( const char * name );
	static	void								Stop_Root_Profile();

	static	void								Reset();
	static	void								Increment_Frame_Counter();
	static	int								Get_Frame_Count_Since_Reset()		{ return FrameCounter; }
	static	float								Get_Time_Since_Reset();

	static	WWProfileIterator *			Get_Iterator();
	static	void								Release_Iterator( WWProfileIterator * iterator );
	static	WWProfileInOrderIterator *	Get_In_Order_Iterator();
	static	void								Release_In_Order_Iterator( WWProfileInOrderIterator * iterator );

	static	WWProfileHierarchyNodeClass *	Get_Root() { return &Root; }

	static	void								Begin_Collecting();
	static	void								End_Collecting(const char* filename);

	static	void								Load_Profile_Log(const char* filename, WWProfileHierarchyInfoClass**& array, unsigned& count);

private:
	static	WWProfileHierarchyNodeClass		Root;
	static	WWProfileHierarchyNodeClass *	CurrentNode;
	static	WWProfileHierarchyNodeClass *	CurrentRootNode;
	static	int									FrameCounter;
	static	__int64								ResetTime;
	static	bool									IsProfileEnabled;

	friend	class		WWProfileInOrderIterator;
};


/*
** WWProfileSampleClass is a simple way to profile a function's scope
** Use the WWPROFILE macro at the start of scope to time
*/
class	WWProfileSampleClass {
	bool IsRoot;
	bool Enabled;
public:
	WWProfileSampleClass( const char * name, bool is_root )		 : IsRoot(is_root), Enabled(WWProfileManager::Is_Profile_Enabled())
	{
		if (Enabled) {
			if (IsRoot) WWProfileManager::Start_Root_Profile( name );
			else WWProfileManager::Start_Profile( name );
		}
	}

	~WWProfileSampleClass()
	{
		if (Enabled) {
			if (IsRoot) WWProfileManager::Stop_Root_Profile();
			else WWProfileManager::Stop_Profile();
		}
	}
};

#ifdef ENABLE_WWPROFILE
#define	WWPROFILE( name )						WWProfileSampleClass _wwprofile( name, false )
#define	WWROOTPROFILE( name )				WWProfileSampleClass _wwprofile( name, true )
#else
#define	WWPROFILE( name )
#define	WWROOTPROFILE( name )
#endif


/*
** WWTimeIt is like WWProfile, but it doesn't save anything, it just times one routine, regardless of thread
*/
class	WWTimeItClass {
public:
	WWTimeItClass( const char * name );
	~WWTimeItClass();
private:
	const char * Name;
	__int64	Time;
};

#ifdef ENABLE_WWPROFILE
#define	WWTIMEIT( name )	WWTimeItClass _wwtimeit( name )
#else
#define	WWTIMEIT( name )
#endif



/*
** WWMeasureItClass is like WWTimeItClass, but it pokes the result into the given float,
** and can be used in the release build.
*/
class	WWMeasureItClass {
public:
	WWMeasureItClass( float * p_result );
	~WWMeasureItClass();

private:
	__int64	Time;
	float *  PResult;
};

// ----------------------------------------------------------------------------
//
// Use the first macro to log time and memory usage within the stack segment.
// Use the second macro to log intermediate values. The intermediate values are
// calculated from the previous intermediate log, so you can log how much each
// item takes by placing the macro after each of the
//
// ----------------------------------------------------------------------------

#ifdef ENABLE_TIME_AND_MEMORY_LOG
#define WWLOG_PREPARE_TIME_AND_MEMORY(t) WWMemoryAndTimeLog memory_and_time_log(t)
#define WWLOG_INTERMEDIATE(t) memory_and_time_log.Log_Intermediate(t)
#else
#define WWLOG_PREPARE_TIME_AND_MEMORY(t)
#define WWLOG_INTERMEDIATE(t)
#endif

struct WWMemoryAndTimeLog
{
	unsigned TimeStart;
	unsigned IntermediateTimeStart;
	int AllocCountStart;
	int IntermediateAllocCountStart;
	int AllocSizeStart;
	int IntermediateAllocSizeStart;
	StringClass Name;
	static unsigned TabCount;

	WWMemoryAndTimeLog(const char* name);
	~WWMemoryAndTimeLog();
	void Log_Intermediate(const char* text);
};
