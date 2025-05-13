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

/* $Header: /Commando/Code/wwmath/ODE.CPP 8     7/02/99 10:32a Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwmath/ODE.CPP                               $* 
 *                                                                                             * 
 *                       Author:: Greg_h                                                       * 
 *                                                                                             * 
 *                     $Modtime:: 6/25/99 6:23p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 8                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Euler_Integrate -- uses Eulers method to integrate a system of ODE's                      * 
 *   Midpoint_Integrate -- midpoint method (Runge-Kutta 2) for integration                     * 
 *   Runge_Kutta_Integrate -- Runge Kutta 4 method                                             * 
 *   Runge_Kutta5_Integrate -- 5th order Runge-Kutta                                           * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "ode.h"
#include <assert.h>

static StateVectorClass		Y0;
static StateVectorClass		Y1;
static StateVectorClass		_WorkVector0;
static StateVectorClass		_WorkVector1;
static StateVectorClass		_WorkVector2;
static StateVectorClass		_WorkVector3;
static StateVectorClass		_WorkVector4;
static StateVectorClass		_WorkVector5;
static StateVectorClass		_WorkVector6;
static StateVectorClass		_WorkVector7;

/*********************************************************************************************** 
 * Euler_Solve -- uses Eulers method to integrate a system of ODE's                            * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * odesys - pointer to the ODE system to integrate                                             * 
 * dt - size of the timestep                                                                   * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 * state vector in odesys will be updated for the next timestep                                * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   08/11/1997 GH  : Created.                                                                 * 
 *   6/25/99    GTH : Updated to the new integrator system                                     *
 *=============================================================================================*/
void IntegrationSystem::Euler_Integrate(ODESystemClass * sys, float dt)
{
	WWASSERT(sys != NULL);

	/*
	** Get the current state
	*/
	Y0.Reset();
	sys->Get_State(Y0);
	Y1.Resize(Y0.Count());

	/*
	** make aliases to the work-vectors we need
	*/
	StateVectorClass & dydt = _WorkVector0;
	dydt.Resize(Y0.Count());

	/*
	** Euler method, just evaluate the derivative, multiply
	** by the time-step and add to the current state vector.
	*/
	sys->Compute_Derivatives(0,NULL,&dydt);

	Y1.Resize(Y0.Count());
	for (int i = 0; i < Y0.Count(); i++) {
		Y1[i] = Y0[i] + dydt[i] * dt;
	}

	sys->Set_State(Y1);
}

/*********************************************************************************************** 
 * Midpoint_Integrate -- midpoint method (Runge-Kutta 2)                                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * sys - pointer to the ODE system to integrate                                                * 
 * dt - size of the timestep                                                                   * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 * state vector in odesys will be updated for the next timestep                                * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   08/11/1997 GH  : Created.                                                                 * 
 *   6/25/99    GTH : Updated to the new integrator system                                     *
 *=============================================================================================*/
void IntegrationSystem::Midpoint_Integrate(ODESystemClass * sys,float dt)
{
	int i;

	/*
	** Get the current state
	*/
	Y0.Reset();
	sys->Get_State(Y0);
	Y1.Resize(Y0.Count());

	/*
	** make aliases to the work-vectors we need
	*/
	StateVectorClass & dydt = _WorkVector0;
	StateVectorClass & ymid = _WorkVector1;
	dydt.Resize(Y0.Count());
	ymid.Resize(Y0.Count());

	/*
	** MidPoint method, first evaluate the derivitives of the
	** state vector just like the Euler method.  
	*/
	sys->Compute_Derivatives(0.0f,NULL,&dydt);

	/*
	** Compute the midpoint between the Euler solution and 
	** the input values.
	*/
	for (i=0; i<Y0.Count(); i++) {
		ymid[i] = Y0[i] + dt * dydt[i] / 2.0f;
	}
	
	/*
	** Re-compute derivatives at this point.  
	*/
	sys->Compute_Derivatives(dt/2.0f,&ymid,&dydt);

	/*
	** Use these derivatives to compute the solution.
	*/
	for (i=0; i<Y0.Count(); i++) {
		Y1[i] = Y0[i] + dt * dydt[i];
	}

	sys->Set_State(Y1);
}


/*********************************************************************************************** 
 * Runge_Kutta_Integrate -- Runge Kutta 4 method                                               * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * odesys - pointer to the ODE system to integrate                                             * 
 * dt - size of the timestep                                                                   * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 * state vector in odesys will be updated for the next timestep                                * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   08/11/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void IntegrationSystem::Runge_Kutta_Integrate(ODESystemClass * sys,float dt)
{
	int i;
	float dt2 = dt / 2.0f;
	float dt6 = dt / 6.0f;

	/*
	** Get the current state
	*/
	Y0.Reset();
	sys->Get_State(Y0);
	Y1.Resize(Y0.Count());

	/*
	** make aliases to the work-vectors we need
	*/
	StateVectorClass & dydt =	_WorkVector0;
	StateVectorClass & dym =	_WorkVector1;
	StateVectorClass & dyt =	_WorkVector2;
	StateVectorClass & yt =		_WorkVector3;
	dydt.Resize(Y0.Count());
	dym.Resize(Y0.Count());
	dyt.Resize(Y0.Count());
	yt.Resize(Y0.Count());

	/*
	** First Step
	*/
	sys->Compute_Derivatives(0.0f,NULL,&dydt);
	for (i=0; i<Y0.Count(); i++) {
		yt[i] = Y0[i] + dt2 * dydt[i];
	}
	
	/*
	** Second Step
	*/
	sys->Compute_Derivatives(dt2, &yt, &dyt);
	for (i=0; i<Y0.Count(); i++) {
		yt[i] = Y0[i] + dt2 * dyt[i];
	}
	
	/*
	** Third Step
	*/
	sys->Compute_Derivatives(dt2, &yt, &dym);
	for (i=0; i<Y0.Count(); i++) {
		yt[i] = Y0[i] + dt*dym[i];
		dym[i] += dyt[i];
	}

	/*
	** Fourth Step
	*/
	sys->Compute_Derivatives(dt, &yt, &dyt);
	for (i=0; i<Y0.Count(); i++) {
		Y1[i] = Y0[i] + dt6 * (dydt[i] + dyt[i] + 2.0f*dym[i]);
	}

	sys->Set_State(Y1);
}

