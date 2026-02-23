/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "ModelIndices.h"
#include "Stats.h"
#include "DMAudio.h"
#include "Pad.h"
#include "main.h"
#include "Frontend.h"
#include "Script.h"
#include "Radar.h"
#include "Messages.h"
#include "CarCtrl.h"
#include "Population.h"
#include "Renderer.h"
#include "Pools.h"

#include "multiplayer/Lobby.h"
#include "multiplayer/MultiGame.h"
#include "multiplayer/net/Adhoc.h"
#include "multiplayer/net/GameChat.h"
#include "multiplayer/LScript.h"

#ifdef GTA_PSP
#include <kernel.h>
#include <psptypes.h>
#include <wlan.h>
#include <pspnet.h>
#include <pspnet_error.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet_adhoc_discover.h>
#else
#include "multiplayer/net/emu/NetAdhocCommon.h"
#include "multiplayer/net/emu/PSPErrorCodes.h"
#include "multiplayer/net/emu/sceNetAdhoc.h"
#include "multiplayer/net/emu/Utils.h"
#endif

#include "leeds/psp_compat.h"


uint8 GetLevelOfCompleteness()
{
    uint8 level = 0;
    if (CStats::IndustrialPassed || gMultiplayerCheat1) level = 1;
    if (CStats::CommercialPassed || gMultiplayerCheat2) level = 2;
    if (CStats::SuburbanPassed || gMultiplayerCheat3) level = 3;
    if (CStats::GetPercentageProgress() >= 100.0 || gMultiplayerCheat4 || gDeveloperFlag) level = 4;
    return level;
}

#ifdef GTA_LIBERTY
bool IsCarAllowedInMultiplayer(int32 nModelID, bool mode)
{
    if (nModelID < MI_FIRST_VEHICLE || nModelID > MI_LAST_VEHICLE)
        return false;

    if (mode) {
        switch (nModelID)
        {
        case MI_MOONBEAM:
        case MI_ESPERANT:
        case MI_KURUMA:
        case MI_BOBCAT:
        case MI_HEARSE:
            break;
        case MI_TAXI:
            if (GetLevelOfCompleteness() >= 0) return true;
            break;
        case MI_AMBULAN:
        case MI_FBICAR:
        case MI_BORGNINE:
        case MI_NOODLEBOY:
            if (GetLevelOfCompleteness() >= 1) return true;
            break;
        case MI_PIZZABOY:
            if (GetLevelOfCompleteness() >= 2) return true;
            break;
        case MI_BFINJECT:
            if (GetLevelOfCompleteness() >= 3) return true;
            break;
        case MI_MRWHOOP:
        case MI_CAMPVAN:
        case MI_BALLOT:
            if (GetLevelOfCompleteness() >= 4) return true;
            break;
        }
    }

    if (nModelID > MI_SANCHEZ) // > 210
        return false;

    switch (nModelID)
    {
    case MI_SPIDER:
    case MI_STINGER:
    case MI_INFERNUS:
    case MI_CHEETAH:
    case MI_HEARSE:
    case MI_YARDIE:
    case MI_YAKUZA:
    case MI_PONTIAC:
    case MI_ESPRIT:
    case MI_FORELLI_CAR:
        return GetLevelOfCompleteness() >= 2;
    case MI_LANDSTAL:
    case MI_PEREN:
    case MI_PONY:
    case MI_KURUMA:
    case MI_BANSHEE:
    case MI_STALLION:
    case MI_BELLYUP:
    case MI_MAFIA:
    case MI_SHELBY:
    case MI_PCJ600:
    case MI_FAGGIO:
        return GetLevelOfCompleteness() >= 0;
    case MI_IDAHO:
    case MI_SENTINEL:
    case MI_MANANA:
    case MI_BLISTA:
    case MI_MULE:
    case MI_MOONBEAM:
    case MI_ESPERANT:
    case MI_PANLANT:
    case MI_YANKEE:
        return GetLevelOfCompleteness() >= 4;
    case MI_LINERUN:
    case MI_FIRETRUCK:
    case MI_TRASH:
    case MI_STRETCH:
    case MI_AMBULAN:
    case MI_FBICAR:
    case MI_TAXI:
    case MI_MRWHOOP:
    case MI_BFINJECT:
    case MI_POLICE:
    case MI_ENFORCER:
    case MI_BUS:
    case MI_RHINO:
    case MI_BARRACKS:
    case MI_DODO:
    case MI_COACH:
    case MI_RCBANDIT:
    case MI_FLATBED:
    case MI_BORGNINE:
    case MI_TOPFUN:
    case MI_CAMPVAN:
    case MI_BALLOT:
    case MI_AMMOTRUK:
    case MI_FERRY:
    case MI_GHOST:
    case MI_SPEEDER:
    case MI_REEFER:
    case MI_PREDATOR:
    case MI_TRAIN:
    case MI_ESCAPE:
    case MI_CHOPPER:
    case MI_AIRTRAIN:
    case MI_DEADDODO:
    case MI_ANGEL:
    case MI_PIZZABOY:
    case MI_NOODLEBOY:
    case MI_FREEWAY:
    case MI_ANGEL2:
        break;
    case MI_PATRIOT:
    case MI_RUMPO:
    case MI_MRWONGS:
    case MI_DIABLOS:
    case MI_HOTROD:
    case MI_SINDACCO_CAR:
    case MI_SANCHEZ2:
        return GetLevelOfCompleteness() >= 1;
    case MI_BOBCAT:
    case MI_SECURICA:
    case MI_CABBIE:
    case MI_COLUMB:
    case MI_HOODS:
        return GetLevelOfCompleteness() >= 3;
    }

    return false;
}
#else
bool IsCarAllowedInMultiplayer(int32 nModelID, bool mode)
{
    if (nModelID < MI_FIRST_VEHICLE || nModelID > MI_AIRTRAIN)
        return false;

    int32 level = GetLevelOfCompleteness();
    if (mode)
    {
        switch (nModelID)
        {
            case MI_TAXI:
            case MI_POLICE:
            {
                if (level >= 0)
                    return true;
                break;
            }
            case MI_AMBULAN:
            case MI_FBICAR:
            case MI_CABBIE:
            case MI_BAGGAGE:
            {
                if (level >= 1)
                    return true;
                break;
            }
            case MI_ENFORCER:
            case MI_CADDY:
            {
                if (level >= 3)
                    return true;
                break;
            }
        }
    }

    if (nModelID < MI_ADMIRAL || nModelID >= MI_COASTG)
        return false;

    switch (nModelID)
    {
        case MI_ADMIRAL:
        case MI_ELECTRAP:
        case MI_GLENDALE:
        case MI_VICECHEE:
        case MI_ANGEL:
        case MI_GANGBUR:
        case MI_INFERNUS:
        case MI_SENTXS:
        case MI_WASHING:
            return (level >= 2);
        case MI_CHEETAH:
        case MI_AUTOGYRO:
        case MI_BAGGAGE:
        case MI_BMXBOY:
        case MI_BMXGIRL:
        case MI_BULLDOZE:
        case MI_CABBIE:
        case MI_CADDY:
        case MI_SPEEDER2:
        case MI_HUEY:
        case MI_HUEYHOSP:
        case MI_FBICAR:
        case MI_FIRETRUCK:
        case MI_HOVERCR:
        case MI_OCEANIC:
        case MI_POLICE:
        case MI_BOBO:
        case MI_MAVERICK:
        case MI_REEFER:
        case MI_SPEEDER:
        case MI_LINERUN:
        case MI_WALTON:
        case MI_BARRACKS:
        case MI_PREDATOR:
        case MI_FLATBED:
        case MI_AMMOTRUK:
        case MI_BIPLANE:
        case MI_YOLA:
        case MI_TAXI:
        case MI_AMBULAN:
        case MI_STRETCH:
        case MI_JETSKI:
        case MI_ENFORCER:
        case MI_COACH:
        case MI_SECURICA:
        case MI_TRASH:
        case MI_TOPFUN:
        case MI_MRWHOOP:
        case MI_RHINO:
        case MI_DINGHY:
        case MI_MARQUIS:
        case MI_RIO:
        case MI_TROPIC:
        case MI_FORKLIFT:
        case MI_SQUALO:
        case MI_JETMAX:
        case MI_VCNMAV:
        case MI_POLMAV:
        case MI_SPARROW:
        case MI_SEASPAR:
        case MI_SCARAB:
        case MI_FBIRANCH:
        case MI_HUNTER:
            return false;
        case MI_BANSHEE:
        case MI_BURRITO:
        case MI_LANDSTAL:
        case MI_MOP50:
        case MI_SANCHEZ:
        case MI_STALLION:
        case MI_PONY:
        case MI_PCJ600:
        case MI_FREEWAY:
        case MI_VOODOO:
        case MI_STREETFI:
        case MI_STINGER:
        case MI_SABRETUR:
            return (level >= 0);
        case MI_PEREN:
        case MI_BOBCAT:
        case MI_PIMP:
        case MI_ELECTRAG:
        case MI_PATRIOT:
        case MI_RUMPO:
        case MI_FAGGIO:
        case MI_QUAD:
        case MI_BOXVILLE:
        case MI_PHEONIX:
        case MI_MESA:
        case MI_CHOLLO:
        case MI_CUBAN:
            return (level > 0);
        case MI_BLISTAC:
        case MI_ESPERANT:
        case MI_HERMES:
        case MI_IDAHO:
        case MI_MANANA:
        case MI_SENTINEL:
        case MI_MOONBEAM:
        case MI_BENSON:
        case MI_MULE:
        case MI_YANKEE:
        case MI_SABRE:
            return (level >= 4);
        case MI_DELUXO:
        case MI_GREENWOO:
        case MI_SANDKING:
        case MI_VIRGO:
        case MI_BFINJECT:
        case MI_COMET:
        case MI_REGINA:
            return (level >= 3);
    }
    return false;
}
#endif

/* TODO#3 - also belongs to CMenuManager */
bool isPadUp()
{
#ifdef MP_FE_MOUSE_IMPROVEMENTS
    //if (CPad::GetPad(0)->GetMouseWheelUpJustDown() && !CPad::GetPad(0)->GetLeftShift()) {
    //    FrontEndMenuManager->m_bShowMouse = false;
    //    return true;
    //}
#endif
    if (CPad::GetPad(0)->GetCharJustDown('W')) // tmp
        return true;
    return CPad::GetPad(0)->GetUpJustDown();
}

/* TODO#3 - also belongs to CMenuManager */
bool isPadDown()
{
#ifdef MP_FE_MOUSE_IMPROVEMENTS
    //if (CPad::GetPad(0)->GetMouseWheelDownJustDown() && !CPad::GetPad(0)->GetLeftShift()) {
    //    FrontEndMenuManager->m_bShowMouse = false;
    //    return true;
    //}
#endif
    if(CPad::GetPad(0)->GetCharJustDown('S')) // tmp
        return true;
    return CPad::GetPad(0)->GetDownJustDown();
}

/* TODO#3 - also belongs to CMenuManager */
bool isPadLeft()
{
#ifdef MP_FE_MOUSE_IMPROVEMENTS
    //if (CPad::GetPad(0)->GetMouseWheelDownJustDown() && CPad::GetPad(0)->GetLeftShift())
    if (CPad::GetPad(0)->GetMouseWheelDownJustDown())
    {
        FrontEndMenuManager->m_bShowMouse = false;
        return true;
    }
#endif
    if (CPad::GetPad(0)->GetCharJustDown('A')) // tmp
        return true;
    return CPad::GetPad(0)->GetLeftJustDown();
}

