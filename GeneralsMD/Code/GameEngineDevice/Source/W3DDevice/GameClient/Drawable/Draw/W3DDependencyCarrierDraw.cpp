// FILE: W3DDependencyCarrierDraw.cpp ////////////////////////////////////////////////////////////////////////////

// Desc: Render stationed Objects with carriers
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ContainModule.h"
#include "W3DDevice/GameClient/Module/W3DDependencyCarrierDraw.h"

//-------------------------------------------------------------------------------------------------
W3DDependencyCarrierDraw::W3DDependencyCarrierDraw(Thing* thing, const ModuleData* moduleData)
	: W3DDependencyModelDraw(thing, moduleData)
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DDependencyCarrierDraw::~W3DDependencyCarrierDraw()
{
}

//-------------------------------------------------------------------------------------------------
// All this does is stop the call path if we haven't been cleared to draw yet
void W3DDependencyCarrierDraw::doDrawModule(const Matrix3D* transformMtx)
{
	Drawable* myDrawable = getDrawable();
	if (!myDrawable)
		return;

	const Object* me = myDrawable->getObject();
	if (!me)
		return;

	if (!me->isContained()) {
		W3DModelDraw::doDrawModule(transformMtx);
	}
	else {

		if (m_dependencyCleared)
		{
			// We've been cleared by the thing we were waiting to draw, so we can draw.
			W3DModelDraw::doDrawModule(transformMtx);
			m_dependencyCleared = FALSE;

			Drawable* theirDrawable = NULL;

			if (me->getContainedBy()) // no enclosing container check here, as carrier wants to draw units anyway
				theirDrawable = me->getContainedBy()->getDrawable();

			if (!theirDrawable)
				return;

			myDrawable->imitateStealthLook(*theirDrawable);

		}
	}
}

//-------------------------------------------------------------------------------------------------
void W3DDependencyCarrierDraw::notifyDrawModuleDependencyCleared()
{
	m_dependencyCleared = TRUE;

  //this is called shortly before rendering so we set the hidden status to that of the parent if contained
	const Object* me = getDrawable()->getObject();
	// No special adjustements when currently not contained
	if (me && me->isContained()) {
		const Object* container = me->getContainedBy();
		if (container && container->getDrawable()) {
			me->getDrawable()->setDrawableHidden(container->getDrawable()->isDrawableEffectivelyHidden());
		}
	}
}

// ------------------------------------------------------------------------------------------------
void W3DDependencyCarrierDraw::adjustTransformMtx(Matrix3D& mtx) const
{
	W3DModelDraw::adjustTransformMtx(mtx);

	// We have an additional adjustment to make, we want to use a bone in our container if there is one
	const Object* me = getDrawable()->getObject();

	// No special adjustements when currently not contained
	if (me && !me->isContained()) return;

	const W3DDependencyModelDrawModuleData* md = getW3DDependencyModelDrawModuleData();

	if (md->m_attachToDrawableBoneInContainer.isNotEmpty()
		&& me
		&& me->getContainedBy()
		// && !me->getContainedBy()->getContain()->isEnclosingContainerFor(me) // no enclosing check
		)
	{
		// If we are currently "riding on", then our client position is determined by the client position of
		// a particular bone in our container object.  Our logic position is updated by OpenContain.
		const Drawable* theirDrawable = me->getContainedBy()->getDrawable();
		if (theirDrawable)
		{
			Matrix3D theirBoneMtx;

			AsciiString boneName;

			short slot = me->getContainedBy()->getContain()->getRiderSlot(me->getID());
			if (slot > -1)
				boneName.format("%s%02d", md->m_attachToDrawableBoneInContainer.str(), slot + 1);

			if ((slot > -1) && theirDrawable->getCurrentWorldspaceClientBonePositions(boneName.str(), theirBoneMtx))
			{
				mtx = theirBoneMtx;
			}
			else if (theirDrawable->getCurrentWorldspaceClientBonePositions(md->m_attachToDrawableBoneInContainer.str(), theirBoneMtx))
			{
				mtx = theirBoneMtx;
			}
			else
			{
				mtx = *theirDrawable->getTransformMatrix();
				DEBUG_LOG(("m_attachToDrawableBoneInContainer %s not found", md->m_attachToDrawableBoneInContainer.str()));
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DDependencyCarrierDraw::crc(Xfer* xfer)
{

	// extend base class
	W3DDependencyModelDraw::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void W3DDependencyCarrierDraw::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	W3DDependencyModelDraw::xfer(xfer);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DDependencyCarrierDraw::loadPostProcess(void)
{

	// extend base class
	W3DDependencyModelDraw::loadPostProcess();

}  // end loadPostProcess
