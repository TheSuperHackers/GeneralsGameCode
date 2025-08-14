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

/* $Header: /G/wwmath/ode.h 9     9/21/99 5:54p Neal_k $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /G/wwmath/ode.h                                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 9/21/99 5:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef ODE_H
#define ODE_H

#include "always.h"
#include "Vector.H"
#include "wwdebug.h"


/*
** StateVectorClass
** The state vector for a system of ordinary differential equations will be
** stored in this form.  It is a dynamically resizeable array so that we don't
** have to hard-code a maximum size.  If needed, in the final product, we could
** do a slight optimization which makes this a normal fixed size array that
** we've determined is "big enough".
*/
class StateVectorClass : public DynamicVectorClass<float>
{
public:
	void Reset(void) { ActiveCount = 0; }
	void Resize(int size) { if (size > VectorMax) { DynamicVectorClass<float>::Resize(size); } }
};


/*
** ODESystemClass
** If a system of Ordinary Differential Equations (ODE's) are put behind an interface
** of this type, they can be integrated using the Integrators defined in this module.
*/
class ODESystemClass
{

public:

	/*
	** Get_Current_State
	** This function should fill the given state vector with the
	** current state of this object.  Each state variable should be
	** inserted into the vector using its 'Add' interface.
	*/
	virtual void	Get_State(StateVectorClass & set_state) = 0;

	/*
	** Set_Current_State
	** This function should read its state from this state vector starting from the
	** given index.  The return value should be the index that the next object should
	** read from (i.e. increment the index past your state)
	*/
	virtual int		Set_State(const StateVectorClass & new_state,int start_index = 0) = 0;

	/*
	** Compute_Derivatives
	** The various ODE solvers will use this interface to ask the ODESystemClass to
	** compute the derivatives of their state.  In some cases, the integrator will
	** pass in a new state vector (test_state) to be used when computing the derivatives.
	** NULL will be passed if they want the derivatives for the initial state.
	** This function works similarly to the Set_State function in that it passes you
	** the index to start reading from and you pass it back the index to continue from.
	*/
	virtual int		Compute_Derivatives(float t,StateVectorClass * test_state,StateVectorClass * dydt,int start_index = 0) = 0;

};


/*
** IntegrationSystem
**
** The Euler_Solve is the simplest but most inaccurate.  It requires only
** a single computation of the derivatives per timestep.
**
** The Midpoint_Solve function will evaluate the derivatives at two points
**
** The Runge_Kutta_Solve requires four evaluations of the derivatives.
** This is the fourth order Runge-Kutta method...
**
** Runge_Kutta5_Solve is an implementation of fifth order Runge-
** Kutta.  It requires six evaluations of the derivatives.
*/

class IntegrationSystem
{
public:

	static void Euler_Integrate(ODESystemClass * sys,float dt);
	static void	Midpoint_Integrate(ODESystemClass * sys,float dt);
	static void	Runge_Kutta_Integrate(ODESystemClass * sys,float dt);
	static void Runge_Kutta5_Integrate(ODESystemClass * odesys,float dt);

};

#endif