/* TODO#3 - also belongs to CMenuManager */
bool isPadRight()
{
#ifdef MP_FE_MOUSE_IMPROVEMENTS
    //if (CPad::GetPad(0)->GetMouseWheelUpJustDown() && CPad::GetPad(0)->GetLeftShift())
    if (CPad::GetPad(0)->GetMouseWheelUpJustDown())
    {
        FrontEndMenuManager->m_bShowMouse = false;
        return true;
    }
#endif
    if (CPad::GetPad(0)->GetCharJustDown('D')) // tmp
        return true;
    return CPad::GetPad(0)->GetRightJustDown();
}

/* TODO#3 */
bool isPadConfirm() {
#ifdef MP_FE_MOUSE_IMPROVEMENTS
    // moved on MP_IS_CLICK_LAST_Y
    //if (CPad::GetPad(0)->GetLeftMouseJustDown()) // vanila control(allow last selected)//bug, selected row, clicking on a non-row is counted as a click on it
    //    return true;
#endif
    return CPad::GetPad(0)->GetEnterJustDown();
}

bool isBackDown() {
    // CPad::GuiBackJustUp
    return CPad::GetPad(0)->GetEscapeJustDown();
}

void inline handleBtnPressSound(int8 curMenuIndex) {
    if (curMenuIndex >= 0) DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
}

//void startNewNetworkGame() {
//    // TODO(LVCS): better place for this
//    FrontEndMenuManager->m_bWantToLoad = true; // quick load
//    //FrontEndMenuManager->m_bWantToRestart = true; // full long init
//    FrontEndMenuManager->DoSettingsBeforeStartingAGame();
//}



cLobby* cLobby::ms_pInstance = nil;

float cLobby::ms_fJoinPrevTime = 0.0f;
uint32 cLobby::m_snTimeInMillisecondsLeftButtonPrev = 0;
uint32 cLobby::m_snTimeInMillisecondsRightButtonPrev = 0;

cLobby::cLobby()
{
    // VCS TODO unk remote info array construct 

    m_lobbyPed = cLobbyPed();
    m_nSocketID_1 = -1;
    m_nSocketID_2 = -1;

    // m_remoteInfo ctor

    field_190 = 0; // VCS new added
    field_194 = 0;
    field_198 = 0;
    field_19C = 0;
    m_bBadCharWarn = 0;
    field_1A4 = 0;
    m_bConnection = false;
    //memset(&this->unk_field_18.field_0, 0, 140);

#ifdef FIX_BUGS
    m_pDrawCB = nil;
    m_pProcessCB = nil;
    m_nSrvListMinIdx = 0;
    m_nSrvListMaxIdx = 0;
    m_nNewHostIdx = 0;
    m_nSavedRaceLapCount = 0; // bug? uninit
    m_nSelectedMenuOptionIndex = 0;
    m_bBadCharWarn = false;
#ifdef MP_FE_MOUSE_IMPROVEMENTS
    m_nLastMouseSelectedMenuOptionY = -1;
#endif
#endif
}


// inlined helpers
eGameType /*cLobby::*/GetNextGameType(eGameType type, int8 inc) {
    // issue#1 looks like those are Lobby class methods that got inlined
    int8 nValue = static_cast<int8>(type) + inc;
    return static_cast<eGameType>(LoopWrap(nValue, ((int8)eGameType::DEATHMATCH), ((int8)eGameType::NUM_MULIT_GAME_TYPES - 1)));
}

eGameLocation /*cLobby::*/GetNextGameLocation(eGameLocation location, int8 inc) {
    int8 nValue = static_cast<int8>(location) + inc;
#ifdef GTA_LIBERTY
    return static_cast<eGameLocation>(LoopWrap(nValue, ((int8)eGameLocation::IND_ZON), ((int8)eGameLocation::NUM_MP_GAME_LOCATION - 1)));
#else
    return static_cast<eGameLocation>(LoopWrap(nValue, ((int8)eGameLocation::VICE_POINT_ZON), ((int8)eGameLocation::NUM_MP_GAME_LOCATION - 1)));
#endif
}

void cLobby::NextScoreLimit(int inc, int minValue, int maxValue) {
    cMultiGame& Game = TheMPGame;
    int32 nScore = Game.GetScoreLimit() + inc;
    Game.SetScoreLimit(LoopWrap(nScore, minValue, maxValue));
}

void cLobby::NextCtfScoreLimit(int8 inc) {
    cMultiGame& Game = TheMPGame;
    int32 nScore = Game.GetCTFScoreLimit() + inc;
    Game.SetCTFScoreLimit(LoopWrap(nScore, 0, MP_MAX_CTF_SCORE));
}

void cLobby::NextCashTarget(int16 step, int16 min, int16 max) {
    cMultiGame& Game = TheMPGame;
    int16 nCash = Game.GetCashTarget() + step;
    Game.SetCashTarget(LoopWrap(nCash, min, max));
}

void cLobby::NextTimeLimit(int inc) {
    cMultiGame& Game = TheMPGame;
    int32 nTime = Game.GetTimeLimit() + inc;
    Game.SetTimeLimit(LoopWrap(nTime, 0, 60));
}

void cLobby::NextVehicle(int8 inc) {
    cMultiGame& Game = TheMPGame;
#if defined(FIX_BUGS) && !defined(GTA_LIBERTY) // can invisible switch vehicle in inactive mode
    if(Game.GetCTFScoreLimit() == (int32)eRaceStyle::RACE_QUADATHLON)
        return;
#endif
    uint16 nCarID = Game.m_nRaceCarID + inc;
    while (!IsCarAllowedInMultiplayer(nCarID, true)) {
        debug("***** while 6: car %d not allowed\n", nCarID);
        nCarID = nCarID + inc;
        nCarID = LoopWrap(nCarID, MI_FIRST_VEHICLE, MI_LAST_VEHICLE);
    }
    Game.m_nRaceCarID = nCarID;
}

void cLobby::NextRaceID(int8 inc) {
    cMultiGame& Game = TheMPGame;
    int8 nTrackID = Game.m_nScenarioOrRaceTrackID + inc;
#ifdef GTA_LIBERTY
    pGame.m_nScenarioOrRaceTrackID = LoopWrap(nTrackID, 0, MP_MAX_RACE_TRACKS);
#else
    int32 nNumRaces = GetScenarioNumForThisGame() - 1;
    Game.m_nScenarioOrRaceTrackID = LoopWrap(nTrackID, 0, nNumRaces);
#endif

}

#ifndef GTA_LIBERTY
void cLobby::NextScenario(int8 inc) {
    cMultiGame& Game = TheMPGame;
    int8 nScenario = Game.m_nScenarioOrRaceTrackID + inc;
    int32 nNumScenarios = GetScenarioNumForThisGame() - 1;
    Game.m_nScenarioOrRaceTrackID = LoopWrap(nScenario, 0, nNumScenarios);
}
#endif

void cLobby::NextRaceLapCount(int8 inc) {
    cMultiGame& Game = TheMPGame;
#if defined(FIX_BUGS) && !defined(GTA_LIBERTY) // can some invisible switch
    if (Game.GetCTFScoreLimit() == (int32)eRaceStyle::RACE_QUADATHLON)
        return;
#endif
    int32 nLapCount = TheMPGame.GetScoreLimit() + inc;
    TheMPGame.SetScoreLimit(LoopWrap(nLapCount, 1, MP_MAX_RACE_LAPS));
}

void cLobby::TogglePowerup() {
    cMultiGame& Game = TheMPGame;
    Game.bPowerUpOn = !Game.bPowerUpOn;
}

void cLobby::ToggleRacePowerup() {
    cMultiGame& Game = TheMPGame;
    Game.bRacePowerUpOn = !Game.bRacePowerUpOn;
}

void cLobby::NextGameLocation(int inc) {
    cMultiGame& Game = TheMPGame;
    Game.SetGameLocation(GetNextGameLocation(Game.GetGameLocation(), inc));
#ifndef GTA_LIBERTY
    while (!IsExistsScenarioForThisGame() || !IsGameLocationAllowed()) { // IsExistsRaceForThisGame? wrong name? or rockstar logic?
        debug("***** while 7: location %d not allowed\n", (int32)Game.GetGameLocation());
        Game.SetGameLocation(GetNextGameLocation(Game.GetGameLocation(), inc));
    }
#endif
}

void cLobby::NextGameStyle(int inc) {
    cMultiGame& Game = TheMPGame;
    Game.eTDMStyle = FREE_MODE ? eTDMStyle::GANG_WAR : eTDMStyle::FFA;
}

#ifndef GTA_LIBERTY
void cLobby::NextRaceGameStyle(int inc)
{
    cMultiGame& Game = TheMPGame;
    int32 nOldScore = Game.GetCTFScoreLimit();
    int32 nScore = Game.GetCTFScoreLimit() + inc;
    int32 nMaxLimit = (int32)((GetLevelOfCompleteness() < 2) ? eRaceStyle::RACE_QUADATHLON : eRaceStyle::RACE_JETSKI);
    Game.SetCTFScoreLimit(LoopWrap(nScore, 0, nMaxLimit));
    Game.SetGameLocation(eGameLocation::VICE_POINT_ZON);
    Game.m_nScenarioOrRaceTrackID = 0;
    while (!IsExistsScenarioForThisGame() || !IsGameLocationAllowed()) {
        debug("***** while 7: location %d not allowed\n", (int32)Game.GetGameLocation());
        Game.SetGameLocation(GetNextGameLocation(Game.GetGameLocation(), 1));
    }
    nScore = Game.GetCTFScoreLimit();
    if ((nOldScore == (int32)eRaceStyle::RACE_QUADATHLON) && (nScore != (int32)eRaceStyle::RACE_QUADATHLON)) // leave quad opt [restore]
    {
        Game.SetScoreLimit(m_nSavedRaceLapCount);
        m_nSavedRaceLapCount = 1;
    }
#ifdef FIX_BUGS // store score
    else if ((nScore == (int32)eRaceStyle::RACE_QUADATHLON) && Game.GetScoreLimit() != 1) // on quad opt [store]
    {
        m_nSavedRaceLapCount = Game.GetScoreLimit();
        Game.SetScoreLimit(1);
    }
#endif
}
#endif

void cLobby::NextGangAOption(int inc) {
    cMultiGame& Game = TheMPGame;
    int8 gangA = (int8)Game.GetTeam1GangID();
    int8 gangB = (int8)Game.GetTeam2GangID();

    while (true) {
        gangA = gangA + inc;
        gangA = LoopWrap(gangA, 0, (MP_MAX_GANGS - 1));        
        uint8 nMinUnlockLevel = gMPGangDefs[gangA].nUnlockLevel;
        uint8 nCurrentLevel = GetLevelOfCompleteness();
        if (gangA != gangB) {
            if (nCurrentLevel >= nMinUnlockLevel) {
                Game.SetTeam1GangID((uint8)gangA);
                break;
            }
        }
        debug("***** while 4: %d == %d || %d > %d\n", gangA, gangB, nMinUnlockLevel, nCurrentLevel);
    }
}

