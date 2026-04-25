#pragma once

class GameInfo;

#include <vector>

void performRandomAssign(GameInfo *game, const std::vector<Int> &lockedTemplates);
std::vector<Int> buildLockedTemplates();
