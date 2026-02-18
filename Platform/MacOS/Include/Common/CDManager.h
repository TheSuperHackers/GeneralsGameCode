#ifndef __CDMANAGER_H__
#define __CDMANAGER_H__

#include "Common/SubsystemInterface.h"

class CDDriveInterface {
public:
  virtual ~CDDriveInterface() {}
};

class CDManagerInterface : public SubsystemInterface {
public:
  virtual Int driveCount() = 0;
  virtual CDDriveInterface *getDrive(Int index) = 0;
  virtual CDDriveInterface *newDrive(const char *path) = 0;
  virtual void refreshDrives() = 0;
  virtual void destroyAllDrives() = 0;

protected:
  virtual CDDriveInterface *createDrive() = 0;
};

extern CDManagerInterface *TheCDManager;

#endif