void cLobby::NextGangBOption(int inc) {
    cMultiGame& Game = TheMPGame;
    int8 gangA = (int8)Game.GetTeam1GangID();
    int8 gangB = (int8)Game.GetTeam2GangID();

    while (true) {
        gangB = gangB + inc;
        gangB = LoopWrap(gangB, 0, (MP_MAX_GANGS - 1));
        uint8 nMinUnlockLevel = gMPGangDefs[gangB].nUnlockLevel;
        uint8 nCurrentLevel = GetLevelOfCompleteness();
        if (gangA != gangB && nCurrentLevel >= nMinUnlockLevel) {
            Game.SetTeam2GangID((uint8)gangB);
            break;
        }
        debug("***** while 5: %d == %d || %d > %d\n", gangA, gangB, nMinUnlockLevel, nCurrentLevel);
    }
}

#ifndef GTA_LIBERTY
void cLobby::NextVipTeam(int inc) {
    cMultiGame& Game = TheMPGame;
    Game.bIsVipTeamTeam2 ^= true;
}
#endif

void cLobby::HandleLeftRightBtnPress(int8 nMenuIndex, int8 inc) {
    cMultiGame& Game = TheMPGame;

#ifdef MULTIGAME_SCM
    if (Game.GetGameType() == eGameType::SCM && nMenuIndex != MGE_GAME_TYPE)
        return;
#endif
    switch (nMenuIndex) {
        case MGE_GAME_TYPE:
            Game.SetGameType(GetNextGameType(Game.GetGameType(), inc));
            InitGameParams(true);
            return;
        case MGE_GAME_LOCATION:
            NextGameLocation(inc);
#ifndef GTA_LIBERTY
            Game.m_nScenarioOrRaceTrackID = 0;
#endif
            return;
#ifdef GTA_LIBERTY
        case MGE_PLAY_CUTSCENE:
            NextCutsceneOption(inc);
            return;
#endif
    default:
        break;
    }

    switch (Game.GetGameType())
    {
        case eGameType::DEATHMATCH:
            switch (nMenuIndex) {
                case MGE_DEATHMATCH_GAME_STYLE:
                    NextGameStyle(inc);
                    break;
                case MGE_DEATHMATCH_KILL_LIMIT:
                    NextScoreLimit(inc, 0, 50);
                    break;
                case MGE_DEATHMATCH_TIME_LIMIT:
                    NextTimeLimit(inc);
                    break;
#ifdef GTA_LIBERTY
                case MGE_DEATHMATCH_POWERUPS:
                    TogglePowerup();
                    break;
#endif
                case MGE_DEATHMATCH_SEL_GANG_1:
                    if (GANG_MODE) NextGangAOption(inc);
                    break;
                case MGE_DEATHMATCH_SEL_GANG_2:
                    if (GANG_MODE) NextGangBOption(inc);
                    break;
            }
            break;
        case eGameType::MULTIRACE:
            switch (nMenuIndex) {
#ifndef GTA_LIBERTY
                case MGE_MULTIRACE_GAME_STYLE:
                    NextRaceGameStyle(inc); // with store score
                    break;
#endif
                case MGE_MULTIRACE_RACE:
                    NextRaceID(inc);
                    break;
                case MGE_MULTIRACE_LAPS:
                    NextRaceLapCount(inc); // can disabled (bugly)
                    break;
                case MGE_MULTIRACE_VEHICLE:
                    NextVehicle(inc); // can disabled kek (fe hide bug)
                    break;
#ifdef GTA_LIBERTY
                case MGE_MULTIRACE_POWERUPS:
                    ToggleRacePowerup();
                    break;
#endif
            }
//#if !defined(FIX_BUGS) && !defined(GTA_LIBERTY) // huh, in orig handlehoststate in bottom function
//            if (Game.GetCTFScoreLimit() == (int32)eRaceStyle::RACE_QUADATHLON && pGame.GetScoreLimit() != 1)
//            {
//                m_nSavedRaceLapCount = Game.GetScoreLimit();
//                Game.SetScoreLimit(1);
//            }
//#endif
            break;
        case eGameType::DEFENDTHEBASE:
            switch (nMenuIndex) {
#ifndef GTA_LIBERTY
                case MGE_DEFENDTHEBASE_SCENARIO:
                    NextScenario(inc);
                    break;
#else
                case MGE_DEFENDTHEBASE_POWERUPS:
                    TogglePowerup();
                    break;
#endif
            }
            break;
        case eGameType::CTF:
            switch (nMenuIndex) {
#ifndef GTA_LIBERTY
                case MGE_CTF_SCORE_SCENARIO:
                    NextScenario(inc);
                    break;
#endif
                case MGE_CTF_SCORE_LIMIT:
                    NextCtfScoreLimit(inc);
                    break;
#ifdef GTA_LIBERTY
                case MGE_CTF_TIME_LIMIT:
                    NextTimeLimit(inc);
                    break;
                case MGE_CTF_POWERUPS:
                    TogglePowerup();
                    break;
#endif
            }
            break;
        case eGameType::TANK:
            switch (nMenuIndex)
            {
                case MGE_TANK_TANK_TIME:
#ifdef GTA_LIBERTY
                    NextScoreLimit(inc, 3, 20);
#else
                    NextScoreLimit(inc, 3, 15);
#endif
                    break;
            }
            break;
        case eGameType::HITPARADE:
            switch (nMenuIndex)
            {
#ifdef GTA_LIBERTY
                case MGE_HITPARADE_POWERUPS:
                    TogglePowerup();
                    break;
#endif
            }
            break;
        case eGameType::SIXTYSECONDS:
            switch (nMenuIndex)
            {
                case MGE_SIXTYSECONDS_CASH_TARGET:
                    NextCashTarget(inc * 1000, 1000, 20000);
                    break;
            }
            break;
#ifndef GTA_LIBERTY
        case eGameType::HUNTERATTACK:
            switch (nMenuIndex)
            {
                case MGE_HUNTERATTACK_KILL_LIMIT:
                    NextScoreLimit(inc, 0, 50);
                    break;
                case MGE_HUNTERATTACK_TIME_LIMIT:
                    NextTimeLimit(inc);
                    break;
            }
            break;
        case eGameType::FLAGBALL:
            switch (nMenuIndex)
            {
                case MGE_FLAGBALL_SCENARIO:
                    NextScenario(inc);
                    break;
            }
            break;
        case eGameType::VIP:
            switch (nMenuIndex)
            {
                case MGE_VIP_VIP_TEAM:
                    NextVipTeam(inc);
                    break;
            }
            break;
#endif
    }
}

void cLobby::NextMenuOption(int step, int min, int max) {
    int32 nOption = m_nSelectedMenuOptionIndex + step;
    m_nSelectedMenuOptionIndex = LoopWrap(nOption, min, max);
}



// cLobby

bool cLobby::IsPressedLeftButton() {
    if (CPad::GetPad(0)->GetAnalogueLeftRight() >= -125) {
        cLobby::m_snTimeInMillisecondsLeftButtonPrev = CTimer::GetTimeInMillisecondsPauseMode();
    }
    else if ((CTimer::GetTimeInMillisecondsPauseMode() - cLobby::m_snTimeInMillisecondsLeftButtonPrev) >= 201) {
        cLobby::m_snTimeInMillisecondsLeftButtonPrev = CTimer::GetTimeInMillisecondsPauseMode();
        return true;
    }
    return false;
}

bool cLobby::IsPressedRightButton() {
    if (CPad::GetPad(0)->GetAnalogueLeftRight() < 126) {
        cLobby::m_snTimeInMillisecondsRightButtonPrev = CTimer::GetTimeInMillisecondsPauseMode();
    }
    else if ((CTimer::GetTimeInMillisecondsPauseMode() - cLobby::m_snTimeInMillisecondsRightButtonPrev) >= 201) {
        cLobby::m_snTimeInMillisecondsRightButtonPrev = CTimer::GetTimeInMillisecondsPauseMode();
        return true;
    }
    return false;
}

// when game start (FE init)
bool cLobby::InitialiseMultiplayer() { // start mp page
    MARKFUNCTION(0x0, 0x08873600);

    debug("!!!!!! CLear suspended\n");
    TheMPGame.m_bHasSuspended = false;
    //FrontEndMenuManagerSettings->fe_set_sub_89C7184(1); // bool?
    m_nSelectedMenuOptionIndex = -1;
    //field_190 = 0;
    //field_194 = 0;
    //field_198 = 0;
    //field_19C = 0;
    InitGameParams(false);
    SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
    LoadSplash("loadsc1");
    if (!FrontEndMenuManager->GetHasJoinedNetwork())
        field_18C = true;
    return TheAdhoc.IsWifiSwitchOn();
}

void cLobby::UpdateLogic() {
    cAdhoc& Adhoc = cAdhoc::Instance();
    cMultiGame& Game = cMultiGame::Instance();

    if (!psp_is_umd_readable() && !Adhoc.IsNextStateNow(&cAdhoc::StateShutdown)) {
        CloseConnection(true);
        return;
    }

    if (Game.m_bHasSuspended) {
        Game.m_bHasSuspended = false;
        if (Game.IsOpen())
            Game.Close();

        Close();
        CloseConnection(true);
        SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
        return;
    }

    if (!Adhoc.IsWifiSwitchOn()) {
        SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
    }

    if (FrontEndMenuManager->IsMPPageActive()) {
#ifdef GTA_PC
        if (!gChat.IsOpen())
#endif
            (this->*m_pProcessCB)();
    }
    else {
        m_nSelectedMenuOptionIndex = 0;
    }
    m_lobbyPed.Update();
}

void cLobby::UpdateRender() {
#ifdef MP_FE_MOUSE_IMPROVEMENTS
    m_nLastMouseSelectedMenuOptionY = -1;
#endif
    if (m_pDrawCB) (this->*m_pDrawCB)();
}

void cLobby::Close() {
    if (HasLuaInitialized())
        cLScript::Shutdown();
    CloseAdhoc();
    m_lobbyPed.Destroy();
}

bool cLobby::ReturnAfterGame() {
    return true;
}

/* TODO#2 */
#ifdef GTA_LIBERTY
void cLobby::CloseConnection() {
    DMAudio.PlayFrontEndSound(SOUND_FRONTEND_BACK, 0);
    CloseAdhoc();
    if (TheAdhoc.IsWifiSwitchOn()) {
        SetCurrentFrontendHandler(&cLobby::HandleMainState, &cLobby::DrawMainState);
        FrontEndMenuManager->m_bHasJoinedNetwork = false;
    }
    else {
        Close();
    }
    m_nSelectedMenuOptionIndex = 0;
}
#else
void cLobby::CloseConnection(bool bSetSingleMode) {
    Close();
    SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
    // byte_8BAB329 = 0; // TODO

    if (bSetSingleMode) {
        DMAudio.PlayFrontEndSound(SOUND_FRONTEND_BACK, 0);
        // FrontEndMenuManager->fe_sub_882E670(); // TODO: implement
        if (TheAdhoc.IsWifiSwitchOn()) {
            FrontEndMenuManager->SetSinglePlayerMode();
            // TODO: PSP UI handling for "Multiplayer_t" - e.g., add to menu or text array
        }
        else {
            if (field_18C || !FrontEndMenuManager->IsMPPageActive()) {
                FrontEndMenuManager->SetSinglePlayerMode();
                // TODO: same UI handling as above
                // TODO: PSP UI handling for "Multiplayer_t" - e.g., add to menu or text array
            }
            else {
                // FE_sub_882FCA0(3, 0); // TODO: implement this frontend sub (likely switch screen or mode)
            }
        }
    }
    m_nSelectedMenuOptionIndex = 0;
    if (m_bConnection) {
        //CPools::GetPedPool()->Initialise();
    }
    m_bConnection = false;
#ifdef FIX_BUGS
    m_bBadCharWarn = false;
#endif
}
#endif

