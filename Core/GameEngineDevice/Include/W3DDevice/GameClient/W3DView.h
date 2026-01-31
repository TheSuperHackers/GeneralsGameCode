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

// FILE: W3DView.h ////////////////////////////////////////////////////////////////////////////////
//
// W3D implementation of the game view window.  View windows are literally
// a window into the game world that have width, height, and camera
// controls for what to look at
//
// Author: Colin Day, April 2001
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/STLTypedefs.h"
#include "GameClient/ParabolicEase.h"
#include "GameClient/View.h"
#include "WW3D2/camera.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Drawable;

///////////////////////////////////////////////////////////////////////////////////////////////////
enum {MAX_WAYPOINTS=25};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
typedef struct
{
	Int			numWaypoints;
	Coord3D	waypoints[MAX_WAYPOINTS+2];		// We pad first & last for interpolation.
	Real		waySegLength[MAX_WAYPOINTS+2];	// Length of each segment;
	Real		cameraAngle[MAX_WAYPOINTS+2];	// Camera Angle;
	Int			timeMultiplier[MAX_WAYPOINTS+2];	// Time speedup factor.
	Real		groundHeight[MAX_WAYPOINTS+1];	// Ground height.
	Real		totalTimeMilliseconds;					// Num of ms to do this movement.
	Real		elapsedTimeMilliseconds;				// Time since start.
	Real		totalDistance;								// Total length of paths.
	Real		curSegDistance;								// How far we are along the current seg.
	Int			shutter;
	Int			curSegment;										// The current segment.
	Int			curShutter;										// The current shutter.
	Int			rollingAverageFrames;					// Number of frames to roll.
	ParabolicEase ease;										// Ease in/out function.
} TMoveAlongWaypointPathInfo;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
typedef struct
{
	Int			numFrames;						///< Number of frames to rotate.
	Int			curFrame;							///< Current frame.
	Int			startTimeMultiplier;
	Int			endTimeMultiplier;
	Int			numHoldFrames;				///< Number of frames to hold the camera before finishing the movement
	ParabolicEase ease;
	Bool		trackObject;					///< Are we tracking an object or just rotating?
	struct Target {
		ObjectID targetObjectID;			///< Target if we are tracking an object instead of just rotating
		Coord3D	targetObjectPos;			///< Target's position (so we can stay looking at that spot if he dies)
	};
	struct Angle {
		Real startAngle;
		Real endAngle;
	};
	union {
		Target target;
		Angle angle;
	};
} TRotateCameraInfo;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
typedef struct
{
	Int			numFrames;						///< Number of frames to pitch.
	Int			curFrame;							///< Current frame.
	Real		angle;
	Real		startPitch;
	Real		endPitch;
	Int			startTimeMultiplier;
	Int			endTimeMultiplier;
	ParabolicEase ease;
} TPitchCameraInfo;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
typedef struct
{
	Int			numFrames;						///< Number of frames to zoom.
	Int			curFrame;							///< Current frame.
	Real		startZoom;
	Real		endZoom;
	Int			startTimeMultiplier;
	Int			endTimeMultiplier;
	ParabolicEase ease;
} TZoomCameraInfo;

// ------------------------------------------------------------------------------------------------
/** W3DView implementation of the game view class.  This allows us to create
  * a "window" into the game world that can be sized and contains controls
	* for manipulating the camera */
// ------------------------------------------------------------------------------------------------
class W3DView : public View, public SubsystemInterface
{

public:
	W3DView();
	~W3DView();

	virtual void init( void );  ///< init/re-init the W3DView
	virtual void reset( void );
	virtual void drawView( void );  ///< draw this view
	virtual void updateView(void);	///<called once per frame to determine the final camera and object transforms
	virtual void stepView(); ///< Update view for every fixed time step

	virtual void draw( );  ///< draw this view
	virtual void update( );	///<called once per frame to determine the final camera and object transforms

	virtual Drawable *pickDrawable( const ICoord2D *screen, Bool forceAttack, PickType pickType );  ///< pick drawable given the screen pixel coords.  If force attack, picks bridges.

	/// all drawables in the 2D screen region will call the 'callback'
	virtual Int iterateDrawablesInRegion( IRegion2D *screenRegion,
																Bool (*callback)( Drawable *draw, void *userData ),
																void *userData );

	virtual void setWidth( Int width );
	virtual void setHeight( Int height );
	virtual void setOrigin( Int x, Int y);			///< Sets location of top-left view corner on display

	virtual void scrollBy( Coord2D *delta );  ///< Shift the view by the given delta

	virtual void forceRedraw();

	virtual void setAngle( Real angle );										///< Rotate the view around the up axis by the given angle
	virtual void setPitch( Real angle );											///< Rotate the view around the horizontal axis to the given angle
	virtual void setAngleAndPitchToDefault( void );							///< Set the view angle back to default

