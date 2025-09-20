#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/FireWeaponAdvancedUpdate.h"
#include "GameLogic/WeaponStatus.h"
#include "GameLogic/GameLogic.h"
#include <GameLogic/TerrainLogic.h>
#include "GameLogic/PartitionManager.h"
#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include <Common/PlayerList.h>
#include <GameClient/RadiusDecal.h>


//-------------------------------------------------------------------------------------------------
FireWeaponAdvancedUpdateModuleData::FireWeaponAdvancedUpdateModuleData()
{
	m_weaponTemplate = NULL;
  m_initialDelayFrames = 0;
	m_exclusiveWeaponDelay = 0;
	m_scatterTargets = std::vector<Coord2D>();
	m_scatterTargetScalar = 1.0;
	m_fireHeight = -1.0; //if <0 we ignore
	m_lockOnRadius = 0.0f;
	m_scatterOnLockedStructuresMajorRadius = false;
	m_fireOffsetXMin = 0.0f;
	m_fireOffsetXMax = 0.0f;
	m_fireOffsetYMin = 0.0f;
	m_fireOffsetYMax = 0.0f;
	m_decalDuration = 0;
	m_scatterRadius = 0;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void FireWeaponAdvancedUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Weapon",								INI::parseWeaponTemplate,	      NULL, offsetof( FireWeaponAdvancedUpdateModuleData, m_weaponTemplate ) },
		{ "InitialDelay",					INI::parseDurationUnsignedInt,	NULL, offsetof( FireWeaponAdvancedUpdateModuleData, m_initialDelayFrames ) },
		{ "ExclusiveWeaponDelay",	INI::parseDurationUnsignedInt,	NULL, offsetof( FireWeaponAdvancedUpdateModuleData, m_exclusiveWeaponDelay ) },
		{ "ScatterTarget",				FireWeaponAdvancedUpdateModuleData::parseScatterTarget,			NULL,							0 },
		{ "ScatterTargetScalar",	INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_scatterTargetScalar)  },
		{ "FireHeight",	          INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_fireHeight)  },
		{ "LockOnRadius",	          INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_lockOnRadius)  },
		{ "ScatterOnLockedStructuresMajorRadius", INI::parseBool, NULL,	offsetof(FireWeaponAdvancedUpdateModuleData, m_scatterOnLockedStructuresMajorRadius)  },
		{ "FireOffsetXMin",	          INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_fireOffsetXMin)  },
		{ "FireOffsetXMax",	          INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_fireOffsetXMax)  },
		{ "FireOffsetYMin",	          INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_fireOffsetYMin)  },
		{ "FireOffsetYMax",	          INI::parseReal,			NULL,							 offsetof(FireWeaponAdvancedUpdateModuleData, m_fireOffsetYMax)  },
		{ "Decal",									RadiusDecalTemplate::parseRadiusDecalTemplate,	NULL, offsetof(FireWeaponAdvancedUpdateModuleData, m_decalTemplate) },
		{ "DecalRadius",						INI::parseReal, NULL, offsetof(FireWeaponAdvancedUpdateModuleData, m_decalRadius) },
		{ "DecalDuration",					INI::parseDurationUnsignedInt, NULL, offsetof(FireWeaponAdvancedUpdateModuleData, m_decalDuration) },
		{ "ScatterRadius",					INI::parseReal, NULL, offsetof(FireWeaponAdvancedUpdateModuleData, m_scatterRadius) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

/*static*/ void FireWeaponAdvancedUpdateModuleData::parseScatterTarget(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
{
	// Accept multiple listings of Coord2D's.
	FireWeaponAdvancedUpdateModuleData* self = (FireWeaponAdvancedUpdateModuleData*)instance;

	Coord2D target;
	target.x = 0;
	target.y = 0;
	INI::parseCoord2D(ini, NULL, &target, NULL);

	self->m_scatterTargets.push_back(target);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireWeaponAdvancedUpdate::FireWeaponAdvancedUpdate( Thing *thing, const ModuleData* moduleData ) :
	UpdateModule( thing, moduleData ),
	m_weapon(NULL)
{
	const WeaponTemplate *tmpl = getFireWeaponAdvancedUpdateModuleData()->m_weaponTemplate;
	if (tmpl)
	{
		m_weapon = TheWeaponStore->allocateNewWeapon(tmpl, PRIMARY_WEAPON);
		m_weapon->loadAmmoNow( getObject() );
	}


  m_initialDelayFrame = TheGameLogic->getFrame() + getFireWeaponAdvancedUpdateModuleData()->m_initialDelayFrames;
	m_radiusDecalRemoveFrame = TheGameLogic->getFrame() + getFireWeaponAdvancedUpdateModuleData()->m_decalDuration;
	m_initialPosition = Coord3D(0, 0, 0);
	m_initialized = false;
	m_deliveryDecal.clear();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireWeaponAdvancedUpdate::~FireWeaponAdvancedUpdate( void )
{
	m_deliveryDecal.clear();

	if (m_weapon)
		deleteInstance(m_weapon);
}

// PARTITION FILTERS!
//-----------------------------------------------------------------------------
class PartitionFilterValidEnemies : public PartitionFilter
{
private:
	const Object* m_obj;
public:
	PartitionFilterValidEnemies(const Object* obj) : m_obj(obj) {}

	virtual Bool allow(Object* objOther)
	{
		// this is way fast (bit test) so do it first.
		if (objOther->isEffectivelyDead())
			return false;

		if (objOther->isDestroyed())
			return false;

		// this is also way fast (bit test) so do it next.
		if (objOther->isOffMap() != m_obj->isOffMap())
			return false;

		Relationship r = m_obj->getRelationship(objOther);
		if (r != ENEMIES)
			return false;

		if (objOther->testStatus(OBJECT_STATUS_MASKED))
			return false;

		if (objOther->isKindOf(KINDOF_UNATTACKABLE))
			return false;

		if (objOther->isAirborneTarget())
			return false;

		// this object is not currently auto-acquireable
		if (objOther->testStatus(OBJECT_STATUS_NO_ATTACK_FROM_AI))
			return false;

		return true;
	}

#if defined(RTS_DEBUG)
	virtual const char* debugGetName() { return "PartitionFilterValidEnemies"; }
#endif
};

/**
 * @brief Calculates a 3D transformation matrix for an object to face a target location.
 *
 * This function computes the rotation needed for an object, which initially faces the
 * positive X-axis, to orient itself towards a specified target. The resulting matrix
 * includes this rotation and the object's original position.
 *
 * @author gemini-2.5
 *
 * @param objectPosition The current 3D coordinates of the object.
 * @param targetPosition The 3D coordinates of the target the object should face.
 * @param outMatrix A reference to a Matrix3D object where the resulting
 *                  transformation matrix will be stored.
 */
void CalculateFacingMatrix(const Coord3D& objectPosition, const Coord3D& targetPosition, Matrix3D& outMatrix)
{
	// 1. Calculate the direction vector (this will be the new X-axis).
	// This vector points from the object's position to the target's position.
	Coord3D newX;
	newX.set(targetPosition.x - objectPosition.x,
		targetPosition.y - objectPosition.y,
		targetPosition.z - objectPosition.z);

	// Edge Case: If the object and target are at the same position,
	// the direction cannot be determined. In this case, we'll return an
	// identity matrix set to the object's position.
	if (newX.lengthSqr() < 0.0001f)
	{
		outMatrix.Make_Identity();
		outMatrix.Set_Translation(Vector3(objectPosition.x, objectPosition.y, objectPosition.z));
		return;
	}
	newX.normalize();

	// 2. Determine the new Y and Z axes.
	// We start by defining a temporary "up" vector. A common choice is the world's
	// up-axis (e.g., Z-axis for a Z-up coordinate system).
	Coord3D worldUp;
	worldUp.set(0.0f, 0.0f, 1.0f); // Assuming a Z-up world

	Coord3D newY;

	// Edge Case: If the object is looking straight up or down, the direction vector (newX)
	// will be parallel to the worldUp vector. Their cross product would be a zero vector,
	// which cannot be normalized.
	// We can detect this by checking if the absolute value of the dot product is close to 1.
	// Since worldUp is (0,0,1), the dot product is simply newX.z.
	if (std::abs(newX.z) > 0.999f)
	{
		// Use a different vector to calculate the cross product. The world's X-axis is a safe choice.
		Coord3D alternateUp(1.0f, 0.0f, 0.0f);
		Coord3D::crossProduct(&newX, &alternateUp, &newY);
	}
	else
	{
		// Calculate the new Y-axis using the cross product. The result is a vector
		// perpendicular to both the direction and the world's up vector.
		Coord3D::crossProduct(&worldUp, &newX, &newY);
	}

	newY.normalize();

	// 3. Calculate the new Z-axis.
	// The cross product of the new X and Y axes gives us the new Z-axis,
	// completing the right-handed coordinate system.
	Coord3D newZ;
	Coord3D::crossProduct(&newX, &newY, &newZ);
	// Normalization is not strictly necessary here if newX and newY are already
	// normalized and perpendicular, but it adds robustness.
	newZ.normalize();

	// 4. Construct the final transformation matrix.
	// The Matrix3D::Set method takes the basis vectors (X, Y, Z) and the position.
	// We need to convert our Coord3D vectors to the Vector3 type expected by the Matrix3D class.
	outMatrix.Set(
		Vector3(newX.x, newX.y, newX.z),
		Vector3(newY.x, newY.y, newY.z),
		Vector3(newZ.x, newZ.y, newZ.z),
		Vector3(objectPosition.x, objectPosition.y, objectPosition.z)
	);
}

Coord3D FireWeaponAdvancedUpdate::getNextTargetPos() {
	const FireWeaponAdvancedUpdateModuleData* data = getFireWeaponAdvancedUpdateModuleData();
	if (!data->m_scatterTargets.empty() && m_weapon->getClipSize()>0) {

		//If we have scatter targets and a clipsize, use the targets
		int shot_index = m_weapon->getClipSize() - m_weapon->getRemainingAmmo();
		if (!data->m_scatterTargets.empty()) {
			size_t target_idx = shot_index % data->m_scatterTargets.size();
			Coord2D scatterOffset = data->m_scatterTargets.at(target_idx);
			Coord3D targetPos = m_initialPosition;
			//Calculate Target from Scatter

			scatterOffset.x *= data->m_scatterTargetScalar;
			scatterOffset.y *= data->m_scatterTargetScalar;

			/*const Coord3D srcPos = m_initialPosition; //*getObject()->getPosition();
			Real angle = 0.0f; // getObject()->getOrientation();
			angle += atan2(targetPos.y - srcPos.y, targetPos.x - srcPos.x);

			Real cosA = Cos(angle);
			Real sinA = Sin(angle);
			Real scatterOffsetRotX = scatterOffset.x * cosA - scatterOffset.y * sinA;
			Real scatterOffsetRotY = scatterOffset.x * sinA + scatterOffset.y * cosA;
			scatterOffset.x = scatterOffsetRotX;
			scatterOffset.y = scatterOffsetRotY;*/

			targetPos.x += scatterOffset.x;
			targetPos.y += scatterOffset.y;
			targetPos.z = TheTerrainLogic->getGroundHeight(targetPos.x, targetPos.y);

			return targetPos;
		}
	}
	return *getObject()->getPosition();
}


void FireWeaponAdvancedUpdate::adjustFireHeight(Object* targetLock, Coord3D* targetPos) {
	const FireWeaponAdvancedUpdateModuleData* data = getFireWeaponAdvancedUpdateModuleData();
	if (data->m_fireHeight >= 0.0) {
		Coord3D pos_target;
		if (targetLock != nullptr) {
			pos_target = *targetLock->getPosition();
		}
		else {
			pos_target = *targetPos;
		}
		Coord3D pos = pos_target;
		pos.z = TheTerrainLogic->getGroundHeight(pos.x, pos.y) + data->m_fireHeight;

		Real offsetX = GameLogicRandomValueReal(data->m_fireOffsetXMin, data->m_fireOffsetXMax);
		Real offsetY = GameLogicRandomValueReal(data->m_fireOffsetYMin, data->m_fireOffsetYMax);
		pos.x += offsetX;
		pos.y += offsetY;

		getObject()->setPosition(&pos);
		//Make the object look down
		Matrix3D rot;
		CalculateFacingMatrix(pos, pos_target, rot);
		getObject()->setTransformMatrix(&rot);
	}
}

Coord3D getScatterRadiusOffset(Real radius) {
	Coord3D firingOffset;
	firingOffset.zero();
	if (radius > 0.0f) {
		Real scatterRadius = GameLogicRandomValueReal(0, radius);
		Real scatterAngleRadian = GameLogicRandomValueReal(0, 2 * PI);

		firingOffset.x = scatterRadius * Cos(scatterAngleRadian);
		firingOffset.y = scatterRadius * Sin(scatterAngleRadian);
	}
	return firingOffset;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime FireWeaponAdvancedUpdate::update( void )
{
	const FireWeaponAdvancedUpdateModuleData* data = getFireWeaponAdvancedUpdateModuleData();

	if (!m_initialized) {
		m_initialPosition = *getObject()->getPosition();
		m_initialized = true;
	}

	if (data->m_decalTemplate.valid() && data->m_decalRadius > 0.0f && data->m_decalDuration>0) {
		if (TheGameLogic->getFrame() >= m_radiusDecalRemoveFrame) {
			if (!m_deliveryDecal.isEmpty()) {
				m_deliveryDecal.clear();
			}
		}
		else {
			if (m_deliveryDecal.isEmpty()) {
				data->m_decalTemplate.createRadiusDecal(m_initialPosition,
					data->m_decalRadius, getObject()->getControllingPlayer(), m_deliveryDecal);
			}
			else {
				m_deliveryDecal.update();
			}
		}
	}

  if ( TheGameLogic->getFrame() < m_initialDelayFrame )
    return UPDATE_SLEEP_NONE;

	// If my weapon is ready, shoot it.
	if( isOkayToFire() )
	{
		Coord3D targetPos = getNextTargetPos();
		Coord3D firingOffset = getScatterRadiusOffset(data->m_scatterRadius);
		targetPos.add(&firingOffset);

		//DEBUG_LOG(("NextPos: %f, %f, %f", targetPos.x, targetPos.y, targetPos.z));

		if (data->m_lockOnRadius > 0.0f) {

			// Check for a valid target near impact
			PartitionFilterValidEnemies filterObvious(getObject());
			PartitionFilterStealthedAndUndetected filterStealth(getObject(), false);
			//PartitionFilterPossibleToAttack filterAttack(ATTACK_NEW_TARGET, getObject(), CMD_FROM_AI);
			PartitionFilterFreeOfFog filterFogged(getObject()->getControllingPlayer()->getPlayerIndex());
			PartitionFilter* filters[5];
			Int numFilters = 0;
			filters[numFilters++] = &filterObvious;
			filters[numFilters++] = &filterStealth;
			//filters[numFilters++] = &filterAttack;
			filters[numFilters++] = &filterFogged;
			filters[numFilters] = NULL;

			Object* targetLock = nullptr;
			// THIS WILL FIND A VALID TARGET WITHIN THE TARGETING RETICLE
			ObjectIterator* iter = ThePartitionManager->iterateObjectsInRange(&targetPos,
				data->m_lockOnRadius,
				FROM_BOUNDINGSPHERE_2D,
				filters,
				ITER_SORTED_NEAR_TO_FAR);
			MemoryPoolObjectHolder holder(iter);
			for (Object* theEnemy = iter->first(); theEnemy && targetLock == nullptr; theEnemy = iter->next())
			{
				if (theEnemy)
				{
					targetLock = theEnemy;
					//DEBUG_LOG(("LockOn: %s", theEnemy->getTemplate()->getName().str()));
				}
			}
			/* scatter at locked buildings within geometry radius */
			if (data->m_scatterOnLockedStructuresMajorRadius && targetLock != nullptr) {
				if (targetLock->isStructure()) {
					Real radius = targetLock->getGeometryInfo().getMajorRadius();
					targetPos = *targetLock->getPosition();
					targetLock = nullptr;

					Coord3D firingOffset = getScatterRadiusOffset(radius);

					targetPos.x += firingOffset.x;
					targetPos.y += firingOffset.y;
				}
			}

			adjustFireHeight(targetLock, &targetPos);
			if (targetLock != nullptr) {
				m_weapon->fireWeapon(getObject(), targetLock);
			}
			else {
				m_weapon->fireWeapon(getObject(), &targetPos);
			}
		}
		else {
			adjustFireHeight(nullptr, &targetPos);
			m_weapon->forceFireWeapon(getObject(), &targetPos);
		}
	}
	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool FireWeaponAdvancedUpdate::isOkayToFire()
{
	const Object *me = getObject();
	const FireWeaponAdvancedUpdateModuleData *data = getFireWeaponAdvancedUpdateModuleData();

	if( m_weapon == NULL )
		return FALSE;

	// Weapon is reloading
	if( m_weapon->getStatus() != READY_TO_FIRE )
		return FALSE;

	if( me->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) )
		return FALSE; // no hitting with a 0% building, cheater

	// Firing a real weapon surpresses this module
	if( data->m_exclusiveWeaponDelay > 0  &&  ( TheGameLogic->getFrame() < (me->getLastShotFiredFrame() + data->m_exclusiveWeaponDelay) ) )
		return FALSE;

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FireWeaponAdvancedUpdate::crc( Xfer *xfer )
{
	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FireWeaponAdvancedUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// weapon
	xfer->xferSnapshot( m_weapon );

	if (version >= 2) {
		xfer->xferUnsignedInt(&m_initialDelayFrame);

		xfer->xferBool(&m_initialized);
		xfer->xferUser(&m_initialPosition, sizeof(Coord3D));
		m_deliveryDecal.xferRadiusDecal(xfer);
		xfer->xferUnsignedInt(&m_radiusDecalRemoveFrame);
	}
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FireWeaponAdvancedUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