void cLobby::SetCurrentFrontendHandler(CallbackHnd control, CallbackHnd view) {
    m_pProcessCB = control;
    m_pDrawCB = view;
    if (control == &cLobby::HandleMainGameState)
    {
        cMultiGame& Game = cMultiGame::Instance();
        Game.m_nAmbientCarBank = CGeneral::GetRandomNumberInRange(0, TOTAL_MULTIPLAYER_CAR_BANKS);
        Game.m_nAmbientPedBank = CGeneral::GetRandomNumberInRange(0, TOTAL_MULTIPLAYER_PED_BANKS);
    }
}

void cLobby::CloseAdhoc() {
    cAdhoc& Adhoc = cAdhoc::Instance();
    if (Adhoc.IsNextStateNow(&cAdhoc::StateShutdown))
        return;

    if (m_nSocketID_1 >= 0) sceNetAdhocPtpClose(m_nSocketID_1, 0);
    if (m_nSocketID_2 >= 0) sceNetAdhocPtpClose(m_nSocketID_2, 0);
    m_nSocketID_2 = -1;
    m_nSocketID_1 = -1;
    Adhoc.Terminate();
}

void cLobby::CheckCloseConnection() {
    if (isBackDown() || !TheAdhoc.IsWifiSwitchOn())
        CloseConnection(true);
}

void cLobby::NoDraw() {
    DONT_OPTIMIZE();
}

void cLobby::HandleGangSelection(int32 optIndex)
{
    static int nRequestedModelID = -1;
    cMultiGame& pGame = TheMPGame;
    int nModelIndex = cMultiGame::s_nPlayerModelIndex;
    uint8 nTeamGangID = pGame.s_nSelectedTeam == eGameTeam::TEAM_B ? pGame.GetTeam2GangID() : pGame.GetTeam1GangID();
    if (FREE_MODE || nTeamGangID == ga_netModelList[pGame.s_nPlayerModelIndex].gangID) {
        if (FREE_MODE && nRequestedModelID >= 0) {
            pGame.s_nPlayerModelIndex = nRequestedModelID;
            m_lobbyPed.SetModelIndex(nRequestedModelID);
            nRequestedModelID = -1;
        }
    }
    else
    {
        if (nRequestedModelID < 0)
            nRequestedModelID = nModelIndex;
#ifdef GTA_LIBERTY // leeds optimise
        while (true) {
            uint8 nSelectedGangID = ga_netModelList[nModelIndex].gangID;
            if (nTeamGangID == nSelectedGangID)
                break;
            debug("***** while 1: %d != %d (%d)\n", nTeamGangID, nSelectedGangID, nModelIndex);
            nModelIndex++;
            if (nModelIndex == MAX_MP_MODELS) nModelIndex = 0;
            pGame.s_nPlayerModelIndex = nModelIndex;
        }
#else
        nModelIndex = gMPGangDefsNetModelListIndices[nTeamGangID];
        pGame.s_nPlayerModelIndex = nModelIndex;
#endif
        m_lobbyPed.SetModelIndex(nModelIndex);
    }

    uint8 nLevel = GetLevelOfCompleteness();
    if (m_nSelectedMenuOptionIndex == optIndex) {
        if (isPadLeft()) {
            //DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0); // play after handle
            while (true) {
                nModelIndex--;
                nModelIndex = LoopWrap(nModelIndex, 0, (MAX_MP_MODELS - 1));
                uint8 nMinUnlockLevel = ga_netModelList[nModelIndex].unlockLevel;
                if (nLevel >= nMinUnlockLevel)
                    break;
                debug("***** while 2: %d > %d (%d)\n", nMinUnlockLevel, nLevel, nModelIndex);

            }
            nRequestedModelID = nModelIndex;
        }
        else if (isPadRight()) {
            //DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
            while (true) {
                nModelIndex++;
                nModelIndex = LoopWrap(nModelIndex, 0, (MAX_MP_MODELS - 1));
                uint8 nMinUnlockLevel = ga_netModelList[nModelIndex].unlockLevel;
                if (nLevel >= nMinUnlockLevel)
                    break;
                debug("***** while 3: %d > %d (%d)\n", nMinUnlockLevel, nLevel, nModelIndex);
            }
            nRequestedModelID = nModelIndex;
        }
    }
}

// void cLobby::SetTextColorStyle // in frontend_mp.cpp

// void cLobby::SetInactiveTextColorStyle(bool selected) // in frontend_mp.cpp

void cLobby::InitGameParams(bool keepParams)
{
    cMultiGame& Game = cMultiGame::Instance();
#ifndef MP_FE_MOUSE_IMPROVEMENTS
    m_nSelectedMenuOptionIndex = 0;
#endif
    if (!keepParams) {
#ifdef GTA_LIBERTY
        Game.SetGameLocation(eGameLocation::IND_ZON);
#else
        Game.SetGameLocation(eGameLocation::VICE_POINT_ZON);
#endif
        Game.ePlayIntroCutscene = static_cast<uint8>(eGameCutscenePlayback::PLAY_ONCE);
        Game.bCutscenePlayed = false;
    }
    NextCutsceneOption(0); // TODO: why this is here?
    Game.eTDMStyle = eTDMStyle::FFA;
    Game.bPowerUpOn = true;
    Game.bRacePowerUpOn = true;
    Game.SetCTFScoreLimit(3);
    Game.SetScoreLimit(3);
    Game.SetTimeLimit(0);
    Game.m_nScenarioOrRaceTrackID = 0;
    Game.m_nRaceCarID = MI_BANSHEE;
    Game.SetCashTarget(5000);
    Game.SetTeam1GangID(0);
    Game.SetTeam2GangID(1);
    switch (Game.GetGameType())
    {
        case eGameType::DEATHMATCH:
            Game.SetScoreLimit(10);
            break;
#ifndef GTA_LIBERTY
        case eGameType::MULTIRACE:
            Game.SetCTFScoreLimit(0);
            break;
#endif
        case eGameType::DEFENDTHEBASE:
            Game.eTDMStyle = eTDMStyle::GANG_WAR;
            break;
        case eGameType::CTF:
            Game.eTDMStyle = eTDMStyle::GANG_WAR;
            Game.SetScoreLimit(10);
            break;
        case eGameType::TANK:
            Game.SetScoreLimit(3);
            break;
#ifndef GTA_LIBERTY
        case eGameType::HUNTERATTACK:
            Game.SetScoreLimit(5);
            break;
        case eGameType::FLAGBALL:
            Game.eTDMStyle = eTDMStyle::GANG_WAR;
            break;
        case eGameType::VIP:
            Game.eTDMStyle = eTDMStyle::GANG_WAR;
            Game.SetScoreLimit(5);
            break;
#endif
    }
#ifndef GTA_LIBERTY
    Game.SetGameLocation(eGameLocation::VICE_POINT_ZON);
    // sync current allowed location
    while (!IsExistsScenarioForThisGame() || !IsGameLocationAllowed()) {
        debug("***** while 7: location %d not allowed\n", (int32)Game.GetGameLocation());
        Game.SetGameLocation(GetNextGameLocation(Game.GetGameLocation(), 1));
    }
#endif
}

// issue#1 yeah, definitely looks like inlined
void cLobby::NextCutsceneOption(int8 inc) {
    cMultiGame& pGame = TheMPGame;
    int32 nValue = static_cast<int32>(pGame.ePlayIntroCutscene) + inc;
    nValue = LoopWrap(nValue, static_cast<uint8>(eGameCutscenePlayback::ALWAYS_PLAY), static_cast<uint8>(eGameCutscenePlayback::PLAY_ONCE));
    pGame.ePlayIntroCutscene = static_cast<uint8>(nValue);
    if (pGame.ePlayIntroCutscene == static_cast<uint8>(eGameCutscenePlayback::PLAY_ONCE) && gMPCutsceneHasPlayed[static_cast<uint8>(pGame.GetGameType())])
        pGame.bCutscenePlayed = true;
}

#ifndef GTA_LIBERTY
bool cLobby::IsExistsScenarioForThisGame()
{
    //bool f;
    //int32 num = GetScenarioNumForThisGame(&f);
    //return f ? (num != 0) : true;

    int32 loc = (int32)TheMPGame.GetGameLocation();
    switch (TheMPGame.GetGameType())
    {
        case eGameType::MULTIRACE:
        {
            int32 nScore = TheMPGame.GetCTFScoreLimit();
            if (nScore == 1)
                return !!gMPScenarioNumsTable[ST_MULTIRACE_CTF_SCORE_1][loc];
            else if (nScore == 2)
                return !!gMPScenarioNumsTable[ST_MULTIRACE_CTF_SCORE_2][loc];
            else // (nScore == 0)
                return !!gMPScenarioNumsTable[ST_MULTIRACE_CTF_SCORE_0][loc];
            break;
        }
        case eGameType::DEFENDTHEBASE:
            return !!gMPScenarioNumsTable[ST_DEFENDTHEBASE][loc];
        case eGameType::CTF:
            return !!gMPScenarioNumsTable[ST_CTF][loc];
        case eGameType::TANK:
            return !!gMPScenarioNumsTable[ST_TANK][loc];
        case eGameType::SIXTYSECONDS:
            return !!gMPScenarioNumsTable[ST_SIXTYSECONDS][loc];
        case eGameType::HUNTERATTACK:
            return !!gMPScenarioNumsTable[ST_HUNTERATTACK][loc];
        case eGameType::FLAGBALL:
            return !!gMPScenarioNumsTable[ST_FLAGBALL][loc];
        case eGameType::VIP:
            return !!gMPScenarioNumsTable[ST_VIP][loc];
    }
    return true;
}

// custom
int32 cLobby::GetScenarioNumForThisGame(bool* pIsFind)
{
    if (pIsFind)
        *pIsFind = true;
    int32 loc = (int32)TheMPGame.GetGameLocation();
    switch (TheMPGame.GetGameType())
    {
        case eGameType::MULTIRACE:
        {
            int32 nScore = TheMPGame.GetCTFScoreLimit();
            if (nScore == 1)
                return gMPScenarioNumsTable[ST_MULTIRACE_CTF_SCORE_1][loc];
            else if (nScore == 2)
                return gMPScenarioNumsTable[ST_MULTIRACE_CTF_SCORE_2][loc];
            else // (nScore == 0)
                return gMPScenarioNumsTable[ST_MULTIRACE_CTF_SCORE_0][loc];
            break;
        }
        case eGameType::DEFENDTHEBASE:
            return gMPScenarioNumsTable[ST_DEFENDTHEBASE][loc];
        case eGameType::CTF:
            return gMPScenarioNumsTable[ST_CTF][loc];
        case eGameType::TANK:
            return gMPScenarioNumsTable[ST_TANK][loc];
        case eGameType::SIXTYSECONDS:
            return gMPScenarioNumsTable[ST_SIXTYSECONDS][loc];
        case eGameType::HUNTERATTACK:
            return gMPScenarioNumsTable[ST_HUNTERATTACK][loc];
        case eGameType::FLAGBALL:
            return gMPScenarioNumsTable[ST_FLAGBALL][loc];
        case eGameType::VIP:
            return gMPScenarioNumsTable[ST_VIP][loc];
    }
    if (pIsFind)
        *pIsFind = false;
    return 0;
}