/*********************************************************************************************** 
 * Runge_Kutta5_Integrate -- 5th order Runge-Kutta                                             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * odesys - pointer to the ODE system to integrate                                             * 
 * dt - size of the timestep                                                                   * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 * state vector in odesys will be updated for the next timestep                                * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   08/11/1997 GH  : Created.                                                                 * 
 *   6/25/99    GTH : Converted to the new Integrator system                                   *
 *=============================================================================================*/
void IntegrationSystem::Runge_Kutta5_Integrate(ODESystemClass * odesys,float dt)
{
	int i;
	int veclen;
	static const float a2 =			0.2f;
	static const float a3 =			0.3f;
	static const float a4 =			0.6f;
	static const float a5 =			1.0f;
	static const float a6 =			0.875f;
	static const float b21 =		0.2f;
	static const float b31 =		3.0f/40.0f;
	static const float b32 =		9.0f/40.0f;
	static const float b41 =		0.3f;
	static const float b42 =		-0.9f;
	static const float b43 =		1.2f;
	static const float b51 =		-11.0f /54.0f;
	static const float b52 =		2.5f;
	static const float b53 =		-70.0f/27.0f;
	static const float b54 =		35.0f/27.0f;
	static const float b61 =		1631.0f/55296.0f;
	static const float b62 =		175.0f/512.0f;
	static const float b63 =		575.0f/13824.0f;
	static const float b64 =		44275.0f/110592.0f;
	static const float b65 =		253.0f/4096.0f;
	static const float c1 =			37.0f/378.0f;
	static const float c3 =			250.0f/621.0f;
	static const float c4 =			125.0f/594.0f;
	static const float c6 =			512.0f/1771.0f;
	static const float dc5 =		-277.0f/14336.0f;
	static const float dc1 =		c1 - 2825.0f/27648.0f;
	static const float dc3 =		c3 - 18575.0f/48384.0f;
	static const float dc4 =		c4 - 13525.0f/55296.0f;
	static const float dc6 =		c6 - 0.25f;

	/*
	** Get the current state
	*/
	Y0.Reset();
	odesys->Get_State(Y0);
	veclen = Y0.Count();
	Y1.Resize(veclen);

	/*
	** make aliases to the work-vectors we need
	*/
	StateVectorClass & dydt	= _WorkVector0;
	StateVectorClass & ak2	= _WorkVector1;
	StateVectorClass & ak3	= _WorkVector2;
	StateVectorClass & ak4	= _WorkVector3;
	StateVectorClass & ak5	= _WorkVector4;
	StateVectorClass & ak6	= _WorkVector5;
	StateVectorClass & ytmp	= _WorkVector6;
	StateVectorClass & yerr	= _WorkVector7;

	dydt.Resize(veclen);
	ak2.Resize(veclen);
	ak3.Resize(veclen);
	ak4.Resize(veclen);
	ak5.Resize(veclen);
	ak6.Resize(veclen);
	ytmp.Resize(veclen);
	yerr.Resize(veclen);

	// First step
	odesys->Compute_Derivatives(0.0f,NULL,&dydt);
	for (i=0;i<veclen;i++) {
		ytmp[i] = Y0[i] + b21*dt*dydt[i];
	}

	// Second step
	odesys->Compute_Derivatives(a2*dt, &ytmp, &ak2);
	for (i=0; i<veclen; i++) {
		ytmp[i] = Y0[i] + dt*(b31*dydt[i] + b32*ak2[i]);
	}
	
	// Third step
	odesys->Compute_Derivatives(a3*dt, &ytmp, &ak3);
	for (i=0; i<veclen; i++) {
		ytmp[i] = Y0[i] + dt*(b41*dydt[i] + b42*ak2[i] + b43*ak3[i]);
	}
	
	// Fourth step
	odesys->Compute_Derivatives(a4*dt, &ytmp, &ak4);
	for (i=0; i<veclen; i++) {
		ytmp[i] = Y0[i] + dt*(b51*dydt[i] + b52*ak2[i] + b53*ak3[i] + b54*ak4[i]);
	}

	// Fifth step
	odesys->Compute_Derivatives(a5*dt, &ytmp, &ak5);
	for (i=0; i<veclen; i++) {
		ytmp[i] = Y0[i] + dt*(b61*dydt[i] + b62*ak2[i] + b63*ak3[i] + b64*ak4[i] + b65*ak5[i]);
	}

	// Sixth step
	odesys->Compute_Derivatives(a6*dt, &ytmp, &ak6);
	for (i=0; i<veclen; i++) {
		Y1[i] = Y0[i] + dt*(c1*dydt[i] + c3*ak3[i] + c4*ak4[i] + c6*ak6[i]);
	}

	// Error approximation!  
	// (maybe I should use this someday? nah not going to use this integrator anyway...)
	for (i=0; i<veclen; i++) {
		yerr[i] = dt*(dc1*dydt[i] + dc3*ak3[i] + dc4*ak4[i] + dc5*ak5[i] + dc6*ak6[i]);
	}

	odesys->Set_State(Y1);
}