	virtual void lookAt( const Coord3D *o );											///< Center the view on the given coordinate
	virtual void initHeightForMap( void );												///<  Init the camera height for the map at the current position.
	virtual void moveCameraTo(const Coord3D *o, Int milliseconds,  Int shutter, Bool orient, Real easeIn, Real easeOut);
	virtual void moveCameraAlongWaypointPath(Waypoint *pWay, Int frames, Int shutter, Bool orient, Real easeIn, Real easeOut);
	virtual Bool isCameraMovementFinished(void);
	virtual Bool isCameraMovementAtWaypointAlongPath(void);
 	virtual void resetCamera(const Coord3D *location, Int frames, Real easeIn, Real easeOut);	///< Move camera to location, and reset to default angle & zoom.
 	virtual void rotateCamera(Real rotations, Int frames, Real easeIn, Real easeOut);					///< Rotate camera about current viewpoint.
	virtual void rotateCameraTowardObject(ObjectID id, Int milliseconds, Int holdMilliseconds, Real easeIn, Real easeOut);	///< Rotate camera to face an object, and hold on it
	virtual void rotateCameraTowardPosition(const Coord3D *pLoc, Int milliseconds, Real easeIn, Real easeOut, Bool reverseRotation);	///< Rotate camera to face a location.
	virtual void cameraModFreezeTime(void){ m_freezeTimeForCameraMovement = true;}					///< Freezes time during the next camera movement.
	virtual void cameraModFreezeAngle(void);												///< Freezes time during the next camera movement.
	virtual Bool isTimeFrozen(void){ return m_freezeTimeForCameraMovement;}					///< Freezes time during the next camera movement.
	virtual void cameraModFinalZoom(Real finalZoom, Real easeIn, Real easeOut);	///< Final zoom for current camera movement.
	virtual void cameraModRollingAverage(Int framesToAverage);			///< Number of frames to average movement for current camera movement.
	virtual void cameraModFinalTimeMultiplier(Int finalMultiplier); ///< Final time multiplier for current camera movement.
	virtual void cameraModFinalPitch(Real finalPitch, Real easeIn, Real easeOut);	///< Final pitch for current camera movement.
	virtual void cameraModLookToward(Coord3D *pLoc);								///< Sets a look at point during camera movement.
	virtual void cameraModFinalLookToward(Coord3D *pLoc);						///< Sets a look at point during camera movement.
	virtual void cameraModFinalMoveTo(Coord3D *pLoc);			///< Sets a final move to.
	// (gth) C&C3 animation controlled camera feature
	virtual void cameraEnableSlaveMode(const AsciiString & thingtemplateName, const AsciiString & boneName);
	virtual void cameraDisableSlaveMode(void);
	virtual void cameraEnableRealZoomMode(void); //WST 10.18.2002
	virtual void cameraDisableRealZoomMode(void);

	virtual	void Add_Camera_Shake(const Coord3D & position,float radius, float duration, float power); //WST 10.18.2002
	virtual Int	 getTimeMultiplier(void) {return m_timeMultiplier;};///< Get the time multiplier.
	virtual void setTimeMultiplier(Int multiple) {m_timeMultiplier = multiple;}; ///< Set the time multiplier.
	virtual void setDefaultView(Real pitch, Real angle, Real maxHeight);
	virtual void zoomCamera( Real finalZoom, Int milliseconds, Real easeIn, Real easeOut );
	virtual void pitchCamera( Real finalPitch, Int milliseconds, Real easeIn, Real easeOut );

	virtual void setHeightAboveGround(Real z);
	virtual void setZoom(Real z);
	virtual void setZoomToDefault( void );									///< Set zoom to default value

	virtual void setFieldOfView( Real angle );							///< Set the horizontal field of view angle

  virtual WorldToScreenReturn worldToScreenTriReturn( const Coord3D *w, ICoord2D *s );	///< Transform world coordinate "w" into screen coordinate "s"
	virtual void screenToTerrain( const ICoord2D *screen, Coord3D *world );  ///< transform screen coord to a point on the 3D terrain
	virtual void screenToWorldAtZ( const ICoord2D *s, Coord3D *w, Real z );  ///< transform screen point to world point at the specified world Z value

	CameraClass *get3DCamera() const { return m_3DCamera; }

	virtual const Coord3D& get3DCameraPosition() const;

	virtual void setCameraLock(ObjectID id);
	virtual void setSnapMode( CameraLockType lockType, Real lockDist );

	/// Add an impulse force to shake the camera
	virtual void shake( const Coord3D *epicenter, CameraShakeType shakeType );

	virtual Real getFXPitch( void ) const { return m_FXPitch; }					///< returns the FX pitch angle