bool cLobby::IsGameLocationAllowed()
{
    int32 lvl = GetLevelOfCompleteness();
    int32 loc = (int32)TheMPGame.GetGameLocation();

    bool allowed = true;
    switch (TheMPGame.GetGameType())
    {
        case eGameType::DEATHMATCH:
        case eGameType::DEFENDTHEBASE:
        case eGameType::CTF:
        case eGameType::TANK:
        case eGameType::HITPARADE:
        case eGameType::SIXTYSECONDS:
        case eGameType::HUNTERATTACK:
        case eGameType::FLAGBALL:
        case eGameType::VIP:
            if (loc < (int32)eGameLocation::DOWNTOWN_ZON && lvl < 2)
                allowed = false;
            break;
        case eGameType::MULTIRACE:
            int32 nScore = TheMPGame.GetCTFScoreLimit();
            if ((nScore == 0 || nScore == 1) && loc < (int32)eGameLocation::DOWNTOWN_ZON && lvl < 2) {
                allowed = false;
            }
            else if (nScore == 2 && lvl < 2) {
                allowed = false;
            }
            break;
    }
    return allowed;
}
#endif

bool cLobby::HasBadCharsInNickname() {
    char szNickname[SCE_NET_ADHOCCTL_NICKNAME_LEN]; // 128   // or SceNetAdhocctlNickname m_szPlayerNickname;
    wchar szNicknameW[(SCE_NET_ADHOCCTL_NICKNAME_LEN * 2) + (sizeof(wchar) * 2)]; // this is bad

    if (sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, szNickname, SCE_NET_ADHOCCTL_NICKNAME_LEN) < 0) {
        szNickname[0] = '\0'; // err
        return false;
    }

    AsciiToUnicode(szNickname, szNicknameW);

    int32 length = 0;
    for (; szNicknameW[length] != L'\0'; ++length) {}
    if (length == 0)
        return false;

    bool hasNonSpace = false;
    int32 checkLimit = (length < MAX_VISIBLE_NICKNAME_CHARS) ? length : MAX_VISIBLE_NICKNAME_CHARS;
    for (int32 i = 0; i < checkLimit; ++i) {
        if (szNicknameW[i] != L' ' && szNicknameW[i] != L'\0') {
            hasNonSpace = true;
            break;
        }
    }

    if (hasNonSpace)
    {
        wchar* gameTextBuffer = new wchar[length + 1]; // :/
        memset(gameTextBuffer, 0, (length + 1) * sizeof(wchar_t));
        ConvertUnicodeToGameText(gameTextBuffer, szNicknameW, 0);
        bool hasBadChars = CFont::HasStringBadChars(gameTextBuffer);
        delete[] gameTextBuffer;
        return hasBadChars;
    }

    return false;
}

// or LaunchMultiplayer
void cLobby::InitMultiGameMode() { // on start mp page stuff (ok)
    MARKFUNCTION(0x0, 0x08874E8C);

    FrontEndMenuManager->SetHasJoinedNetwork(true);
#ifndef FIX_BUGS
    // Why disable scripts and not allow the player to return to the game if the player doesn't even launch a multiplayer game?
    // Let's do this directly at the mp game launch (LoadMultiplayer)
#ifdef MULTIGAME_SCM
    if (pGame.GetGameType() != eGameType::SCM)
#endif
        CTheScripts::Shutdown();
#endif
    m_bConnection = true;
    TheRadar->ClearAllBlips();
    //CMenuManager::m_TargetIsOn = 0; // TODO: implement this flag
    TheRadar->m_ShowMapPlayerPos = false;
#ifndef FIX_BUGS
#ifdef MULTIGAME_SCM
    if (pGame.GetGameType() != eGameType::SCM)
#endif
    {
        CMessages::ClearPreviousBriefArray();
        CMessages::AddToPreviousBriefArray(TheText.Get("MP_SNEW"), -1, -1, -1, -1, -1, -1, 0);
    }
#endif
    TheAdhoc.Configure(GTA_PRODUCT_ID, GTA_TITLE_ID);
    if (!TheAdhoc.IsNextStateNow(&cAdhoc::StateShutdown)) {
        m_nSrvListMinIdx = 0;
        m_nSrvListMaxIdx = MAX_VISIBLE_ROOMS_ROWS;
        m_nNewHostIdx = 0;
    }
    else {
        debug("adhoc connection failed: %s\n", TheAdhoc.GetExitReason().c_str());
    }
    m_nSelectedMenuOptionIndex = 0;
}

void cLobby::HandleMainGameState() {
    DONT_OPTIMIZE();

    cAdhoc& Adhoc = TheAdhoc;

    // state when we see options join a game / host a game
    if (!Adhoc.IsNextStateNow(&cAdhoc::StateShutdown))
    {
        if (isPadUp()) {
            DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
            NextMenuOption(-1, 0, 1);
        }
        if (isPadDown()) {
            DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
            NextMenuOption(1, 0, 1);
        }

        // confirm our join/host
#ifdef MP_FE_MOUSE_IMPROVEMENTS
        if (isPadConfirm() || MP_IS_CLICK_LAST_Y())
#else
        if (isPadConfirm())
#endif
        {
            DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
            switch (m_nSelectedMenuOptionIndex)
            {
                case MGPO_MAIN_PAGE_JOIN_A_GAME:
                {
                    TODO(); // m_nSelectedMenuOptionIndex is index for new page take it from enum
                    m_nSelectedMenuOptionIndex = 0;
                    InitGameParams(false);
                    SetCurrentFrontendHandler(&cLobby::HandleJoinGameState, &cLobby::DrawJoinGameScreen);
                    break;
                }
                case MGPO_MAIN_PAGE_HOST_A_GAME:
                {
                    Adhoc.StartHosting();
                    TODO(); // m_nSelectedMenuOptionIndex is index for new page take it from enum
                    m_nSelectedMenuOptionIndex = 0;
                    InitGameParams(false);
                    break;
                }
            }
        }

        Adhoc.Update();
        bool doSetup = Adhoc.m_bPendingHostStart ||
            Adhoc.IsNextStateNow(&cAdhoc::StateCancelAllTargetsIfNotJoined) ||
            Adhoc.IsNextStateNow(&cAdhoc::StateIdle);

        if (doSetup)
        {
            m_nSelectedMenuOptionIndex = 0;
            memset(m_aConnections, 0, sizeof(m_aConnections));
            m_nSendInfoTimer = 0;
            Adhoc.AssignPlayerIDs();
            SetCurrentFrontendHandler(&cLobby::HandleHostGameState, &cLobby::DrawHostGameScreen);
            m_lobbyPed.Create();
            if (!Adhoc.IsHost()) {
                tMacAddr& playerMac = Adhoc.GetPlayerMacAddress();
                m_nSocketID_2 = sceNetAdhocPtpListen(playerMac.GetBytesSCE(), GTA_SERVER_PORT, GTA_RXBUFLEN, GTA_REXMT_INTERVAL, GTA_REXMT_COUNT, 1, ADHOC_F_BLOCK);
                if (m_nSocketID_2 < 0) {
                    SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
                }
                else {
#ifdef GTA_PSP
                    sceKernelSysClock clock = sceKernelGetSystemTimeWide();
                    uint32 sec, usec;
                    sceKernelSysClock2USecWide(clock, &sec, &usec);
                    ms_fJoinPrevTime = static_cast<float>(sec) + (usec / 1000000.0f);
#else
                    ms_fJoinPrevTime = mp_time_now_d();
#endif
                }
            }
        }
        CheckCloseConnection();
        return;
    }

    if (!Adhoc.IsWifiSwitchOn() || !psp_is_umd_readable()) {
        CheckCloseConnection();
        return;
    }

    // 1st seen mp tab (starting mp.. text)
    if (!FrontEndMenuManager->GetHasJoinedNetwork())
    {
        if (isPadUp()) {
            DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
            NextMenuOption(-1, 0, 1);
        }
        if (isPadDown()) {
            DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
            NextMenuOption(1, 0, 1);
        }

        // first enter in mp tab (shows yes no text)
#ifdef MP_FE_MOUSE_IMPROVEMENTS
        if (isPadConfirm() || MP_IS_CLICK_LAST_Y())
#else
        if (isPadConfirm())
#endif
        {
            DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
            if (!m_bBadCharWarn)
            {
                switch (m_nSelectedMenuOptionIndex)
                {
                    case MGPO_LAUNCH_PAGE_NO: // No (exit to sp mode (strating mp... text + without yes/no rows))
                    {
#ifdef GTA_LIBERTY
                        CloseConnection();
#else
                        CloseConnection(true);
#endif
                        break;
                    }
                    case MGPO_LAUNCH_PAGE_YES: // Yes  (go to multiplayer)
                    {
#ifdef GTA_LIBERTY
                        m_bUnk = 0;
                        FrontEndMenuManager.m_bHasJoinedNetwork = true;
#ifndef FIX_BUGS
                        // Why disable scripts and not allow the player to return to the game if the player doesn't even launch a multiplayer game?
                        // Let's do this directly at the mp game launch (LoadMultiplayer)
#ifdef MULTIGAME_SCM
                        if (pGame.GetGameType() != eGameType::SCM)
#endif
                            CTheScripts::Shutdown();
#endif
                        CRadar::ClearAllBlips();
                        //CMenuManager::m_TargetIsOn = 0;
                        CRadar::m_ShowMapPlayerPos = false;
#ifndef FIX_BUGS
#ifdef MULTIGAME_SCM
                        if (pGame.GetGameType() != eGameType::SCM)
#endif
                        {
                            CMessages::ClearPreviousBriefArray();
                            CMessages::AddToPreviousBriefArray(TheText.Get("MP_SNEW"), -1, -1, -1, -1, -1, -1, 0);
                        }
#endif
#else
                        if (HasBadCharsInNickname()) {
                            FrontEndMenuManager->SetHasJoinedNetwork(true);
                            m_bBadCharWarn = true;
                        }
                        else {
                            InitMultiGameMode();
                        }
#endif
                        break;
                    }
                }
            }
        }
        CheckCloseConnection();
        return;
    }

#ifndef GTA_LIBERTY
    if (m_bBadCharWarn) {
#ifdef MP_FE_MOUSE_IMPROVEMENTS
        if (isPadConfirm() || MP_IS_CLICK_LAST_Y()) // justDown
#else
        if (isPadConfirm())
#endif
        {
            DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
            m_bBadCharWarn = false;
            InitMultiGameMode();
        }
        CheckCloseConnection();
        return;
    }
#endif

    // When Cross (Enter) on "Dont talk" unentered purple tab

#ifndef FIX_BUGS
    // Why disable scripts and not allow the player to return to the game if the player doesn't even launch a multiplayer game?
    // Let's do this directly at the mp game launch (LoadMultiplayer)
#ifdef MULTIGAME_SCM
    if (pGame.GetGameType() != eGameType::SCM)
#endif
        CTheScripts::Shutdown();
#endif
    Adhoc.Configure(GTA_PRODUCT_ID, GTA_TITLE_ID);
    if (!Adhoc.IsNextStateNow(&cAdhoc::StateShutdown)) {
        m_nSrvListMinIdx = 0;
        m_nSrvListMaxIdx = MAX_VISIBLE_ROOMS_ROWS;
        m_nNewHostIdx = 0;
    }
    else {
        debug("adhoc connection failed: %s\n", Adhoc.GetExitReason().c_str()); // from lcs
    }
    m_nSelectedMenuOptionIndex = 0;

    CheckCloseConnection();
}

