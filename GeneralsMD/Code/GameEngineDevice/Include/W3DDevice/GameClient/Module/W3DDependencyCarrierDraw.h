// FILE: W3DDependencyCarrierDraw.h /////////////////////////////////////////////////////////////////////////
// Desc:   Draw as dependent when garrisoned in carrier
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ModelState.h"
#include "Common/DrawModule.h"
#include "W3DDevice/GameClient/Module/W3DDependencyModelDraw.h"

//-------------------------------------------------------------------------------------------------
class W3DDependencyCarrierDraw : public W3DDependencyModelDraw
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(W3DDependencyCarrierDraw, "W3DDependencyCarrierDraw")
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(W3DDependencyCarrierDraw, W3DDependencyModelDrawModuleData)

public:

	W3DDependencyCarrierDraw(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	virtual void doDrawModule(const Matrix3D* transformMtx) override;
	virtual void notifyDrawModuleDependencyCleared();///< if you were waiting for something before you drew, it's ready now
	virtual void adjustTransformMtx(Matrix3D& mtx) const override;

protected:

};