	virtual Bool setViewFilterMode(FilterModes filterMode);			///< Turns on viewport special effect (black & white mode)
	virtual Bool setViewFilter(FilterTypes filter);			///< Turns on viewport special effect (black & white mode)
	virtual void setViewFilterPos(const Coord3D *pos);			///<  Passes a position to the special effect filter.
	virtual FilterModes getViewFilterMode(void) {return m_viewFilterMode;}			///< Turns on viewport special effect (black & white mode)
	virtual FilterTypes getViewFilterType(void) {return m_viewFilter;}			///< Turns on viewport special effect (black & white mode)
	virtual void setFadeParameters(Int fadeFrames, Int direction);
	virtual void set3DWireFrameMode(Bool enable);	///<enables custom wireframe rendering of 3D viewport

	Bool updateCameraMovements(void);
	virtual void forceCameraConstraintRecalc(void) { calcCameraConstraints(); }

	virtual void setGuardBandBias( const Coord2D *gb ) { m_guardBandBias.x = gb->x; m_guardBandBias.y = gb->y; }


private:

	CameraClass *m_3DCamera;												///< camera representation for 3D scene
	CameraClass *m_2DCamera;												///< camera for UI overlayed on top of 3D scene
	FilterModes m_viewFilterMode;
	FilterTypes m_viewFilter;
	Bool m_isWireFrameEnabled;
	Bool m_nextWireFrameEnabled;											///< used to delay wireframe changes by 1 frame (needed for transitions).


	Coord2D m_shakeOffset;													///< the offset to add to the camera position
	Real m_shakeAngleCos;														///< the cosine of the orientation of the oscillation
	Real m_shakeAngleSin;														///< the sine of the orientation of the oscillation
	Real m_shakeIntensity;													///< the intensity of the oscillation
	Vector3 m_shakerAngles;													//WST 11/12/2002 new multiple instance camera shaker system

	TRotateCameraInfo	m_rcInfo;
	Bool		m_doingRotateCamera;										///< True if we are doing a camera rotate.

	TPitchCameraInfo m_pcInfo;
	Bool m_doingPitchCamera;
	TZoomCameraInfo m_zcInfo;
	Bool m_doingZoomCamera;

	Bool m_doingScriptedCameraLock;									///< True if we are following a unit via script

	Real		m_FXPitch;															///< Camera effects pitch.  0 = flat, infinite = look down, 1 = normal.

	TMoveAlongWaypointPathInfo m_mcwpInfo;					///< Move camera along waypoint path info.
	Bool	m_doingMoveCameraOnWaypointPath;				///< If true, moving camera along waypoint path.
	Bool	m_CameraArrivedAtWaypointOnPathFlag;

	Bool		m_freezeTimeForCameraMovement;
	Int			m_timeMultiplier;												///< Time speedup multiplier.

	Bool		m_cameraHasMovedSinceRequest;					///< If true, throw out all saved locations
	VecPosRequests	m_locationRequests;		///< These are cached. New requests are added here

	Coord3D m_cameraOffset;													///< offset for camera from view center
	Coord3D m_previousLookAtPosition;													///< offset for camera from view center
	Coord2D m_scrollAmount;													///< scroll speed
	Real m_scrollAmountCutoff;											///< scroll speed at which we do not adjust height

	Real m_groundLevel;															///< height of ground.

	Region2D m_cameraConstraint;										///< m_pos should be constrained to be within this area
	Bool m_cameraConstraintValid;										///< if f, recalc cam constraints

	void setCameraTransform( void );								///< set the transform matrix of m_3DCamera, based on m_pos & m_angle
	void buildCameraTransform( Matrix3D *transform ) ;			///< calculate (but do not set) the transform matrix of m_3DCamera, based on m_pos & m_angle
	void calcCameraConstraints() ;			///< recalc m_cameraConstraint
	void moveAlongWaypointPath(Real milliseconds); ///< Move camera along path.
	void getPickRay(const ICoord2D *screen, Vector3 *rayStart, Vector3 *rayEnd);	///<returns a line segment (ray) originating at the given screen position
	void setupWaypointPath(Bool orient);					///< Calculates distances & angles for moving along a waypoint path.
	void rotateCameraOneFrame(void);							///< Do one frame of a rotate camera movement.
	void zoomCameraOneFrame(void);							///< Do one frame of a zoom camera movement.
	void pitchCameraOneFrame(void);							///< Do one frame of a pitch camera movement.
	void getAxisAlignedViewRegion(Region3D &axisAlignedRegion);	///< Find 3D Region enclosing all possible drawables.
	void calcDeltaScroll(Coord2D &screenDelta);

	// (gth) C&C3 animation controlled camera feature
	Bool				m_isCameraSlaved;
	Bool				m_useRealZoomCam;
	AsciiString		m_cameraSlaveObjectName;
	AsciiString		m_cameraSlaveObjectBoneName;
};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern Real TheW3DFrameLengthInMsec;