// cLobby::DrawMainGameScreen

// TODO(MP): re-check function, missing implementation
void cLobby::HandleHostGameState() {
    DONT_OPTIMIZE();

    cMultiGame& pGame = TheMPGame;
    //FrontEndMenuManager->GetKeyPresses(&nButtons);
#ifndef GTA_LIBERTY
    NextCutsceneOption(0);
#endif
    tLobbyRemoteInfo* pInfo = TheAdhoc.GetMatchingInfo(MP_HOST_INDEX);
    tLobbyRemoteInfo connInfo{}; // ctor broadcast
    static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
    if (pInfo) {
        memcpy(&connInfo, pInfo, sizeof(tLobbyRemoteInfo));
    }

    TheAdhoc.Update();

    if (!TheAdhoc.IsWifiSwitchOn())
    {
        m_nSocketID_1 = -1;
        m_nSocketID_2 = -1;
        DMAudio.PlayFrontEndSound(SOUND_FRONTEND_BACK, 0);
        SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
        m_nSelectedMenuOptionIndex = 0;
        return;
    }

    if (isBackDown() || TheAdhoc.IsNextStateNow(&cAdhoc::StateShutdown))
    {
        if (TheAdhoc.IsWifiSwitchOn())
        {
            TheAdhoc.CancelMatchingTarget();
            if (m_nSocketID_1 >= 0) sceNetAdhocPtpClose(m_nSocketID_1, 0);
            if (m_nSocketID_2 >= 0) sceNetAdhocPtpClose(m_nSocketID_2, 0);
        }
        m_nSocketID_2 = -1;
        m_nSocketID_1 = -1;
        DMAudio.PlayFrontEndSound(SOUND_FRONTEND_BACK, 0);
        SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
        m_nSelectedMenuOptionIndex = 0;
        return;
    }

    bool allowSetup = TheAdhoc.m_bPendingHostStart ||
        TheAdhoc.IsNextStateNow(&cAdhoc::StateCancelAllTargetsIfNotJoined) ||
        TheAdhoc.IsNextStateNow(&cAdhoc::StateIdle);
    if (allowSetup && TheAdhoc.GetMatchingInfo(MP_HOST_INDEX))
    {
        UpdateClient(&connInfo, true);
        if (gIsMultiplayerGame) {
#ifdef GTA_PSP
            sceKernelSysClock clock = sceKernelGetSystemTimeWide();
            uint32 sec, usec;
            sceKernelSysClock2USecWide(clock, &sec, &usec);
            ms_fJoinPrevTime = static_cast<float>(sec) + (usec / 1000000.0f);
#else
            ms_fJoinPrevTime = mp_time_now_d();
#endif
        }

        if (!TheAdhoc.IsHost())
        {
#ifdef GTA_PSP
            sceKernelSysClock clock = sceKernelGetSystemTimeWide();
            uint32 sec, usec;
            sceKernelSysClock2USecWide(clock, &sec, &usec);
            float currentTime = static_cast<float>(sec) + (usec / 1000000.0f);
#else
            float currentTime = mp_time_now_d();
#endif

            if ((currentTime - ms_fJoinPrevTime) >= MAX_LOBBY_WAIT_DELAY)
            {
                if (TheAdhoc.IsWifiSwitchOn())
                {
                    TheAdhoc.CancelMatchingTarget();
                    if (m_nSocketID_1 >= 0) sceNetAdhocPtpClose(m_nSocketID_1, 0);
                    if (m_nSocketID_2 >= 0) sceNetAdhocPtpClose(m_nSocketID_2, 0);
                }
                m_nSocketID_2 = -1;
                m_nSocketID_1 = -1;
                DMAudio.PlayFrontEndSound(SOUND_FRONTEND_BACK, 0);
                SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
                m_nSelectedMenuOptionIndex = 0;
                return;
            }
        }

        int8 nOptIndex = 0;
        eGameType curGameType = pGame.GetGameType();
        switch (curGameType)
        {
            case eGameType::DEATHMATCH:
                nOptIndex = MGE_MAX_DEATHMATCH_DEFAULT;
                break;
            case eGameType::MULTIRACE:
                nOptIndex = MGE_MAX_MULTIRACE;
                break;
            case eGameType::DEFENDTHEBASE:
                nOptIndex = MGE_MAX_DEFENDTHEBASE;
                break;
            case eGameType::CTF:
                nOptIndex = MGE_MAX_CTF;
                break;
            case eGameType::TANK:
                nOptIndex = MGE_MAX_TANK;
                break;
            case eGameType::HITPARADE:
                nOptIndex = MGE_MAX_HITPARADE;
                break;
            case eGameType::SIXTYSECONDS:
                nOptIndex = MGE_MAX_SIXTYSECONDS;
                break;
#ifndef GTA_LIBERTY
            case eGameType::HUNTERATTACK:
                nOptIndex = MGE_MAX_HUNTERATTACK;
                break;
            case eGameType::FLAGBALL:
                nOptIndex = MGE_MAX_FLAGBALL;
                break;
            case eGameType::VIP:
                nOptIndex = MGE_MAX_VIP;
                break;
#if 0 // beta
            case eGameType::COLLECTTHEGOLD:
                nOptIndex = MGE_MAX_COLLECTTHEGOLD;
                break;
            case eGameType::COPSANDROBBERS:
                nOptIndex = MGE_MAX_COPSANDROBBERS;
                break;
#endif
#ifdef MULTIGAME_SCM
            case eGameType::SCM:
                nOptIndex = MGE_MAX_SCM;
                break;
#endif
#endif
            default:
                break;
        }
        if (curGameType == eGameType::DEATHMATCH && GANG_MODE)
            nOptIndex = nOptIndex + 2; // or nOptIndex = MGE_MAX_DEATHMATCH;

#ifdef MULTIGAME_SCM
        if (pGame.GetGameType() != eGameType::SCM)
#endif
            HandleGangSelection(nOptIndex);

        int8 nCurrentIndex = m_nSelectedMenuOptionIndex;
        int8 nMaxMenuIndex = nOptIndex + 2; // accounts for reset and start game options

        if (m_nSelectedMenuOptionIndex == nOptIndex && GANG_MODE) // change our gang
        {
            if (isPadLeft() || isPadRight()) // button == 0 || button == 1 || isPadLeft || isPadRight ?
            {
                //DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0); // real in vcs code
                TODO();
                pGame.s_nSelectedTeam = pGame.s_nSelectedTeam == eGameTeam::TEAM_A ? eGameTeam::TEAM_B : eGameTeam::TEAM_A;
            }
        }

        // When we client, limit settings row selection
        if (!TheAdhoc.IsHost())
        {
            if (GANG_MODE)
            {
                if (isPadUp() || isPadDown())
                    m_nSelectedMenuOptionIndex = nOptIndex;

#ifndef MP_FE_MOUSE_IMPROVEMENTS // hover sound conflict, allow select but dont change
                if (m_nSelectedMenuOptionIndex < nOptIndex)
                    m_nSelectedMenuOptionIndex = nOptIndex;
                if (nOptIndex < m_nSelectedMenuOptionIndex)
                    m_nSelectedMenuOptionIndex = nOptIndex;
#endif
            }
            else
            {
#ifndef MP_FE_MOUSE_IMPROVEMENTS
                m_nSelectedMenuOptionIndex = nOptIndex;
#endif
            }
        }

        if (isPadUp())
        {
            handleBtnPressSound(nCurrentIndex);
            nCurrentIndex--;
            if (nCurrentIndex == -1)
                nCurrentIndex = nOptIndex + 2;
        }
        else if (isPadDown())
        {
            handleBtnPressSound(nCurrentIndex);
            nCurrentIndex++;
            if (nCurrentIndex > nMaxMenuIndex)
                nCurrentIndex = 0;
        }

        if (nMaxMenuIndex < m_nSelectedMenuOptionIndex)
            m_nSelectedMenuOptionIndex = 0;
        if (m_nSelectedMenuOptionIndex == -1)
            m_nSelectedMenuOptionIndex = 0;
        if (m_nSelectedMenuOptionIndex < 0)
            m_nSelectedMenuOptionIndex = nMaxMenuIndex;
        if (TheAdhoc.IsNextStateNow(&cAdhoc::StateInitialise) || TheAdhoc.IsNextStateNow(&cAdhoc::StateConnectLobbyGroup))
            m_nSelectedMenuOptionIndex = -1;

        if (isPadLeft()) {
            handleBtnPressSound(nCurrentIndex);
            HandleLeftRightBtnPress(nCurrentIndex, -1);
        }
        else if (isPadRight()) {
            handleBtnPressSound(nCurrentIndex);
            HandleLeftRightBtnPress(nCurrentIndex, 1);
        }
#ifdef MP_FE_MOUSE_IMPROVEMENTS
        if (isPadConfirm() || MP_IS_CLICK_LAST_Y())
#else
        if (isPadConfirm())
#endif
        {
            if (nMaxMenuIndex - 1 == nCurrentIndex)
            {
                InitGameParams(false); // reset to defaults
#ifdef MP_FE_MOUSE_IMPROVEMENTS
                handleBtnPressSound(m_nSelectedMenuOptionIndex); // no rst select idx, no sound
#endif
            }
            else if (nMaxMenuIndex == nCurrentIndex) // start game
            {
                if (!TheAdhoc.IsNextStateNow(&cAdhoc::StateShutdown) &&
                    !TheAdhoc.IsNextStateNow(&cAdhoc::StateInitialise) &&
                    !TheAdhoc.IsNextStateNow(&cAdhoc::StateConnectLobbyGroup))
                {
                    uint8 nConnPlayers = TheAdhoc.GetNumberOfConnectedPlayers();
                    bool bGangsFilledUp = TheAdhoc.GetNumberOfNonEmptyGangs() >= 2;
                    bool bCanStartGame = (nConnPlayers >= 2) && (GANG_MODE ? bGangsFilledUp : true);
                    if (gDeveloperFlag || bCanStartGame) {
                        m_nSelectedMenuOptionIndex = 0;
                        memset(m_aConnections, 0, sizeof(m_aConnections));
                        tLobbyRemoteInfo* info = TheAdhoc.GetMatchingInfo(MP_HOST_INDEX);
                        static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
                        memcpy(&m_remoteInfo, info, sizeof(tLobbyRemoteInfo));
                        m_nWaitTime = 0;
                        SetCurrentFrontendHandler(&cLobby::HandleHostStartGameState, &cLobby::NoDraw);
                    }

                }
            } // start game curr row
        } // confirm

#if !defined(FIX_BUGS) && !defined(GTA_LIBERTY) // huh, moved into NextRaceGameStyle, store score
        if (pGame.GetGameType() == eGameType::MULTIRACE && pGame.GetCTFScoreLimit() == (int32)eRaceStyle::RACE_QUADATHLON && pGame.GetScoreLimit() != 1)
        {
            m_nSavedRaceLapCount = pGame.GetScoreLimit();
            pGame.SetScoreLimit(1);
        }
#endif
    }

#ifndef GTA_LIBERTY
    NextCutsceneOption(0);
#endif
    TheAdhoc.UpdateGameParams();
    tLobbyRemoteInfo* pInfoNew = TheAdhoc.GetMatchingInfo(MP_HOST_INDEX);
    static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
    if (memcmp(&connInfo, pInfoNew, sizeof(tLobbyRemoteInfo)))
        memset(m_aConnections, 0, sizeof(m_aConnections));
}

