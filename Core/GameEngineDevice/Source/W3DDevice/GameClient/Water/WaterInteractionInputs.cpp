#include <algorithm>
#include <cmath>
#include "W3DDevice/GameClient/W3DWater.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

void WaterRenderSystem::updateWaterBodies()
{
    m_waterBodies.clear();
    if (TheGameLogic && TheTerrainLogic) {
        for (const Object* object = TheGameLogic->getFirstObject(); object; object = object->getNextObject()) {
            if (object->isDestroyed() || object->isEffectivelyDead()) continue;
            if (!object->isKindOf(KINDOF_BOAT) && !object->isKindOf(KINDOF_VEHICLE)
                && !object->isKindOf(KINDOF_INFANTRY)) continue;
            const Coord3D& position = *object->getPosition();
            Real water_height = 0;
            if (!TheTerrainLogic->isUnderwater(position.x,position.y,&water_height)) continue;
            const float radius = object->getGeometryInfo().getBoundingCircleRadius();
            if (std::abs(position.z-water_height) > (std::max)(radius,3.0f)) continue;
            const float heading = object->getOrientation();
            m_waterBodies.push_back({static_cast<std::uint32_t>(object->getID()),
                {position.x,position.y},{std::cos(heading),std::sin(heading)},radius});
        }
    }
    Graphics::Get_Water_Renderer().Waves().Update_Bodies(m_waterTime,m_waterBodies);
}
