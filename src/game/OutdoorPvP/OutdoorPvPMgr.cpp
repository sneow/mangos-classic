/*
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "OutdoorPvPMgr.h"
#include "Policies/SingletonImp.h"
#include "OutdoorPvP.h"
#include "GameObject.h"
#include "World.h"
#include "Log.h"
#include "OutdoorPvPEP.h"
#include "OutdoorPvPSI.h"

INSTANTIATE_SINGLETON_1(OutdoorPvPMgr);

OutdoorPvPMgr::OutdoorPvPMgr()
{
    m_updateTimer.SetInterval(TIMER_OPVP_MGR_UPDATE);
    memset(&m_scripts, 0, sizeof(m_scripts));
}

OutdoorPvPMgr::~OutdoorPvPMgr()
{
    for (uint8 i = 0; i < MAX_OPVP_ID; ++i)
        delete m_scripts[i];
}

#define LOAD_OPVP_ZONE(a)                                           \
    if (sWorld.getConfig(CONFIG_BOOL_OUTDOORPVP_##a##_ENABLED))     \
    {                                                               \
        m_scripts[OPVP_ID_##a] = new OutdoorPvP##a();               \
        ++counter;                                                  \
    }
/**
   Function which loads all outdoor pvp scripts
 */
void OutdoorPvPMgr::InitOutdoorPvP()
{
    uint8 counter = 0;

    LOAD_OPVP_ZONE(SI);
    LOAD_OPVP_ZONE(EP);

    sLog.outString();
    sLog.outString(">> Loaded %u Outdoor PvP zones", counter);
}

OutdoorPvP* OutdoorPvPMgr::GetScript(uint32 zoneId)
{
    switch (zoneId)
    {
        case ZONE_ID_SILITHUS:
            return m_scripts[OPVP_ID_SI];
        case ZONE_ID_EASTERN_PLAGUELANDS:
            return m_scripts[OPVP_ID_EP];
        default:
            return NULL;
    }
}

OutdoorPvP* OutdoorPvPMgr::GetScriptOfAffectedZone(uint32 zoneId)
{
    switch (zoneId)
    {
        case ZONE_ID_TEMPLE_OF_AQ:
        case ZONE_ID_RUINS_OF_AQ:
        case ZONE_ID_GATES_OF_AQ:
            return m_scripts[OPVP_ID_SI];
        case ZONE_ID_STRATHOLME:
        case ZONE_ID_SCHOLOMANCE:
            return m_scripts[OPVP_ID_EP];
        default:
            return NULL;
    }
}

/**
   Function that handles the players which enters a specific zone

   @param   player to be handled in the event
   @param   zone id used for the current outdoor pvp script
 */
void OutdoorPvPMgr::HandlePlayerEnterZone(Player* player, uint32 zoneId)
{
    if (OutdoorPvP* script = GetScript(zoneId))
        script->HandlePlayerEnterZone(player, true);
    else if (OutdoorPvP* script = GetScriptOfAffectedZone(zoneId))
        script->HandlePlayerEnterZone(player, false);
}

/**
   Function that handles the player who leaves a specific zone

   @param   player to be handled in the event
   @param   zone id used for the current outdoor pvp script
 */
void OutdoorPvPMgr::HandlePlayerLeaveZone(Player* player, uint32 zoneId)
{
    // teleport: called once from Player::CleanupsBeforeDelete, once from Player::UpdateZone
    if (OutdoorPvP* script = GetScript(zoneId))
        script->HandlePlayerLeaveZone(player, true);
    else if (OutdoorPvP* script = GetScriptOfAffectedZone(zoneId))
        script->HandlePlayerLeaveZone(player, false);
}

void OutdoorPvPMgr::Update(uint32 diff)
{
    m_updateTimer.Update(diff);
    if (!m_updateTimer.Passed())
        return;

    for (uint8 i = 0; i < MAX_OPVP_ID; ++i)
        if (m_scripts[i])
            m_scripts[i]->Update(m_updateTimer.GetCurrent());

    m_updateTimer.Reset();
}

/**
   Function that gets the capture point slider value

   @param   capture point entry
 */
int8 OutdoorPvPMgr::GetCapturePointSliderValue(uint32 entry)
{
    std::map<uint32, int8>::iterator itr = m_capturePointSlider.find(entry);

    if (itr != m_capturePointSlider.end())
        return itr->second;

    // return default value if we can't find any
    return CAPTURE_SLIDER_NEUTRAL;
}