// cLobby::DrawHostGameScreen

// when we host
void cLobby::HandleHostStartGameState() {
    DONT_OPTIMIZE();

    cAdhoc& Adhoc = cAdhoc::Instance();
    tLobbyRemoteInfo* matchingInfo = Adhoc.GetMatchingInfo(MP_HOST_INDEX);

    if (isBackDown() || Adhoc.IsNextStateNow(&cAdhoc::StateShutdown) || !matchingInfo)
    {
        if (m_nSocketID_1 >= 0) {
            sceNetAdhocPtpClose(m_nSocketID_1, 0);
            m_nSocketID_1 = -1;
        }
        Adhoc.Terminate();
        DMAudio.PlayFrontEndSound(SOUND_FRONTEND_BACK, 0);
        SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
        return;
    }

    int32 result = 0;
    bool allConnected = false;

    if (m_nSocketID_1 < 0) {
        allConnected = true;

        tMacAddr selfMac = Adhoc.GetPlayerMacAddress();
        for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
            tAdhocPeerData& peer = m_remoteInfo.m_nPeersConnInfo[i];

            if (peer.macAddr == selfMac) {
                m_aConnections[i] = true;
                continue;
            }

            // Skip broadcast or already connected
            if (peer.macAddr.IsBroadcast() || m_aConnections[i]) {
                continue;
            }

            // Need connect: open socket
            m_nSocketID_1 = sceNetAdhocPtpOpen(selfMac.GetBytesSCE(), GTA_PTP_PORT, peer.macAddr.GetBytesSCE(),
                GTA_SERVER_PORT, GTA_RXBUFLEN, GTA_REXMT_INTERVAL, GTA_REXMT_COUNT, ADHOC_F_BLOCK);
            if (m_nSocketID_1 < 0) {
                // Error: back to main
                SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
                return;
            }
            m_nSendState = eHostSendState::HOST_SEND_CONNECTING;
            m_aConnections[i] = true;
            allConnected = false;
            break;
        }
    }

    // If socket open, handle states
    if (m_nSocketID_1 >= 0) {
        if (m_nSendState == eHostSendState::HOST_SEND_CONNECTING)
        {
            result = sceNetAdhocPtpConnect(m_nSocketID_1, 0, ADHOC_F_NONBLOCK);
            if (result >= 0 || result == SCE_NET_ADHOC_ERROR_WOULD_BLOCK) {
                if (result >= 0) // useless
                    m_nSendState = eHostSendState::HOST_SEND_SENDING_INFO;
            }
        }

        if (m_nSendState == eHostSendState::HOST_SEND_SENDING_INFO)
        {
            int32 dataSize = sizeof(tLobbyRemoteInfo);
            static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
            result = sceNetAdhocPtpSend(m_nSocketID_1, &m_remoteInfo, &dataSize, 0, ADHOC_F_NONBLOCK);
            if (result >= 0 || result == SCE_NET_ADHOC_ERROR_WOULD_BLOCK) {
                if (result >= 0) // useless
                    m_nSendState = eHostSendState::HOST_SEND_FLUSHING;
            }
        }

        if (m_nSendState == eHostSendState::HOST_SEND_FLUSHING)
        {
            result = sceNetAdhocPtpFlush(m_nSocketID_1, 0, ADHOC_F_NONBLOCK);
            if (result >= 0 || result == SCE_NET_ADHOC_ERROR_WOULD_BLOCK) {
                if (result >= 0) // useless
                {
                    sceNetAdhocPtpClose(m_nSocketID_1, 0);
                    m_nSocketID_1 = -1;
                }
            }
        }

        if (result < 0 && result != SCE_NET_ADHOC_ERROR_WOULD_BLOCK) {
            ++m_nWaitTime;
        }
        if (m_nWaitTime > GTA_DEFAULT_WAIT_TIME) {
            sceNetAdhocPtpClose(m_nSocketID_1, 0);
            m_nSocketID_1 = -1;
            SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
        }
    }

    // If all connected (no new opens or states completed)
    if (allConnected) {
        debug("CONNECTED!\n");
        LoadMultiplayer();
    }
}

