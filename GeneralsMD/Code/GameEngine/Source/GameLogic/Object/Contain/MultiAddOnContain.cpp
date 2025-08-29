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

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FILE: MultiAddOnContain.cpp ////////////////////////////////////////////////////////////////////////
//  Author: Mark Lorenzen, April, 2003
//
//  Desc:
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/MultiAddOnContain.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Weapon.h"




// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
MultiAddOnContainModuleData::MultiAddOnContainModuleData()
{
  //	m_initialPayload.count = 0;
  m_drawPips = TRUE;
  m_addOnEntries.clear();
  m_addOnBoneName = "FIREPOINT";
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MultiAddOnContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  TransportContainModuleData::buildFieldParse(p);

  static const FieldParse dataFieldParse[] =
  {
    { "PayloadTemplateName",  INI::parseAsciiStringVectorAppend, NULL, offsetof(MultiAddOnContainModuleData, m_payloadTemplateNameData) },
    { "ShouldDrawPips",  INI::parseBool, NULL, offsetof(MultiAddOnContainModuleData, m_drawPips) },
    { "AddOnBoneName",  INI::parseAsciiString, NULL, offsetof(MultiAddOnContainModuleData, m_addOnBoneName) },
    { "AddOnEntry", parseAddOnEntry, NULL, 0 },
    { 0, 0, 0, 0 }
  };
  p.add(dataFieldParse);
}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MultiAddOnContainModuleData::parseAddOnEntry( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
  DEBUG_LOG((">>> MultiAddOnContainModuleData::parseAddOnEntry 0"));
	MultiAddOnContainModuleData* self = (MultiAddOnContainModuleData*)instance;
	const char* riderName = ini->getNextToken();
	const char* addOnName = ini->getNextToken();
  DEBUG_LOG((">>> MultiAddOnContainModuleData::parseAddOnEntry 1"));
  self->m_addOnEntries.insert_or_assign(NAMEKEY(riderName), AsciiString(addOnName));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
MultiAddOnContain::MultiAddOnContain(Thing* thing, const ModuleData* moduleData) :
  TransportContain(thing, moduleData)
{
  DEBUG_LOG((">>> MultiAddOnContain::INIT 0"));

  m_payloadCreated = FALSE;

  m_addOnList.clear();

  const MultiAddOnContainModuleData* data = getMultiAddOnContainModuleData();

  // why slotCapacity and not containMax?
  // What about units requiring multiple slots?
  // Should we base it off GarrisonContain instead?
  for (UnsignedInt i = 0; i < data->m_slotCapacity; i++) {
    AddOnSlotData addOn;
    addOn.occupantID = INVALID_ID;
    addOn.portableID = INVALID_ID;
    addOn.slot = i;
    m_addOnSlots.push_back(addOn);
  }

  DEBUG_LOG((">>> MultiAddOnContain::INIT 1, m_addOnSlots.size() = %d", m_addOnSlots.size()));
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MultiAddOnContain::~MultiAddOnContain(void)
{

}


void MultiAddOnContain::onObjectCreated(void)
{
  MultiAddOnContain::createPayload();
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
//UpdateSleepTime MultiAddOnContain::update()
//{
//  //DEBUG_LOG((">>> MultiAddOnContain::update 0, m_addOnList.size() = %d", m_addOnList.size()));
//  // TODO
//  //for (ContainedItemList::iterator it = m_addOnList.begin(); it != m_addOnList.end(); ++it) {
//  for (Object* obj : m_addOnList) {
//    if (obj) {
//      obj->setPosition(getObject()->getPosition());
//      obj->setOrientation(getObject()->getOrientation()); //Why orientation and not Transform?
//    }
//  }
//
// //DEBUG_LOG((">>> MultiAddOnContain::update 1, m_addOnList.size() = %d", m_addOnList.size()));
//  return TransportContain::update(); // extend base
//}


//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MultiAddOnContain::redeployOccupants(void)
{
  TransportContain::redeployOccupants();

  redeployAddOns();
}


//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MultiAddOnContain::createPayload()
{
  MultiAddOnContainModuleData* self = (MultiAddOnContainModuleData*)getMultiAddOnContainModuleData();


  // Any number of different passengers can be loaded here at init time
  Object* object = getObject();
  ContainModuleInterface* contain = object->getContain();
  if (contain)
  {
    contain->enableLoadSounds(FALSE);

    TemplateNameList list = self->m_payloadTemplateNameData;
    TemplateNameIterator iter = list.begin();
    while (iter != list.end())
    {
      const ThingTemplate* temp = TheThingFactory->findTemplate(*iter);
      if (temp)
      {
        Object* payload = TheThingFactory->newObject(temp, object->getTeam());

        if (contain->isValidContainerFor(payload, true))
        {
          contain->addToContain(payload);
        }
        else
        {
          DEBUG_CRASH(("MultiAddOnContain::createPayload: %s is full, or not valid for the payload %s!", object->getName().str(), self->m_initialPayload.name.str()));
        }

      }

      ++iter;
    }

    contain->enableLoadSounds(TRUE);

  } // endif contain

  m_payloadCreated = TRUE;

}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//VecObjectPtr& MultiAddOnContain::getPortableStructures(void) const
//{
//  DEBUG_LOG((">>> MultiAddOnContainModuleData::getPortableStructures 0"));
//  VecObjectPtr portableStructures;
//  int i = 0;
//  for (std::vector<AddOnSlotData>::const_iterator it = m_addOnSlots.begin(); it != m_addOnSlots.end(); it++) {
//    DEBUG_LOG((">>> MultiAddOnContainModuleData::getPortableStructures[%d]: occupantID = %d, portableID = %d, slot = %d",
//      i, (*it).occupantID, (*it).portableID, (*it).slot));
//    i++;
//    Object* obj = TheGameLogic->findObjectByID((*it).portableID);
//    if (obj)
//      portableStructures.push_back(obj);
//  }
//  DEBUG_LOG((">>> MultiAddOnContainModuleData::getPortableStructures 1"));
//  return portableStructures;
//}


// ------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MultiAddOnContain::onBodyDamageStateChange(const DamageInfo* damageInfo,
  BodyDamageType oldState,
  BodyDamageType newState)  ///< state change callback
{
  // Need to apply state change to the portable structure
  if (newState != BODY_RUBBLE)
  {
    for (Object* obj : m_addOnList) {
      if (obj) {
        obj->getBodyModule()->setDamageState(newState);
      }
    }
  }

}

//-------------------------------------------------------------------------------------------------
void MultiAddOnContain::onDie(const DamageInfo* damageInfo)
{
  for (Object* obj : m_addOnList) {
    if (obj)
      obj->kill();
  }

  TransportContain::onDie(damageInfo);//extend base class
}

//-------------------------------------------------------------------------------------------------
void MultiAddOnContain::onDelete(void)
{
  for (Object* obj : m_addOnList) {
    if (obj)
      TheGameLogic->destroyObject(obj);
  }
    
  TransportContain::onDelete();
}

// ------------------------------------------------------------------------------------------------
void MultiAddOnContain::onCapture(Player* oldOwner, Player* newOwner)
{
  //  Need to setteam() the portable structure, that's all;

  for (Object* obj : m_addOnList) {
    if (obj)
      obj->setTeam(newOwner->getDefaultTeam());
  }
}

//-------------------------------------------------------------------------------------------------
void MultiAddOnContain::onSelling()
{
  // An OpenContain tells everyone to leave.
  orderAllPassengersToExit(CMD_FROM_AI, TRUE);

  /*for (Object* obj : m_addOnList) {
    if (obj)
      TheGameLogic->destroyObject(obj);
  }*/
}

//-------------------------------------------------------------------------------------------------
//void MultiAddOnContain::addToContainList(Object* obj)
//{
//  if (obj->isKindOf(KINDOF_PORTABLE_STRUCTURE))
//  {
//    // This is probably redundant and should happen in addToContain instead
//    //m_portableStructureIDs.push_back(obj->getID());
//    //obj->friend_setContainedBy(getObject());//fool portable into thinking my object is his container
//  }
//  else {
//    TransportContain::addToContainList(obj);
//  }
//}

//-------------------------------------------------------------------------------------------------
//void MultiAddOnContain::addToContain(Object* obj)
//{
//  if (obj->isKindOf(KINDOF_PORTABLE_STRUCTURE))
//  {
//    const MultiAddOnContainModuleData* d = getMultiAddOnContainModuleData();
//
//    ////Find the slot we belong to
//    //for (slot = 0; slot < d->m_slotCapacity; slot++) {
//    //  if (m_addOnSlots[i].occupantID == obj->getID()) {
//    //    if (m_addOnSlots[i].portableID != INVALID_ID) {
//    //      DEBUG_LOG(("MultiAddOnContain::createAddOnForRider - portable for slot %d is already occupied?!", slot));
//    //    }
//    //    break;
//    //  }E
//    //}
//    //m_portableStructureIDs.push_back(obj->getID());
//
//    // TODO
//
//    obj->friend_setContainedBy(getObject());//fool portable into thinking my object is his container
//
//  }
//  else {
//    TransportContain::addToContain(obj);
//    
//  }
//}

//-------------------------------------------------------------------------------------------------
//void MultiAddOnContain::removeFromContain(Object* obj, Bool exposeStealthUnits)
//{
//  if (obj->isKindOf(KINDOF_PORTABLE_STRUCTURE))
//  {
//    for (VecObjectIDIt it = m_portableStructureIDs.begin(); it != m_portableStructureIDs.end(); ) {
//      if (obj->getID() == (*it))
//        m_portableStructureIDs.erase(it);
//    }
//  }
//  else
//  {
//    // Get corresponding Structure and remove it
//    TransportContain::removeFromContain(obj, exposeStealthUnits);
//  }
//}


//-------------------------------------------------------------------------------------------------
Bool MultiAddOnContain::isValidContainerFor(const Object* obj, Bool checkCapacity) const
{
  const MultiAddOnContainModuleData* d = getMultiAddOnContainModuleData();

  // TODO: Check list of allowed passengers
  if (d->m_addOnEntries.find(NAMEKEY(obj->getTemplate()->getName())) == d->m_addOnEntries.end())
    return FALSE;

  return TransportContain::isValidContainerFor(obj, checkCapacity);
}


//-------------------------------------------------------------------------------------------------
//const Object* MultiAddOnContain::friend_getRider() const
//{
//  // The draw order dependency bug for riders means that our draw module needs to cheat to get around it.
//
//  if (m_portableStructureID != INVALID_ID)
//  {
//    const Object* portableAsRider = TheGameLogic->findObjectByID(m_portableStructureID);
//    return portableAsRider;
//  }
//
//  return NULL;
//}

//-------------------------------------------------------------------------------------------------
Bool MultiAddOnContain::isEnclosingContainerFor(const Object* obj) const
{

  //for (std::vector<AddOnSlotData>::const_iterator it = m_addOnSlots.begin(); it != m_addOnSlots.end(); it++) {
  //  if ((*it).portableID == obj->getID()) {
  //    const Object* portableAsRider = TheGameLogic->findObjectByID((*it).portableID);
  //    if (portableAsRider == obj)
  //      return FALSE;
  //  }
  //}

  for (Object* addOn : m_addOnList) {
    if (addOn == obj) {
      //DEBUG_LOG((">>> MultiAddOnContain::isEnclosingContainerFor -> object found -> FALSE"));
      return FALSE;
    }
  }

  return TransportContain::isEnclosingContainerFor(obj);
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// if my object gets selected, then my visible passengers should, too
// this gets called from
void MultiAddOnContain::clientVisibleContainedFlashAsSelected()
{
  for (Object* obj : m_addOnList) {
    if (obj) {
      Drawable* draw = obj->getDrawable();
      if (draw)
      {
        draw->flashAsSelected();
      }
    }
  }
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool MultiAddOnContain::isPassengerAllowedToFire(ObjectID id) const
{
  if (getObject() && getObject()->getContainedBy()) // nested containment voids firing, always
    return FALSE;

  //if (id == INVALID_ID && m_addOnList.size() > 0) {
  //  return TRUE;  // This handles cases where we just check if we can attack at all.
  //}

  // Wait, the portable structures aren't actually contained, so we skip this
  //for (std::vector<AddOnSlotData>::iterator it = m_addOnSlots.begin(); it != m_addOnSlots.end(); ) {
  //  if (*it->portableID == id && id != INVALID_ID) {
  //    return TRUE;  // Portable structures can always fire
  //  }
  //}

  return TransportContain::isPassengerAllowedToFire(id); //extend for everything else
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MultiAddOnContain::onContaining(Object* obj, Bool wasSelected)
{
  DEBUG_LOG((">>> MultiAddOnContainModuleData::onContaining 0"));
  // extend base class
  TransportContain::onContaining(obj, wasSelected);

  // Spawn the Turret.
  // This should be the only place where we do this! No turret should ever enter the actual container
  createAddOnForRider(obj);

  DEBUG_LOG((">>> MultiAddOnContainModuleData::onContaining 1"));
}  // end onContaining

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MultiAddOnContain::onRemoving(Object* obj)
{
  // extend base class
  TransportContain::onRemoving(obj);

  // Remove the corresponding turret
  removeAddOnForRider(obj);

} // end onRemoving


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool MultiAddOnContain::createAddOnForRider(Object* obj)
{
  const MultiAddOnContainModuleData* d = getMultiAddOnContainModuleData();

  DEBUG_LOG((">>> MultiAddOnContainModuleData::createAddOnForRider 0"));

  // Get next free slot.
  UnsignedInt slot = -1;
  //for (std::vector<AddOnSlotData>::iterator it = m_addOnSlots.begin(); it != m_addOnSlots.end(); ) {
  //}
  for (UnsignedInt i = 0; i < d->m_slotCapacity; i++) {

    if (m_addOnSlots[i].occupantID == INVALID_ID) {
      // sanity checks
      if (m_addOnSlots[i].portableID != INVALID_ID)
        DEBUG_LOG(("MultiAddOnContain::createAddOnForRider - WARNING: - Portable in slot %d is not empty", i));

      if (m_addOnSlots[i].slot != i)
        DEBUG_LOG(("MultiAddOnContain::createAddOnForRider - WARNING: - Slot number is invalid?!", i));

      if (slot == -1)
        slot = i; // we choose this slot, but still continue the loop for sanity checks.
    }

    if (m_addOnSlots[i].occupantID == obj->getID()) {
      DEBUG_LOG(("MultiAddOnContain::createAddOnForRider - WARNING: - Object %d already exists in slot %d?!", obj->getID(), i));
      return false;   
    }
  }
  if (slot == -1) {
    DEBUG_LOG(("MultiAddOnContain::createAddOnForRider - WARNING: - no free slot found!"));
    return false;
  }

  // create the actual object
  Object* addOnObj = NULL;
  AddOnEntryMap::const_iterator it = d->m_addOnEntries.find(NAMEKEY(obj->getTemplate()->getName()));
  if (it != d->m_addOnEntries.end())
  {
    AsciiString tmplName = (*it).second;
    if (tmplName.isNotEmpty()) {
      const ThingTemplate* thing = TheThingFactory->findTemplate(tmplName);
      if (thing)
      {
        addOnObj = TheThingFactory->newObject(thing, getObject()->getTeam());
      }
    }
  }

  if (addOnObj == NULL) {
    DEBUG_LOG(("MultiAddOnContain::createAddOnForRider - WARNING: - failed to create AddOn object!"));
    return false;
  }

  // TODO: figure out if we need to align to a bone
  addOnObj->friend_setContainedBy(getObject());//fool portable into thinking my object is his container
  addOnObj->setPosition(getObject()->getPosition());
  addOnObj->setTransformMatrix(getObject()->getTransformMatrix());

  // Need to hide if they are hidden.
  if (getObject()->getDrawable() && addOnObj->getDrawable() && getObject()->getDrawable()->isDrawableEffectivelyHidden())
    addOnObj->getDrawable()->setDrawableHidden(TRUE);

  // Register rider and addOn
  m_addOnSlots[slot].occupantID = obj->getID();
  m_addOnSlots[slot].portableID = addOnObj->getID();

  m_addOnList.push_back(addOnObj);

  redeployAddOns();

  DEBUG_LOG((">>> MultiAddOnContainModuleData::createAddOnForRider 1"));

  return true;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool MultiAddOnContain::removeAddOnForRider(Object* obj)
{
  const MultiAddOnContainModuleData* d = getMultiAddOnContainModuleData();

  // Search slots for our object
  UnsignedInt slot = -1;

  for (UnsignedInt i = 0; i < d->m_slotCapacity; i++) {
    if (m_addOnSlots[i].occupantID == obj->getID()) {
      // sanity checks
      if (m_addOnSlots[i].portableID == INVALID_ID)
        DEBUG_LOG(("MultiAddOnContain::removeAddOnForRider - WARNING: - Portable in slot %d is empty?!", i));

      if (m_addOnSlots[i].slot != i)
        DEBUG_LOG(("MultiAddOnContain::removeAddOnForRider - WARNING: - Slot number is invalid?!", i));

      if (slot == -1) {
        slot = i;
      }
      else {
        DEBUG_LOG(("MultiAddOnContain::removeAddOnForRider - WARNING: - Occupant in multiple slots?", i));
      }
    }
  }
  if (slot == -1) {
    DEBUG_LOG(("MultiAddOnContain::removeAddOnForRider - WARNING: - rider was not found!"));
    return false;
  }

  Object* addOnObj = TheGameLogic->findObjectByID(m_addOnSlots[slot].portableID);
  if (!addOnObj) {
    DEBUG_LOG(("MultiAddOnContain::removeAddOnForRider - WARNING: - rider object was not found!"));
    return false;
  }

  ContainedItemsList::iterator it = std::find(m_addOnList.begin(), m_addOnList.end(), addOnObj);
  if (it != m_addOnList.end())
  {
    m_addOnList.erase(it);
  }

  m_addOnSlots[slot].occupantID = INVALID_ID;
  m_addOnSlots[slot].portableID = INVALID_ID;

  TheGameLogic->destroyObject(addOnObj);

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MultiAddOnContain::redeployAddOns(void)
{
  for (const AddOnSlotData& addOn : m_addOnSlots) {
    Object* obj = TheGameLogic->findObjectByID(addOn.portableID);
    if (obj) {
      putObjAtSlot(obj, addOn.slot);
    }
  }
}

//-------------------------------------------------------------------------------------------------
/** Place the object at the 3D position of the next fire point to use */
//-------------------------------------------------------------------------------------------------
void MultiAddOnContain::putObjAtSlot(Object* obj, short slot)
{
  const MultiAddOnContainModuleData* d = getMultiAddOnContainModuleData();

  // Load and cache addOn positions
  //UnsignedShort count = 0;
  //if (m_addOnPoints.size() == 0 && m_noAddOnPointsInArt == false)
  //{
  //  Matrix3D matrix;
  //  for (int i = 0; i < m_addOnSlots.size(); i++) {
  //    AsciiString boneName;
  //    boneName.format("%s%02d", d->m_addOnBoneName.str(), i + 1);
  //    Bool found = getObject()->getSingleLogicalBonePositionOnTurret(TURRET_MAIN, boneName.str(), NULL, &matrix);
  //    if (found)
  //      count++;
  //    m_addOnPoints.push_back(matrix);
  //  }

  //  //
  //  // if there is still no firepoints in the art, we'll set a flag so that we don't
  //  // ever go through the art stuff again
  //  //
  //  if (count == 0)
  //    m_noAddOnPointsInArt = TRUE;
  //}  // end if

  //
  // if there are no fire points in the art we just put the object at the center
  // of the object
  //
  //if (m_noAddOnPointsInArt == TRUE)
  //{
  //  obj->setOrientation(getObject()->getOrientation());
  //  obj->setPosition(getObject()->getPosition());
  //  return;

  //}  // end if

  // get the position
  Matrix3D matrix;
  Bool found = FALSE;
  AsciiString boneName;
  boneName.format("%s%02d", d->m_addOnBoneName.str(), slot + 1);
  if (d->m_passengersInTurret)
  {
    found = getObject()->getSingleLogicalBonePositionOnTurret(TURRET_MAIN, boneName.str(), NULL, &matrix);
  }
  else
  {
    found = getObject()->getSingleLogicalBonePosition(boneName.str(), NULL, &matrix);
  }

  if (!found) {
    obj->setOrientation(getObject()->getOrientation());
    obj->setPosition(getObject()->getPosition());
    return;
  }

  obj->setTransformMatrix(&matrix);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
short MultiAddOnContain::getRiderSlot(ObjectID riderID) const {
  for (std::vector<AddOnSlotData>::const_iterator it = m_addOnSlots.begin(); it != m_addOnSlots.end(); it++) {
    if ((*it).occupantID == riderID) {
      return (*it).slot;
    }
  }
  return -1;
}
// ------------------------------------------------------------------------------------------------
short MultiAddOnContain::getPortableSlot(ObjectID portableID) const {
  for (std::vector<AddOnSlotData>::const_iterator it = m_addOnSlots.begin(); it != m_addOnSlots.end(); it++) {
    if ((*it).portableID == portableID) {
      return (*it).slot;
    }
  }
  return -1;
}
// ------------------------------------------------------------------------------------------------

//const ContainedItemsList* MultiAddOnContain::getAddOnList() const {
//  if (m_addOnList_valid) {
//    return &m_addOnList;
//  }
//
//  m_addOnList.clear();
//  for (std::vector<AddOnSlotData>::const_iterator it = m_addOnSlots.begin(); it != m_addOnSlots.end(); ) {
//    Object* obj = TheGameLogic->findObjectByID((*it).portableID);
//    if (obj)
//      portableStructures.push_back(obj);
//  }
//  return &portableStructures;
//}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void MultiAddOnContain::crc(Xfer* xfer)
{

  // extend base class
  TransportContain::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
  * Version Info:
  * 1: Initial version */
  // ------------------------------------------------------------------------------------------------
void MultiAddOnContain::xfer(Xfer* xfer)
{

  // version
  XferVersion currentVersion = 2;
  XferVersion version = currentVersion;
  xfer->xferVersion(&version, currentVersion);


  // List of AddOns
  // save/load each item
  for (std::vector<AddOnSlotData>::iterator it = m_addOnSlots.begin(); it != m_addOnSlots.end(); ++it)
  {
    AddOnSlotData entry = (*it);
    xfer->xferObjectID(&entry.occupantID);
    xfer->xferObjectID(&entry.portableID);
    // xfer->xferUnsignedShort(&entry->slot);  We don't need this

    if (xfer->getXferMode() == XFER_LOAD)
    {
      Object* obj = TheGameLogic->findObjectByID(entry.portableID);
      m_addOnList.push_back(obj);
    }
  }

  // extend base class
  TransportContain::xfer(xfer);


}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void MultiAddOnContain::loadPostProcess(void)
{

  // extend base class
  TransportContain::loadPostProcess();

}  // end loadPostProcess
