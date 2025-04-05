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
* FILE
*     $Archive: /Commando/Code/wwlib/wwNotify.h $
*
* DESCRIPTION
*     These templates provide implementation of the Subject-Observer pattern.
*
* PROGRAMMER
*     Steve Clinard
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Modtime: 11/13/01 10:49a $
*     $Revision: 8 $
*
* 06/28/02 KM Notify name change to avoid MAX conflicts                                       *
******************************************************************************/

#ifndef __NOTIFY_H__
#define __NOTIFY_H__

// Reduce warning level for STL
#if defined(_MSC_VER)
#pragma warning(push, 3)
#endif

#include <vector>
#include <algorithm>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <assert.h>

template<typename Event> class Notifier;
template<typename Event> class Observer;

template<typename Event> class Observer
	{
	public:
		typedef std::vector< Notifier<Event>* > NotifierColl;

		Observer() :
				mNotifiers(NULL)
			{}

		virtual ~Observer()
			{StopObserving();}

		//! Handle event notification
		virtual void HandleNotification(Event&) = 0;

		//! Notifier has ended notification of this event
		virtual void NotificationEnded(const Notifier<Event>& notifier)
			{
			NotifierColl::iterator pos = std::find(mNotifiers.begin(),
				mNotifiers.end(), &notifier);

			if (pos != mNotifiers.end())
				{
				mNotifiers.erase(pos);
				}
			}

		//! Request notification of this event
		virtual void NotifyMe(Notifier<Event>& notifier)
			{notifier.AddObserver(*this);}

		//! Stop observing event
		void StopObserving()
			{
			while (mNotifiers.size() > 0)
				{
				Notifier<Event>* notifier = mNotifiers.back();
				assert(notifier && "ERROR: NULL pointer in collection.");
				notifier->RemoveObserver(*this);
				}
			}

	protected:
		Observer(const Observer<Event>& observer);
		const Observer<Event>& operator=(const Observer<Event>&);

	private:
		friend class Notifier<Event>;
		NotifierColl mNotifiers;
	};


#define DECLARE_OBSERVER(Event) \
	virtual void NotifyMe(Notifier<Event>& observer) \
		{Notifier<Event>::AddObserver(observer);}


template<typename Event> class Notifier
	{
	public:
		typedef std::vector< Observer<Event>* > ObserverColl;

		Notifier()
			{}

		virtual ~Notifier()
			{
			for (int index = mObservers.size(); index--;)
				{
				mObservers[index]->NotificationEnded(*this);
				}
			}

		//! Send event notification to all observers of this event.
		virtual void NotifyObservers(Event& event)
			{
			for (unsigned int index = 0; index < mObservers.size(); index++)
				{
				mObservers[index]->HandleNotification(event);
				}
			}

		//! Add an observer of this event
		virtual void AddObserver(Observer<Event>& observer)
			{
			ObserverColl::iterator pos = std::find(mObservers.begin(),
				mObservers.end(), &observer);

			if (pos == mObservers.end())
				{
				observer.mNotifiers.push_back(this);
				mObservers.push_back(&observer);
				}
			}

		//! Remove an observer of this event
		virtual void RemoveObserver(Observer<Event>& observer)
			{
			ObserverColl::iterator pos = std::find(mObservers.begin(),
				mObservers.end(), &observer);

			if (pos != mObservers.end())
				{
				observer.NotificationEnded(*this);
				mObservers.erase(pos);
				}
			}

		virtual bool HasObservers(void) const
			{return !mObservers.empty();}

	private:
		//! Observer collection
		ObserverColl mObservers;
	};


#define DECLARE_NOTIFIER(Event) \
	virtual void NotifyObservers(Event& event) \
		{Notifier<Event>::NotifyObservers(event);} \
	virtual void AddObserver(Observer<Event>& observer) \
		{Notifier<Event>::AddObserver(observer);} \
	virtual void RemoveObserver(Observer<Event>& observer) \
		{Notifier<Event>::RemoveObserver(observer);} 


/*-----------------------------------------------------------------------------
 * The following templates are useful for defining unique types to use as
 * Events from types such as strings or integers.
 *---------------------------------------------------------------------------*/

/* TypedEvent<T. V>
 * 
 * The first type (T) must be a class or other unique type. This need not
 * be a "real" class. It could be a forward declared class, which is enough
 * to make the template class unique.
 *
 * The second type (V) is the event data. "UString" and "int" are obvious
 * choices.
 *
 * Typedef'ing the template class is a good thing to do.
 */
template<typename T, typename V>
class TypedEvent
	{
	public:
		TypedEvent(V& value) :
				mValue(value)
			{}

		inline V& operator()()
			{return mValue;}

		inline V& Subject(void)
			{return mValue;}

	protected:
		V& mValue;
	};

template<typename T, typename O>
class TypedEventPtr
	{
	public:
		TypedEventPtr(O* subject) :
				mSubject(subject)
			{}

		inline O* Subject(void)
			{return mSubject;}

		inline O* operator()()
			{return mSubject;}

	protected:
		O* mSubject;
	};

template<typename A, typename O>
class TypedActionPtr :
		public TypedEventPtr<A, O>
	{
	public:
		A GetAction(void) const
			{return mAction;}

		TypedActionPtr(A action, O* data) :
				TypedEventPtr<A, O>(data),
				mAction(action)
			{}

		~TypedActionPtr()
			{}

	protected:
		A mAction;
	};

template<typename A, typename B>
class TypedEventPair
	{
	public:
		TypedEventPair(A itemA, B itemB) :
				mItemA(itemA),
				mItemB(itemB)
			{}

		inline A GetItemA(void)
			{return mItemA;}

		inline B GetItemB(void)
			{return mItemB;}

	protected:
		A mItemA;
		B mItemB;
	};

#endif // __NOTIFY_H__