void cLobby::HandleJoinGameState() {
    DONT_OPTIMIZE();

    cAdhoc& Adhoc = cAdhoc::Instance();
    bool allowInput = !Adhoc.IsNextStateNow(&cAdhoc::StateShutdown) &&
        !Adhoc.IsNextStateNow(&cAdhoc::StateInitialise) &&
        !Adhoc.IsNextStateNow(&cAdhoc::StateConnectLobbyGroup);

    if (allowInput)
    {
        if (isPadUp())
        {
            if (m_nNewHostIdx > 0)
                DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
            NextMenuOption(-1, 0, m_nNewHostIdx);
        }

        if (isPadDown())
        {
            if (m_nNewHostIdx > 0)
                DMAudio.PlayFrontEndSound(SOUND_FRONTEND_ENTER_OR_ADJUST, 0);
            NextMenuOption(1, 0, m_nNewHostIdx);
        }
    }

    static int nLastHostIdx = 0;
    if (m_nNewHostIdx < nLastHostIdx && m_nSelectedMenuOptionIndex != nLastHostIdx)
        m_nSelectedMenuOptionIndex = Max(m_nSelectedMenuOptionIndex - (nLastHostIdx - m_nNewHostIdx), 0);
    nLastHostIdx = m_nNewHostIdx;
    m_nSelectedMenuOptionIndex = Min(m_nSelectedMenuOptionIndex, m_nNewHostIdx);

    if (Adhoc.IsNextStateNow(&cAdhoc::StateShutdown))
        return;

#ifdef MP_FE_MOUSE_IMPROVEMENTS
    if (isPadConfirm() || MP_IS_CLICK_LAST_Y()) {
#else
    if (isPadConfirm()) {
#endif
        DMAudio.PlayFrontEndSound(SOUND_FRONTEND_HIGHLIGHT_OPTION, 0);
        if (m_nSelectedMenuOptionIndex == m_nNewHostIdx) {
            Adhoc.StartHosting();
            TODO(); // m_nSelectedMenuOptionIndex is index for new page take it from enum
            m_nSelectedMenuOptionIndex = 0;
        }
        else {
            int I = 0;
            for (int index = 0; index < MP_NUM_PEERS; ++index) {
                tLobbyRemoteInfo* pInfo = Adhoc.GetMatchingInfo(index);
                if (pInfo) {
                    if (I == m_nSelectedMenuOptionIndex) {
                        Adhoc.SelectServer(index);
                        break;
                    }
                    I++;
                }
            }
        }
    }
    Adhoc.Update();
    bool bClose = false;
    if (isBackDown()) {
        bClose = true;
    }
    else if (Adhoc.IsNextStateNow(&cAdhoc::StateShutdown)) {
        bClose = true;
    }
    if (bClose) {
        if (Adhoc.IsWifiSwitchOn()) {
            Adhoc.CancelMatchingTarget();
            if (m_nSocketID_1 >= 0) sceNetAdhocPtpClose(m_nSocketID_1, 0);
            if (m_nSocketID_2 >= 0) sceNetAdhocPtpClose(m_nSocketID_2, 0);
        }
        m_nSocketID_1 = -1;
        m_nSocketID_2 = -1;
        DMAudio.PlayFrontEndSound(SOUND_FRONTEND_BACK, 0);
        SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
        TODO(); // m_nSelectedMenuOptionIndex is index for new page take it from enum
        m_nSelectedMenuOptionIndex = 0;
        return;
    }

    bool doSetup = Adhoc.m_bPendingHostStart ||
        Adhoc.IsNextStateNow(&cAdhoc::StateCancelAllTargetsIfNotJoined) ||
        Adhoc.IsNextStateNow(&cAdhoc::StateIdle);

    if (doSetup) {
        TODO(); // m_nSelectedMenuOptionIndex is index for new page take it from enum
        m_nSelectedMenuOptionIndex = 0;
        memset(m_aConnections, 0, sizeof(m_aConnections));
        m_nSendInfoTimer = 0;
        Adhoc.AssignPlayerIDs();
        SetCurrentFrontendHandler(&cLobby::HandleHostGameState, &cLobby::DrawHostGameScreen);
        m_lobbyPed.Create();
        if (!Adhoc.IsHost()) {
            tMacAddr& addr = Adhoc.GetPlayerMacAddress();
            m_nSocketID_2 = sceNetAdhocPtpListen(addr.GetBytesSCE(), GTA_SERVER_PORT, GTA_RXBUFLEN, GTA_REXMT_INTERVAL, GTA_REXMT_COUNT, 1, ADHOC_F_BLOCK);
            if (m_nSocketID_2 < 0) {
                SetCurrentFrontendHandler(&cLobby::HandleMainGameState, &cLobby::DrawMainGameScreen);
            }
            else {
#ifdef GTA_PSP
                sceKernelSysClock clock = sceKernelGetSystemTimeWide();
                uint32 sec, usec;
                sceKernelSysClock2USecWide(clock, &sec, &usec);
                ms_fJoinPrevTime = static_cast<float>(sec) + (usec / 1000000.0f);
#else
                ms_fJoinPrevTime = mp_time_now_d();
#endif
            }
        }
    }
}

// cLobby::DrawJoinGameScreen

void cLobby::UpdateClient(tLobbyRemoteInfo* pConnInfo, bool bUpdatePeer) {
    cAdhoc& pAdhoc = TheAdhoc;
    tLobbyRemoteInfo* pMatchingInfo = pAdhoc.GetMatchingInfo(MP_HOST_INDEX);

    // Open PDP socket if needed (on connection event)
    if (pAdhoc.GetAdhocPdp().m_nPdpID < 0 && pAdhoc.bConnEvent) {
        tListenAddr listenAddr;
        listenAddr.mac = pAdhoc.GetPlayerMacAddress();
        listenAddr.port = GTA_PDP_PORT;
        pAdhoc.GetAdhocPdp().OpenPDP(listenAddr);
        if (pAdhoc.GetAdhocPdp().m_nPdpID < 0) {
            debug("error opening socket :(\n");
        }
    }

    if (pAdhoc.GetAdhocPdp().m_nPdpID >= 0) {
        // Receive PDP packet
        tListenAddr recvAddr; // ctor broadcast + port 1
        //recvAddr.mac.SetBroadcast();
        //recvAddr.port = NET_SESSION_DEFAULT_PORT;
        tLobbyRemoteInfo recvInfo;
        static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
        if (pAdhoc.GetAdhocPdp().RecvPDPPacket(&recvInfo, sizeof(tLobbyRemoteInfo), recvAddr) == sizeof(tLobbyRemoteInfo))
        {
            tMacAddr hostMac = pMatchingInfo->m_HostPeerData.peerAddr.mac;
            if (hostMac == recvInfo.m_HostPeerData.peerAddr.mac) // pMatchingInfo == recvInfo
            {
                if (recvAddr.mac == hostMac)
                {
                    pAdhoc.SetGameParams(recvInfo);
#ifdef GTA_PSP
                    sceKernelSysClock clock = sceKernelGetSystemTimeWide();
                    uint32 sec, usec;
                    sceKernelSysClock2USecWide(clock, &sec, &usec);
                    ms_fJoinPrevTime = static_cast<float>(sec) + (usec / 1000000.0f);
#else
                    ms_fJoinPrevTime = mp_time_now_d();
#endif
                }

                // Find peer by received MAC
                bool bFound = false;
                int32 nPeerId = 0;
                for (; nPeerId < MP_NUM_PEERS; ++nPeerId) {
                    if (recvAddr.mac == pMatchingInfo->m_nPeersConnInfo[nPeerId].macAddr) {
                        bFound = true;
                        break;
                    }
                }

                if (bFound)
                {
                    // Update if data changed and matching started
                    static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
                    if (pAdhoc.IsHost() && memcmp(pConnInfo, &recvInfo, sizeof(tLobbyRemoteInfo)) != 0) {
                        if (pAdhoc.IsHost() && bUpdatePeer)
                            pAdhoc.SetPeerConnInfo(nPeerId, recvInfo);
                    }
                    else if (!m_aConnections[nPeerId]) {
                        debug("connected peer %i\n", nPeerId);
                        m_aConnections[nPeerId] = true;
                        pAdhoc.FindAndClearMacAddr(pMatchingInfo->m_nPeersConnInfo[nPeerId].macAddr);
                    }
                }
            }
        }

        // Check if all peers are connected (skip self and broadcast)
        bool bAllConnected = true;
        for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
            tMacAddr ownMac = pAdhoc.GetPlayerMacAddress();
            if (pMatchingInfo->m_nPeersConnInfo[i].macAddr == ownMac) {
                m_aConnections[i] = true; // Self is always connected
            }
            else if (!pMatchingInfo->m_nPeersConnInfo[i].macAddr.IsBroadcast() && !m_aConnections[i]) {
                bAllConnected = false;
            }
        }

        // Send own info periodically (every 15 ticks)
        m_nSendInfoTimer--;
        if (m_nSendInfoTimer <= 0) {
            tLobbyRemoteInfo sendInfo;
            memcpy(&sendInfo, pMatchingInfo, sizeof(tLobbyRemoteInfo)); // copy
            // Update own peer data in sendInfo
            for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
                tMacAddr ownMac = pAdhoc.GetPlayerMacAddress();
                if (sendInfo.m_nPeersConnInfo[i].macAddr == ownMac) {
                    sendInfo.m_nPeersConnInfo[i].nSelectedPeerModelID = cMultiGame::s_nPlayerModelIndex;
                    sendInfo.m_nPeersConnInfo[i].nTeamID = static_cast<int16>(cMultiGame::s_nSelectedTeam);
                }
            }

            m_nSendInfoTimer = NUM_SEND_INFO_TICKS;

            // Send to each non-broadcast, non-self peer (skip self if not all connected)
            for (int32 i = 0; i < MP_NUM_PEERS; ++i) {
                tAdhocPeerData& peer = sendInfo.m_nPeersConnInfo[i];
                if (peer.macAddr.IsBroadcast())
                    continue;

                if (peer.macAddr == pAdhoc.GetPlayerMacAddress())
                    continue;

                if (peer.macAddr == sendInfo.m_HostPeerData.peerAddr.mac && !bAllConnected)
                    continue;

                tListenAddr targetAddr;
                targetAddr.mac = peer.macAddr;
                targetAddr.port = GTA_PDP_PORT;
                static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
                pAdhoc.GetAdhocPdp().SendPDPPacket(&sendInfo, sizeof(tLobbyRemoteInfo), targetAddr);
            }
        }
    }

    // Handle PTP accept (if listen socket open and no active socket)
    if (m_nSocketID_1 < 0 && m_nSocketID_2 >= 0 && pMatchingInfo) {
        tLobbyRemotePeer acceptInfo;
        acceptInfo.peerAddr.mac.SetBroadcast();
        acceptInfo.peerAddr.port = GTA_PTP_PORT;
        //int32 nResult = sceNetAdhocPtpAccept(m_nSocketID_2, PSPSDKMACARG(&acceptInfo.peerAddr.mac), &acceptInfo.peerAddr.port, 0, 1); // not nil port
        int32 nResult = sceNetAdhocPtpAccept(m_nSocketID_2, PSPSDKMACARG(&acceptInfo.peerAddr.mac), nil, 0, ADHOC_F_NONBLOCK);
        if (nResult >= 0 || nResult == SCE_NET_ADHOC_ERROR_WOULD_BLOCK) {
            if (nResult >= 0) {
                debug("sceNetAdhocPtpAccept succeeded\n");
                m_nSocketID_1 = nResult;
            }
        }
        else {
            debug("error on sceNetAdhocPtpAccept(): %x\n", nResult);
        }
    }

    // If active socket open, receive start game packet
    if (m_nSocketID_1 >= 0) {
        tLobbyRemoteInfo joinInfo;
        static_assert(sizeof(tLobbyRemoteInfo) == 136, "tLobbyRemoteInfo");
        int32 bufSize = sizeof(tLobbyRemoteInfo);
        int32 nResult = sceNetAdhocPtpRecv(m_nSocketID_1, &joinInfo, &bufSize, 0, ADHOC_F_NONBLOCK);
        if (nResult < 0) {
            if (nResult != SCE_NET_ADHOC_ERROR_WOULD_BLOCK) {
                debug("sceNetAdhocPtpRecv failed\n");
                sceNetAdhocPtpClose(m_nSocketID_1, 0);
                m_nSocketID_1 = -1;
                return;
            }
            return;
        }
        debug("sceNetAdhocPtpRecv succeeded\n");
        sceNetAdhocPtpClose(m_nSocketID_1, 0);
        m_nSocketID_1 = -1;
        if (bufSize == sizeof(tLobbyRemoteInfo))
        {
            // Check if from host
            if (pMatchingInfo->m_HostPeerData.peerAddr.mac == joinInfo.m_HostPeerData.peerAddr.mac) // pMatchingInfo == joinInfo
            {
                debug("Starting game\n");
                sceNetAdhocPtpClose(m_nSocketID_2, 0);
                m_nSocketID_2 = -1;
                memcpy(&m_remoteInfo, &joinInfo, sizeof(tLobbyRemoteInfo));
                pAdhoc.SetGameParams(joinInfo);
                LoadMultiplayer();
            }
        }
    }
}

void cLobby::LoadMultiplayer() { // start mp game (before ok)
    MARKFUNCTION(0x0, 0x0887C998);

#if (!defined(FINAL) && !defined(MASTER))
    { // dbg
        OpenConsole(); // check the lua logs [print(123)]
        gDebugDrawStuff = 1; // text
        gMPDebugPrintLevel = 1; // simsch
        //gbShowCollisionLines2 = true; // collision
        gbDrawVersionText = true; // instanse ID
    }
#endif

    FrontEndMenuManager->SetLockSwitchOnAndOff(true); // psp lock exit by joined, mp mode todo  // !!! cuustom

    cMultiGame& Game = cMultiGame::Instance();
    cAdhoc& Adhoc = cAdhoc::Instance();
#ifdef FIX_BUGS
#ifdef MULTIGAME_SCM
    if (pGame.GetGameType() != eGameType::SCM)
#endif
    {
        CTheScripts::Shutdown();
        CMessages::ClearPreviousBriefArray();
        CMessages::AddToPreviousBriefArray(TheText.Get("MP_SNEW"), -1, -1, -1, -1, -1, -1, 0);
    }
#endif

    gbMultiplayerSplash = true;
    ResetLoadingScreenBar();
#ifdef GTA_PSP
    if ((sceUmdGetDriveStat() & 0x20) == 0)
        LoadingScreen("MULTI GAME", "", nil);
#endif
    if (m_nSocketID_1 >= 0) sceNetAdhocPtpClose(m_nSocketID_1, 0);
    if (m_nSocketID_2 >= 0) sceNetAdhocPtpClose(m_nSocketID_2, 0);
    m_nSocketID_2 = -1;
    m_nSocketID_1 = -1;
    m_lobbyPed.Destroy();
    Adhoc.GetAdhocPdp().ClosePDP();
    gIsMultiplayerGame = true;
    if (!Game.GetCutsceneSkipEnabled())
    {
        gMPCutsceneHasPlayed[static_cast<uint8>(Game.GetGameType())] = true;
        debug("Setting Game Type %d Cutscene Played to TRUE\n", Game.GetGameType());
    }
    Adhoc.OnStartGame();
    CGame::StartNewGame(); // LCS: CMenuManager addr block, VCS CGame, but can inlined CMenuManager::InitialiseWhenNewGame() into CGame
#ifdef GTA_PSP
    if ((sceUmdGetDriveStat() & 0x20) == 0)
        LoadingScreen("MULTI GAME", "", nil);
#endif
    switch (Game.GetGameType())
    {
        case eGameType::DEATHMATCH:
            LoadingScreen("DEATHMATCH", "", "mpload0", true);
            break;
        case eGameType::MULTIRACE:
            LoadingScreen("TURISMO", "", "mpload1", true);
            break;
        case eGameType::DEFENDTHEBASE:
            LoadingScreen("DTB", "", "mpload2", true);
            break;
        case eGameType::CTF:
            LoadingScreen("CTF", "", "mpload3", true);
            break;
        case eGameType::TANK:
            LoadingScreen("TANK", "", "mpload4", true);
            break;
        case eGameType::HITPARADE:
            LoadingScreen("HIT PARADE", "", "mpload5", true);
            break;
        case eGameType::SIXTYSECONDS:
            LoadingScreen("SIXTY SECS", "", "mpload6", true);
            break;
#ifndef GTA_LIBERTY
        case eGameType::HUNTERATTACK:
            LoadingScreen("HUNTER ATTACK", "", "mpload7", true);
            break;
        case eGameType::FLAGBALL:
            LoadingScreen("FLAG BALL", "", "mpload8", true);
            break;
        case eGameType::VIP:
            LoadingScreen("VIP", "", "mpload9", true);
            break;
#endif
        default:
            LoadingScreen(nil, "", "loadsc2", true);
            break;
    }
#ifdef GTA_PSP
    sceKernelPowerLock();
#endif
#ifdef MULTIGAME_SCM
    if (pGame.GetGameType() != eGameType::SCM)
#endif
    cLScript::Initialize();
#ifdef GTA_PSP
    sceKernelPowerUnlock();
    LoadingScreen("MULTI GAME", "", nil); // reset mg splash
#endif
    DMAudio.ChangeMusicMode(MUSICMODE_CUTSCENE);
    DMAudio.SetMusicFadeVol(0);
    DMAudio.SetEffectsFadeVol(0);
    Adhoc.Update();
    LoadingScreen("MULTI GAME", "", nil);
#ifndef GTA_LIBERTY
    //unk_field_18.field_0 = 0;
#endif
}
